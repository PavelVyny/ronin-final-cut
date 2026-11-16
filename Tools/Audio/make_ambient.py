"""
KIND: offline tool (ffmpeg with rubberband, numpy, no network).

Turns the ElevenLabs ambient track into the game's soft loop under the comic and the intro (T-094): pitched to the
main theme's key, cut to 16 bars at 120 BPM with a crossfaded seam so the loop does not click, and levelled quieter
than the theme.

    python Tools/Audio/make_ambient.py <in.mp3> <out.wav> <semitones> <start_s>

Example: make_ambient.py Audio/Eleven/Music/mus_ambient_full.mp3 Audio/Eleven/Music/mus_ambient.wav 3 4.0
(ElevenLabs gave a drone on A although the prompt asked for C minor; +3 semitones puts it on C.)
"""
import subprocess
import sys
import wave

import numpy as np

RATE = 48000
LOOP_SECONDS = 32.0
SEAM_SECONDS = 1.0
MEAN_DB = -20.0
PEAK_DB = -1.0


def decode(path, semitones):
    pitch = 2.0 ** (semitones / 12.0)
    raw = subprocess.run(["ffmpeg", "-v", "error", "-i", path, "-af", f"rubberband=pitch={pitch:.6f}", "-f", "f32le", "-ac", "2",
                          "-ar", str(RATE), "-"], capture_output=True, check=True).stdout
    return np.frombuffer(raw, dtype=np.float32).reshape(-1, 2)


def main():
    source, target, semitones, start = sys.argv[1], sys.argv[2], float(sys.argv[3]), float(sys.argv[4])
    audio = decode(source, semitones)
    first = int(start * RATE)
    length = int(LOOP_SECONDS * RATE)
    seam = int(SEAM_SECONDS * RATE)
    if first + length + seam > len(audio):
        sys.exit(f"too short: {len(audio) / RATE:.1f} s")
    loop = audio[first:first + length].copy()
    # The head is crossfaded with what follows the cut, so the end runs on into the start.
    ramp = np.linspace(0.0, 1.0, seam, dtype=np.float32)[:, None]
    loop[:seam] = loop[:seam] * ramp + audio[first + length:first + length + seam] * (1.0 - ramp)
    mean = np.sqrt(np.mean(loop ** 2))
    gain = 10 ** (MEAN_DB / 20) / max(mean, 1e-9)
    gain = min(gain, 10 ** (PEAK_DB / 20) / max(np.abs(loop).max(), 1e-9))
    loop *= gain
    pcm = (np.clip(loop, -1, 1) * 32767).astype(np.int16)
    with wave.open(target, "wb") as out:
        out.setnchannels(2)
        out.setsampwidth(2)
        out.setframerate(RATE)
        out.writeframes(pcm.tobytes())
    print(f"[ambient] {target}: {LOOP_SECONDS:.0f} s, gain {20 * np.log10(gain):+.1f} dB")


if __name__ == "__main__":
    main()
