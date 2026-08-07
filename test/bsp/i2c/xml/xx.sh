#!/vendor/bin/sh

i2c_dev=${1:-"/dev/i2c-1"}

# MX-00h: Software Reset
i2ccmd -d $i2c_dev -a 0x1c -o 0x00 -w "0x00 0x00"
sleep 0.1

# Private register initialization from the known-good script
i2ccmd -d $i2c_dev -a 0x1c -o 0x6A -w "0x00 0x3D"
i2ccmd -d $i2c_dev -a 0x1c -o 0x6C -w "0x36 0x00"

i2ccmd -d $i2c_dev -a 0x1c -o 0x6A -w "0x00 0x12"
i2ccmd -d $i2c_dev -a 0x1c -o 0x6C -w "0x0A 0xA8"

i2ccmd -d $i2c_dev -a 0x1c -o 0x6A -w "0x00 0x14"
i2ccmd -d $i2c_dev -a 0x1c -o 0x6C -w "0x0A 0xAA"

i2ccmd -d $i2c_dev -a 0x1c -o 0x6A -w "0x00 0x20"
i2ccmd -d $i2c_dev -a 0x1c -o 0x6C -w "0x61 0x10"

i2ccmd -d $i2c_dev -a 0x1c -o 0x6A -w "0x00 0x21"
i2ccmd -d $i2c_dev -a 0x1c -o 0x6C -w "0xE0 0xE0"

i2ccmd -d $i2c_dev -a 0x1c -o 0x6A -w "0x00 0x23"
i2ccmd -d $i2c_dev -a 0x1c -o 0x6C -w "0x18 0x04"

# MX-FAh: MCLK Detection
i2ccmd -d $i2c_dev -a 0x1c -o 0xFA -w "0x3C 0x00"

# MX-8Dh: Class-D Amplifier Control
i2ccmd -d $i2c_dev -a 0x1c -o 0x8D -w "0xA8 0x00"

# MX-FAh: I2S Clock Gating
i2ccmd -d $i2c_dev -a 0x1c -o 0xFA -w "0x34 0x01"

# MX-2Eh: DSP Path
i2ccmd -d $i2c_dev -a 0x1c -o 0x2E -w "0x0C 0x00"

# MX-63h: VREF1/VREF2, MBIAS, Bandgap and LDO2 Power
i2ccmd -d $i2c_dev -a 0x1c -o 0x63 -w "0xE8 0x1C"

# MX-80h: SYSCLK Source = External MCLK
i2ccmd -d $i2c_dev -a 0x1c -o 0x80 -w "0x00 0x00"

# MX-70h: RT5640 I2S1 Master, 16-bit Standard I2S
i2ccmd -d $i2c_dev -a 0x1c -o 0x70 -w "0x00 0x00"

# MX-73h: BCLK = 32 * LRCK, DAC OSR = 64fs
# 48kHz: LRCK = 48kHz, BCLK = 1.536MHz
i2ccmd -d $i2c_dev -a 0x1c -o 0x73 -w "0x01 0x14"

# MX-19h: DAC1 Left/Right Digital Volume
i2ccmd -d $i2c_dev -a 0x1c -o 0x19 -w "0xAF 0xAF"

# MX-29h: I2S1 to Stereo DAC Mixer
i2ccmd -d $i2c_dev -a 0x1c -o 0x29 -w "0x80 0x80"

# MX-2Ah: Stereo DAC1 Mixer
i2ccmd -d $i2c_dev -a 0x1c -o 0x2A -w "0x14 0x14"

# MX-2Ch: DACL1/DACR1 Routing
i2ccmd -d $i2c_dev -a 0x1c -o 0x2C -w "0x22 0x00"

# MX-46h: DACL1 to SPK MIXL
i2ccmd -d $i2c_dev -a 0x1c -o 0x46 -w "0x00 0x36"

# MX-47h: DACR1 to SPK MIXR
i2ccmd -d $i2c_dev -a 0x1c -o 0x47 -w "0x00 0x36"

# MX-48h: SPKVOLL to SPOLMIX
i2ccmd -d $i2c_dev -a 0x1c -o 0x48 -w "0xE8 0x00"

# MX-49h: SPKVOLR to SPORMIX
i2ccmd -d $i2c_dev -a 0x1c -o 0x49 -w "0x28 0x00"

# MX-61h: I2S1, DACL1, DACR1 and Class-D Power
# ADC remains powered off
i2ccmd -d $i2c_dev -a 0x1c -o 0x61 -w "0x98 0x01"

# MX-65h: SPK MIXL/R Power
# RECMIX and OUTMIX remain powered off
i2ccmd -d $i2c_dev -a 0x1c -o 0x65 -w "0x30 0x00"

# MX-66h: SPKVOLL/R Power
# Headphone and other output volume blocks remain powered off
i2ccmd -d $i2c_dev -a 0x1c -o 0x66 -w "0xC0 0x00"

sleep 0.1

# MX-01h: Enable and Unmute SPOL/SPOR
i2ccmd -d $i2c_dev -a 0x1c -o 0x01 -w "0x18 0x18"
