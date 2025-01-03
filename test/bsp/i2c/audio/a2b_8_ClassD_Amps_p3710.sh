#!/bin/bash
#
# SPDX-FileCopyrightText: Copyright (c) 2022-2024 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
# SPDX-License-Identifier: LicenseRef-NvidiaProprietary
#
# NVIDIA CORPORATION and its licensors retain all intellectual property
# and proprietary rights in and to this software, related documentation
# and any modifications thereto.  Any use, reproduction, disclosure or
# distribution of this software and related documentation without an express
# license agreement from NVIDIA CORPORATION is strictly prohibited.

# Initializes masterNode(AD2428)/slaveNode0(AD2422)/slaveNode1(AD2422)
# usage: <scriptname> <i2c interface ID> <A2B Main node chip address>
# E.g., a2b_8_ClassD_Amps_p3710.sh 16 0x6a

echo "Script name is: $0"
echo "I2C interface ID $1"
echo "Device address $2"

if [ $# -lt 2 ]
then
    echo "Two arguments expected!! Please pass i2c interface ID as arg#1, device address as arg#2!"
    exit 1
fi

node_addr=$2
subnode_addr=$(( node_addr | 1 ))

i2cwrite () {
    if ! i2cset -y $1 $2 $3 $4; then
        echo FAILED: i2cset -y $1 $2 $3 $4
    fi
}

# A2B Master
i2cset -y $1 $2 0x12 0x84   #REG_A2B0_CONTROL
i2cset -y $1 $2 0x16 0x80   #REG_A2B0_INTSRC
i2cget -y $1 $2 0x17        #REG_A2B0_INTTYPE
i2cget -y $1 $2 0x16        #REG_A2B0_INTSRC
i2cget -y $1 $2 0x02        #REG_A2B0_VEDNOR
i2cget -y $1 $2 0x03        #REG_A2B0_PRODUCT
i2cget -y $1 $2 0x04        #REG_A2B0_VERSION
i2cset -y $1 $2 0x1B 0x77   #REG_A2B0_INTMSK0
i2cset -y $1 $2 0x1D 0x78   #REG_A2B0_INTMSK1
i2cset -y $1 $2 0x1D 0x0F   #REG_A2B0_INTMSK2
i2cset -y $1 $2 0x1E 0xEF   #REG_A2B0_BECCTL
i2cset -y $1 $2 0x1A 0x01   #REG_A2B0_INTPND2
i2cset -y $1 $2 0x0F 0x71   #REG_A2B0_RESPCYCS
i2cset -y $1 $2 0x12 0x81   #REG_A2B0_CONTROL
i2cset -y $1 $2 0x41 0x02   #REG_A2B0_I2SGCFG
i2cset -y $1 $2 0x09 0x01   #REG_A2B0_SWCTL
i2cset -y $1 $2 0x13 0x71   #REG_A2B0_DISCVRY
sleep 1
i2cget -y $1 $2 0x16        #REG_A2B0_INTSRC
i2cget -y $1 $2 0x17        #REG_A2B0_INTTYPE
i2cset -y $1 $2 0x09 0x21   #REG_A2B0_SWCTL
i2cset -y $1 $2 0x01 0x00   #REG_A2B0_NODEADR
# Slave 0
i2cget -y $1 $subnode_addr 0x02        #REG_A2B0_VEDNOR
i2cget -y $1 $subnode_addr 0x03        #REG_A2B0_PRODUCT
i2cget -y $1 $subnode_addr 0x04        #REG_A2B0_VERSION
i2cget -y $1 $subnode_addr 0x05        #REG_A2B0_CAPABILITY
i2cset -y $1 $subnode_addr 0x09 0x01   #REG_A2B0_SWCTL
i2cget -y $1 $subnode_addr 0x1B        #REG_A2B0_INTMSK0
i2cget -y $1 $subnode_addr 0x1C        #REG_A2B0_INTMSK1
i2cset -y $1 $subnode_addr 0x1B 0x10   #REG_A2B0_INTMSK0
i2cset -y $1 $subnode_addr 0x1C 0x00   #REG_A2B0_INTMSK1
i2cset -y $1 $2 0x13 0x6D   #REG_A2B0_DISCVRY
sleep 1
i2cget -y $1 $2 0x16        #REG_A2B0_INTSRC
i2cget -y $1 $2 0x17        #REG_A2B0_INTTYPE
i2cset -y $1 $2 0x01 0x00   #REG_A2B0_NODEADR
i2cset -y $1 $subnode_addr 0x09 0x01   #REG_A2B0_SWCTL
i2cset -y $1 $2 0x01 0x01   #REG_A2B0_NODEADR
i2cget -y $1 $subnode_addr 0x02        #REG_A2B0_VEDNOR
i2cget -y $1 $subnode_addr 0x03        #REG_A2B0_PRODUCT
i2cget -y $1 $subnode_addr 0x04        #REG_A2B0_VERSION
i2cget -y $1 $subnode_addr 0x05        #REG_A2B0_CAPABILITY
#==== Discovery Done

# Slave Node 1 Config
# /* A2B Master Node */
i2cwrite $1 $2 0x01 0x01 #/* NODEADR */
i2cwrite $1 $subnode_addr 0x0B 0x80 #/* LDNSLOTS */
i2cwrite $1 $subnode_addr 0x3F 0x01 #/* I2CCFG */
i2cwrite $1 $subnode_addr 0x41 0x60 #/* I2SGCFG */
i2cwrite $1 $subnode_addr 0x42 0x0B #/* I2SCFG */

i2cwrite $1 $subnode_addr 0x4A 0x80 #/* GPIODAT */
i2cwrite $1 $subnode_addr 0x4D 0x80 #/* GPIOOEN */
i2cwrite $1 $subnode_addr 0x4E 0x06 #/* GPIOIEN */

i2cwrite $1 $subnode_addr 0x50 0x06 #/* PINTEN */
i2cwrite $1 $subnode_addr 0x51 0x06 #/* PINTINV */
i2cwrite $1 $subnode_addr 0x52 0x00 #/* PINCFG */

i2cwrite $1 $subnode_addr 0x5A 0x41 #/* CLK2CFG */
i2cwrite $1 $subnode_addr 0x65 0xF0 #/* DNMASK0 */
i2cwrite $1 $subnode_addr 0x82 0x02 #/* GPIOD1MSK */
i2cwrite $1 $subnode_addr 0x83 0x04 #/* GPIOD2MSK */
i2cwrite $1 $subnode_addr 0x82 0x06 #/* GPIODINV */
i2cwrite $1 $subnode_addr 0x83 0x06 #/* GPIODEN */
i2cwrite $1 $subnode_addr 0x96 0x00 #/* MBOX1CTL */
i2cwrite $1 $subnode_addr 0x1B 0xFF #/* INTMSK0 */
i2cwrite $1 $subnode_addr 0x1C 0x06 #/* INTMSK1 */

# /* A2B Master Node */
i2cwrite $1 $2 0x01 0x01 #/* NODEADR */
# /* Enable Class D Peripheral Programming on Slave Node 1 */
i2cwrite $1 $subnode_addr 0x00 0x10 #/* CHIP - Set the chip address */
i2cwrite $1 $2 0x01 0x21 #/* NODEADR - Enable PERI */
# /* Start Slave 1 Peripheral Programming */
i2cwrite $1 $subnode_addr 0x04 0x80 #/* IC 1.POWER_CTRL Register.POWER_CTRL */
i2cwrite $1 $subnode_addr 0x05 0x8A #/* IC 1.AMP_DAC_CTRL Register.AMP_DAC_CTRL */
i2cwrite $1 $subnode_addr 0x06 0x02 #/* IC 1.DAC_CTRL Register.DAC_CTRL */
i2cwrite $1 $subnode_addr 0x07 0x40 #/* IC 1.VOL_LEFT_CTRL Register.VOL_LEFT_CTRL */
i2cwrite $1 $subnode_addr 0x08 0x40 #/* IC 1.VOL_RIGHT_CTRL Register.VOL_RIGHT_CTRL */
i2cwrite $1 $subnode_addr 0x09 0x54 #/* IC 1.SAI_CTRL1 Register.SAI_CTRL1 */
i2cwrite $1 $subnode_addr 0x0A 0x07 #/* IC 1.SAI_CTRL2 Register.SAI_CTRL2 */
i2cwrite $1 $subnode_addr 0x0B 0x00 #/* IC 1.SLOT_LEFT_CTRL Register.SLOT_LEFT_CTRL */
i2cwrite $1 $subnode_addr 0x0C 0x01 #/* SLOT_RIGHT_CTRL Register.SLOT_RIGHT_CTRL" */
i2cwrite $1 $subnode_addr 0x0E 0xA0 #/* IC 1.LIM_LEFT_CTRL1 Register.LIM_LEFT_CTRL1 */
i2cwrite $1 $subnode_addr 0x0F 0x51 #/* IC 1.LIM_LEFT_CTRL2 Register.LIM_LEFT_CTRL2 */
i2cwrite $1 $subnode_addr 0x10 0x22 #/* IC 1.LIM_LEFT_CTRL3 Register.LIM_LEFT_CTRL3 */
i2cwrite $1 $subnode_addr 0x11 0xA8 #/* IC 1.LIM_RIGHT_CTRL1 Register.LIM_RIGHT_CTRL1 */
i2cwrite $1 $subnode_addr 0x12 0x51 #/* IC 1.LIM_RIGHT_CTRL2 Register.LIM_RIGHT_CTRL2 */
i2cwrite $1 $subnode_addr 0x13 0x22 #/* IC 1.LIM_RIGHT_CTRL3 Register.LIM_RIGHT_CTRL3 */
i2cwrite $1 $subnode_addr 0x14 0xFF #/* IC 1.CLIP_LEFT_CTRL Register.CLIP_LEFT_CTRL */
i2cwrite $1 $subnode_addr 0x15 0xFF #/* IC 1.CLIP_RIGHT_CTRL Register.CLIP_RIGHT_CTRL */
i2cwrite $1 $subnode_addr 0x16 0x00 #/* IC 1.FAULT_CTRL1 Register.FAULT_CTRL1 */
i2cwrite $1 $subnode_addr 0x17 0x30 #/* IC 1.FAULT_CTRL2 Register.FAULT_CTRL2 */
i2cwrite $1 $subnode_addr 0x1C 0x00 #/* IC 1.SOFT_RESET Register.SOFT_RESET */

i2cwrite $1 $2 0x01 0x01 #/* NODEADR */
# /* Enable Class D Peripheral Programming on Slave Node 1 */
i2cwrite $1 $subnode_addr 0x00 0x11 #/* CHIP - Set the chip address */
i2cwrite $1 $2 0x01 0x21 #/* NODEADR - Enable PERI */
# /* Start Slave 1 Peripheral Programming */
i2cwrite $1 $subnode_addr 0x04 0x80 #/* IC 1.POWER_CTRL Register.POWER_CTRL */
i2cwrite $1 $subnode_addr 0x05 0x8A #/* IC 1.AMP_DAC_CTRL Register.AMP_DAC_CTRL */
i2cwrite $1 $subnode_addr 0x06 0x02 #/* IC 1.DAC_CTRL Register.DAC_CTRL */
i2cwrite $1 $subnode_addr 0x07 0x40 #/* IC 1.VOL_LEFT_CTRL Register.VOL_LEFT_CTRL */
i2cwrite $1 $subnode_addr 0x08 0x40 #/* IC 1.VOL_RIGHT_CTRL Register.VOL_RIGHT_CTRL */
i2cwrite $1 $subnode_addr 0x09 0x54 #/* IC 1.SAI_CTRL1 Register.SAI_CTRL1 */
i2cwrite $1 $subnode_addr 0x0A 0x07 #/* IC 1.SAI_CTRL2 Register.SAI_CTRL2 */
i2cwrite $1 $subnode_addr 0x0B 0x00 #/* IC 1.SLOT_LEFT_CTRL Register.SLOT_LEFT_CTRL */
i2cwrite $1 $subnode_addr 0x0C 0x01 #/* SLOT_RIGHT_CTRL Register.SLOT_RIGHT_CTRL" */
i2cwrite $1 $subnode_addr 0x0E 0xA0 #/* IC 1.LIM_LEFT_CTRL1 Register.LIM_LEFT_CTRL1 */
i2cwrite $1 $subnode_addr 0x0F 0x51 #/* IC 1.LIM_LEFT_CTRL2 Register.LIM_LEFT_CTRL2 */
i2cwrite $1 $subnode_addr 0x10 0x22 #/* IC 1.LIM_LEFT_CTRL3 Register.LIM_LEFT_CTRL3 */
i2cwrite $1 $subnode_addr 0x11 0xA8 #/* IC 1.LIM_RIGHT_CTRL1 Register.LIM_RIGHT_CTRL1 */
i2cwrite $1 $subnode_addr 0x12 0x51 #/* IC 1.LIM_RIGHT_CTRL2 Register.LIM_RIGHT_CTRL2 */
i2cwrite $1 $subnode_addr 0x13 0x22 #/* IC 1.LIM_RIGHT_CTRL3 Register.LIM_RIGHT_CTRL3 */
i2cwrite $1 $subnode_addr 0x14 0xFF #/* IC 1.CLIP_LEFT_CTRL Register.CLIP_LEFT_CTRL */
i2cwrite $1 $subnode_addr 0x15 0xFF #/* IC 1.CLIP_RIGHT_CTRL Register.CLIP_RIGHT_CTRL */
i2cwrite $1 $subnode_addr 0x16 0x00 #/* IC 1.FAULT_CTRL1 Register.FAULT_CTRL1 */
i2cwrite $1 $subnode_addr 0x17 0x30 #/* IC 1.FAULT_CTRL2 Register.FAULT_CTRL2 */
i2cwrite $1 $subnode_addr 0x1C 0x00 #/* IC 1.SOFT_RESET Register.SOFT_RESET */

# /* Setup A2B Master to program Slave Node 0 */
i2cwrite $1 $2 0x01 0x00 #/* NODEADR */
# Slave Node 0 Config
i2cwrite $1 $subnode_addr 0x0B 0x80 #/* LDNSLOTS */
i2cwrite $1 $subnode_addr 0x3F 0x01 #/* I2CCFG */
i2cwrite $1 $subnode_addr 0x41 0x60 #/* I2SGCFG */
i2cwrite $1 $subnode_addr 0x42 0x0B #/* I2SCFG */

i2cwrite $1 $subnode_addr 0x4A 0x80 #/* GPIODAT */
i2cwrite $1 $subnode_addr 0x4D 0x80 #/* GPIOOEN */
i2cwrite $1 $subnode_addr 0x4E 0x06 #/* GPIOIEN */

i2cwrite $1 $subnode_addr 0x50 0x06 #/* PINTEN */
i2cwrite $1 $subnode_addr 0x51 0x06 #/* PINTINV */
i2cwrite $1 $subnode_addr 0x52 0x00 #/* PINCFG */

i2cwrite $1 $subnode_addr 0x5A 0x41 #/* CLK2CFG */
i2cwrite $1 $subnode_addr 0x65 0x0F #/* DNMASK0 */
i2cwrite $1 $subnode_addr 0x82 0x02 #/* GPIOD1MSK */
i2cwrite $1 $subnode_addr 0x83 0x04 #/* GPIOD2MSK */
i2cwrite $1 $subnode_addr 0x82 0x06 #/* GPIODINV */
i2cwrite $1 $subnode_addr 0x83 0x06 #/* GPIODEN */
i2cwrite $1 $subnode_addr 0x96 0x00 #/* MBOX1CTL */
i2cwrite $1 $subnode_addr 0x1B 0xFF #/* INTMSK0 */
i2cwrite $1 $subnode_addr 0x1C 0x06 #/* INTMSK1 */

i2cwrite $1 $2 0x01 0x00 #/* NODEADR */
# /* Enable Class D Peripheral Programming on Slave Node 0 */
i2cwrite $1 $subnode_addr 0x00 0x10 #/* CHIP - Set the chip address */
i2cwrite $1 $2 0x01 0x20 #/* NODEADR - Enable PERI */

# /* Start Slave 1 Peripheral Programming */
i2cwrite $1 $subnode_addr 0x04 0x80 #/* IC 1.POWER_CTRL Register.POWER_CTRL */
i2cwrite $1 $subnode_addr 0x05 0x8A #/* IC 1.AMP_DAC_CTRL Register.AMP_DAC_CTRL */
i2cwrite $1 $subnode_addr 0x06 0x02 #/* IC 1.DAC_CTRL Register.DAC_CTRL */
i2cwrite $1 $subnode_addr 0x07 0x40 #/* IC 1.VOL_LEFT_CTRL Register.VOL_LEFT_CTRL */
i2cwrite $1 $subnode_addr 0x08 0x40 #/* IC 1.VOL_RIGHT_CTRL Register.VOL_RIGHT_CTRL */
i2cwrite $1 $subnode_addr 0x09 0x54 #/* IC 1.SAI_CTRL1 Register.SAI_CTRL1 */
i2cwrite $1 $subnode_addr 0x0A 0x07 #/* IC 1.SAI_CTRL2 Register.SAI_CTRL2 */
i2cwrite $1 $subnode_addr 0x0B 0x00 #/* IC 1.SLOT_LEFT_CTRL Register.SLOT_LEFT_CTRL */
i2cwrite $1 $subnode_addr 0x0C 0x01 #/* SLOT_RIGHT_CTRL Register.SLOT_RIGHT_CTRL" */
i2cwrite $1 $subnode_addr 0x0E 0xA0 #/* IC 1.LIM_LEFT_CTRL1 Register.LIM_LEFT_CTRL1 */
i2cwrite $1 $subnode_addr 0x0F 0x51 #/* IC 1.LIM_LEFT_CTRL2 Register.LIM_LEFT_CTRL2 */
i2cwrite $1 $subnode_addr 0x10 0x22 #/* IC 1.LIM_LEFT_CTRL3 Register.LIM_LEFT_CTRL3 */
i2cwrite $1 $subnode_addr 0x11 0xA8 #/* IC 1.LIM_RIGHT_CTRL1 Register.LIM_RIGHT_CTRL1 */
i2cwrite $1 $subnode_addr 0x12 0x51 #/* IC 1.LIM_RIGHT_CTRL2 Register.LIM_RIGHT_CTRL2 */
i2cwrite $1 $subnode_addr 0x13 0x22 #/* IC 1.LIM_RIGHT_CTRL3 Register.LIM_RIGHT_CTRL3 */
i2cwrite $1 $subnode_addr 0x14 0xFF #/* IC 1.CLIP_LEFT_CTRL Register.CLIP_LEFT_CTRL */
i2cwrite $1 $subnode_addr 0x15 0xFF #/* IC 1.CLIP_RIGHT_CTRL Register.CLIP_RIGHT_CTRL */
i2cwrite $1 $subnode_addr 0x16 0x00 #/* IC 1.FAULT_CTRL1 Register.FAULT_CTRL1 */
i2cwrite $1 $subnode_addr 0x17 0x30 #/* IC 1.FAULT_CTRL2 Register.FAULT_CTRL2 */
i2cwrite $1 $subnode_addr 0x1C 0x00 #/* IC 1.SOFT_RESET Register.SOFT_RESET */


i2cwrite $1 $2 0x01 0x00 #/* NODEADR */
# /* Enable Class D Peripheral Programming on Slave Node 0 */
i2cwrite $1 $subnode_addr 0x00 0x11 #/* CHIP - Set the chip address */
i2cwrite $1 $2 0x01 0x20 #/* NODEADR - Enable PERI */

# /* Start Slave 1 Peripheral Programming */
i2cwrite $1 $subnode_addr 0x04 0x80 #/* IC 1.POWER_CTRL Register.POWER_CTRL */
i2cwrite $1 $subnode_addr 0x05 0x8A #/* IC 1.AMP_DAC_CTRL Register.AMP_DAC_CTRL */
i2cwrite $1 $subnode_addr 0x06 0x02 #/* IC 1.DAC_CTRL Register.DAC_CTRL */
i2cwrite $1 $subnode_addr 0x07 0x40 #/* IC 1.VOL_LEFT_CTRL Register.VOL_LEFT_CTRL */
i2cwrite $1 $subnode_addr 0x08 0x40 #/* IC 1.VOL_RIGHT_CTRL Register.VOL_RIGHT_CTRL */
i2cwrite $1 $subnode_addr 0x09 0x54 #/* IC 1.SAI_CTRL1 Register.SAI_CTRL1 */
i2cwrite $1 $subnode_addr 0x0A 0x07 #/* IC 1.SAI_CTRL2 Register.SAI_CTRL2 */
i2cwrite $1 $subnode_addr 0x0B 0x00 #/* IC 1.SLOT_LEFT_CTRL Register.SLOT_LEFT_CTRL */
i2cwrite $1 $subnode_addr 0x0C 0x01 #/* SLOT_RIGHT_CTRL Register.SLOT_RIGHT_CTRL" */
i2cwrite $1 $subnode_addr 0x0E 0xA0 #/* IC 1.LIM_LEFT_CTRL1 Register.LIM_LEFT_CTRL1 */
i2cwrite $1 $subnode_addr 0x0F 0x51 #/* IC 1.LIM_LEFT_CTRL2 Register.LIM_LEFT_CTRL2 */
i2cwrite $1 $subnode_addr 0x10 0x22 #/* IC 1.LIM_LEFT_CTRL3 Register.LIM_LEFT_CTRL3 */
i2cwrite $1 $subnode_addr 0x11 0xA8 #/* IC 1.LIM_RIGHT_CTRL1 Register.LIM_RIGHT_CTRL1 */
i2cwrite $1 $subnode_addr 0x12 0x51 #/* IC 1.LIM_RIGHT_CTRL2 Register.LIM_RIGHT_CTRL2 */
i2cwrite $1 $subnode_addr 0x13 0x22 #/* IC 1.LIM_RIGHT_CTRL3 Register.LIM_RIGHT_CTRL3 */
i2cwrite $1 $subnode_addr 0x14 0xFF #/* IC 1.CLIP_LEFT_CTRL Register.CLIP_LEFT_CTRL */
i2cwrite $1 $subnode_addr 0x15 0xFF #/* IC 1.CLIP_RIGHT_CTRL Register.CLIP_RIGHT_CTRL */
i2cwrite $1 $subnode_addr 0x16 0x00 #/* IC 1.FAULT_CTRL1 Register.FAULT_CTRL1 */
i2cwrite $1 $subnode_addr 0x17 0x30 #/* IC 1.FAULT_CTRL2 Register.FAULT_CTRL2 */
i2cwrite $1 $subnode_addr 0x1C 0x00 #/* IC 1.SOFT_RESET Register.SOFT_RESET */

# /* Final Master node programming */
i2cwrite $1 $2 0x42 0x91 #/* I2SCFG */
i2cwrite $1 $2 0x52 0x00 #/* PINCFG */
i2cwrite $1 $2 0x01 0x00 #/* NODEADR */
i2cwrite $1 $subnode_addr 0x0D 0x08 #/* DNSLOTS Node 0 */
i2cwrite $1 $2 0x0D 0x08 #/* DNSLOTS Master */
i2cwrite $1 $2 0x01 0x00 #/* NODEADR */
i2cwrite $1 $subnode_addr 0x09 0x01 #/* SWCTL Node 0 */
i2cwrite $1 $2 0x09 0x01 #/* SWCTL Master */
i2cwrite $1 $2 0x01 0x80 #/* NODEADR */
i2cwrite $1 $2 0x01 0x00 #/* NODEADR */

i2cwrite $1 $2 0x10 0x44 #/* SLOTFMT */
i2cwrite $1 $2 0x11 0x03 #/* DATCTL */

i2cwrite $1 $2 0x12 0x81 #/* CONTROL */
sleep 1
i2cwrite $1 $2 0x01 0x01 #/* NODEADR */
i2cwrite $1 $2 0x12 0x82 #/* CONTROL */
