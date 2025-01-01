#!/usr/bin/env bash

if [ -n "$1" ]; then
    setterm -linewrap on
    resize

    #amixer cset name="AMX1 RX1 Mux" "ADMAIF1"
    #amixer cset name="AMX1 RX2 Mux" "ADMAIF2"
    #amixer cset name="AMX1 RX3 Mux" "ADMAIF3"
    #amixer cset name="AMX1 RX4 Mux" "ADMAIF4"
    #amixer cset name="AMX1 RX5 Mux" "ADMAIF5"
    #amixer cset name="AMX1 RX6 Mux" "ADMAIF6"
    #amixer cset name="AMX1 RX7 Mux" "ADMAIF7"
    #amixer cset name="AMX1 RX8 Mux" "ADMAIF8"
    #amixer cset name="AMX1 RX9 Mux" "ADMAIF9"
    #amixer cset name="AMX1 RX10 Mux" "ADMAIF10"
    #amixer cset name="AMX1 RX11 Mux" "ADMAIF11"
    #amixer cset name="AMX1 RX12 Mux" "ADMAIF12"
    #amixer cset name="AMX1 RX13 Mux" "ADMAIF13"
    #amixer cset name="AMX1 RX14 Mux" "ADMAIF14"
    #amixer cset name="AMX1 RX15 Mux" "ADMAIF15"
    #amixer cset name="AMX1 RX16 Mux" "ADMAIF16"
    #amixer cset name="I2S7 Mux" "AMX1"
    #aplay -D hw:0,2 /usr/share/sounds/alsa/Rear_Right.wav

    amixer cset name="I2S7 Mux" ADMAIF1
    aplay -D hw:0,0 /usr/share/sounds/alsa/Rear_Right.wav

    amixer cset name="ADMAIF5 Mux" I2S7
    #amixer cset name="MVC1 Mux" I2S7
    #amixer cset name="MVC1 Volume" 1000
    #amixer cset name="ADMAIF5 Mux" MVC1
else
    arecord -D hw:0,4 -f S32_LE -c 8 -r 48000 -d 1 record.wav
    tar -cvf - record.wav | xz -9 --extreme | base64
fi
