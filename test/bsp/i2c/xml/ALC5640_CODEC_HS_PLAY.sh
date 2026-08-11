#!/usr/bin/env bash

i2c_dev=${1:-9}
debug() { echo "Running command $((++line_count)): $@"; "$@"; }
echo 12288000 > /sys/kernel/debug/bpmp/debug/clk/aud_mclk/rate
echo 1 > /sys/kernel/debug/bpmp/debug/clk/aud_mclk/state
sleep 1

debug i2ctransfer -f -y $i2c_dev w3@0x1C 0x00 0x00 0x00 #MX-00h: Software Reset
debug sleep 0.1
debug i2ctransfer -f -y $i2c_dev w3@0x1C 0x6A 0x00 0x3D #PR-3Dh: ADC/DAC RESET Control, Enable ADC and DAC Clock Generators
debug i2ctransfer -f -y $i2c_dev w3@0x1C 0x6C 0x36 0x00
debug i2ctransfer -f -y $i2c_dev w3@0x1C 0x6A 0x00 0x12 #PR-12h:
debug i2ctransfer -f -y $i2c_dev w3@0x1C 0x6C 0x0A 0xA8
debug i2ctransfer -f -y $i2c_dev w3@0x1C 0x6A 0x00 0x14 #PR-14h:
debug i2ctransfer -f -y $i2c_dev w3@0x1C 0x6C 0x0A 0xAA
debug i2ctransfer -f -y $i2c_dev w3@0x1C 0x6A 0x00 0x20 #PR-20h:
debug i2ctransfer -f -y $i2c_dev w3@0x1C 0x6C 0x61 0x10
debug i2ctransfer -f -y $i2c_dev w3@0x1C 0x6A 0x00 0x21 #PR-21h:
debug i2ctransfer -f -y $i2c_dev w3@0x1C 0x6C 0xE0 0xE0
debug i2ctransfer -f -y $i2c_dev w3@0x1C 0x6A 0x00 0x23 #PR-23h:
debug i2ctransfer -f -y $i2c_dev w3@0x1C 0x6C 0x18 0x04
debug i2ctransfer -f -y $i2c_dev w3@0x1C 0xFA 0x3C 0x00 #MX-FAh: General Control 1, MCLK Detection ON 
debug i2ctransfer -f -y $i2c_dev w3@0x1C 0x8D 0xA8 0x00 #MX-8Dh: Class-D Amp Output Control, AC+DC ratio gain = 3.3x
debug i2ctransfer -f -y $i2c_dev w3@0x1C 0xFA 0x34 0x01 #MX-FAh: General Control 1, I2S Clock Gating enable
debug i2ctransfer -f -y $i2c_dev w3@0x1C 0x2E 0x0C 0x00 #MX-2Eh: DSP_PATH2
debug i2ctransfer -f -y $i2c_dev w3@0x1C 0x63 0xE8 0x1C #MX-63h: VREF1/VREF2, MBIAS, Bandgap and LDO2 Power
debug i2ctransfer -f -y $i2c_dev w3@0x1C 0x80 0x00 0x00 #MX-80h: SYSCLK Source = External MCLK
debug i2ctransfer -f -y $i2c_dev w3@0x1C 0x70 0x00 0x00 #MX-70h: RT5640 I2S1 Master, 16-bit I2S; BCLK1/LRCK1 output, DACDAT1 input
debug i2ctransfer -f -y $i2c_dev w3@0x1C 0x73 0x01 0x14 #MX-73h: BCLK = 32 * LRCK, DAC OSR = 64fs; 48kHz: LRCK = 48kHz, BCLK = 1.536MHz
debug 
debug ################################################################################
debug # Playback: I2S1 -> DAC1 L/R -> SPKMIXL/R -> SPKVOLL/R -> Class-D -> SPO L/R
debug ################################################################################
debug i2ctransfer -f -y $i2c_dev w3@0x1C 0x19 0xAF 0xAF #MX-19h: DAC1 Left/Right Digital Volume
debug i2ctransfer -f -y $i2c_dev w3@0x1C 0x29 0x80 0x80 #MX-29h: I2S1 to Stereo DAC Mixer
debug i2ctransfer -f -y $i2c_dev w3@0x1C 0x2A 0x14 0x14 #MX-2Ah: Stereo DAC1 Mixer
debug i2ctransfer -f -y $i2c_dev w3@0x1C 0x2C 0x22 0x00 #MX-2Ch: DACL1/DACR1 Routing
debug i2ctransfer -f -y $i2c_dev w3@0x1C 0x46 0x00 0x36 #MX-46h: DACL1 to SPK MIXL - gain
debug i2ctransfer -f -y $i2c_dev w3@0x1C 0x47 0x00 0x36 #MX-47h: DACR1 to SPK MIXR - gain
debug i2ctransfer -f -y $i2c_dev w3@0x1C 0x48 0xE8 0x00 #MX-48h: SPKVOLL to SPOLMIX - path
debug i2ctransfer -f -y $i2c_dev w3@0x1C 0x49 0x28 0x00 #MX-49h: SPKVOLR to SPORMIX - path
debug i2ctransfer -f -y $i2c_dev w3@0x1C 0x61 0x98 0x01 #MX-61h: I2S1, DACL1, DACR1 and Class-D Power; ADC remains powered off
debug i2ctransfer -f -y $i2c_dev w3@0x1C 0x65 0x30 0x00 #MX-65h: SPK MIXL/R Power; RECMIX and OUTMIX remain powered off
debug i2ctransfer -f -y $i2c_dev w3@0x1C 0x66 0xC0 0x00 #MX-66h: SPKVOLL/R Power; Headphone and other output volume blocks remain powered off
debug sleep 0.1
debug i2ctransfer -f -y $i2c_dev w3@0x1C 0x01 0x18 0x18 #MX-01h: Enable and Unmute SPOL/SPOR

debug i2ctransfer -f -y $i2c_dev w3@0x1c 0x27 0x20 0x20
debug i2ctransfer -f -y $i2c_dev w3@0x1c 0x61 0x98 0x07 #0x61：Digital Power 1, 保留 I2S1、DAC L/R、Class-D，并打开 ADC L/R
debug i2ctransfer -f -y $i2c_dev w3@0x1c 0x62 0x80 0x00 #0x62：Digital Power 2, 打开 Stereo ADC Filter

#https://developer.nvidia.com/docs/drive/drive-os/7.0.3/public/drive-os-linux-sdk/embedded-software-components/DRIVE_AGX_SoC/Audio/DebugInfo.html
#https://developer.nvidia.com/docs/drive/drive-os/7.0.3/public/drive-os-linux-sdk/platform-customization/Device_Tree/Audio_Device_Tree/i2s_properties.html
#sound {
#		clocks = <&bpmp_clks TEGRA234_CLK_PLLA>,
#				<&bpmp_clks TEGRA234_CLK_PLLA_OUT0>,
#				<&bpmp_clks TEGRA234_CLK_AUD_MCLK>;
#		clock-names = "pll_a", "pll_a_out0", "extern1";
#		assigned-clocks = <&bpmp_clks TEGRA234_CLK_AUD_MCLK>;
#		assigned-clock-parents = <&bpmp_clks TEGRA234_CLK_PLLA_OUT0>;
#		assigned-clock-rates = <12288000>;
#		/delete-property/ nvidia-audio-card,mclk-fs;
#	};

#nvidia@tegra-ubuntu:~$ sudo su
#[sudo] password for nvidia:
#root@tegra-ubuntu:/home/nvidia# cd /sys/kernel/debug/bpmp/debug/clk/
#root@tegra-ubuntu:/sys/kernel/debug/bpmp/debug/clk# cat aud_mclk/parent
#plla_out0
#root@tegra-ubuntu:/sys/kernel/debug/bpmp/debug/clk# cat plla_out0/parent
#pll_a
#root@tegra-ubuntu:/sys/kernel/debug/bpmp/debug/clk# cat pll_a/rate
#294911718
#root@tegra-ubuntu:/sys/kernel/debug/bpmp/debug/clk# cat pll_a/state
#0
#root@tegra-ubuntu:/sys/kernel/debug/bpmp/debug/clk# cat plla_out0/rate
#49151953
#root@tegra-ubuntu:/sys/kernel/debug/bpmp/debug/clk# cat plla_out0/state
#0
#root@tegra-ubuntu:/sys/kernel/debug/bpmp/debug/clk# cat aud_mclk/rate
#49151953
#root@tegra-ubuntu:/sys/kernel/debug/bpmp/debug/clk# cat aud_mclk/state
#0
#root@tegra-ubuntu:/sys/kernel/debug/bpmp/debug/clk# echo 1 > pll_a/state
#root@tegra-ubuntu:/sys/kernel/debug/bpmp/debug/clk# echo 1 > plla_out0/state
#root@tegra-ubuntu:/sys/kernel/debug/bpmp/debug/clk# echo 12288000 > aud_mclk/rate
#root@tegra-ubuntu:/sys/kernel/debug/bpmp/debug/clk# echo 1 > aud_mclk/state
#root@tegra-ubuntu:/sys/kernel/debug/bpmp/debug/clk# cat aud_mclk/rate
#12287988
#root@tegra-ubuntu:/sys/kernel/debug/bpmp/debug/clk#
