#!/bin/sh

#./busconfig.sh adi_a2b_commandlist.xml; ./make_sub_bus.sh adi_a2b_commandlist.xml jj.xml 0; ./busconfig.sh jj.xml

infile="$1"
outfile="$2"
parent="$3"

awk -v parent="$parent" '
BEGIN {
    # parent 视为十进制 0~15
    p = parent + 0

    node0  = sprintf("%02X", p)
    node20 = sprintf("%02X", p + 32)

    seen_action = 0
    prev = ""
}

function emit(chip) {
    print "    <action instr=\"writeXbytes\" SpiCmd=\"0\" SpiCmdWidth=\"0\" addr_width=\"1\" data_width=\"1\" len=\"2\" addr=\"1\" i2caddr=\"104\" AddrIncr=\"0\" Protocol=\"I2C\" ParamName=\"REG_A2B0_NODEADR\">" node0 "</action>"
    print "    <action instr=\"writeXbytes\" SpiCmd=\"0\" SpiCmdWidth=\"0\" addr_width=\"1\" data_width=\"1\" len=\"2\" addr=\"0\" i2caddr=\"105\" AddrIncr=\"0\" Protocol=\"I2C\" ParamName=\"REG_A2B0_CHIP\">" chip "</action>"
    print "    <action instr=\"writeXbytes\" SpiCmd=\"0\" SpiCmdWidth=\"0\" addr_width=\"1\" data_width=\"1\" len=\"2\" addr=\"1\" i2caddr=\"104\" AddrIncr=\"0\" Protocol=\"I2C\" ParamName=\"REG_A2B0_NODEADR\">" node20 "</action>"
}

{
    sub(/\r$/, "")

    # 非 i2c 行，原样输出
    if ($0 !~ /i2caddr="/) {
        print $0
        next
    }

    # 提取原始 i2caddr（用于逻辑判断）
    curr = $0
    sub(/.*i2caddr="/, "", curr)
    sub(/".*/, "", curr)

    # 第一次 action：无条件插入
    if (!seen_action) {
        seen_action = 1
        emit(68)
    }
    # i2caddr 发生变化
    else if (curr != prev) {
        if (curr == "104")
            emit(68)
        else if (curr == "105")
            emit(69)
    }

    # === 输出内容处理 ===
    if (curr == "104") {
        sub(/i2caddr="104"/, "i2caddr=\"105\"")
    }

    print $0
    prev = curr
}
' "$infile" > "$outfile"
