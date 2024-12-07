#!/usr/bin/env bash

i2c_dev=16

line_count=0
debug() { eval echo "Running command $((++line_count)): $1" && eval $1; }

xml_content=$(cat "adi_a2b_commandlist.xml")
actions=$(echo "$xml_content" | grep -Eo '<action[^>]*>.*?</action>|<action[^>]*\s*/>')

echo "$actions" | while read -r action; do
    instr=$(echo "$action" | sed -n 's/.*instr="\([^"]*\)".*/\1/p')

    #SpiCmd=$(echo "$action" | sed -n 's/.*SpiCmd="\([^"]*\)".*/\1/p')
    #SpiCmdWidth=$(echo "$action" | sed -n 's/.*SpiCmdWidth="\([^"]*\)".*/\1/p')
    #addr_width=$(echo "$action" | sed -n 's/.*addr_width="\([^"]*\)".*/\1/p')
    #data_width=$(echo "$action" | sed -n 's/.*data_width="\([^"]*\)".*/\1/p')
    len=$(echo "$action" | sed -n 's/.*len="\([^"]*\)".*/\1/p')
    addr=$(echo "$action" | sed -n 's/.* addr="\([^"]*\)".*/\1/p' | xargs -I {} printf "0x%02X" {})
    i2caddr=$(echo "$action" | sed -n 's/.*i2caddr="\([^"]*\)".*/\1/p' | xargs -I {} printf "0x%02X" {})
    #Protocol=$(echo "$action" | sed -n 's/.*Protocol="\([^"]*\)".*/\1/p')

    content=$(echo "$action" | sed -n 's/.*>\(.*\)<\/action>/\1/p')

    if [[ "$instr" == "writeXbytes" ]]; then
        content_with_prefix=$(echo "$content" | sed 's/\([^ ]*\)/0x\1/g')
        debug 'i2cset -y $i2c_dev "$i2caddr" "$addr" $content_with_prefix'
    elif [[ "$instr" == "read" ]]; then
        debug 'i2cget -y $i2c_dev "$i2caddr" "$addr" "$((len - 1))"'
    elif [[ "$instr" == "delay" ]]; then
        delay_sec=$(bc <<< "scale=3; $((16#$content)) / 1000")
        debug 'perl -e "select(undef, undef, undef, $delay_sec)"'
    else
        echo "Unknown instruction: $instr"
    fi
done
