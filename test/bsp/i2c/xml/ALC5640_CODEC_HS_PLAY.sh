#!/vendor/bin/sh

i2c_dev=${1:-9}
i2ctransfer -f -y $i2c_dev w3@0x1C 0x00 0x00 0x00 #MX-00h: Software Reset
sleep 0.1
i2ctransfer -f -y $i2c_dev w3@0x1C 0x6A 0x00 0x3D #PR-3Dh: ADC/DAC RESET Control, Enable ADC and DAC Clock Generators
i2ctransfer -f -y $i2c_dev w3@0x1C 0x6C 0x36 0x00
i2ctransfer -f -y $i2c_dev w3@0x1C 0x6A 0x00 0x12 #PR-12h:
i2ctransfer -f -y $i2c_dev w3@0x1C 0x6C 0x0A 0xA8
i2ctransfer -f -y $i2c_dev w3@0x1C 0x6A 0x00 0x14 #PR-14h:
i2ctransfer -f -y $i2c_dev w3@0x1C 0x6C 0x0A 0xAA
i2ctransfer -f -y $i2c_dev w3@0x1C 0x6A 0x00 0x20 #PR-20h:
i2ctransfer -f -y $i2c_dev w3@0x1C 0x6C 0x61 0x10
i2ctransfer -f -y $i2c_dev w3@0x1C 0x6A 0x00 0x21 #PR-21h:
i2ctransfer -f -y $i2c_dev w3@0x1C 0x6C 0xE0 0xE0
i2ctransfer -f -y $i2c_dev w3@0x1C 0x6A 0x00 0x23 #PR-23h:
i2ctransfer -f -y $i2c_dev w3@0x1C 0x6C 0x18 0x04
i2ctransfer -f -y $i2c_dev w3@0x1C 0xFA 0x3C 0x00 #MX-FAh: General Control 1, MCLK Detection ON 
i2ctransfer -f -y $i2c_dev w3@0x1C 0x8D 0xA8 0x00 #MX-8Dh: Class-D Amp Output Control, AC+DC ratio gain = 3.3x
i2ctransfer -f -y $i2c_dev w3@0x1C 0xFA 0x34 0x01 #MX-FAh: General Control 1, I2S Clock Gating enable
i2ctransfer -f -y $i2c_dev w3@0x1C 0x2E 0x0C 0x00 #MX-2Eh: DSP_PATH2
i2ctransfer -f -y $i2c_dev w3@0x1C 0x63 0xE8 0x1C #MX-63h: VREF1/VREF2, MBIAS, Bandgap and LDO2 Power
i2ctransfer -f -y $i2c_dev w3@0x1C 0x80 0x00 0x00 #MX-80h: SYSCLK Source = External MCLK
i2ctransfer -f -y $i2c_dev w3@0x1C 0x70 0x00 0x00 #MX-70h: RT5640 I2S1 Master, 16-bit I2S; BCLK1/LRCK1 output, DACDAT1 input
i2ctransfer -f -y $i2c_dev w3@0x1C 0x73 0x01 0x14 #MX-73h: BCLK = 32 * LRCK, DAC OSR = 64fs; 48kHz: LRCK = 48kHz, BCLK = 1.536MHz

################################################################################
# Playback: I2S1 -> DAC1 L/R -> SPKMIXL/R -> SPKVOLL/R -> Class-D -> SPO L/R
################################################################################
i2ctransfer -f -y $i2c_dev w3@0x1C 0x19 0xAF 0xAF #MX-19h: DAC1 Left/Right Digital Volume
i2ctransfer -f -y $i2c_dev w3@0x1C 0x29 0x80 0x80 #MX-29h: I2S1 to Stereo DAC Mixer
i2ctransfer -f -y $i2c_dev w3@0x1C 0x2A 0x14 0x14 #MX-2Ah: Stereo DAC1 Mixer
i2ctransfer -f -y $i2c_dev w3@0x1C 0x2C 0x22 0x00 #MX-2Ch: DACL1/DACR1 Routing
i2ctransfer -f -y $i2c_dev w3@0x1C 0x46 0x00 0x36 #MX-46h: DACL1 to SPK MIXL - gain
i2ctransfer -f -y $i2c_dev w3@0x1C 0x47 0x00 0x36 #MX-47h: DACR1 to SPK MIXR - gain
i2ctransfer -f -y $i2c_dev w3@0x1C 0x48 0xE8 0x00 #MX-48h: SPKVOLL to SPOLMIX - path
i2ctransfer -f -y $i2c_dev w3@0x1C 0x49 0x28 0x00 #MX-49h: SPKVOLR to SPORMIX - path
i2ctransfer -f -y $i2c_dev w3@0x1C 0x61 0x98 0x01 #MX-61h: I2S1, DACL1, DACR1 and Class-D Power; ADC remains powered off
i2ctransfer -f -y $i2c_dev w3@0x1C 0x65 0x30 0x00 #MX-65h: SPK MIXL/R Power; RECMIX and OUTMIX remain powered off
i2ctransfer -f -y $i2c_dev w3@0x1C 0x66 0xC0 0x00 #MX-66h: SPKVOLL/R Power; Headphone and other output volume blocks remain powered off
sleep 0.1
i2ctransfer -f -y $i2c_dev w3@0x1C 0x01 0x18 0x18 #MX-01h: Enable and Unmute SPOL/SPOR
