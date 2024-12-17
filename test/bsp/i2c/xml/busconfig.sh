#!/usr/bin/env bash

i2c_dev=16
xml_content=$(cat "$1" 2>/dev/null || cat "adi_a2b_commandlist.xml")
actions=$(echo "$xml_content" | grep -Eo '<action[^>]*>.*?</action>|<action[^>]*\s*/>')

debug() { eval echo "Running command $((++line_count)): $1"; eval $1; }

echo "$actions" | while read -r action; do
    instr=$(echo "${action#*instr=\"}" | cut -d'"' -f1)
    content=$(echo "${action#*\">}" | cut -d'<' -f1)

    if [[ "$instr" != "delay" ]]; then
        #SpiCmd=$(echo "$action" | sed -n 's/.*SpiCmd="\([^"]*\)".*/\1/p')
        #SpiCmdWidth=$(echo "$action" | sed -n 's/.*SpiCmdWidth="\([^"]*\)".*/\1/p')
        addr_width=$(echo "${action#*addr_width=\"}" | cut -d'"' -f1)
        #data_width=$(echo "${action#*data_width=\"}" | cut -d'"' -f1)
        len=$(echo "${action#*len=\"}" | cut -d'"' -f1)
        addr=$(printf "%0$((addr_width * 2))X" "$(echo "${action#* addr=\"}" | cut -d'"' -f1)")
        i2caddr=$(printf "0x%02X" "$(echo "${action#*i2caddr=\"}" | cut -d'"' -f1)")
        #protocol=$(echo "$action" | sed -n 's/.*Protocol="\([^"]*\)".*/\1/p')

        addr_bytes=""
        for ((i = 0; i < $addr_width; i++)); do addr_bytes+=" 0x${addr:$((i * 2)):2}"; done
    fi

    if [[ "$instr" == "writeXbytes" ]]; then
        content_with_prefix=$(echo "$content" | sed 's/\([^ ]*\)/0x\1/g')
        debug 'i2ctransfer -f -y $i2c_dev w$len@"$i2caddr"$addr_bytes $content_with_prefix'
    elif [[ "$instr" == "read" ]]; then
        debug 'i2ctransfer -f -y $i2c_dev w"$addr_width"@"$i2caddr"$addr_bytes r"$((len - addr_width))"'
    elif [[ "$instr" == "delay" ]]; then
        delay_value=0
        for byte in $content; do delay_value=$(( (delay_value << 8) | (16#$byte) )); done
        delay_sec=$(bc <<< "scale=3; $delay_value / 1000")
        debug 'sleep $delay_sec' #debug 'perl -e "select(undef, undef, undef, $delay_sec)"'
    else
        echo "Unknown instruction: $instr"
    fi
done
