#!/usr/bin/env bash

if [ -n "$1" ]; then
    setterm -linewrap on
    resize

    amixer cset name="MVC1 Mux" ADMAIF1
    amixer cset name="MVC1 Volume" 10

    #amixer cset name="SFC1 Mux" ADMAIF1
    amixer cset name="SFC1 Mux" MVC1
    amixer cset name="AMX1 RX3 Mux" SFC1
    amixer cset name="I2S7 Mux" AMX1
    amixer cset name="SFC1 Input Sample Rate" 44100
    amixer cset name="SFC1 Output Sample Rate" 48000
    #aplay -D hw:0,0 44100_2ch_32bit.wav

    #amixer cset name="I2S7 Mux" ADMAIF1
    #aplay -D hw:0,0 /usr/share/sounds/alsa/Rear_Right.wav

    amixer cset name="ADMAIF5 Mux" I2S7
    #amixer cset name="MVC2 Mux" I2S7
    #amixer cset name="MVC2 Volume" 1000
    #amixer cset name="ADMAIF5 Mux" MVC2

    #amixer cset name="ADX1 Mux" I2S7
    #amixer cset name="ADMAIF5 Mux" "ADX1 TX1"
    #amixer cset name="ADMAIF6 Mux" "ADX1 TX2"
else
    arecord -D hw:0,4 -f S32_LE -c 8 -r 48000 -d 1 record.wav
    tar -cvf - record.wav | xz -9 --extreme | base64
fi
