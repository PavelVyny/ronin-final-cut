"""Stub music stems until Kirill's tracks arrive (T-024).

Four looping stems of the same length and tempo, named the way Kirill delivers them, plus the proof-of-concept loop
(a bare metronome) that plays when every stem is cut. Synthesised here from sine, saw and noise: no samples, nothing
third party. 120 BPM, 8 bars, 48 kHz, 16-bit stereo.

    python Tools/Audio/make_stub_stems.py [out_dir]     (default Audio/Stub)
"""

import sys
import wave
from pathlib import Path

import numpy as np

RATE = 48000
BPM = 120
BARS = 8
BEAT = 60.0 / BPM
LENGTH = BARS * 4 * BEAT
N = int(round(LENGTH * RATE))
# Am, F, C, G: two bars each.
ROOTS = [57, 53, 48, 55]
CHORDS = [[57, 60, 64], [53, 57, 60], [48, 52, 55], [55, 59, 62]]
# Lead arpeggio: chord tones by sixteenth, an octave up.
ARP = [0, 1, 2, 1, 2, 0, 1, 2]


def hz(note):
    return 440.0 * 2.0 ** ((note - 69) / 12.0)


def place(track, start, sound):
    """Adds sound at start seconds, wrapping round the end so the loop is seamless."""
    i = int(round(start * RATE)) % N
    end = i + len(sound)
    if end <= N:
        track[i:end] += sound
    else:
        cut = N - i
        track[i:] += sound[:cut]
        track[: end - N] += sound[cut:]


def env(n, attack, decay):
    t = np.arange(n) / RATE
    a = np.clip(t / max(attack, 1e-4), 0, 1)
    return a * np.exp(-t / decay)


def saw(freq, n):
    t = np.arange(n) / RATE
    return 2.0 * ((t * freq) % 1.0) - 1.0


def square(freq, n):
    return np.sign(np.sin(2 * np.pi * freq * np.arange(n) / RATE))


def lowpass(x, k):
    """One-pole low pass, k in (0, 1]: smaller is darker. Run over two laps so the loop point has no click."""
    lap = np.concatenate([x, x])
    y = np.empty_like(lap)
    acc = 0.0
    for i, v in enumerate(lap):
        acc += k * (v - acc)
        y[i] = acc
    return y[len(x):]


def chord_at(bar):
    return bar // 2 % 4


def drums():
    track = np.zeros(N)
    rng = np.random.default_rng(7)
    kick_n = int(0.25 * RATE)
    t = np.arange(kick_n) / RATE
    kick = np.sin(2 * np.pi * (50 * t + 90 * (1 - np.exp(-t * 30)) / 30)) * np.exp(-t * 12)
    snare_n = int(0.18 * RATE)
    snare = rng.uniform(-1, 1, snare_n) * env(snare_n, 0.001, 0.05) * 0.6
    hat_n = int(0.05 * RATE)
    hat = np.diff(rng.uniform(-1, 1, hat_n + 1)) * env(hat_n, 0.0005, 0.012) * 0.25
    for beat in range(BARS * 4):
        place(track, beat * BEAT, kick)
        if beat % 2 == 1:
            place(track, beat * BEAT, snare)
        place(track, beat * BEAT, hat)
        place(track, (beat + 0.5) * BEAT, hat)
    return track


def bass():
    track = np.zeros(N)
    note_n = int(BEAT / 2 * RATE)
    for eighth in range(BARS * 8):
        root = ROOTS[chord_at(eighth // 8)] - 12
        tone = saw(hz(root), note_n) * env(note_n, 0.005, 0.18)
        place(track, eighth * BEAT / 2, tone * 0.5)
    return lowpass(track, 0.08)


def pad():
    track = np.zeros(N)
    span_n = int(8 * BEAT * RATE)
    shape = np.minimum(1, np.minimum(np.arange(span_n) / (0.6 * RATE), (span_n - np.arange(span_n)) / (0.6 * RATE)))
    for part in range(4):
        tone = sum(saw(hz(n) * d, span_n) for n in CHORDS[part] for d in (0.997, 1.003)) / 6.0
        place(track, part * 8 * BEAT, tone * shape * 0.35)
    return lowpass(track, 0.05)


def lead():
    track = np.zeros(N)
    note_n = int(BEAT / 4 * RATE)
    for sixteenth in range(BARS * 16):
        chord = CHORDS[chord_at(sixteenth // 16)]
        note = chord[ARP[sixteenth % len(ARP)]] + 12
        tone = square(hz(note), note_n) * env(note_n, 0.002, 0.07)
        place(track, sixteenth * BEAT / 4, tone * 0.18)
    return lowpass(track, 0.25)


def poc_loop():
    track = np.zeros(N)
    click_n = int(0.03 * RATE)
    for beat in range(BARS * 4):
        freq = 1760 if beat % 4 == 0 else 880
        place(track, beat * BEAT, np.sin(2 * np.pi * freq * np.arange(click_n) / RATE) * env(click_n, 0.0005, 0.008) * 0.5)
    return track


def write(path, mono):
    peak = np.max(np.abs(mono)) or 1.0
    pcm = (np.clip(mono / peak * 0.7, -1, 1) * 32767).astype(np.int16)
    stereo = np.repeat(pcm[:, None], 2, axis=1)
    with wave.open(str(path), "wb") as out:
        out.setnchannels(2)
        out.setsampwidth(2)
        out.setframerate(RATE)
        out.writeframes(stereo.tobytes())


def main():
    out_dir = Path(sys.argv[1] if len(sys.argv) > 1 else "Audio/Stub")
    out_dir.mkdir(parents=True, exist_ok=True)
    stems = {
        "mus_main_1_drums.wav": drums,
        "mus_main_2_bass.wav": bass,
        "mus_main_3_pad.wav": pad,
        "mus_main_4_lead.wav": lead,
        "mus_poc_loop.wav": poc_loop,
    }
    for name, make in stems.items():
        write(out_dir / name, make())
        print(f"[stems] {out_dir / name}  {LENGTH:.1f} s")


if __name__ == "__main__":
    main()
