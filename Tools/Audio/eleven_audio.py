"""
KIND: offline tool (calls the ElevenLabs API, writes WAVs and a ledger under Audio/Eleven/).

The game's sound made with ElevenLabs in the jam (Sat 14 Nov): the sound effects and the music stems under the names of
Kirill's delivery, as an example for him and a fallback if his files do not come.

    python Tools/Audio/eleven_audio.py credits
    python Tools/Audio/eleven_audio.py sfx <jobs.json> [--only name,name]
    python Tools/Audio/eleven_audio.py music <jobs.json>
    python Tools/Audio/eleven_audio.py stems <song.wav> <out_dir>
    python Tools/Audio/eleven_audio.py design <voices.json>
    python Tools/Audio/eleven_audio.py speak <vo_lines.json> [--only name,name]

sfx jobs: [{"name": "sfx_katana_slash_01", "text": "...", "seconds": 0.8, "influence": 0.6, "loop": false}]
music jobs: [{"name": "mus_main_full", "prompt": "...", "ms": 32000}]
design jobs: [{"name": "vo_publisher_a", "description": "...", "text": "100 to 1000 characters"}]
The key is ELEVENLABS_API_KEY, read from --env (default: this project's .env) and never printed.
Every request goes to Audio/Eleven/ledger.json with its time, size and the request id the API returns.
"""
import argparse
import base64
import datetime
import io
import json
import os
import sys
import zipfile

import subprocess

import requests

HERE = os.path.dirname(os.path.abspath(__file__))
ROOT = os.path.normpath(os.path.join(HERE, "..", ".."))
OUT = os.path.join(ROOT, "Audio", "Eleven")
API = "https://api.elevenlabs.io"


def read_key(env_path):
    with open(env_path, encoding="utf-8") as handle:
        for line in handle:
            if line.startswith("ELEVENLABS_API_KEY="):
                return line.split("=", 1)[1].strip().strip('"')
    sys.exit(f"[eleven] ELEVENLABS_API_KEY is missing in {env_path}")


def book(row):
    path = os.path.join(OUT, "ledger.json")
    rows = []
    if os.path.exists(path):
        with open(path, encoding="utf-8") as handle:
            rows = json.load(handle)
    row["utc"] = datetime.datetime.now(datetime.timezone.utc).strftime("%Y-%m-%dT%H:%M:%SZ")
    rows.append(row)
    with open(path, "w", encoding="utf-8") as handle:
        json.dump(rows, handle, indent=1)


def credits(key):
    reply = requests.get(f"{API}/v1/user/subscription", headers={"xi-api-key": key}, timeout=30)
    reply.raise_for_status()
    sub = reply.json()
    left = sub.get("character_limit", 0) - sub.get("character_count", 0)
    print(f"[eleven] tier {sub.get('tier')}, credits used {sub.get('character_count')} of {sub.get('character_limit')}, left {left}")


def sfx(key, jobs_path, only):
    with open(jobs_path, encoding="utf-8") as handle:
        jobs = json.load(handle)
    folder = os.path.join(OUT, "Sfx")
    os.makedirs(folder, exist_ok=True)
    for job in jobs:
        if only and job["name"] not in only:
            continue
        body = {"text": job["text"], "prompt_influence": job.get("influence", 0.5), "loop": job.get("loop", False),
                "model_id": "eleven_text_to_sound_v2"}
        if job.get("seconds"):
            body["duration_seconds"] = job["seconds"]
        reply = requests.post(f"{API}/v1/sound-generation", params={"output_format": "wav_48000"},
                              headers={"xi-api-key": key}, json=body, timeout=180)
        status = "ok" if reply.ok else f"http {reply.status_code}"
        book({"kind": "sfx", "name": job["name"], "status": status, "bytes": len(reply.content),
              "request_id": reply.headers.get("request-id", ""), "text": job["text"]})
        if not reply.ok:
            print(f"[eleven] {job['name']}: {status} {reply.text[:200]}")
            continue
        with open(os.path.join(folder, job["name"] + ".wav"), "wb") as handle:
            handle.write(reply.content)
        print(f"[eleven] {job['name']}: ok, {len(reply.content) // 1024} KB")


def music(key, jobs_path):
    with open(jobs_path, encoding="utf-8") as handle:
        jobs = json.load(handle)
    folder = os.path.join(OUT, "Music")
    os.makedirs(folder, exist_ok=True)
    for job in jobs:
        body = {"prompt": job["prompt"], "music_length_ms": job["ms"], "force_instrumental": True, "model_id": "music_v1"}
        reply = requests.post(f"{API}/v1/music", params={"output_format": "mp3_48000_192"}, headers={"xi-api-key": key},
                              json=body, timeout=600)
        status = "ok" if reply.ok else f"http {reply.status_code}"
        book({"kind": "music", "name": job["name"], "status": status, "bytes": len(reply.content),
              "request_id": reply.headers.get("request-id", ""), "prompt": job["prompt"]})
        if not reply.ok:
            print(f"[eleven] {job['name']}: {status} {reply.text[:200]}")
            continue
        # MP3 as it came; the stem split and the loop cut work from it (Tools/Audio/cut_loops.py).
        with open(os.path.join(folder, job["name"] + ".mp3"), "wb") as handle:
            handle.write(reply.content)
        print(f"[eleven] {job['name']}: ok, {len(reply.content) // 1024} KB")


def stems(key, song_path, out_dir):
    os.makedirs(out_dir, exist_ok=True)
    with open(song_path, "rb") as handle:
        reply = requests.post(f"{API}/v1/music/stem-separation", headers={"xi-api-key": key},
                              files={"file": (os.path.basename(song_path), handle, "audio/wav")}, timeout=600)
    status = "ok" if reply.ok else f"http {reply.status_code}"
    book({"kind": "stems", "name": os.path.basename(song_path), "status": status, "bytes": len(reply.content),
          "request_id": reply.headers.get("request-id", "")})
    if not reply.ok:
        sys.exit(f"[eleven] stems: {status} {reply.text[:200]}")
    with zipfile.ZipFile(io.BytesIO(reply.content)) as archive:
        archive.extractall(out_dir)
        print("[eleven] stems:", ", ".join(archive.namelist()))


def design(key, jobs_path):
    """Voice casting: each job describes a voice; the API returns three previews reading our text, saved as MP3s with
    their generated_voice_id in casting.json (an id is what /v1/text-to-voice turns into a permanent voice)."""
    with open(jobs_path, encoding="utf-8") as handle:
        jobs = json.load(handle)
    folder = os.path.join(OUT, "Voice", "Casting")
    os.makedirs(folder, exist_ok=True)
    index_path = os.path.join(folder, "casting.json")
    index = {}
    if os.path.exists(index_path):
        with open(index_path, encoding="utf-8") as handle:
            index = json.load(handle)
    for job in jobs:
        body = {"voice_description": job["description"], "text": job["text"], "model_id": job.get("model", "eleven_ttv_v3")}
        if "seed" in job:
            body["seed"] = job["seed"]
        if "reference" in job:  # a preview to stay close to: only eleven_ttv_v3 takes it
            with open(os.path.join(ROOT, job["reference"]), "rb") as handle:
                body["reference_audio_base64"] = base64.b64encode(handle.read()).decode()
            body["prompt_strength"] = job.get("prompt_strength", 0.5)
        reply = requests.post(f"{API}/v1/text-to-voice/design", params={"output_format": "mp3_44100_128"},
                              headers={"xi-api-key": key}, json=body, timeout=300)
        status = "ok" if reply.ok else f"http {reply.status_code}"
        book({"kind": "voice_design", "name": job["name"], "status": status, "request_id": reply.headers.get("request-id", ""),
              "description": job["description"], "text": job["text"]})
        if not reply.ok:
            print(f"[eleven] {job['name']}: {status} {reply.text[:200]}")
            continue
        for number, preview in enumerate(reply.json()["previews"], 1):
            name = f"{job['name']}_{number}"
            with open(os.path.join(folder, name + ".mp3"), "wb") as handle:
                handle.write(base64.b64decode(preview["audio_base_64"]))
            index[name] = {"generated_voice_id": preview["generated_voice_id"], "description": job["description"],
                           "seconds": preview.get("duration_secs")}
            print(f"[eleven] {name}: ok, {preview.get('duration_secs', 0):.1f} s")
    with open(index_path, "w", encoding="utf-8") as handle:
        json.dump(index, handle, indent=1)


def save_voice(key, cast_name, voice_name, voices):
    """The permanent voice made from a casting preview, created once and remembered in voices.json."""
    if voice_name in voices:
        return voices[voice_name]
    with open(os.path.join(OUT, "Voice", "Casting", "casting.json"), encoding="utf-8") as handle:
        preview = json.load(handle)[cast_name]
    body = {"voice_name": voice_name, "voice_description": preview["description"][:1000],
            "generated_voice_id": preview["generated_voice_id"]}
    reply = requests.post(f"{API}/v1/text-to-voice", headers={"xi-api-key": key}, json=body, timeout=120)
    book({"kind": "voice_save", "name": voice_name, "status": "ok" if reply.ok else f"http {reply.status_code}",
          "request_id": reply.headers.get("request-id", ""), "from": cast_name})
    if not reply.ok:
        sys.exit(f"[eleven] saving {voice_name}: http {reply.status_code} {reply.text[:200]}")
    voices[voice_name] = reply.json()["voice_id"]
    print(f"[eleven] voice {voice_name} saved from {cast_name}")
    return voices[voice_name]


def speak(key, jobs_path, only):
    """The voice-over: the cast is saved as permanent voices, then each line is spoken into Voice/Lines as an MP3 and a
    48 kHz mono WAV for the engine (ffmpeg)."""
    with open(jobs_path, encoding="utf-8") as handle:
        jobs = json.load(handle)
    voices_path = os.path.join(OUT, "Voice", "voices.json")
    voices = {}
    if os.path.exists(voices_path):
        with open(voices_path, encoding="utf-8") as handle:
            voices = json.load(handle)
    ids = {name: save_voice(key, cast, name, voices) for name, cast in jobs["cast"].items()}
    with open(voices_path, "w", encoding="utf-8") as handle:
        json.dump(voices, handle, indent=1)
    folder = os.path.join(OUT, "Voice", "Lines")
    os.makedirs(folder, exist_ok=True)
    for line in jobs["lines"]:
        if only and line["name"] not in only:
            continue
        body = {"text": line["text"], "model_id": jobs.get("model", "eleven_v3")}
        reply = requests.post(f"{API}/v1/text-to-speech/{ids[line['voice']]}", params={"output_format": "mp3_44100_192"},
                              headers={"xi-api-key": key}, json=body, timeout=180)
        status = "ok" if reply.ok else f"http {reply.status_code}"
        book({"kind": "tts", "name": line["name"], "status": status, "bytes": len(reply.content),
              "request_id": reply.headers.get("request-id", ""), "voice": line["voice"], "text": line["text"]})
        if not reply.ok:
            print(f"[eleven] {line['name']}: {status} {reply.text[:200]}")
            continue
        mp3 = os.path.join(folder, line["name"] + ".mp3")
        with open(mp3, "wb") as handle:
            handle.write(reply.content)
        subprocess.run(["ffmpeg", "-v", "error", "-y", "-i", mp3, "-ac", "1", "-ar", "48000", mp3[:-4] + ".wav"], check=True)
        print(f"[eleven] {line['name']}: ok")


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("action", choices=["credits", "sfx", "music", "stems", "design", "speak"])
    parser.add_argument("paths", nargs="*")
    parser.add_argument("--env", default=os.path.join(ROOT, ".env"))
    parser.add_argument("--only", default="")
    args = parser.parse_args()
    key = read_key(args.env)
    os.makedirs(OUT, exist_ok=True)
    if args.action == "credits":
        credits(key)
    elif args.action == "sfx":
        sfx(key, args.paths[0], set(filter(None, args.only.split(","))))
    elif args.action == "music":
        music(key, args.paths[0])
    elif args.action == "design":
        design(key, args.paths[0])
    elif args.action == "speak":
        speak(key, args.paths[0], set(filter(None, args.only.split(","))))
    else:
        stems(key, args.paths[0], args.paths[1])


if __name__ == "__main__":
    main()
