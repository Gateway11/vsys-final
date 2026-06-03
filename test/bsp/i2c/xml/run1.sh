#!/usr/bin/env bash

echo "Disable Fault Check" > /dev/a2b1_ctrl
i2ctransfer -f -y 2 w2@0x68 0x11 0x00
i2ctransfer -f -y 2 w2@0x68 0x53 0x06

amixer cset name="I2S1 Mux" ADMAIF1
amixer cset name="ADMAIF5 Mux" I2S1

aplay -D hw:0,0 out_sine_48000_16bit_32ch_10s.wav &
sleep 1
arecord -D hw:0,4 -f S16_LE -c 32 -r 48000 -d 5 record.wav
xxd record.wav | head -n 20
