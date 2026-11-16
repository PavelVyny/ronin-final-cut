"""
KIND: offline tool (ffmpeg and numpy, no network).

Turns the ElevenLabs track and its stems into the four music layers of the game (mus_main_1_drums ... _4_lead): one
cut of 8 bars from the same start in every stem, so they play in step, with a short crossfade at the seam so the loop
does not click.

    python Tools/Audio/cut_loops.py probe <stems_dir>                  loudness of each stem, tempo of the drums
    python Tools/Audio/cut_loops.py cut <stems_dir> <out_dir> <start_s> <bpm> <layer=stem+stem> ...

Example: cut Audio/Eleven/Music/stems Audio/Eleven/Music 8.0 120 1_drums=drums 2_bass=bass 3_pad=other 4_lead=guitar+piano
"""
import subprocess
import sys
import wave
from pathlib import Path

import numpy as np

RATE = 48000
BARS = 8
SEAM_SECONDS = 0.05


def decode(path):
    raw = subprocess.run(["ffmpeg", "-v", "error", "-i", str(path), "-f", "f32le", "-ac", "2", "-ar", str(RATE), "-"],
                         capture_output=True, check=True).stdout
    return np.frombuffer(raw, dtype=np.float32).reshape(-1, 2)


LEAD_FROM_HZ = 800.0


def split_other(other):
    """The pad and the lead come mixed in one stem: the centred highs are taken as the lead, the rest as the pad."""
    mid = other.mean(axis=1)
    spectrum = np.fft.rfft(mid)
    freqs = np.fft.rfftfreq(len(mid), 1 / RATE)
    spectrum[freqs < LEAD_FROM_HZ] = 0
    lead_mono = np.fft.irfft(spectrum, n=len(mid)).astype(np.float32)
    lead = np.stack([lead_mono, lead_mono], axis=1)
    return {"pad_of_other": other - lead, "lead_of_other": lead}


def source(stems_dir, name):
    if name in ("pad_of_other", "lead_of_other"):
        return split_other(decode(Path(stems_dir) / "other.mp3"))[name]
    return decode(Path(stems_dir) / f"{name}.mp3")


def first_beat(signal, after):
    """Time of the strongest kick in the beat after `after` seconds: the cut starts on it."""
    mono = np.abs(signal).mean(axis=1)
    hop = 48
    frames = mono[: len(mono) // hop * hop].reshape(-1, hop).mean(axis=1)
    onset = np.maximum(np.diff(frames), 0)
    lo = int(after * RATE / hop)
    hi = lo + int(0.5 * RATE / hop)
    return (lo + int(np.argmax(onset[lo:hi]))) * hop / RATE


def tempo(signal):
    mono = np.abs(signal).mean(axis=1)
    hop = 480  # 10 ms
    frames = mono[: len(mono) // hop * hop].reshape(-1, hop)
    energy = np.sqrt((frames ** 2).mean(axis=1))
    onset = np.maximum(np.diff(energy), 0)
    onset -= onset.mean()
    corr = np.correlate(onset, onset, mode="full")[len(onset) - 1:]
    lags = np.arange(len(corr)) * hop / RATE
    window = (lags > 60 / 180) & (lags < 60 / 80)
    best = lags[window][np.argmax(corr[window])]
    return 60 / best


def probe(stems_dir):
    for path in sorted(Path(stems_dir).glob("*.mp3")):
        signal = decode(path)
        rms = np.sqrt((signal ** 2).mean())
        print(f"{path.stem:8s} rms {20 * np.log10(rms + 1e-9):6.1f} dB  {len(signal) / RATE:.1f} s")
    drums = decode(Path(stems_dir) / "drums.mp3")
    print(f"drums tempo about {tempo(drums):.1f} BPM")
    for after in (4.0, 6.0, 8.0):
        print(f"kick after {after} s at {first_beat(drums, after):.3f} s")


def write(path, signal):
    pcm = (np.clip(signal, -1, 1) * 32767).astype(np.int16)
    with wave.open(str(path), "wb") as out:
        out.setnchannels(2)
        out.setsampwidth(2)
        out.setframerate(RATE)
        out.writeframes(pcm.tobytes())


def cut(stems_dir, out_dir, start, bpm, layers):
    length = int(round(BARS * 4 * 60 / bpm * RATE))
    seam = int(SEAM_SECONDS * RATE)
    begin = int(start * RATE)
    fade_in = np.linspace(0, 1, seam)[:, None]
    for layer in layers:
        name, sources = layer.split("=")
        mix = sum(source(stems_dir, name_) for name_ in sources.split("+"))
        piece = mix[begin : begin + length + seam].copy()
        if len(piece) < length + seam:
            sys.exit(f"[loops] {name}: the track ends before 8 bars from {start} s")
        # The audio just past the end fades into the start: the jump at the seam becomes a blend.
        loop = piece[:length].copy()
        loop[:seam] = loop[:seam] * fade_in + piece[length:] * (1 - fade_in)
        write(Path(out_dir) / f"mus_main_{name}.wav", loop)
        print(f"[loops] mus_main_{name}.wav  {length / RATE:.2f} s from {sources}")


if __name__ == "__main__":
    if sys.argv[1] == "probe":
        probe(sys.argv[2])
    else:
        cut(sys.argv[2], sys.argv[3], float(sys.argv[4]), float(sys.argv[5]), sys.argv[6:])
