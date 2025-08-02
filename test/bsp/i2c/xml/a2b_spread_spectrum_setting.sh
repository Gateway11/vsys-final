#!/usr/bin/env bash

if [ $# -ne 3 ]; then
    echo "示例: $0 0x88 0x83 0x83"
    exit 1
fi

set -x
i2ctransfer -f -y 16 w2@0x68 0x01 0x80

i2ctransfer -f -y 16 w2@0x69 0x40 $1
i2ctransfer -f -y 16 w2@0x69 0x2E $2
i2ctransfer -f -y 16 w2@0x69 0x30 $3

i2ctransfer -f -y 16 w2@0x69 0x00 0x68
i2ctransfer -f -y 16 w2@0x68 0x01 0x20
i2ctransfer -f -y 16 w2@0x69 0x01 0x80

i2ctransfer -f -y 16 w2@0x68 0x01 0x00
i2ctransfer -f -y 16 w2@0x69 0x00 0x69
i2ctransfer -f -y 16 w2@0x68 0x01 0x20

i2ctransfer -f -y 16 w2@0x69 0x40 $1
i2ctransfer -f -y 16 w2@0x69 0x2E $2
i2ctransfer -f -y 16 w2@0x69 0x30 $3

#####################################################

#Enable Voltage Monitor here
i2ctransfer -f -y 16 w2@0x68 0xE0 0x01 #MMRPAGE
i2ctransfer -f -y 16 w2@0x68 0x00 0x02 #VMTR_VEN, Enable VBUS
i2ctransfer -f -y 16 w2@0x68 0x25 0x02 #VMTR_VMIN1, program Min voltage as 0
i2ctransfer -f -y 16 w2@0x68 0x24 0x50 #VMTR_VMAX1, program Max voltage for 10V
i2ctransfer -f -y 16 w2@0x68 0xE0 0x00 #MMRPAGE

#Read Voltage Monitor
i2ctransfer -f -y 16 w2@0x68 0xE0 0x01 #MMRPAGE
i2ctransfer -f -y 16 w1@0x68 0x02 r1 #VMTR_MXSTAT, maxThres
i2ctransfer -f -y 16 w1@0x68 0x03 r1 #VMTR_MNSTAT, minThres
i2ctransfer -f -y 16 w1@0x68 0x23 r1 #VMTR_VLTG1, Read the measured voltage
i2ctransfer -f -y 16 w2@0x68 0xE0 0x00 #MMRPAGE
