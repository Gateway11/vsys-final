#!/bin/bash
#
# SPDX-FileCopyrightText: Copyright (c) 2021-2024 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
# SPDX-License-Identifier: LicenseRef-NvidiaProprietary
#
# NVIDIA CORPORATION and its licensors retain all intellectual property
# and proprietary rights in and to this software, related documentation
# and any modifications thereto.  Any use, reproduction, disclosure or
# distribution of this software and related documentation without an express
# license agreement from NVIDIA CORPORATION is strictly prohibited.

# Initializes masterNode(AD2433)/slaveNode(AD2425) for slave I2s loopback mode
# Master Node I2c Address @ "$2"

if [ $# -lt 2 ]
then
    echo "Two arguments expected!! Please pass i2c device ID as arg#1 and master address as arg#2"
    exit 1
fi

master_address=$2
slave_address=$(( master_address | 0x01 ))

echo "Script name is: $0"
echo "I2C device ID $1"
echo "Master address $2"
echo "Slave address $slave_address"

i2cwrite () {
  if ! i2cset -y $1 $2 $3 $4; then
    echo FAILED: i2cset -y $1 $2 $3 $4
    exit 1
  fi
}

i2cset -y "$1" "$2" 0x12 0x84
sleep 1
i2cset -y "$1" "$2" 0x1B 0x77
#i2cset -y "$1" "$2" 0x1C 0x00
i2cset -y "$1" "$2" 0x1D 0x0F
i2cset -y "$1" "$2" 0x0F 0x70
i2cset -y "$1" "$2" 0x12 0x01
i2cset -y "$1" "$2" 0x41 0x02
i2cset -y "$1" "$2" 0x09 0x01
i2cset -y "$1" "$2" 0x13 0x70
sleep 1
i2cset -y "$1" "$2" 0x1A 0x01
#i2cset -y "$1" "$2" 0x01 0x00
i2cset -y "$1" "$2" 0x09 0x21
i2cset -y "$1" "$2" 0x01 0x00
#i2cset -y "$1" "$slave_address" 0x0A 0x00
#i2cset -y "$1" "$slave_address" 0x0B 0x00
i2cset -y "$1" "$slave_address" 0x0C 0x04
i2cset -y "$1" "$slave_address" 0x3F 0x01
i2cset -y "$1" "$slave_address" 0x47 0x1F
i2cset -y "$1" "$slave_address" 0x48 0x00
i2cset -y "$1" "$slave_address" 0x4A 0x10
i2cset -y "$1" "$slave_address" 0x4D 0x10
#i2cset -y "$1" "$slave_address" 0x4E 0x00
#i2cset -y "$1" "$slave_address" 0x50 0x00
#i2cset -y "$1" "$slave_address" 0x51 0x00
i2cset -y "$1" "$slave_address" 0x52 0x00
#i2cset -y "$1" "$slave_address" 0x20 0x00
i2cset -y "$1" "$slave_address" 0x59 0x01
i2cset -y "$1" "$slave_address" 0x5A 0x81
#i2cset -y "$1" "$slave_address" 0x60 0x00
#i2cset -y "$1" "$slave_address" 0x61 0x00
#i2cset -y "$1" "$slave_address" 0x62 0x00
#i2cset -y "$1" "$slave_address" 0x63 0x00
#i2cset -y "$1" "$slave_address" 0x64 0x00
#i2cset -y "$1" "$slave_address" 0x65 0x00
#i2cset -y "$1" "$slave_address" 0x66 0x00
#i2cset -y "$1" "$slave_address" 0x67 0x00
#i2cset -y "$1" "$slave_address" 0x68 0x00
#i2cset -y "$1" "$slave_address" 0x69 0x00
#i2cset -y "$1" "$slave_address" 0x81 0x00
#i2cset -y "$1" "$slave_address" 0x82 0x00
#i2cset -y "$1" "$slave_address" 0x83 0x00
#i2cset -y "$1" "$slave_address" 0x84 0x00
#i2cset -y "$1" "$slave_address" 0x85 0x00
#i2cset -y "$1" "$slave_address" 0x86 0x00
#i2cset -y "$1" "$slave_address" 0x87 0x00
#i2cset -y "$1" "$slave_address" 0x88 0x00
#i2cset -y "$1" "$slave_address" 0x8A 0x00
#i2cset -y "$1" "$slave_address" 0x80 0x00
#i2cset -y "$1" "$slave_address" 0x90 0x00
#i2cset -y "$1" "$slave_address" 0x96 0x00
#i2cset -y "$1" "$slave_address" 0x5C 0x00
#i2cset -y "$1" "$slave_address" 0x58 0x00
#i2cset -y "$1" "$slave_address" 0x57 0x00
i2cset -y "$1" "$slave_address" 0x1B 0x77
i2cset -y "$1" "$slave_address" 0x1C 0x7F
i2cset -y "$1" "$slave_address" 0x1E 0xEF
#i2cset -y "$1" "$2" 0x01 0x00
#i2cset -y "$1" "$2" 0x3F 0x00
i2cset -y "$1" "$2" 0x42 0x11
#i2cset -y "$1" "$2" 0x44 0x00
#i2cset -y "$1" "$2" 0x45 0x00
#i2cset -y "$1" "$2" 0x47 0x00
#i2cset -y "$1" "$2" 0x5D 0x00
#i2cset -y "$1" "$2" 0x48 0x00
#i2cset -y "$1" "$2" 0x4A 0x00
#i2cset -y "$1" "$2" 0x4D 0x00
#i2cset -y "$1" "$2" 0x4E 0x00
#i2cset -y "$1" "$2" 0x50 0x00
#i2cset -y "$1" "$2" 0x51 0x00
i2cset -y "$1" "$2" 0x52 0x01
#i2cset -y "$1" "$2" 0x20 0x00
#i2cset -y "$1" "$2" 0x59 0x00
#i2cset -y "$1" "$2" 0x5A 0x00
#i2cset -y "$1" "$2" 0x81 0x00
#i2cset -y "$1" "$2" 0x82 0x00
#i2cset -y "$1" "$2" 0x83 0x00
#i2cset -y "$1" "$2" 0x84 0x00
#i2cset -y "$1" "$2" 0x85 0x00
#i2cset -y "$1" "$2" 0x86 0x00
#i2cset -y "$1" "$2" 0x87 0x00
#i2cset -y "$1" "$2" 0x88 0x00
#i2cset -y "$1" "$2" 0x8A 0x00
#i2cset -y "$1" "$2" 0x80 0x00
#i2cset -y "$1" "$2" 0x57 0x00
#i2cset -y "$1" "$2" 0x2E 0x00
#i2cset -y "$1" "$2" 0x30 0x00
i2cset -y "$1" "$2" 0x1E 0xEF
#i2cset -y "$1" "$2" 0x0D 0x00
i2cset -y "$1" "$2" 0x0E 0x04
i2cset -y "$1" "$2" 0x09 0x01
#i2cset -y "$1" "$2" 0x40 0x00
#i2cset -y "$1" "$2" 0x01 0x00
i2cset -y "$1" "$2" 0x10 0x44
i2cset -y "$1" "$2" 0x11 0x03
i2cset -y "$1" "$2" 0x56 0x01
i2cset -y "$1" "$2" 0x12 0x01

#verify expected devices
   master_vendor=$(i2cget -y "$1" "$2" 0x2)
   master_product=$(i2cget -y "$1" "$2" 0x3)
   master_version=$(i2cget -y "$1" "$2" 0x4)
   slave_vendor=$(i2cget -y "$1" "$slave_address" 0x2)
   slave_product=$(i2cget -y "$1" "$slave_address" 0x3)

   slave_version=$(i2cget -y "$1" "$slave_address" 0x4)

   echo
   echo MASTER:
   echo VENDOR ID = $master_vendor
   echo PRODUCT ID = $master_product
   echo VERSION ID = $master_version
   echo
   echo SLAVE:
   echo VENDOR ID = $slave_vendor
   echo PRODUCT ID = $slave_product
   echo VERSION ID = $slave_version
   echo

        if [ $master_vendor != "0xad" ]
  then
                echo FAIL: Unexpected Master Node Vendor ID "("$master_vendor")"
    exit 1
  fi

        if [ $master_product != "0x33" ]
  then
                echo FAIL: Unexpected Master Node Product ID "("$master_product")"
    exit 1
  fi

        if [ $slave_vendor != "0xad" ]
  then
                echo FAIL: Unexpected slave Node Vendor ID "("$slave_vendor")"
    exit 1
  fi

        if [ $slave_product != "0x22" ]
  then
                echo FAIL: Unexpected slave Node Product ID "("$slave_product")"
    exit 1
  fi





echo A2B Loopback Initialization Complete
echo

exit 0
