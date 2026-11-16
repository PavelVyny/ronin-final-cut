"""
KIND: offline tool (ffmpeg, no network).

Sets the ElevenLabs sounds to the mix of the game before import: each file is brought to a mean loudness by its role
(frequent small sounds quiet, rare big moments loud), the peak kept under -1 dB. Raw files stay in Audio/Eleven/Sfx and
Audio/Eleven/Music; the levelled ones go to Audio/Eleven/Game under the same names.

    python Tools/Audio/level_audio.py
"""
import re
import subprocess
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
SOURCES = [ROOT / "Audio" / "Eleven" / "Sfx", ROOT / "Audio" / "Eleven" / "Music"]
OUT = ROOT / "Audio" / "Eleven" / "Game"

# Mean loudness in dB by file. The music stems keep their own balance (they are cut from one mix).
TARGET = {
    "sfx_katana_slash_01": -22, "sfx_shuriken_throw_01": -24, "sfx_dash_01": -24, "sfx_boss_hit_01": -22,
    "sfx_parry_01": -18, "sfx_hero_hurt_01": -19,
    "sfx_slam_01": -14, "sfx_explosion_01": -16, "sfx_ultimate_01": -13, "sfx_final_blow_01": -12,
    "sfx_steal_card_01": -18, "sfx_steal_01": -16, "sfx_everything_back_01": -16, "sfx_ultimate_ready_01": -18, "sfx_heal_cash_01": -18,
    "mus_poc_loop": -18,
}
PEAK_LIMIT = -1.0


def levels(path):
    report = subprocess.run(["ffmpeg", "-i", str(path), "-af", "volumedetect", "-f", "null", "-"],
                            capture_output=True, text=True).stderr
    mean = float(re.search(r"mean_volume: (-?[\d.]+) dB", report).group(1))
    peak = float(re.search(r"max_volume: (-?[\d.]+) dB", report).group(1))
    return mean, peak


def main():
    OUT.mkdir(parents=True, exist_ok=True)
    for folder in SOURCES:
        for path in sorted(folder.glob("*.wav")):
            name = path.stem
            if name.startswith("mus_main"):
                gain = 0.0
            elif name in TARGET:
                mean, peak = levels(path)
                gain = min(TARGET[name] - mean, PEAK_LIMIT - peak)
            else:
                continue
            subprocess.run(["ffmpeg", "-v", "error", "-y", "-i", str(path), "-af", f"volume={gain:.2f}dB",
                            "-ar", "48000", "-c:a", "pcm_s16le", str(OUT / path.name)], check=True)
            print(f"[level] {name}: {gain:+.1f} dB")


if __name__ == "__main__":
    main()
