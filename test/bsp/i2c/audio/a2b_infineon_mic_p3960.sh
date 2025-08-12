#!/bin/bash
#
# SPDX-FileCopyrightText: Copyright (c) 2024 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
# SPDX-License-Identifier: LicenseRef-NvidiaProprietary
#
# NVIDIA CORPORATION and its licensors retain all intellectual property
# and proprietary rights in and to this software, related documentation
# and any modifications thereto.  Any use, reproduction, disclosure or
# distribution of this software and related documentation without an express
# license agreement from NVIDIA CORPORATION is strictly prohibited.

# Initializes Main node(AD2433) and Two Sub nodes (AD2428) for capture
# I2C Interface ID is @ "$1"
# Main Node I2c Address @ "$2"

if [ $# -lt 2 ]; then
    echo "Two arguments expected!! Please pass i2c device ID as arg#1 and master address as arg#2"
    exit 1
fi

main_address=$2
bus_address=$((main_address | 0x01))

echo "Script name is: $0"
echo "I2C device ID $1"
echo "Main Node address $2"
echo "Sub node address $bus_address"

# A2B Master
i2cset -y "$1" "$2" 0x12 0x84 #REG_A2B0_CONTROL
i2cset -y "$1" "$2" 0x16 0x80 #REG_A2B0_INTSRC
i2cget -y "$1" "$2" 0x17      #REG_A2B0_INTTYPE
i2cget -y "$1" "$2" 0x16      #REG_A2B0_INTSRC
i2cget -y "$1" "$2" 0x02      #REG_A2B0_VEDNOR
i2cget -y "$1" "$2" 0x03      #REG_A2B0_PRODUCT
i2cget -y "$1" "$2" 0x04      #REG_A2B0_VERSION
i2cset -y "$1" "$2" 0x1B 0x77 #REG_A2B0_INTMSK0
i2cset -y "$1" "$2" 0x1D 0x78 #REG_A2B0_INTMSK1
i2cset -y "$1" "$2" 0x1D 0x0F #REG_A2B0_INTMSK2
i2cset -y "$1" "$2" 0x1E 0xEF #REG_A2B0_BECCTL
i2cset -y "$1" "$2" 0x1A 0x01 #REG_A2B0_INTPND2
i2cset -y "$1" "$2" 0x0F 0x71 #REG_A2B0_RESPCYCS
i2cset -y "$1" "$2" 0x12 0x81 #REG_A2B0_CONTROL
i2cset -y "$1" "$2" 0x41 0x02 #REG_A2B0_I2SGCFG
i2cset -y "$1" "$2" 0x09 0x01 #REG_A2B0_SWCTL
i2cset -y "$1" "$2" 0x13 0x71 #REG_A2B0_DISCVRY
sleep 1
i2cget -y "$1" "$2" 0x16      #REG_A2B0_INTSRC
i2cget -y "$1" "$2" 0x17      #REG_A2B0_INTTYPE
i2cset -y "$1" "$2" 0x09 0x21 #REG_A2B0_SWCTL
i2cset -y "$1" "$2" 0x01 0x00 #REG_A2B0_NODEADR
# Sub node 0
i2cget -y "$1" "$bus_address" 0x02      #REG_A2B0_VEDNOR
i2cget -y "$1" "$bus_address" 0x03      #REG_A2B0_PRODUCT
i2cget -y "$1" "$bus_address" 0x04      #REG_A2B0_VERSION
i2cget -y "$1" "$bus_address" 0x05      #REG_A2B0_CAPABILITY
i2cset -y "$1" "$bus_address" 0x09 0x01 #REG_A2B0_SWCTL
i2cget -y "$1" "$bus_address" 0x1B      #REG_A2B0_INTMSK0
i2cget -y "$1" "$bus_address" 0x1C      #REG_A2B0_INTMSK1
i2cset -y "$1" "$bus_address" 0x1B 0x10 #REG_A2B0_INTMSK0
i2cset -y "$1" "$bus_address" 0x1C 0x00 #REG_A2B0_INTMSK1
i2cset -y "$1" "$2" 0x13 0x6D           #REG_A2B0_DISCVRY
sleep 1
i2cget -y "$1" "$2" 0x16                #REG_A2B0_INTSRC
i2cget -y "$1" "$2" 0x17                #REG_A2B0_INTTYPE
i2cset -y "$1" "$2" 0x01 0x00           #REG_A2B0_NODEADR
i2cset -y "$1" "$bus_address" 0x09 0x21 #REG_A2B0_SWCTL
i2cset -y "$1" "$2" 0x01 0x01           #REG_A2B0_NODEADR
i2cget -y "$1" "$bus_address" 0x02      #REG_A2B0_VEDNOR
i2cget -y "$1" "$bus_address" 0x03      #REG_A2B0_PRODUCT
i2cget -y "$1" "$bus_address" 0x04      #REG_A2B0_VERSION
i2cget -y "$1" "$bus_address" 0x05      #REG_A2B0_CAPABILITY

#==== Discovery Done

#Sub node 1
i2cset -y "$1" "$bus_address" 0x0B 0x80 #REG_A2B0_LDNSLOTS
i2cset -y "$1" "$bus_address" 0x0C 0x04 #REG_A2B0_LUPSLOTS
i2cset -y "$1" "$bus_address" 0x3F 0x01 #REG_A2B0_I2CCFG
# Not programming REG_A2B0_I2SGCFG (0x41 and 0x42) for Slave 1
i2cset -y "$1" "$bus_address" 0x47 0x1F #REG_A2B0_PDMCTL
i2cset -y "$1" "$bus_address" 0x4A 0x10 #REG_A2B0_GPIODAT
i2cset -y "$1" "$bus_address" 0x4D 0x10 #REG_A2B0_GPIOOEN
i2cset -y "$1" "$bus_address" 0x52 0x00 #REG_A2B0_PINCFG
i2cset -y "$1" "$bus_address" 0x5A 0x81 #REG_AD242X0_CLK2CFG Original 0xC1
i2cset -y "$1" "$bus_address" 0x1B 0x77 #REG_A2B0_INTMSK0
i2cset -y "$1" "$bus_address" 0x1C 0x7F #REG_A2B0_INTMSK1
i2cset -y "$1" "$bus_address" 0x1E 0xEF #REG_A2B0_BECCTL
#Main node
i2cget -y "$1" "$2" 0x16      #REG_A2B0_INTSRC
i2cset -y "$1" "$2" 0x01 0x00 #REG_A2B0_NODEADR

#Sub node 0
i2cset -y "$1" "$bus_address" 0x0B 0x80 #REG_A2B0_LDNSLOTS
i2cset -y "$1" "$bus_address" 0x0C 0x04 #REG_A2B0_LUPSLOTS
i2cset -y "$1" "$bus_address" 0x0E 0x04 #REG_A2B0_UPSLOTS
i2cset -y "$1" "$bus_address" 0x3F 0x01 #REG_A2B0_I2CCFG
# Not programming REG_A2B0_I2SGCFG (0x41 and 0x42) for Sub node 0
i2cset -y "$1" "$bus_address" 0x47 0x1F #REG_A2B0_PDMCTL
i2cset -y "$1" "$bus_address" 0x4A 0x10 #REG_A2B0_GPIODAT
i2cset -y "$1" "$bus_address" 0x4D 0x10 #REG_A2B0_GPIOOEN
i2cset -y "$1" "$bus_address" 0x52 0x00 #REG_A2B0_PINCFG
#i2cset -y "$1" "$bus_address" 0x5A 0x81   #REG_AD242X0_CLK2CFG Original 0xC1
i2cset -y "$1" "$bus_address" 0x1B 0x77 #REG_A2B0_INTMSK0
i2cset -y "$1" "$bus_address" 0x1C 0x7F #REG_A2B0_INTMSK1
i2cset -y "$1" "$bus_address" 0x1E 0xEF #REG_A2B0_BECCTL
i2cset -y "$1" "$bus_address" 0x09 0x01 #REG_A2B0_SWCTL
#Main node
i2cget -y "$1" "$2" 0x16      #REG_A2B0_INTSRC
i2cset -y "$1" "$2" 0x09 0x01 #REG_A2B0_SWCTL
i2cset -y "$1" "$2" 0x0E 0x08 #REG_A2B0_UPSLOTS
i2cset -y "$1" "$2" 0x41 0x02 #REG_A2B0_I2SGCFG (different from EVK 0x24)
i2cset -y "$1" "$2" 0x42 0x11 #REG_A2B0_I2SGCFG (different from EVK 0x24)
i2cset -y "$1" "$2" 0x52 0x00 #REG_A2B0_PINCFG
i2cset -y "$1" "$2" 0x1B 0x77 #REG_A2B0_INTMSK0
i2cset -y "$1" "$2" 0x1C 0x00 #REG_A2B0_INTMSK1 (different from EVK 0x78)
i2cset -y "$1" "$2" 0x1D 0x0F #REG_A2B0_INTMSK2
i2cset -y "$1" "$2" 0x1E 0xEF #REG_A2B0_BECCTL
i2cset -y "$1" "$2" 0x01 0x00 #REG_A2B0_NODEADR
i2cset -y "$1" "$2" 0x01 0x00 #REG_A2B0_NODEADR
i2cset -y "$1" "$2" 0x40 0x00 #REG_A2B0_PLLCTL
i2cset -y "$1" "$2" 0x01 0x00 #REG_A2B0_NODEADR
i2cset -y "$1" "$2" 0x10 0x66 #REG_A2B0_SLOTFMT
i2cset -y "$1" "$2" 0x11 0x03 #REG_A2B0_DATCTL
i2cset -y "$1" "$2" 0x12 0x81 #REG_A2B0_CONTROL
i2cset -y "$1" "$2" 0x01 0x01 #REG_A2B0_NODEADR
#Slave 1
i2cset -y "$1" "$bus_address" 0x09 0x00 #REG_A2B0_SWCTL
#Master
i2cset -y "$1" "$2" 0x12 0x82 #REG_A2B0_CONTROL
i2cget -y "$1" "$2" 0x02      #REG_A2B0_VEDNOR
i2cget -y "$1" "$2" 0x03      #REG_A2B0_PRODUCT
i2cget -y "$1" "$2" 0x04      #REG_A2B0_VERSION

#verify expected devices
main_vendor=$(i2cget -y "$1" "$2" 0x2)
main_product=$(i2cget -y "$1" "$2" 0x3)
main_version=$(i2cget -y "$1" "$2" 0x4)
i2cset -y "$1" "$2" 0x01 0x00 #REG_A2B0_NODEADR
subnode1_vendor=$(i2cget -y "$1" "$bus_address" 0x2)
subnode1_product=$(i2cget -y "$1" "$bus_address" 0x3)

subnode1_version=$(i2cget -y "$1" "$bus_address" 0x4)
i2cset -y "$1" "$2" 0x01 0x01 #REG_A2B0_NODEADR
subnode2_vendor=$(i2cget -y "$1" "$bus_address" 0x2)
subnode2_product=$(i2cget -y "$1" "$bus_address" 0x3)

subnode2_version=$(i2cget -y "$1" "$bus_address" 0x4)
echo
echo MAIN NODE:
echo VENDOR ID = $main_vendor
echo PRODUCT ID = $main_product
echo VERSION ID = $main_version
echo
echo SUB NODE I:
echo VENDOR ID = $subnode1_vendor
echo PRODUCT ID = $subnode1_product
echo VERSION ID = $subnode1_version
echo
echo SUB NODE II:
echo VENDOR ID = $subnode2_vendor
echo PRODUCT ID = $subnode2_product
echo VERSION ID = $subnode2_version
echo

if [ $main_vendor != "0xad" ]; then
    echo FAIL: Unexpected Master Node Vendor ID "("$main_vendor")"
    exit 1
fi

if [ $main_product != "0x33" ]; then
    echo FAIL: Unexpected Master Node Product ID "("$main_product")"
    exit 1
fi

if [ $subnode1_vendor != "0xad" ]; then
    echo FAIL: Unexpected slave Node Vendor ID "("$subnode1_vendor")"
    exit 1
fi

if [ $subnode1_product != "0x28" ]; then
    echo FAIL: Unexpected slave Node Product ID "("$subnode1_product")"
    exit 1
fi
if [ $subnode2_vendor != "0xad" ]; then
    echo FAIL: Unexpected slave Node Vendor ID "("$subnode2_vendor")"
    exit 1
fi

if [ $subnode2_product != "0x28" ]; then
    echo FAIL: Unexpected slave Node Product ID "("$subnode2_product")"
    exit 1
fi

echo A2B Loopback Initialization Complete
echo

exit 0
