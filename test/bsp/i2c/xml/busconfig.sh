#!/usr/bin/env bash

dev=${2:-"/dev/spidev3.0"}
debug() { echo "Running command $((++line_count)): $@"; "$@"; }

grep '<action' "${1:-adi_a2b_commandlist_spi.xml}" | while read -r action; do
    instr=$(echo "${action#*instr=\"}" | cut -d'"' -f1)
    content=$(echo "${action#*\">}" | cut -d'<' -f1 | tr -d '\r')
    [[ "$instr" == "delay" ]] && { debug sleep $(bc <<< "scale=3; $((16#${content// /})) / 1000"); continue; }

    addr_width=$(echo "${action#*addr_width=\"}" | cut -d'"' -f1)
    addr=$(printf "%0$((addr_width * 2))X" "$(echo "${action#* addr=\"}" | cut -d'"' -f1)")
    addr_bytes=""; for ((i = 0; i < $addr_width; i++)); do addr_bytes+="\x${addr:$((i * 2)):2}"; done
    len=$(echo "${action#*len=\"}" | cut -d'"' -f1)

    if [[ "${action#*Protocol=\"}" == I2C* ]]; then
        i2caddr=$(printf "0x%02X" "$(echo "${action#*i2caddr=\"}" | cut -d'"' -f1)")
        [[ "$instr" != "read" ]] && { \
            debug i2ctransfer -f -y $dev w$len@$i2caddr${addr_bytes//\\x/ 0x} 0x${content// / 0x} || true; } || \
            debug i2ctransfer -f -y $dev w$addr_width@$i2caddr${addr_bytes//\\x/ 0x} r$((len - addr_width))
    else
        spiCmdWidth=$(echo "${action#*SpiCmdWidth=\"}" | cut -d'"' -f1)
        spiCmd=$(printf "%0$((spiCmdWidth * 2))X" "$(echo "${action#*SpiCmd=\"}" | cut -d'"' -f1)")
        spi_cmd_bytes=""; for ((i = 0; i < $spiCmdWidth; i++)); do spi_cmd_bytes+="\x${spiCmd:$((i * 2)):2}"; done
        dummy=""; for ((i=0; i<len-addr_width; i++)); do dummy="${dummy}\\x00"; done
        case "$spiCmd" in #2/7 MOSI  | 0x02 | NODE | ADDR | DATA[0] | ... | DATA[N-1] |
            00|02*|07|C*) #0   MOSI  | 0x00 | ADDR | DATA[0] | ... | DATA[N-1] |        #C* MOSI  | 110b:LEN-1 | NODE | ADDR |
                debug spidev_test -D $dev -s 1000000 -b 8 -v -p $spi_cmd_bytes$addr_bytes${content:+\\x${content// /\\x}} ;;
            01|04)  #MOSI  | 0x01 | ADDR | LEN-1 | DUMMY   | ... | DUMMY     |
                debug spidev_test -D $dev -s 1000000 -b 8 -v -p $spi_cmd_bytes$addr_bytes$dummy ;;
            05)     #MOSI  | 0x05 | DUMMY   | ... | DUMMY     |
                debug spidev_test -D $dev -s 1000000 -b 8 -v -p $spi_cmd_bytes$dummy ;;
            *)
                echo "Unknown spi protocol: $spiCmd" ;;
        esac
    fi
done
