#!/usr/bin/env bash

rm record.wav
rm output_wav.txt

if [ -n "$1" ]; then
    amixer cset name="I2S7 Mux" ADMAIF1
    aplay -D hw:0,0 /usr/share/sounds/alsa/Rear_Right.wav

    amixer cset name="ADMAIF5 Mux" I2S7
fi

arecord -D hw:0,4 -f S32_LE -c 8 -r 48000 -d 2 record.wav
tar -cvf - record.wav | xz -9 --extreme | base64 > output_wav.txt

cat output_wav.txt
