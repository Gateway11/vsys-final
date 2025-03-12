#!/usr/bin/env bash

if [[ $EUID -ne 0 ]]; then
    echo "切换到 root 执行此脚本..."
    exec sudo "$0" "$@"
fi

echo "已是 root，继续执行..."

amixer cset name="ADMAIF5" I2S7

if [[ -e /dev/a2b_ctrl ]]; then
    echo "RX SLAVE0 1" > /dev/a2b_ctrl
else
    i2ctransfer -f -y 16 w2@0x68 0x01 0x00
    i2ctransfer -f -y 16 w2@0x69 0x42 0x91
    #i2ctransfer -f -y 16 w2@0x69 0x42 0x11
    i2ctransfer -f -y 16 w2@0x69 0x47 0x00
fi
