#!/usr/bin/env bash
echo "Disable Fault Check" > /dev/a2b1_ctrl

I2S_DEV=${1:-7}
amixer cset name="I2S${I2S_DEV} Mux" ADMAIF1
amixer cset name="ADMAIF5 Mux" I2S${I2S_DEV}

# Mapping between I2S device number and I2C bus number
# I2S1 -> I2C bus 2
# I2S4 -> I2C bus 3
# I2S7 -> I2C bus 16
declare -A BUS_MAP=(["1"]="2" ["4"]="3" ["7"]="16")
i2ctransfer -f -y ${BUS_MAP[${I2S_DEV}]} w2@0x68 0x11 0x00
i2ctransfer -f -y ${BUS_MAP[${I2S_DEV}]} w2@0x68 0x53 0x06

pkill -9 aplay
aplay -D hw:0,0 out_sine_48000_16bit_32ch_10s.wav &
sleep 1
arecord -D hw:0,4 -f S16_LE -c 32 -r 48000 -d 2 record.wav
xxd record.wav | head -n 20
