#!/usr/bin/env python3
import sys
import math
import struct

DEFAULT_SAMPLE_RATE = 48000
DEFAULT_BIT_DEPTH   = 16
DEFAULT_CHANNELS    = 32
DEFAULT_DURATION    = 300     # seconds (5 minutes)
TONE_FREQ           = 1000.0  # 1 kHz


def print_usage(prog):
    print(f"""Usage:
  {prog} [sample_rate] [bit_depth] [channels] [duration_sec]

Arguments:
  sample_rate     Sample rate in Hz (default: {DEFAULT_SAMPLE_RATE})
  bit_depth       PCM bit depth: 16 or 32 (default: {DEFAULT_BIT_DEPTH})
  channels        Number of channels (default: {DEFAULT_CHANNELS})
  duration_sec    Duration in seconds (default: {DEFAULT_DURATION} = 5 minutes)

Examples:
  {prog}
  {prog} 48000 16 32 300
  {prog} 48000 32 8 10
""")


def gen_filename(sr, bits, ch, dur):
    return f"out_sine_{sr}_{bits}bit_{ch}ch_{dur}s.wav"


def write_wav_header(f, sample_rate, bit_depth, channels, data_size):
    bytes_per_sample = bit_depth // 8
    byte_rate = sample_rate * channels * bytes_per_sample
    block_align = channels * bytes_per_sample
    riff_size = 36 + data_size

    # RIFF header
    f.write(b"RIFF")
    f.write(struct.pack("<I", riff_size))
    f.write(b"WAVE")

    # fmt chunk
    f.write(b"fmt ")
    f.write(struct.pack("<I", 16))          # fmt chunk size
    f.write(struct.pack("<H", 1))           # PCM
    f.write(struct.pack("<H", channels))
    f.write(struct.pack("<I", sample_rate))
    f.write(struct.pack("<I", byte_rate))
    f.write(struct.pack("<H", block_align))
    f.write(struct.pack("<H", bit_depth))

    # data chunk header
    f.write(b"data")
    f.write(struct.pack("<I", data_size))


def main():
    if len(sys.argv) <= 1 or sys.argv[1] in ("-h", "--help"):
        print_usage(sys.argv[0])
        return

    sample_rate  = int(sys.argv[1]) if len(sys.argv) > 1 else DEFAULT_SAMPLE_RATE
    bit_depth    = int(sys.argv[2]) if len(sys.argv) > 2 else DEFAULT_BIT_DEPTH
    channels     = int(sys.argv[3]) if len(sys.argv) > 3 else DEFAULT_CHANNELS
    duration_sec = int(sys.argv[4]) if len(sys.argv) > 4 else DEFAULT_DURATION

    if bit_depth not in (16, 32):
        print("Error: only 16-bit and 32-bit PCM are supported")
        return

    filename = gen_filename(sample_rate, bit_depth, channels, duration_sec)

    total_frames = sample_rate * duration_sec
    bytes_per_sample = bit_depth // 8
    data_size = total_frames * channels * bytes_per_sample

    if data_size > 0xFFFFFFFF:
        print("Error: WAV file exceeds 4GB RIFF limit")
        return

    with open(filename, "wb") as f:
        write_wav_header(f, sample_rate, bit_depth, channels, data_size)

        for n in range(total_frames):
            s = math.sin(2.0 * math.pi * TONE_FREQ * n / sample_rate)

            if bit_depth == 16:
                v = int(s * 32767)
                frame = struct.pack("<h", v) * channels
            else:  # 32-bit PCM
                v = int(s * 2147483647)
                frame = struct.pack("<i", v) * channels

            f.write(frame)

    print(f"Generated {filename} "
          f"({sample_rate} Hz, {bit_depth}-bit, {channels} ch, "
          f"{duration_sec} sec, 1 kHz)")


if __name__ == "__main__":
    main()
