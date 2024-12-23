#!/usr/bin/env bash

if [ -n "$1" ]; then
    setterm -linewrap on
    resize

    amixer cset name="I2S7 Mux" ADMAIF1
    aplay -D hw:0,0 /usr/share/sounds/alsa/Rear_Right.wav

    amixer cset name="ADMAIF5 Mux" I2S7
else
    #arecord -D hw:0,4 -f S32_LE -c 8 -r 48000 -d 2 record.wav
    #tar -cvf - record.wav | xz -9 --extreme | base64
    arecord -D hw:0,4 -f S32_LE -c 8 -r 48000 -d 2 | xz -9 --extreme | base64
fi
