#!/usr/bin/env bash

i2c_dev=16
xml_content=$(cat "$1" 2>/dev/null || cat "adi_a2b_commandlist.xml")
actions=$(echo "$xml_content" | grep -Eo '<action[^>]*>.*?</action>|<action[^>]*\s*/>')

debug() { eval echo "Running command $((++line_count)): $1"; eval $1; }

echo "$actions" | while read -r action; do
    instr=$(echo "$action" | sed -n 's/.*instr="\([^"]*\)".*/\1/p')

    #SpiCmd=$(echo "$action" | sed -n 's/.*SpiCmd="\([^"]*\)".*/\1/p')
    #SpiCmdWidth=$(echo "$action" | sed -n 's/.*SpiCmdWidth="\([^"]*\)".*/\1/p')
    addr_width=$(echo "$action" | sed -n 's/.*addr_width="\([^"]*\)".*/\1/p')
    #data_width=$(echo "$action" | sed -n 's/.*data_width="\([^"]*\)".*/\1/p')
    len=$(echo "$action" | sed -n 's/.*len="\([^"]*\)".*/\1/p')
    addr=$(echo "$action" | sed -n 's/.* addr="\([^"]*\)".*/\1/p' | xargs -I {} printf "%0$((addr_width * 2))X" {})
    i2caddr=$(echo "$action" | sed -n 's/.*i2caddr="\([^"]*\)".*/\1/p' | xargs -I {} printf "0x%02X" {})
    #protocol=$(echo "$action" | sed -n 's/.*Protocol="\([^"]*\)".*/\1/p')
    content=$(echo "$action" | sed -n 's/.*>\(.*\)<\/action>/\1/p')

    addr_bytes=""
    if [[ "$instr" != "delay" ]]; then
        for ((i = 0; i < $addr_width; i++)); do addr_bytes+=" 0x${addr:$((i * 2)):2}"; done
    fi

    if [[ "$instr" == "writeXbytes" ]]; then
        content_with_prefix=$(echo "$content" | sed 's/\([^ ]*\)/0x\1/g')
        debug 'i2ctransfer -f -y $i2c_dev w"$addr_width"@"$i2caddr""$addr_bytes" "$content_with_prefix"'
    elif [[ "$instr" == "read" ]]; then
        debug 'i2ctransfer -f -y $i2c_dev w"$addr_width"@"$i2caddr""$addr_bytes" r"$((len - addr_width))"'
    elif [[ "$instr" == "delay" ]]; then
        delay_value=0
        for byte in $content; do delay_value=$(( (delay_value << 8) | (16#$byte) )); done
        delay_sec=$(bc <<< "scale=3; $delay_value / 1000")
        debug 'sleep $delay_sec' #debug 'perl -e "select(undef, undef, undef, $delay_sec)"'
    else
        echo "Unknown instruction: $instr"
    fi
done
