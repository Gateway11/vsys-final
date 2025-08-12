#!/bin/bash
#
# SPDX-FileCopyrightText: Copyright (c) 2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
# SPDX-License-Identifier: LicenseRef-NvidiaProprietary
#
# NVIDIA CORPORATION and its licensors retain all intellectual property
# and proprietary rights in and to this software, related documentation
# and any modifications thereto.  Any use, reproduction, disclosure or
# distribution of this software and related documentation without an express
# license agreement from NVIDIA CORPORATION is strictly prohibited.

# Supports a few configuration of 1 AD2433 main node and 2(8ch 32-bit)/4(16ch 32-bit)
# /8(32ch 16-bit) AD2428 subnodes
# Subnodes are either infinenon mic boards or class D amps having 4 mics and 4 spks
# respectively

# Function for delay in milliseconds
sleep_ms() {
    sleep $(echo "scale=3; $1/1000" | bc)
}

# Function to display usage instructions
show_usage() {
    echo "Usage:"
    echo "For normal configuration:"
    echo "  $0 -b <i2c_bus_id> -a <main_address> -n <sub_node_count> -c <channel_count> -s <sample_size> -r <sample_rate> -p <channels_per_node> -m <is_playback> [-t normal]"
    echo
    echo "For increased/reduced rate configuration:"
    echo "  $0 -b <i2c_bus_id> -a <main_address> -t <ir|rr>"
    echo
    echo "Configuration Details:"
    echo "1. Increased Rate (IR) Configuration:"
    echo "   - Master Node: TDM4, 32-bit, FSYNC = 48kHz (AD2433)"
    echo "   - Sub Node: TDM2, 32-bit, SFF * 2 = 96kHz (Increased Rate, Reduce bit Disabled, AD2433)"
    echo "   - Audio Settings:"
    echo "     * Playback: aplay -D hw:0,0 -r 48000 -c 4 -f S32_LE <file>"
    echo "     * Recording: arecord -D hw:0,0 -r 96000 -c 2 -f S32_LE <file>"
    echo
    echo "2. Reduced Rate (RR) Configuration:"
    echo "   - Master Node: TDM2, 32-bit, FSYNC = 48kHz (AD2433)"
    echo "   - Sub Node: TDM2, 32-bit, SFF/2 = 24kHz (Reduced Rate, AD2433)"
    echo "   - Audio Settings:"
    echo "     * Playback: aplay -D hw:0,0 -r 48000 -c 2 -f S32_LE <file>"
    echo "     * Recording: arecord -D hw:0,0 -r 24000 -c 2 -f S32_LE <file>"
    echo
    echo "Arguments:"
    echo "  -b <i2c_bus_id>        I2C bus ID (e.g., 1, 2, etc.)"
    echo "  -a <main_address>      I2C main address (in hexadecimal, e.g., 0x40)"
    echo "  -t <config_type>       Configuration type: 'normal' (default), 'ir' (increased rate), or 'rr' (reduced rate)"
    echo
    echo "Additional arguments required for normal configuration:"
    echo "  -n <sub_node_count>    Number of subnodes on the A2B bus"
    echo "  -c <channel_count>     Total number of audio channels"
    echo "  -s <sample_size>       Audio sample size (e.g., 16, 24, 32)"
    echo "  -r <sample_rate>       Audio sample rate (e.g., 48000)"
    echo "  -p <channels_per_node> Number of channels per subnode"
    echo "  -m <is_playback>       Mode: 1 for playback, 0 for capture"
    echo
    echo "Example for normal configuration:"
    echo "  $0 -b 16 -a 0x68 -n 8 -c 32 -s 16 -r 48000 -p 4 -m 1"
    echo
    echo "Example for increased rate configuration:"
    echo "  $0 -b 16 -a 0x6a -t ir"
    echo
    echo "Example for reduced rate configuration:"
    echo "  $0 -b 16 -a 0x6a -t rr"
    exit 1
}

# Function to read and verify node details
read_and_verify_node() {
    local i2c_bus=$1
    local address=$2
    local expected_vendor=$3
    local expected_product=$4
    local expected_version=$5
    local node_type=$6
    local node_id=$7

    # Read node information
    vendor=$(i2cget -y "$i2c_bus" "$address" 0x02)
    product=$(i2cget -y "$i2c_bus" "$address" 0x03)
    version=$(i2cget -y "$i2c_bus" "$address" 0x04)

    # Print node information
    echo
    echo "$node_type NODE $node_id:"
    echo "VENDOR ID = $vendor"
    echo "PRODUCT ID = $product"
    echo "VERSION ID = $version"

    # Verify node details
    if [ "$vendor" != "$expected_vendor" ]; then
        echo "FAIL: Unexpected $node_type Node Vendor ID ($vendor)"
        exit 1
    fi

    if [ "$product" != "$expected_product" ]; then
        echo "FAIL: Unexpected $node_type Node Product ID ($product)"
        exit 1
    fi

    if [ -n "$expected_version" ] && [ "$version" != "$expected_version" ]; then
        echo "FAIL: Unexpected $node_type Node Version ID ($version)"
        exit 1
    fi
}

# Function to initialize infineon subnode
initialize_infineon_subnode() {
    local i2c_bus=$1
    local bus_addr=$2
    local node_id=$3
    local sub_cfg=$4
    local is_last_node=$5
    local upslot_value=$6

    # Set node address
    i2cset -y "$i2c_bus" "$main_address" 0x01 "$node_id" #REG_A2B0_NODEADR

    # Common configuration for all subnodes
    i2cset -y "$i2c_bus" "$bus_addr" 0x0B 0x80 #REG_A2B0_LDNSLOTS
    i2cset -y "$i2c_bus" "$bus_addr" 0x0C 0x04 #REG_A2B0_LUPSLOTS

    # Program UPSLOTS dynamically
    echo "upslot value is ${upslot_value}"
    i2cset -y "$i2c_bus" "$bus_addr" 0x0E "$upslot_value" #REG_A2B0_UPSLOTS
    i2cset -y "$i2c_bus" "$bus_addr" 0x3F "$sub_cfg" #REG_A2B0_I2CCFG
    # Not programming REG_A2B0_I2SGCFG (0x41 and 0x42)
    i2cset -y "$i2c_bus" "$bus_addr" 0x47 0x1F #REG_A2B0_PDMCTL
    i2cset -y "$i2c_bus" "$bus_addr" 0x4A 0x10 #REG_A2B0_GPIODAT
    i2cset -y "$i2c_bus" "$bus_addr" 0x4D 0x10 #REG_A2B0_GPIOOEN
    i2cset -y "$i2c_bus" "$bus_addr" 0x52 0x00 #REG_A2B0_PINCFG
     # Additional configurations based on node position
    if [ "$is_last_node" -eq 1 ]; then
        # Special configuration for last node
        echo "Skipping REG_AD242X0_CLK2CFG for Subnode $node_id"
        #i2cset -y "$i2c_bus" "$bus_addr" 0x5A 0x81 #REG_AD242X0_CLK2CFG Original 0xC1
    else
        # Programming REG_AD242X0_CLK2CFG for other nodes
        i2cset -y "$i2c_bus" "$bus_addr" 0x5A 0x81 #REG_AD242X0_CLK2CFG Original 0xC1
    fi
    i2cset -y "$i2c_bus" "$bus_addr" 0x1B 0x77 #REG_A2B0_INTMSK0
    i2cset -y "$i2c_bus" "$bus_addr" 0x1C 0x7F #REG_A2B0_INTMSK1
    i2cset -y "$i2c_bus" "$bus_addr" 0x1E 0xEF #REG_A2B0_BECCTL
    #Main node
    if [ "$is_last_node" -eq 0 ]; then
        # Read interrupts for the main node
        i2cget -y "$i2c_bus" "$main_address" 0x16      #REG_A2B0_INTSRC
    else
        echo "Skipping reading INTSRC"
    fi
}

# Function to initialize class D Amp subnode
initialize_spk_subnode() {
    local i2c_bus=$1
    local bus_addr="$bus_address"
    local node_id=$3
    local sub_cfg=$4
    local is_last_node=$5

    # Set node address
    i2cset -y "$i2c_bus" "$main_address" 0x01 "$node_id" #REG_A2B0_NODEADR

    # Common configuration for all subnodes
    i2cset -y "$i2c_bus" "$bus_addr" 0x0B 0x80 #REG_A2B0_LDNSLOTS
    i2cset -y "$i2c_bus" "$bus_addr" 0x0D "$channel" #REG_A2B0_DNSLOTS

    i2cset -y "$i2c_bus" "$bus_addr" 0x3F "$sub_cfg" #/* I2CCFG */
    i2cset -y "$i2c_bus" "$bus_addr" 0x41 0x60 #/* I2SGCFG */
    i2cset -y "$i2c_bus" "$bus_addr" 0x42 0x0B #/* I2SCFG */

    i2cset -y "$i2c_bus" "$bus_addr" 0x4A 0x80 #/* GPIODAT */
    i2cset -y "$i2c_bus" "$bus_addr" 0x4D 0x80 #/* GPIOOEN */
    i2cset -y "$i2c_bus" "$bus_addr" 0x4E 0x06 #/* GPIOIEN */

    i2cset -y "$i2c_bus" "$bus_addr" 0x50 0x06 #/* PINTEN */
    i2cset -y "$i2c_bus" "$bus_addr" 0x51 0x06 #/* PINTINV */
    i2cset -y "$i2c_bus" "$bus_addr" 0x52 0x00 #/* PINCFG */

    i2cset -y "$i2c_bus" "$bus_addr" 0x5A 0x41 #/* CLK2CFG */
    # Node-specific I2C commands
    case "$node_id" in
        0)
            i2cset -y "$i2c_bus" "$bus_addr" 0x65 0x0F #/* DNMASK0 */
            ;;
        1)
            i2cset -y "$i2c_bus" "$bus_addr" 0x65 0xF0 #/* DNAMASK0 */
            ;;
        2)
            i2cset -y "$i2c_bus" "$bus_addr" 0x66 0x0F #/* DNMASK1 */
            ;;
        3)
            i2cset -y "$i2c_bus" "$bus_addr" 0x66 0xF0 #/* DNAMASK1 */
            ;;
        4)
            i2cset -y "$i2c_bus" "$bus_addr" 0x67 0x0F #/* DNMASK2 */
            ;;
        5)
            i2cset -y "$i2c_bus" "$bus_addr" 0x67 0xF0 #/* DNAMASK2 */
            ;;
        6)
            i2cset -y "$i2c_bus" "$bus_addr" 0x68 0x0F #/* DNMASK3 */
            ;;
        7)
            i2cset -y "$i2c_bus" "$bus_addr" 0x68 0xF0 #/* DNAMASK3 */
            ;;
        *)
            echo "Invalid node_id: $node_id"
            ;;
    esac
    i2cset -y "$i2c_bus" "$bus_addr" 0x82 0x02 #/* GPIOD1MSK */
    i2cset -y "$i2c_bus" "$bus_addr" 0x83 0x04 #/* GPIOD2MSK */
    i2cset -y "$i2c_bus" "$bus_addr" 0x82 0x06 #/* GPIODINV */
    i2cset -y "$i2c_bus" "$bus_addr" 0x83 0x06 #/* GPIODEN */
    i2cset -y "$i2c_bus" "$bus_addr" 0x96 0x00 #/* MBOX1CTL */
    i2cset -y "$i2c_bus" "$bus_addr" 0x1B 0xFF #/* INTMSK0 */
    i2cset -y "$i2c_bus" "$bus_addr" 0x1C 0x06 #/* INTMSK1 */

    i2cset -y "$i2c_bus" "$main_address" 0x01 "$node_id" #/* NODEADR */
    # /* Enable Class D Peripheral Programming on Slave Node 1 */
    i2cset -y "$i2c_bus" "$bus_addr" 0x00 0x10 #/* CHIP - Set the chip address */
    i2cset -y "$i2c_bus" "$main_address" 0x01 0x21 #/* NODEADR - Enable PERI */
    # /* Start Slave 1 Peripheral Programming */
    i2cset -y "$i2c_bus" "$bus_addr" 0x04 0x80 #/* IC 1.POWER_CTRL Register.POWER_CTRL */
    i2cset -y "$i2c_bus" "$bus_addr" 0x05 0x8A #/* IC 1.AMP_DAC_CTRL Register.AMP_DAC_CTRL */
    i2cset -y "$i2c_bus" "$bus_addr" 0x06 0x02 #/* IC 1.DAC_CTRL Register.DAC_CTRL */
    i2cset -y "$i2c_bus" "$bus_addr" 0x07 0x40 #/* IC 1.VOL_LEFT_CTRL Register.VOL_LEFT_CTRL */
    i2cset -y "$i2c_bus" "$bus_addr" 0x08 0x40 #/* IC 1.VOL_RIGHT_CTRL Register.VOL_RIGHT_CTRL */
    i2cset -y "$i2c_bus" "$bus_addr" 0x09 0x54 #/* IC 1.SAI_CTRL1 Register.SAI_CTRL1 */
    i2cset -y "$i2c_bus" "$bus_addr" 0x0A 0x07 #/* IC 1.SAI_CTRL2 Register.SAI_CTRL2 */
    i2cset -y "$i2c_bus" "$bus_addr" 0x0B 0x00 #/* IC 1.SLOT_LEFT_CTRL Register.SLOT_LEFT_CTRL */
    i2cset -y "$i2c_bus" "$bus_addr" 0x0C 0x01 #/* SLOT_RIGHT_CTRL Register.SLOT_RIGHT_CTRL" */
    i2cset -y "$i2c_bus" "$bus_addr" 0x0E 0xA0 #/* IC 1.LIM_LEFT_CTRL1 Register.LIM_LEFT_CTRL1 */
    i2cset -y "$i2c_bus" "$bus_addr" 0x0F 0x51 #/* IC 1.LIM_LEFT_CTRL2 Register.LIM_LEFT_CTRL2 */
    i2cset -y "$i2c_bus" "$bus_addr" 0x10 0x22 #/* IC 1.LIM_LEFT_CTRL3 Register.LIM_LEFT_CTRL3 */
    i2cset -y "$i2c_bus" "$bus_addr" 0x11 0xA8 #/* IC 1.LIM_RIGHT_CTRL1 Register.LIM_RIGHT_CTRL1 */
    i2cset -y "$i2c_bus" "$bus_addr" 0x12 0x51 #/* IC 1.LIM_RIGHT_CTRL2 Register.LIM_RIGHT_CTRL2 */
    i2cset -y "$i2c_bus" "$bus_addr" 0x13 0x22 #/* IC 1.LIM_RIGHT_CTRL3 Register.LIM_RIGHT_CTRL3 */
    i2cset -y "$i2c_bus" "$bus_addr" 0x14 0xFF #/* IC 1.CLIP_LEFT_CTRL Register.CLIP_LEFT_CTRL */
    i2cset -y "$i2c_bus" "$bus_addr" 0x15 0xFF #/* IC 1.CLIP_RIGHT_CTRL Register.CLIP_RIGHT_CTRL */
    i2cset -y "$i2c_bus" "$bus_addr" 0x16 0x00 #/* IC 1.FAULT_CTRL1 Register.FAULT_CTRL1 */
    i2cset -y "$i2c_bus" "$bus_addr" 0x17 0x30 #/* IC 1.FAULT_CTRL2 Register.FAULT_CTRL2 */
    i2cset -y "$i2c_bus" "$bus_addr" 0x1C 0x00 #/* IC 1.SOFT_RESET Register.SOFT_RESET */

    i2cset -y "$i2c_bus" "$main_address" 0x01 "$node_id" #/* NODEADR */
    # /* Enable Class D Peripheral Programming on Slave Node 1 */
    i2cset -y "$i2c_bus" "$bus_addr" 0x00 0x11 #/* CHIP - Set the chip address */
    i2cset -y "$i2c_bus" "$main_address" 0x01 0x21 #/* NODEADR - Enable PERI */
    # /* Start Slave 1 Peripheral Programming */
    i2cset -y "$i2c_bus" "$bus_addr" 0x04 0x80 #/* IC 1.POWER_CTRL Register.POWER_CTRL */
    i2cset -y "$i2c_bus" "$bus_addr" 0x05 0x8A #/* IC 1.AMP_DAC_CTRL Register.AMP_DAC_CTRL */
    i2cset -y "$i2c_bus" "$bus_addr" 0x06 0x02 #/* IC 1.DAC_CTRL Register.DAC_CTRL */
    i2cset -y "$i2c_bus" "$bus_addr" 0x07 0x40 #/* IC 1.VOL_LEFT_CTRL Register.VOL_LEFT_CTRL */
    i2cset -y "$i2c_bus" "$bus_addr" 0x08 0x40 #/* IC 1.VOL_RIGHT_CTRL Register.VOL_RIGHT_CTRL */
    i2cset -y "$i2c_bus" "$bus_addr" 0x09 0x54 #/* IC 1.SAI_CTRL1 Register.SAI_CTRL1 */
    i2cset -y "$i2c_bus" "$bus_addr" 0x0A 0x07 #/* IC 1.SAI_CTRL2 Register.SAI_CTRL2 */
    i2cset -y "$i2c_bus" "$bus_addr" 0x0B 0x00 #/* IC 1.SLOT_LEFT_CTRL Register.SLOT_LEFT_CTRL */
    i2cset -y "$i2c_bus" "$bus_addr" 0x0C 0x01 #/* SLOT_RIGHT_CTRL Register.SLOT_RIGHT_CTRL" */
    i2cset -y "$i2c_bus" "$bus_addr" 0x0E 0xA0 #/* IC 1.LIM_LEFT_CTRL1 Register.LIM_LEFT_CTRL1 */
    i2cset -y "$i2c_bus" "$bus_addr" 0x0F 0x51 #/* IC 1.LIM_LEFT_CTRL2 Register.LIM_LEFT_CTRL2 */
    i2cset -y "$i2c_bus" "$bus_addr" 0x10 0x22 #/* IC 1.LIM_LEFT_CTRL3 Register.LIM_LEFT_CTRL3 */
    i2cset -y "$i2c_bus" "$bus_addr" 0x11 0xA8 #/* IC 1.LIM_RIGHT_CTRL1 Register.LIM_RIGHT_CTRL1 */
    i2cset -y "$i2c_bus" "$bus_addr" 0x12 0x51 #/* IC 1.LIM_RIGHT_CTRL2 Register.LIM_RIGHT_CTRL2 */
    i2cset -y "$i2c_bus" "$bus_addr" 0x13 0x22 #/* IC 1.LIM_RIGHT_CTRL3 Register.LIM_RIGHT_CTRL3 */
    i2cset -y "$i2c_bus" "$bus_addr" 0x14 0xFF #/* IC 1.CLIP_LEFT_CTRL Register.CLIP_LEFT_CTRL */
    i2cset -y "$i2c_bus" "$bus_addr" 0x15 0xFF #/* IC 1.CLIP_RIGHT_CTRL Register.CLIP_RIGHT_CTRL */
    i2cset -y "$i2c_bus" "$bus_addr" 0x16 0x00 #/* IC 1.FAULT_CTRL1 Register.FAULT_CTRL1 */
    i2cset -y "$i2c_bus" "$bus_addr" 0x17 0x30 #/* IC 1.FAULT_CTRL2 Register.FAULT_CTRL2 */
    i2cset -y "$i2c_bus" "$bus_addr" 0x1C 0x00 #/* IC 1.SOFT_RESET Register.SOFT_RESET */

}

# Function to initialize increased rate configuration
initialize_ir_config() {
    local i2c_bus=$1
    local master_addr=$2
    local bus_addr=$3

    echo "Initializing A2B Increased Rate Configuration..."
    echo "I2C Bus: $i2c_bus"
    echo "Master Address: $master_addr"
    echo "Bus Address: $bus_addr"

    # Master Node Initial Configuration
    echo "Configuring Master Node..."

    # Reset control register and read vendor ID
    i2cset -y $i2c_bus $master_addr 0x12 0x04  # Control register reset
    i2cget -y $i2c_bus $master_addr 0x02 b      # Read vendor ID
    i2cget -y $i2c_bus $master_addr 0x5e b      # Read BSDSTAT register

    # Initialize control registers
    i2cset -y $i2c_bus $master_addr 0x12 0x04   # Control register config
    sleep_ms 19
    i2cget -y $i2c_bus $master_addr 0x17 b      # Read INTTYPE register
    i2cset -y $i2c_bus $master_addr 0x12 0x80   # Enable master mode
    sleep_ms 19

    # Interrupt configuration
    i2cget -y $i2c_bus $master_addr 0x17 b      # Check interrupt status
    i2cget -y $i2c_bus $master_addr 0x02 b      # Verify vendor ID
    i2cset -y $i2c_bus $master_addr 0x1b 0x10 0x00 0x09 i  # Configure interrupt mask
    i2cset -y $i2c_bus $master_addr 0x1a 0x01   # Set INTPND2
    i2cset -y $i2c_bus $master_addr 0x0f 0x8F   # Set response cycles to 0x8F

    # Enable master node
    i2cset -y $i2c_bus $master_addr 0x12 0x81   # Enable master node
    sleep_ms 1
    i2cget -y $i2c_bus $master_addr 0x17 b      # Check interrupt status
    i2cget -y $i2c_bus $master_addr 0xa5 b      # Read SWSTAT2

    # Configure I2S and discovery
    i2cset -y $i2c_bus $master_addr 0x41 0x21   # I2S global config
    i2cset -y $i2c_bus $master_addr 0x09 0x01   # Switch control
    i2cset -y $i2c_bus $master_addr 0x13 0x8F   # Set discovery register to 0x8F
    sleep_ms 32

    echo "Configuring Slave Node..."

    # Slave node configuration
    i2cget -y $i2c_bus $master_addr 0x17 b      # Check interrupt status
    i2cset -y $i2c_bus $master_addr 0x09 0x21   # Switch control update
    i2cset -y $i2c_bus $master_addr 0x01 0x00   # Set node address

    # Read slave vendor ID and configure slots
    i2cget -y $i2c_bus $bus_addr 0x02 b       # Read slave vendor ID
    i2cset -y $i2c_bus $bus_addr 0x0b 0x80    # Configure downstream slots
    i2cset -y $i2c_bus $bus_addr 0x42 0x11    # I2S configuration
    i2cset -y $i2c_bus $bus_addr 0x43 0x05    # I2S rate setting
    i2cset -y $i2c_bus $bus_addr 0x65 0x0F    # Downstream mask
    i2cset -y $i2c_bus $bus_addr 0xb2 0x03    # SPI clock divider
    i2cset -y $i2c_bus $bus_addr 0x1b 0x00 0x00 i  # Interrupt mask

    echo "Finalizing configuration..."

    # Final master node configuration
    i2cset -y $i2c_bus $master_addr 0x09 0x01   # Switch control
    i2cset -y $i2c_bus $master_addr 0x0d 0x04   # Downstream slots
    i2cset -y $i2c_bus $master_addr 0x41 0x21   # I2S global config
    i2cset -y $i2c_bus $master_addr 0x42 0x11   # I2S configuration
    i2cset -y $i2c_bus $master_addr 0xb2 0x03   # SPI clock divider
    i2cset -y $i2c_bus $master_addr 0x1b 0x10 0x00 0x09 i  # Interrupt mask
    i2cset -y $i2c_bus $master_addr 0x01 0x00   # Node address
    i2cset -y $i2c_bus $master_addr 0x01 0x80   # Set node address

    # Configure PLL and slot format
    i2cset -y $i2c_bus $bus_addr 0x40 0x00    # PLL control
    i2cset -y $i2c_bus $master_addr 0x01 0x00   # Reset node address
    i2cset -y $i2c_bus $master_addr 0x10 0x46   # Slot format changed to 0x26
    i2cset -y $i2c_bus $master_addr 0x11 0x01   # Data control
    i2cset -y $i2c_bus $master_addr 0x12 0x81   # Control register
    sleep_ms 1

    # Final checks and settings
    i2cget -y $i2c_bus $master_addr 0x17 b      # Check interrupt status
    i2cset -y $i2c_bus $master_addr 0x01 0x00   # Reset node address
    i2cset -y $i2c_bus $bus_addr 0x09 0x00    # Switch control
    i2cset -y $i2c_bus $master_addr 0x12 0x82   # Final control setting

    echo "A2B Increased Rate Configuration completed"
}

# Function to initialize reduced rate configuration
initialize_rr_config() {
    local i2c_bus=$1
    local master_addr=$2
    local bus_addr=$3

    echo "Starting A2B Reduced Rate Configuration..."
    echo "I2C Bus: $i2c_bus"
    echo "Master Node Address: 0x$(printf '%02X' $master_addr)"
    echo "Bus Address: 0x$(printf '%02X' $bus_addr)"

    # Following exact XML sequence
    i2cset -y $i2c_bus $master_addr 0x12 0x04 # REG_A2B0_CONTROL
    i2cget -y $i2c_bus $master_addr 0x02 b    # REG_A2B0_VENDOR
    i2cget -y $i2c_bus $master_addr 0x5E b    # REG_AD243X0_BSDSTAT
    i2cset -y $i2c_bus $master_addr 0x12 0x04 # REG_A2B0_CONTROL

    sleep_ms 19

    i2cget -y $i2c_bus $master_addr 0x17 b    # REG_A2B0_INTTYPE
    i2cset -y $i2c_bus $master_addr 0x12 0x80 # REG_A2B0_CONTROL

    sleep_ms 19

    i2cget -y $i2c_bus $master_addr 0x17 b    # REG_A2B0_INTTYPE
    i2cget -y $i2c_bus $master_addr 0x02 b    # REG_A2B0_VENDOR
    i2cset -y $i2c_bus $master_addr 0x1B 0x10 # REG_A2B0_INTMSK0
    i2cset -y $i2c_bus $master_addr 0x1A 0x01 # REG_A2B0_INTPND2
    i2cset -y $i2c_bus $master_addr 0x0F 0x83 # REG_A2B0_RESPCYCS
    i2cset -y $i2c_bus $master_addr 0x12 0x81 # REG_A2B0_CONTROL

    sleep_ms 1

    i2cget -y $i2c_bus $master_addr 0x17 b    # REG_A2B0_INTTYPE
    i2cget -y $i2c_bus $master_addr 0xA5 b    # REG_AD243X0_SWSTAT2
    i2cset -y $i2c_bus $master_addr 0x41 0x20 # REG_A2B0_I2SGCFG
    i2cset -y $i2c_bus $master_addr 0x09 0x01 # REG_A2B0_SWCTL
    i2cset -y $i2c_bus $master_addr 0x13 0x83 # REG_A2B0_DISCVRY

    sleep_ms 32

    i2cget -y $i2c_bus $master_addr 0x17 b    # REG_A2B0_INTTYPE
    i2cset -y $i2c_bus $master_addr 0x09 0x21 # REG_A2B0_SWCTL
    i2cset -y $i2c_bus $master_addr 0x01 0x00 # REG_A2B0_NODEADR

    i2cget -y $i2c_bus $bus_addr 0x02 b    # REG_A2B0_VENDOR
    i2cset -y $i2c_bus $bus_addr 0x0B 0x80 # REG_A2B0_LDNSLOTS
    i2cset -y $i2c_bus $bus_addr 0x42 0x11 # REG_A2B0_I2SCFG
    i2cset -y $i2c_bus $bus_addr 0x43 0x81 # REG_A2B0_I2SRATE
    i2cset -y $i2c_bus $bus_addr 0x65 0x01 # REG_AD242X0_DNMASK0
    i2cset -y $i2c_bus $bus_addr 0xB2 0x03 # REG_AD243X0_SPICKDIV
    i2cset -y $i2c_bus $bus_addr 0x1B 0x00 # REG_A2B0_INTMSK0

    i2cset -y $i2c_bus $master_addr 0x09 0x01 # REG_A2B0_SWCTL
    i2cset -y $i2c_bus $master_addr 0x0D 0x01 # REG_A2B0_DNSLOTS
    i2cset -y $i2c_bus $master_addr 0x41 0x20 # REG_A2B0_I2SGCFG
    i2cset -y $i2c_bus $master_addr 0x42 0x11 # REG_A2B0_I2SCFG
    i2cset -y $i2c_bus $master_addr 0xB2 0x03 # REG_AD243X0_SPICKDIV
    i2cset -y $i2c_bus $master_addr 0x1B 0x10 # REG_A2B0_INTMSK0
    i2cset -y $i2c_bus $master_addr 0x01 0x00 # REG_A2B0_NODEADR
    i2cset -y $i2c_bus $master_addr 0x01 0x80 # REG_A2B0_NODEADR

    i2cset -y $i2c_bus $bus_addr 0x40 0x00 # REG_A2B0_PLLCTL

    i2cset -y $i2c_bus $master_addr 0x01 0x00 # REG_A2B0_NODEADR
    i2cset -y $i2c_bus $master_addr 0x10 0x26 # REG_A2B0_SLOTFMT
    i2cset -y $i2c_bus $master_addr 0x11 0x01 # REG_A2B0_DATCTL
    i2cset -y $i2c_bus $master_addr 0x12 0x81 # REG_A2B0_CONTROL

    sleep_ms 1

    i2cget -y $i2c_bus $master_addr 0x17 b    # REG_A2B0_INTTYPE
    i2cset -y $i2c_bus $master_addr 0x01 0x00 # REG_A2B0_NODEADR
    i2cset -y $i2c_bus $bus_addr 0x09 0x00 # REG_A2B0_SWCTL
    i2cset -y $i2c_bus $master_addr 0x12 0x82 # REG_A2B0_CONTROL

    echo "A2B Reduced Rate Configuration completed"
    echo "Master Node (0x$(printf '%02X' $master_addr)): TDM2, 32-bit, FSYNC = 48kHz"
    echo "Sub Node (0x$(printf '%02X' $bus_addr)): TDM2, 32-bit, SFF/2 = 24kHz"
}

# Check if arguments are insufficient or in invalid range
if [ $# -lt 6 ] || ([ $# -gt 6 ] && [ $# -lt 16 ]); then
    echo "Error: Insufficient arguments"
    show_usage
fi

# Parse the command-line arguments
while getopts "b:a:n:c:s:r:p:m:t:" opt; do
    case $opt in
        b) i2c_bus_id=$OPTARG ;;
        a) main_address=$OPTARG ;;
        n) sub_node_count=$OPTARG ;;
        c) channel=$OPTARG ;;
        s) sample_size=$OPTARG ;;
        r) sample_rate=$OPTARG ;;
        p) channels_per_node=$OPTARG ;;
        m) is_playback=$OPTARG ;;
        t) config_type=$OPTARG ;;
        *) show_usage ;;
    esac
done

# Validate required arguments
if [ "$config_type" = "ir" ] || [ "$config_type" = "rr" ]; then
    # For IR and RR configurations, only bus_id and main_address are required
    if [ -z "$i2c_bus_id" ] || [ -z "$main_address" ]; then
        echo "Error: Missing required arguments for ${config_type} configuration."
        echo "Required arguments for ${config_type}: -b <i2c_bus_id> -a <main_address> -t ${config_type}"
        exit 1
    fi
    # Set dummy values for IR/RR configurations for the sake of completeness
    sub_node_count=1 #dummy value
    channel=4 #dummy value
    sample_size=32 #dummy value
    sample_rate=48000 #dummy value
    channels_per_node=2 #dummy value
    is_playback=1 #dummy value ''
else
    # For normal configuration, all arguments are required
    if [ -z "$i2c_bus_id" ] || [ -z "$main_address" ] || [ -z "$sub_node_count" ] || \
       [ -z "$channel" ] || [ -z "$sample_size" ] || [ -z "$sample_rate" ] || \
       [ -z "$channels_per_node" ] || [ -z "$is_playback" ]; then
        echo "Error: Missing required arguments for normal configuration."
        show_usage
    fi
fi

# Set default configuration type if not specified
if [ -z "$config_type" ]; then
    config_type="normal"
fi

# Calculate the bus address
bus_address=$((main_address | 0x01))

# Output the parsed values for verification
echo "Parsed Arguments:"
echo "  I2C Bus ID: $i2c_bus_id"
echo "  Main Address: $main_address"
echo "  Bus Address: $bus_address"
echo "  Sub Node Count: $sub_node_count"
echo "  Channel Count: $channel"
echo "  Sample Size: $sample_size"
echo "  Sample Rate: $sample_rate"
echo "  Channels Per Node: $channels_per_node"
echo "  Playback Mode: $is_playback"
echo "  Configuration Type: $config_type"

# Choose configuration based on type
case $config_type in
    "ir")
        initialize_ir_config "$i2c_bus_id" "$main_address" "$bus_address"
        exit 0
        ;;
    "rr")
        initialize_rr_config "$i2c_bus_id" "$main_address" "$bus_address"
        exit 0
        ;;
    "normal")
        # Proceed with the existing normal configuration
        ;;
    *)
        echo "Invalid configuration type. Must be 'normal', 'ir', or 'rr'"
        exit 1
        ;;
esac

# Proceed with the rest of the script logic
tdm_cfg=""

case $sub_node_count in
    2)
        main_resp_cycs=0x71
        subnode_discvry_cycs=("0x71" "0x6D")
        echo "Main response cycles: ${main_resp_cycs}"
        echo "Subnode discovery cycles: ${subnode_discvry_cycs[@]}"
        ;;
    4)
        if [ "$is_playback" -eq 1 ]; then
            echo "Playback mode enabled"
            main_resp_cycs=0xA7
            subnode_discvry_cycs=("0xA7" "0xA3" "0x9F" "0x9B")
            echo "Main response cycles: ${main_resp_cycs}"
            echo "Subnode discovery cycles: ${subnode_discvry_cycs[@]}"
        else
            echo "Capture mode enabled"
            main_resp_cycs=0x4C
            subnode_discvry_cycs=("0x4C" "0x48" "0x44" "0x40")
            echo "Main response cycles: ${main_resp_cycs}"
            echo "Subnode discovery cycles: ${subnode_discvry_cycs[@]}"
        fi
        ;;
    8)
       if [ "$is_playback" -eq 1 ]; then
            echo "Playback mode enabled"
            # Main response cycles
            main_resp_cycs=0xD1
            # Subnode discovery cycles
            subnode_discvry_cycs=("0xD1" "0xCD" "0xC9" "0xC5" "0xC1" "0xBD" "0xB9" "0xB5")
            echo "Main response cycles: ${main_resp_cycs}"
            echo "Subnode discovery cycles: ${subnode_discvry_cycs[@]}"
        else
            echo "Capture mode enabled"
            # Main response cycles
            main_resp_cycs=0x57
            # Subnode discovery cycles
            subnode_discvry_cycs=("0x57" "0x53" "0x4F" "0x4B" "0x47" "0x43" "0x3F" "0x3B")
            echo "Main response cycles: ${main_resp_cycs}"
            echo "Subnode discovery cycles: ${subnode_discvry_cycs[@]}"
        fi
        ;;
    *)
        echo "Invalid number of sub nodes."
        exit 1
        ;;
esac

# Populate tdm_cfg based on channel value
case $channel in
    4)
        tdm_cfg=0x1
        main_node_upslots=0x04
        ;;
    8)
        tdm_cfg=0x2
        main_node_upslots=0x08
        ;;
    16)
        tdm_cfg=0x4
        main_node_upslots=0x10
        ;;
    32)
        tdm_cfg=0x7
        main_node_upslots=0x20
        ;;
    *)
        echo "Invalid channel value. Supported values are 4, 8, 16, or 32."
        exit 1
        ;;
esac

# Populate main_node_sltfmt based on sample_size value
case $sample_size in
    16)
        tdm_cfg=$((tdm_cfg | 0x10))
        main_node_slotfmt=0x22
        ;;
    32)
        main_node_slotfmt=0x66
        ;;
    *)
        echo "Invalid sample_size. Supported values are 16, or 32."
        exit 1
        ;;
esac

# Populate sub node frame rate based on sample rate  value
case $sample_rate in
    44100)
        sub_cfg=0x05
        ;;
    48000)
        sub_cfg=0x01
        ;;
    *)
        echo "Invalid sample_size. Supported values are 16, or 32."
        exit 1
        ;;
esac

echo "Main node bringup"

# Bringup main node
i2cset -y "$i2c_bus_id" "$main_address" 0x12 0x84 #REG_A2B0_CONTROL
i2cset -y "$i2c_bus_id" "$main_address" 0x16 0x80 #REG_A2B0_INTSRC
i2cget -y "$i2c_bus_id" "$main_address" 0x17      #REG_A2B0_INTTYPE
i2cget -y "$i2c_bus_id" "$main_address" 0x16      #REG_A2B0_INTSRC
i2cget -y "$i2c_bus_id" "$main_address" 0x02      #REG_A2B0_VEDNOR
i2cget -y "$i2c_bus_id" "$main_address" 0x03      #REG_A2B0_PRODUCT
i2cget -y "$i2c_bus_id" "$main_address" 0x04      #REG_A2B0_VERSION
i2cset -y "$i2c_bus_id" "$main_address" 0x1B 0x77 #REG_A2B0_INTMSK0
i2cset -y "$i2c_bus_id" "$main_address" 0x1D 0x78 #REG_A2B0_INTMSK1
i2cset -y "$i2c_bus_id" "$main_address" 0x1D 0x0F #REG_A2B0_INTMSK2
i2cset -y "$i2c_bus_id" "$main_address" 0x1E 0xEF #REG_A2B0_BECCTL
i2cset -y "$i2c_bus_id" "$main_address" 0x1A 0x01 #REG_A2B0_INTPND2
i2cset -y "$i2c_bus_id" "$main_address" 0x0F "$main_resp_cycs" #REG_A2B0_RESPCYCS
i2cset -y "$i2c_bus_id" "$main_address" 0x12 0x81 #REG_A2B0_CONTROL
i2cset -y "$i2c_bus_id" "$main_address" 0x41 "$tdm_cfg" #REG_A2B0_I2SGCFG
i2cset -y "$i2c_bus_id" "$main_address" 0x09 0x01 #REG_A2B0_SWCTL

echo "Sub node discovery"

# Iterate over subnodes
for ((subnode=0; subnode<sub_node_count; subnode++)); do
    discvry_cycs=${subnode_discvry_cycs[$subnode]}

    echo "discvry_cycs is ${discvry_cycs}"
    # Subnode Discovery
    i2cset -y "$i2c_bus_id" "$main_address" 0x13 "$discvry_cycs" # REG_A2B0_DISCVRY
    sleep 1
    i2cget -y "$i2c_bus_id" "$main_address" 0x16                 # REG_A2B0_INTSRC
    i2cget -y "$i2c_bus_id" "$main_address" 0x17                 # REG_A2B0_INTTYPE

    # Set node address for current subnode
    if [ "$subnode" -eq 0 ]; then
        echo "sub node 0"
        i2cset -y "$i2c_bus_id" "$main_address" 0x09 0x21 # REG_A2B0_SWCTL
    else
        prev_subnode=$((subnode - 1))
        echo "sub node next"
        i2cset -y "$i2c_bus_id" "$main_address" 0x01 "$prev_subnode" # REG_A2B0_NODEADR
        i2cset -y "$i2c_bus_id" "$bus_address" 0x09 0x01           # REG_A2B0_SWCTL Note that this is 0x21 for infi
    fi
    i2cset -y "$i2c_bus_id"  "$main_address" 0x01 "$subnode"         # REG_A2B0_NODEADR

    # Subnode Configuration
    i2cget -y "$i2c_bus_id" "$bus_address" 0x02      # REG_A2B0_VENDOR
    i2cget -y "$i2c_bus_id" "$bus_address" 0x03      # REG_A2B0_PRODUCT
    i2cget -y "$i2c_bus_id" "$bus_address" 0x04      # REG_A2B0_VERSION
    i2cget -y "$i2c_bus_id" "$bus_address" 0x05      # REG_A2B0_CAPABILITY
    if [ "$subnode" -ne "$((sub_node_count - 1))" ]; then
        i2cset -y "$i2c_bus_id" "$bus_address" 0x09 0x01 # REG_A2B0_SWCTL
        i2cget -y "$i2c_bus_id" "$bus_address" 0x1B      # REG_A2B0_INTMSK0
        i2cget -y "$i2c_bus_id" "$bus_address" 0x1C      # REG_A2B0_INTMSK1
        i2cset -y "$i2c_bus_id" "$bus_address" 0x1B 0x10 # REG_A2B0_INTMSK0
        i2cset -y "$i2c_bus_id" "$bus_address" 0x1C 0x00 # REG_A2B0_INTMSK1
    fi
done

echo "Sub node discovery Complete"

# Verify main node
read_and_verify_node "$i2c_bus_id" "$main_address" "0xad" "0x33" "" "MAIN" 0

# Verify subnodes
expected_vendor="0xad"
expected_product="0x28"
expected_version=""

for ((subnode=1; subnode<=sub_node_count; subnode++)); do
    # Set node address for the current subnode
    i2cset -y "$i2c_bus_id" "$main_address" 0x01 "$((subnode - 1))"
    read_and_verify_node "$i2c_bus_id" "$bus_address" "$expected_vendor" "$expected_product" "$expected_version" "SUB" "$subnode"
done

echo "Initialize Sub nodes"

# Initialize subnodes in reverse order

# Calculate last node ID
last_node_id=$((sub_node_count - 1))

if [ "$is_playback" -eq 1 ]; then
    echo "Playback mode enabled"

    for ((subnode=last_node_id; subnode>=0; subnode--)); do
        echo "Initializing node ${subnode}"

        # Determine if the node is the last one
        if [ "$subnode" -eq "$last_node_id" ]; then
            is_last_node=1
        else
            is_last_node=0
        fi

        # Call the initialization function
        initialize_spk_subnode "$i2c_bus_id" "$bus_address" "$subnode" "$sub_cfg" "$is_last_node"
    done
else
    echo "Capture mode enabled"

    for ((subnode=last_node_id; subnode>=0; subnode--)); do
        echo "Initializing node ${subnode}"

        # Determine if the node is the last one
        if [ "$subnode" -eq "$last_node_id" ]; then
            is_last_node=1
        else
            is_last_node=0
        fi

        # Calculate UPSLOTS value dynamically
        upslot_value=$(((last_node_id - subnode) * channels_per_node))

        echo "Upslot value for ${subnode} is ${upslot_value}"

        # Call the initialization function
        initialize_infineon_subnode "$i2c_bus_id" "$bus_address" "$subnode" "$sub_cfg" "$is_last_node" "$upslot_value"
    done
fi


if [ "$is_playback" -eq 0 ]; then

    i2cset -y "$i2c_bus_id" "$bus_address" 0x09 0x01 #REG_A2B0_SWCTL

    echo "Initialize main node"
    i2cget -y "$i2c_bus_id" "$main_address" 0x16      #REG_A2B0_INTSRC
    i2cset -y "$i2c_bus_id" "$main_address" 0x09 0x01 #REG_A2B0_SWCTL
    i2cset -y "$i2c_bus_id" "$main_address" 0x0E "$main_node_upslots" #REG_A2B0_UPSLOTS
    i2cset -y "$i2c_bus_id" "$main_address" 0x41 "$tdm_cfg" #REG_A2B0_I2SGCFG (different from EVK 0x24)
    i2cset -y "$i2c_bus_id" "$main_address" 0x42 0x11 #REG_A2B0_I2SCFG (different from EVK 0x24)
    i2cset -y "$i2c_bus_id" "$main_address" 0x52 0x00 #REG_A2B0_PINCFG
    i2cset -y "$i2c_bus_id" "$main_address" 0x1B 0x77 #REG_A2B0_INTMSK0
    i2cset -y "$i2c_bus_id" "$main_address" 0x1C 0x00 #REG_A2B0_INTMSK1 (different from EVK 0x78)
    i2cset -y "$i2c_bus_id" "$main_address" 0x1D 0x0F #REG_A2B0_INTMSK2
    i2cset -y "$i2c_bus_id" "$main_address" 0x1E 0xEF #REG_A2B0_BECCTL
    i2cset -y "$i2c_bus_id" "$main_address" 0x01 0x00 #REG_A2B0_NODEADR
    i2cset -y "$i2c_bus_id" "$main_address" 0x01 0x00 #REG_A2B0_NODEADR
    i2cset -y "$i2c_bus_id" "$main_address" 0x40 0x00 #REG_A2B0_PLLCTL
    i2cset -y "$i2c_bus_id" "$main_address" 0x01 0x00 #REG_A2B0_NODEADR
    i2cset -y "$i2c_bus_id" "$main_address" 0x10 "$main_node_slotfmt" #REG_A2B0_SLOTFMT
    i2cset -y "$i2c_bus_id" "$main_address" 0x11 0x03 #REG_A2B0_DATCTL
    i2cset -y "$i2c_bus_id" "$main_address" 0x12 0x81 #REG_A2B0_CONTROL
    i2cset -y "$i2c_bus_id" "$main_address" 0x01 "$((sub_node_count - 1))" #REG_A2B0_NODEADR
    # Last in line node
    i2cset -y "$i2c_bus_id" "$bus_address" 0x09 0x00 #REG_A2B0_SWCTL
    # Main node
    i2cset -y "$i2c_bus_id" "$main_address" 0x12 0x82 #REG_A2B0_CONTROL
    i2cget -y "$i2c_bus_id" "$main_address" 0x02      #REG_A2B0_VEDNOR
    i2cget -y "$i2c_bus_id" "$main_address" 0x03      #REG_A2B0_PRODUCT
    i2cget -y "$i2c_bus_id" "$main_address" 0x04      #REG_A2B0_VERSION
else
    # Final Master node programming
    i2cset -y "$i2c_bus_id" "$main_address" 0x42 0x91 #/* I2SCFG */
    i2cset -y "$i2c_bus_id" "$main_address" 0x52 0x00 #/* PINCFG */
    i2cset -y "$i2c_bus_id" "$main_address" 0x01 0x00
    i2cset -y "$i2c_bus_id" "$bus_address" 0x0D "$channel"
    i2cset -y "$i2c_bus_id" "$main_address" 0x0D "$channel" #/* DNSLOTS Master */
    i2cset -y "$i2c_bus_id" "$main_address" 0x01 0x00 #/* NODEADR */
    i2cset -y "$i2c_bus_id" "$main_address" 0x09 0x01 #/* SWCTL Master */
    i2cset -y "$i2c_bus_id" "$main_address" 0x01 0x80 #/* NODEADR */
    i2cset -y "$i2c_bus_id" "$main_address" 0x01 0x00 #/* NODEADR */

    if [ "$sample_size" -eq 16 ]; then
        i2cset -y "$i2c_bus_id" "$main_address" 0x10 0x22 #/* SLOTFMT */
    else
        i2cset -y "$i2c_bus_id" "$main_address" 0x10 0x44 #/* SLOTFMT */
    fi
    i2cset -y "$i2c_bus_id" "$main_address" 0x11 0x03 #/* DATCTL */

    i2cset -y "$i2c_bus_id" "$main_address" 0x12 0x81 #/* CONTROL */
    sleep 1
    i2cset -y "$i2c_bus_id" "$main_address" 0x01 "$((sub_node_count - 1))" #/* NODEADR */
    i2cset -y "$i2c_bus_id" "$main_address" 0x12 0x82 #/* CONTROL */
fi

echo "A2B Bus Setup Complete"

exit 0

