"""
KIND: offline tool (rewrites a 16-bit wav so its hit starts at once).

    python Tools/Audio/trim_onset.py <in.wav> <out.wav> [--lead 0.04] [--threshold 0.3]

Cuts the silence before the first sample louder than threshold of the peak, keeping lead seconds before it with a short
fade in, so a sound cued on a picture lands with it (the comic's glass shatter).
"""
import argparse
import struct
import wave


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("src")
    parser.add_argument("dst")
    parser.add_argument("--lead", type=float, default=0.04)
    parser.add_argument("--threshold", type=float, default=0.3)
    args = parser.parse_args()
    with wave.open(args.src) as w:
        params = w.getparams()
        data = w.readframes(w.getnframes())
    channels, rate = params.nchannels, params.framerate
    samples = list(struct.unpack("<%dh" % (len(data) // 2), data))
    peak = max(abs(s) for s in samples) or 1
    onset = next(i for i, s in enumerate(samples) if abs(s) > args.threshold * peak) // channels
    start = max(0, onset - round(args.lead * rate))
    out = samples[start * channels:]
    fade = (onset - start) * channels
    for i in range(fade):
        out[i] = round(out[i] * i / fade)
    with wave.open(args.dst, "wb") as w:
        w.setparams(params)
        w.writeframes(struct.pack("<%dh" % len(out), *out))
    print("onset %.3f s, cut %.3f s, now %.2f s" % (onset / rate, start / rate, len(out) / channels / rate))


if __name__ == "__main__":
    main()
