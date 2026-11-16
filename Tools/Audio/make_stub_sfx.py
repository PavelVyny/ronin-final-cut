"""Stub sound effects until Kirill's sounds arrive (T-028).

One short sound per cue of JamSoundRules, named the way Kirill delivers them (sfx_<what>_01.wav). Synthesised here
from sine, saw, square and noise: no samples, nothing third party. 48 kHz, 16-bit mono.

    python Tools/Audio/make_stub_sfx.py [out_dir]     (default Audio/Stub/Sfx)
"""

import sys
import wave
from pathlib import Path

import numpy as np

RATE = 48000
RNG = np.random.default_rng(29)


def samples(seconds):
    return int(round(seconds * RATE))


def times(n):
    return np.arange(n) / RATE


def env(n, attack, decay):
    t = times(n)
    return np.clip(t / max(attack, 1e-4), 0, 1) * np.exp(-t / decay)


def noise(n):
    return RNG.uniform(-1, 1, n)


def sweep(f0, f1, n, wave_fn=np.sin):
    """A tone gliding from f0 to f1 over n samples (exponential glide)."""
    freq = f0 * (f1 / f0) ** (np.arange(n) / max(n - 1, 1))
    phase = 2 * np.pi * np.cumsum(freq) / RATE
    return wave_fn(phase)


def saw_wave(phase):
    return 2.0 * ((phase / (2 * np.pi)) % 1.0) - 1.0


def square_wave(phase):
    return np.sign(np.sin(phase))


def lowpass(x, k):
    """One-pole low pass, k in (0, 1]: smaller is darker. k may be an array to move the cut-off over time."""
    k = np.broadcast_to(k, x.shape)
    y = np.empty_like(x)
    acc = 0.0
    for i, v in enumerate(x):
        acc += k[i] * (v - acc)
        y[i] = acc
    return y


def highpass(x, k):
    return x - lowpass(x, k)


def pad_to(x, n):
    return np.concatenate([x, np.zeros(max(0, n - len(x)))])[: max(n, len(x))]


def mix(*parts):
    n = max(len(p) for p in parts)
    return sum(pad_to(p, n) for p in parts)


def katana_slash():
    n = samples(0.28)
    whoosh = highpass(noise(n), np.linspace(0.02, 0.4, n)) * env(n, 0.03, 0.07)
    ring = (np.sin(2 * np.pi * 2350 * times(n)) + 0.6 * np.sin(2 * np.pi * 3170 * times(n))) * env(n, 0.002, 0.09) * 0.35
    return mix(whoosh, ring)


def shuriken_throw():
    n = samples(0.16)
    return highpass(noise(n), 0.3) * env(n, 0.01, 0.04) * 0.8


def explosion():
    n = samples(0.9)
    rumble = lowpass(noise(n), np.linspace(0.25, 0.01, n)) * env(n, 0.002, 0.25) * 3
    thump = sweep(120, 40, n) * env(n, 0.002, 0.15)
    return mix(rumble, thump)


def slam():
    n = samples(0.6)
    thump = sweep(90, 32, n) * env(n, 0.002, 0.2)
    dirt = lowpass(noise(n), 0.05) * env(n, 0.001, 0.08) * 2
    return mix(thump, dirt)


def dash():
    n = samples(0.24)
    shape = np.sin(np.pi * np.arange(n) / n) ** 2
    return highpass(lowpass(noise(n), np.linspace(0.05, 0.6, n)), 0.05) * shape * 0.8


def parry():
    n = samples(0.5)
    t = times(n)
    ping = sum(np.sin(2 * np.pi * f * t) * a for f, a in ((1800, 1.0), (2710, 0.6), (4130, 0.35))) * env(n, 0.001, 0.12)
    click = noise(samples(0.01)) * 0.6
    return mix(ping, click)


def ultimate():
    n = samples(0.7)
    rise = sweep(200, 1600, samples(0.25), saw_wave) * env(samples(0.25), 0.2, 1.0) * 0.3
    cut = highpass(noise(n), 0.15) * env(n, 0.003, 0.12)
    boom = sweep(150, 45, n) * env(n, 0.002, 0.18) * 0.7
    return np.concatenate([rise, mix(cut, boom)])


def boss_hit():
    n = samples(0.14)
    thud = sweep(180, 90, n) * env(n, 0.001, 0.04)
    click = highpass(noise(n), 0.4) * env(n, 0.0005, 0.008) * 0.5
    return mix(thud, click)


def hero_hurt():
    n = samples(0.26)
    return lowpass(sweep(260, 110, n, square_wave), 0.2) * env(n, 0.003, 0.09)


def steal_card():
    n = samples(0.09)
    first = np.sin(2 * np.pi * 880 * times(n)) * env(n, 0.002, 0.03)
    second = np.sin(2 * np.pi * 1320 * times(n)) * env(n, 0.002, 0.05)
    return np.concatenate([first, second])


def steal():
    n = samples(0.8)
    fall = lowpass(sweep(700, 110, n, saw_wave), 0.15) * env(n, 0.01, 0.35)
    hit = lowpass(noise(samples(0.2)), 0.1) * env(samples(0.2), 0.001, 0.05)
    return mix(fall, hit)


def everything_back():
    note = samples(0.12)
    tones = [np.sin(2 * np.pi * f * times(note)) * env(note, 0.003, 0.08) for f in (523, 659, 784, 1047)]
    tail = np.sin(2 * np.pi * 1047 * times(samples(0.6))) * env(samples(0.6), 0.003, 0.25)
    return np.concatenate(tones[:-1] + [tail])


def final_blow():
    hit = ultimate()
    n = samples(1.8)
    boom = mix(sweep(80, 28, n) * env(n, 0.002, 0.5), lowpass(noise(n), 0.03) * env(n, 0.002, 0.6) * 4)
    return mix(hit, np.concatenate([np.zeros(samples(0.25)), boom]))


def ultimate_ready():
    # A rising three-note chime: the blade is charged.
    notes = []
    for f in (660, 880, 1320):
        n = samples(0.12)
        t = times(n)
        notes.append((np.sin(2 * np.pi * f * t) + 0.4 * np.sin(2 * np.pi * 2 * f * t)) * env(n, 0.003, 0.08))
    tail = samples(0.35)
    shimmer = np.sin(2 * np.pi * 1320 * times(tail)) * env(tail, 0.002, 0.15) * 0.5
    return np.concatenate(notes + [shimmer])


# Name, maker, loudness against the loudest. The order is EJamCue.
CUES = [
    ("sfx_katana_slash_01", katana_slash, 0.7),
    ("sfx_shuriken_throw_01", shuriken_throw, 0.5),
    ("sfx_explosion_01", explosion, 0.9),
    ("sfx_slam_01", slam, 0.9),
    ("sfx_dash_01", dash, 0.5),
    ("sfx_parry_01", parry, 0.8),
    ("sfx_ultimate_01", ultimate, 1.0),
    ("sfx_boss_hit_01", boss_hit, 0.45),
    ("sfx_hero_hurt_01", hero_hurt, 0.7),
    ("sfx_steal_card_01", steal_card, 0.6),
    ("sfx_steal_01", steal, 0.85),
    ("sfx_everything_back_01", everything_back, 0.8),
    ("sfx_final_blow_01", final_blow, 1.0),
    ("sfx_ultimate_ready_01", ultimate_ready, 0.7),
]


def write(path, mono, loudness):
    # A short fade at the end so a cut tail never clicks.
    fade = min(len(mono), samples(0.01))
    mono = mono.copy()
    mono[-fade:] *= np.linspace(1, 0, fade)
    peak = np.max(np.abs(mono)) or 1.0
    pcm = (np.clip(mono / peak * 0.8 * loudness, -1, 1) * 32767).astype(np.int16)
    with wave.open(str(path), "wb") as out:
        out.setnchannels(1)
        out.setsampwidth(2)
        out.setframerate(RATE)
        out.writeframes(pcm.tobytes())


def main():
    out_dir = Path(sys.argv[1] if len(sys.argv) > 1 else "Audio/Stub/Sfx")
    out_dir.mkdir(parents=True, exist_ok=True)
    for name, make, loudness in CUES:
        sound = make()
        write(out_dir / f"{name}.wav", sound, loudness)
        print(f"[sfx] {out_dir / name}.wav  {len(sound) / RATE:.2f} s")


if __name__ == "__main__":
    main()
