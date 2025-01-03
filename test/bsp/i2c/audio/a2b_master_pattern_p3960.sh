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

# Initializes masterNode(AD2428) for fixed pattern genration
# Master Node I2c Address @ "$2"/0x6a/0x6c/0x6e

if [ $# -lt 2 ]
then
    echo "Atleast two arguments expected!! Please pass i2c device ID as arg#1, device address as arg#2 and functionality (loopback/fixed-pattern) as arg#3!"
    exit 1
fi

echo "Script name is: $0"
echo "I2C device ID $1"
echo "Device address $2"

i2cwrite () {
    if ! i2cset -y $1 $2 $3 $4; then
        echo FAILED: i2cset -y $1 $2 $3 $4
    fi
}

if [[ "$3" == "loopback" ]]; then
  i2s_test_reg_val=0x06
  echo "Functionality loopback"
elif [[ "$3" == "fixed-pattern" ]]; then
  i2s_test_reg_val=0x01
  echo "Functionality fixed-pattern"
else
  echo "Choosing Default functionaliy fixed-pattern."
  i2s_test_reg_val=0x01
fi

i2cwrite "$1" "$2"	0x12  0x84   #/* CONTROL */
sleep 1                      #/* A2B_Delay */
i2cwrite "$1" "$2"	0x1B  0x7F   #/* INTMSK0 */
i2cwrite "$1" "$2"	0x1C  0x78   #/* INTMSK1 */
i2cwrite "$1" "$2"	0x1D  0x0F   #/* INTMSK2 */
i2cwrite "$1" "$2"	0x0F  0x7D   # RESPCYCS
i2cwrite "$1" "$2"	0x12  0x81   #/* CONTROL */
i2cwrite "$1" "$2"	0x41  0x2   #/* I2SGCFG */
i2cwrite "$1" "$2"	0x09  0x01   #/* SWCTL */

sleep 1                      #/* A2B_Delay */
i2cwrite "$1" "$2"	0x1A  0x01   #/* INTPND2 */
i2cwrite "$1" "$2"	0x42  0x11   #/* I2SCFG */
i2cwrite "$1" "$2"	0x52  0x01   #/* PINCFG */ /*Set DRVSTR=Strong, Mike changed to make DIN signal improvement.
i2cwrite "$1" "$2"	0x30  0x82   #/* TXBCTL *//*TXB DRV STR OVERWRITE 0x00:High,0x02:Medium,0x03:Low*/
i2cwrite "$1" "$2"	0x11  0x00   #/* DATCTL */
i2cwrite "$1" "$2"	0x53  $i2s_test_reg_val   #Bus Loopback /Mike added comment.

#verify expected devices
master_vendor=$(i2cget -y $1 $2 0x2)
master_product=$(i2cget -y $1 $2 0x3)
master_version=$(i2cget -y $1 $2 0x4)

echo MASTER:
echo VENDOR ID = $master_vendor
echo PRODUCT ID = $master_product
echo VERSION ID = $master_version
echo

  if [ $master_vendor != "0xad" ]
    then
		echo FAIL: Unexpected Master Node Vendor ID "("$master_vendor")"
  fi


echo A2B Loopback Initialization Complete
echo


exit 0
