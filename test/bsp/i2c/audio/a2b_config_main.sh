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

# Initializes masterNode(AD2433) for fixed pattern generation
# Master Node I2c Base Address A2B1=0x68, A2B2=0x6A, A2B3 = 0x6C

set -u

i2c_interface_id=""
a2b_chip_addresses=""
a2b_function_mode="2"
sample_size="32"
tdm_ch="8"
sample_rate=48000

input_i2c_interface_id=""
input_a2b_chip_addresses=""
input_a2b_function_mode=""
input_sample_size=""
input_tdm_ch=""
input_sample_rate=""

configure_all_devices=0
helper_script_name=""
script_dir=$(dirname "$(realpath "$0")")
compatible_string=$(tr -d '\0' < /proc/device-tree/compatible)
tegra_model="$(echo $compatible_string | sed -re 's/.*nvidia,(tegra[0-9]{3}).*$/\1/')"

function fail_print_exit {
    echo "$1"
    show_usage
    exit 1
}

function get_script_name {
    case "${a2b_function_mode}" in
        "1")  helper_script_name="a2b_master_pattern_p3960.sh"     ;;
        "2")  helper_script_name="a2b_master_pattern_p3960.sh"     ;;
        "3")  helper_script_name="a2b_8_ClassD_Amps_p3710.sh"      ;;
        "4")  helper_script_name="a2b_4mic_p3960.sh"               ;;
        "5")  helper_script_name="a2b_init_slave_loopback_p3960.sh";;
          *)  echo "Functionality ${1} is not supported."
              exit 1;;
    esac
}

function show_usage {
    echo "Usage: $(basename ${0}) [options]"
    echo "Options:"
    echo "  -b <Sample Size>           :   Sample size Either 16 or 32"
    echo "  -d <A2B device address>    :   Main node device address on the I2C bus."
    echo "                                 Ferrix(AD2433): A2B1=0x68(Default), A2B2=0x6A, A2B3 = 0x6C"
    echo "  -i <I2C Interface ID>      :   I2C Interface ID which is being used to communicate with A2B Chips"
    echo "  -f <Usecase>               :   1-> Main node Fixed Pattern (0xB38F0E32) Capture"
    echo "                                 2-> Main node Internal Loopback (Captures data from Playback Path)"
    echo "                                 3-> Two daisy-chained Speaker Subnodes connected to Main node"
    echo "                                 4-> Single 4 Channel Hosiden Mic Array connected to Main node"
    echo "                                 5-> A2B2/A2B3 to A2B1 External Loopback"
#    echo "  -n <Sub Node no>           :   Node numbers to be read/write. Shall be multiple entries e.g -n 1 2 "
    echo "  -r <Sub node sample_rate>  :   Sub node sample_rate 48000."
    echo "  -t <TDM Channels>          :   TDM Channels 8."
}

function check_board_compatibility {
    if grep -q "\b${compatible_string}\b" <<< "$1"; then
        return 1
    else
        return 0
    fi
}

function configure_a2b {
    local cmdline=""
    local chip_address="$1"
    get_script_name

    echo "Calling A2B Configuration script ${helper_script_name}"
    case "${a2b_function_mode}" in
        "1")
            cmdline="${script_dir}/${helper_script_name} ${i2c_interface_id} ${chip_address} fixed-pattern"
            ;;
        "2")
            cmdline="${script_dir}/${helper_script_name} ${i2c_interface_id} ${chip_address} loopback"
            ;;
        *)
            cmdline="${script_dir}/${helper_script_name} ${i2c_interface_id} ${chip_address}"
            ;;
    esac
    echo "${cmdline}"
    eval "${cmdline}"
}

case "${tegra_model}" in
"tegra264")
    check_board_compatibility "p3960"
    if [ "$?" -eq 0 ]; then
        echo "Detected T264 P3960 (Ferrix TS1) Board. This has total of 3 A2B AD2433 Chip"
        i2c_interface_id="16"
        a2b_chip_addresses=("0x68" "0x6a" "0x6c")
    else
        echo "Unsupported board ${tegra_model}, exiting..."
    fi
    ;;
*)
    echo "Error: Unsupported chip ${tegra_model}!"
    exit 1
    ;;
esac

while getopts ":b:d:i:f:n:r:t:" option; do
case ${option} in
b)
    if [ "${OPTARG}" -eq 32 ]; then
        input_sample_size="${OPTARG}"
    else
        fail_print_exit "Sorry, bit depth of ${OPTARG} is currently not supported. Only 32 bit data is supported."
    fi
    ;;
d)
    input_a2b_chip_addresses="${OPTARG}"
    ;;
i)
    input_i2c_interface_id="${OPTARG}"
    ;;
f)
    if [ "${OPTARG}" -lt 6 ] && [ "${OPTARG}" -gt 0 ]; then
        input_a2b_function_mode="${OPTARG}"
    else
        fail_print_exit "Invalid function mode ${OPTARG}!"
    fi
    ;;
#n)
#    fail_print_exit "Sorry, this functionality (-n) is currently not supported."
#    ;;
r)
    if [ ${OPTARG} -eq 48000 ]; then
        input_sample_rate="${OPTARG}"
    else
        fail_print_exit "Sorry, Currently only 48000KHz sampling rate is supported."
    fi
    ;;
t)
    if [ ${OPTARG} -eq 8 ]; then
        input_tdm_ch="${OPTARG}"
    else
        fail_print_exit "Sorry, currently only 8 channels are supported."
    fi
    ;;
*)
    fail_print_exit "ERROR: Invalid option ${option}!"
    exit 1
    ;;
esac
done

if ! [ -z "${input_i2c_interface_id}" ]; then
    i2c_interface_id="${input_i2c_interface_id}"
    echo "I2C Interface ID is ${i2c_interface_id[0]}"
else
    echo "No I2C Interface ID is provided. Selected Default: ${i2c_interface_id}"
fi

if ! [ -z "${input_a2b_chip_addresses}" ]; then
    a2b_chip_addresses=("${input_a2b_chip_addresses}")
    echo -n "A2B Chip Address is "
else
    echo -n "No A2B Chip address is provided. Selected Default: "
    configure_all_devices=1
fi
for address in ${a2b_chip_addresses[@]}
do
    echo -n "$address "
done
echo ""

if ! [ -z "${input_sample_size}" ]; then
    sample_size="${input_sample_size}"
    echo "Sample Size is ${sample_size}"
else
    echo "No Sample Size is provided. Selected Default: ${sample_size}"
fi

if ! [ -z "${input_tdm_ch}" ]; then
    tdm_ch="${input_tdm_ch}"
    echo "Number of channels: ${tdm_ch}"
else
    echo "Number of channels is not provided. Selected Default: ${tdm_ch}"
fi

if ! [ -z "${input_sample_rate}" ]; then
    sample_rate="${input_sample_rate}"
    echo "Sample Rate: ${sample_rate} Hz"
else
    echo "Sample rate is not provided. Selected Default: ${sample_rate} Hz"
fi

if ! [ -z "${input_a2b_function_mode}" ]; then
    a2b_function_mode="${input_a2b_function_mode}"
    echo "A2B Function Mode: ${a2b_function_mode}"
else
    echo "A2B function mode is not provided. Selected Default: ${a2b_function_mode}"
fi

#Loop for multiple Main node of same device type
if [ "${configure_all_devices}" = "1" ]; then
    echo "Since no specific I2C Address were given, configuring all ${#a2b_chip_addresses[@]} A2B Chips on this Board"
fi
for address in ${a2b_chip_addresses[@]}
do
    echo "A2B Main Node config started for A2B Chip Address: ${address}"
    configure_a2b $address
done

exit 0
