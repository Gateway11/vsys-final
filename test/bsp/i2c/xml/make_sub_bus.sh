#!/bin/sh
#./busconfig.sh adi_a2b_commandlist.xml; ./make_sub_bus.sh adi_a2b_commandlist.xml 0; ./busconfig.sh output.xml

infile=${1:-"adi_a2b_commandlist_acc.xml"}
outfile="output.xml"
parent=${2:-0}

awk -v p="$parent" '
BEGIN {
    node0  = sprintf("%02X", p + 0)
    node20 = sprintf("%02X", p + 32)
    prev = ""
}

function emit(chip) {
    print "    <action instr=\"writeXbytes\" SpiCmd=\"0\" SpiCmdWidth=\"0\" addr_width=\"1\" data_width=\"1\" len=\"2\" addr=\"1\" i2caddr=\"104\" AddrIncr=\"0\" Protocol=\"I2C\" ParamName=\"REG_A2B0_NODEADR\">" node0 "</action>"
    print "    <action instr=\"writeXbytes\" SpiCmd=\"0\" SpiCmdWidth=\"0\" addr_width=\"1\" data_width=\"1\" len=\"2\" addr=\"0\" i2caddr=\"105\" AddrIncr=\"0\" Protocol=\"I2C\" ParamName=\"REG_A2B0_CHIP\">" chip "</action>"
    print "    <action instr=\"writeXbytes\" SpiCmd=\"0\" SpiCmdWidth=\"0\" addr_width=\"1\" data_width=\"1\" len=\"2\" addr=\"1\" i2caddr=\"104\" AddrIncr=\"0\" Protocol=\"I2C\" ParamName=\"REG_A2B0_NODEADR\">" node20 "</action>"
}

{
    sub(/\r$/, "")
    if (match($0, /i2caddr="[^"]+"/)) {
        curr = substr($0, RSTART + 9, RLENGTH - 10)

        # 1. 第一次 action(prev="")：无条件插入; 2. i2caddr 发生变化
        if (curr != prev) {
            emit(curr - 36)
            prev = curr
        }

        sub(/i2caddr="104"/, "i2caddr=\"105\"")
    }

    print $0
}
' "$infile" > "$outfile"
