#!/bin/bash

line_count=0
xml_content=$(cat "adi_a2b_commandlist.xml")
actions=$(echo "$xml_content" | grep -Eo '<action[^>]*>.*?</action>|<action[^>]*\s*/>')

function debug {
    eval echo "Running command $((++line_count)): $1"; eval $1
}

echo "$actions" | while read -r action; do
    instr=$(echo "$action" | sed -n 's/.*instr="\([^"]*\)".*/\1/p')
    
    #SpiCmd=$(echo "$action" | sed -n 's/.*SpiCmd="\([^"]*\)".*/\1/p')
    #SpiCmdWidth=$(echo "$action" | sed -n 's/.*SpiCmdWidth="\([^"]*\)".*/\1/p')
    #addr_width=$(echo "$action" | sed -n 's/.*addr_width="\([^"]*\)".*/\1/p')
    #data_width=$(echo "$action" | sed -n 's/.*data_width="\([^"]*\)".*/\1/p')
    len=$(echo "$action" | sed -n 's/.*len="\([^"]*\)".*/\1/p')
    addr=$(echo "$action" | sed -n 's/.* addr="\([^"]*\)".*/\1/p')
    i2caddr=$(echo "$action" | sed -n 's/.*i2caddr="\([^"]*\)".*/\1/p')
    #Protocol=$(echo "$action" | sed -n 's/.*Protocol="\([^"]*\)".*/\1/p')

    content=$(echo "$action" | sed -n 's/.*>\(.*\)<\/action>/\1/p')
    content_with_prefix=$(echo "$content" | sed 's/\([^ ]*\)/0x\1/g')

    hex_addr=$(echo $(printf "%02X" "$addr") | sed 's/\([^ ]*\)/0x\1/g')
    hex_i2caddr=$(echo $(printf "%02X" "$i2caddr") | sed 's/\([^ ]*\)/0x\1/g')

    if [[ "$instr" == "writeXbytes" ]]; then
        debug 'i2cset -y 16 "$hex_i2caddr" "$hex_addr" $content_with_prefix'
    elif [[ "$instr" == "read" ]]; then
        debug 'i2cget -y 16 "$hex_i2caddr" "$hex_addr" "$((len - 1))"'
    elif [[ "$instr" == "delay" ]]; then
        delay_sec=$(bc <<< "scale=3; $((16#$content)) / 1000")
        debug 'perl -e "select(undef, undef, undef, $delay_sec)"'
    else
        echo "Unknown instruction: $instr"
    fi
done
