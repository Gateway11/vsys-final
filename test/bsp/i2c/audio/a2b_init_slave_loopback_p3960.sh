#!/bin/bash

###############################################################################
# SPDX-FileCopyrightText: Copyright (c) 2021-2024 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
# SPDX-License-Identifier: LicenseRef-NvidiaProprietary
#
# NVIDIA CORPORATION, its affiliates and licensors retain all intellectual
# property and proprietary rights in and to this material, related
# documentation and any modifications thereto. Any use, reproduction,
# disclosure or distribution of this material and related documentation
# without an express license agreement from NVIDIA CORPORATION or
# its affiliates is strictly prohibited.
#
# Initializes masterNode(AD2433)/slaveNode(AD2433) for slave I2s loopback mode
# Master Node I2c Address @ "$2"
##############################################################################

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
echo "Slave address ${slave_address}"

i2cwrite () {
    if ! i2cset -y $1 $2 $3 $4; then
        echo FAILED: i2cset -y $1 $2 $3 $4
        exit 1
    fi
}

i2cwrite "$1" "$2" 0x12  0x84   #/* CONTROL */
sleep 1                      #/* A2B_Delay */
i2cwrite "$1" "$2" 0x1B  0x7F   #/* INTMSK0 */
i2cwrite "$1" "$2" 0x1C  0x78   #/* INTMSK1 */
i2cwrite "$1" "$2" 0x1D  0x0F   #/* INTMSK2 */
i2cwrite "$1" "$2" 0x0F  0x7D   #/* RESPCYCS */
i2cwrite "$1" "$2" 0x12  0x81   #/* CONTROL */
i2cwrite "$1" "$2" 0x41  0x2   #/* I2SGCFG */
i2cwrite "$1" "$2" 0x09  0x01   #/* SWCTL */
i2cwrite "$1" "$2" 0x13  0x7D   #/* DISCVRY */
sleep 1                      #/* A2B_Delay */
i2cwrite "$1" "$2" 0x1A  0x01   #/* INTPND2 */
i2cwrite "$1" "$2" 0x09  0x21   #/* SWCTL */
i2cwrite "$1" "$2" 0x01  0x00   #/* NODEADR */
i2cwrite "$1" "${slave_address}" 0x0B  0x80   #/* LDNSLOTS */
i2cwrite "$1" "${slave_address}" 0x0C  0x08   #/* LUPSLOTS */
i2cwrite "$1" "${slave_address}" 0x3F  0x01   #/* I2CCFG */
i2cwrite "$1" "${slave_address}" 0x41  0x2   #/* I2SGCFG */
i2cwrite "$1" "${slave_address}" 0x42  0x11   #/* I2SCFG */
i2cwrite "$1" "${slave_address}" 0x47  0x18   #/* PDMCTL */
i2cwrite "$1" "${slave_address}" 0x4A  0x10   #/* GPIODAT */
i2cwrite "$1" "${slave_address}" 0x4D  0x10   #/* GPIOOEN */
i2cwrite "$1" "${slave_address}" 0x52  0x01   #/* PINCFG */
i2cwrite "$1" "${slave_address}" 0x65  0xFF   #/* DNMASK0 */
i2cwrite "$1" "${slave_address}" 0x1B  0x7F   #/* INTMSK0 */
i2cwrite "$1" "${slave_address}" 0x1C  0x7F   #/* INTMSK1 */
i2cwrite "$1" "$2" 0x42  0x11   #/* I2SCFG */
i2cwrite "$1" "$2" 0x0D  0x08   #/* DNSLOTS */
i2cwrite "$1" "$2" 0x0E  0x08   #/* UPSLOTS */
i2cwrite "$1" "$2" 0x09  0x01   #/* SWCTL */
i2cwrite "$1" "$2" 0x10  0x66   #/* SLOTFMT */
i2cwrite "$1" "$2" 0x11  0x03   #/* DATCTL *//*Normal=0x03, 0x00 shows only PREAMBLE on the Scope*/
#i2cwrite "$1" "$2" 0x30  0x82   #/* TXBCTL *//*TXB DRV STR OVERWRITE 0x00:High,0x02:Medium,0x03:Low*/
i2cwrite "$1" "$2" 0x11  0x03   #/* DATCTL */
i2cwrite "$1" "$2" 0x12  0x81   #/* CONTROL */
i2cwrite "$1" "$2" 0x01  0x00   #/* NODEADR */
i2cwrite "$1" "${slave_address}" 0x53  0x00   #Bus Loopback /Mike added comment.

#verify expected devices
     master_vendor=$(i2cget -y $1 "$2" 0x2)
     master_product=$(i2cget -y $1 "$2" 0x3)
     master_version=$(i2cget -y $1 "$2" 0x4)
     slave_vendor=$(i2cget -y $1 "${slave_address}" 0x2)
     slave_product=$(i2cget -y $1 "${slave_address}" 0x3)
     slave_version=$(i2cget -y $1 "${slave_address}" 0x4)

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

	if [ $slave_product != "0x33" ] && [ $slave_product != "0x28" ]; then
		echo FAIL: Unexpected slave Node Product ID "("$slave_product")"
        exit 1
    fi


echo A2B Loopback Initialization Complete
echo

exit 0

