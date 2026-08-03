#!/usr/bin/env bash

spi_dev=${2:-"/dev/spidev7.0"}
xml_content=$(cat "$1" 2>/dev/null || cat "adi_a2b_commandlist.xml")
actions=$(echo "$xml_content" | grep -Eo '<action[^>]*>.*?</action>|<action[^>]*\s*/>')

debug() { printf 'Running command %d:' "$((++line_count))"; printf ' %s' "$@"; printf '\n'; "$@"; }

echo "$actions" | while read -r action; do
    instr=$(echo "${action#*instr=\"}" | cut -d'"' -f1)
    content=$(echo "${action#*\">}" | cut -d'<' -f1)

    if [[ "$instr" != "delay" ]]; then
        addr_width=$(echo "${action#*addr_width=\"}" | cut -d'"' -f1)
        len=$(echo "${action#*len=\"}" | cut -d'"' -f1)
        addr=$(printf "%0$((addr_width * 2))X" "$(echo "${action#* addr=\"}" | cut -d'"' -f1)")
        i2caddr=$(printf "0x%02X" "$(echo "${action#*i2caddr=\"}" | cut -d'"' -f1)")
        spiCmd=$(printf "%02X" "$(echo "${action#*SpiCmd=\"}" | cut -d'"' -f1)")
        spiCmdWidth=$(printf "0x%02X" "$(echo "${action#*SpiCmdWidth=\"}" | cut -d'"' -f1)")

        addr_bytes=""; for ((i = 0; i < $addr_width; i++)); do addr_bytes+="\x${addr:$((i * 2)):2}"; done
        dummy=""; for i in $(seq 1 "$len"); do dummy="${dummy}\\x00"; done

        case "$spiCmd" in
            00|02|06)
                debug spidev_test -D "$spi_dev" -s 1000000 -b 8 -v -p "\x$spiCmd$addr_bytes$(echo $content | sed 's/\([^ ]*\)/\\\x\1/g')"
                ;;
            01|04)
                sleep 0.02
                debug spidev_test -D "$spi_dev" -s 1000000 -b 8 -v -p "\x$spiCmd$addr_bytes\\x$(printf '%02X' "$((len - 1))")$dummy"
                ;;
            05)
                # Slave register read request
                debug spidev_test -D "$spi_dev" -s 1000000 -b 8 -v -p "\x$(printf '%02X' "$((0xC0 | (len - 1)))")$addr_bytes"
                debug sleep 0.02
                debug spidev_test -D "$spi_dev" -s 1000000 -b 8 -v -p "\x$spiCmd$dummy"
                ;;
            07)
                # Remote I2C peripheral write
                debug spidev_test -D "$spi_dev" -s 1000000 -b 8 -v -p "\x02$addr_bytes$(echo $content | sed 's/\([^ ]*\)/\\\x\1/g')"
                debug sleep 0.02
                debug spidev_test -D "$spi_dev" -s 1000000 -b 8 -v -p "\x$spiCmd$addr_bytes$(echo $content | sed 's/\([^ ]*\)/\\\x\1/g')"
                ;;
            08)
                # Remote I2C peripheral read request
                debug spidev_test -D "$spi_dev" -s 1000000 -b 8 -v -p "\x02$addr_bytes$(echo $content | sed 's/\([^ ]*\)/\\\x\1/g')"
                debug sleep 0.02
                debug spidev_test -D "$spi_dev" -s 1000000 -b 8 -v -p "\x07$addr_bytes$(echo $content | sed 's/\([^ ]*\)/\\\x\1/g')"
                debug sleep 0.02
                debug spidev_test -D "$spi_dev" -s 1000000 -b 8 -v -p "\x08$addr_bytes\\x$(printf '%02X' "$((len - 1))")"
                debug sleep 0.02
                debug spidev_test -D "$spi_dev" -s 1000000 -b 8 -v -p "\x05$dummy"
                ;;
            *)
                echo "Unknown read command: $spiCmd"
                ;;
        esac
    else
        delay_value=0
        for byte in $content; do delay_value=$(( (delay_value << 8) | (16#$byte) )); done
        delay_sec=$(bc <<< "scale=3; $delay_value / 1000")
        debug sleep "$delay_sec"
    fi
done
