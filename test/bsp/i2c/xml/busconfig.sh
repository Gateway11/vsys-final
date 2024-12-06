#!/bin/bash

xml_content=$(cat "adi_a2b_commandlist.xml")
actions=$(echo "$xml_content" | grep -Eo '<action[^>]*>.*?</action>|<action[^>]*\s*/>')

echo "$actions" | while read -r action; do
    instr=$(echo "$action" | sed -n 's/.*instr="\([^"]*\)".*/\1/p')
    
    SpiCmd=$(echo "$action" | sed -n 's/.*SpiCmd="\([^"]*\)".*/\1/p')
    SpiCmdWidth=$(echo "$action" | sed -n 's/.*SpiCmdWidth="\([^"]*\)".*/\1/p')
    addr_width=$(echo "$action" | sed -n 's/.*addr_width="\([^"]*\)".*/\1/p')
    data_width=$(echo "$action" | sed -n 's/.*data_width="\([^"]*\)".*/\1/p')
    len=$(echo "$action" | sed -n 's/.*len="\([^"]*\)".*/\1/p')
    addr=$(echo "$action" | sed -n 's/.* addr="\([^"]*\)".*/\1/p')
    i2caddr=$(echo "$action" | sed -n 's/.*i2caddr="\([^"]*\)".*/\1/p')
    Protocol=$(echo "$action" | sed -n 's/.*Protocol="\([^"]*\)".*/\1/p')

    if [[ "$instr" != "read" ]]; then
        content=$(echo "$action" | sed -n 's/.*>\(.*\)<\/action>/\1/p')
        content_with_prefix=$(echo "$content" | sed 's/\([^ ]*\)/0x\1/g')
    else
        content_with_prefix=""
    fi

    hex_addr=$(printf "%02X" "$addr")
    hex_i2caddr=$(printf "%02X" "$i2caddr")

    action_entry="instr: $instr, SpiCmd: $SpiCmd, SpiCmdWidth: $SpiCmdWidth, addr_width: $addr_width, data_width: $data_width, len: $len, addr: 0x$hex_addr, i2caddr: 0x$hex_i2caddr, Protocol: $Protocol, Content: $content_with_prefix"

    if [[ "$instr" == "writeXbytes" ]]; then
        hex_data=$(echo "$content" | sed 's/\(..\)/\1 /g')
        #i2cset -y 16 "$i2caddr" "$addr" $content_with_prefix
    elif [[ "$instr" == "read" ]]; then
        i2cget -y 16 "$i2caddr" "$addr" "$((len - 1))"
    elif [[ "$instr" == "delay" ]]; then
        delay_ms=$((16#$content))
        delay_sec=$(bc <<< "scale=3; $delay_ms / 1000")
        sleep "$delay_sec"
    else
        echo "Unknown instruction: $instr"
    fi

    echo "Action Data:"
    echo "$action_entry"
    echo "-------------------------"
done
