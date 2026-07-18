#!/usr/bin/env bash

I2S_DEV=${1:-7}
amixer cset name="I2S${I2S_DEV} Mux" ADMAIF1
amixer cset name="ADMAIF5 Mux" I2S${I2S_DEV}

declare -A A2B_BUS=(["1"]="1" ["4"]="2" ["7"]="")
echo "Disable Fault Check" > /dev/a2b${A2B_BUS[$I2S_DEV]}_ctrl

declare -A I2C_MAP=(["1"]="2" ["4"]="3" ["7"]="16")
#i2ctransfer -f -y ${I2C_MAP[$I2S_DEV]} w2@0x68 0x11 0x00
i2ctransfer -f -y ${I2C_MAP[$I2S_DEV]} w2@0x68 0x53 0x06

pkill -9 -f "aplay|python3 -"
python3 - <<'PY' | aplay -D hw:0,0 -f S16_LE -r 48000 -c 32 &

import sys, math, struct

sr, ch, freq = 48000, 32, 1000

# 生成一个周期的正弦波
period = sr // freq

data=b''.join(
    struct.pack("<h", int(32767 * math.sin(2 * math.pi * freq * i / sr))) * ch
    # struct.pack("<i", int(2147483647 * math.sin(2 * math.pi * freq * i / sr))) * ch
    for i in range(period)
)

while True:
    sys.stdout.buffer.write(data)

PY

sleep 1
arecord -D hw:0,4 -f S16_LE -c 32 -r 48000 -d 2 record.wav
xxd record.wav | head -20
