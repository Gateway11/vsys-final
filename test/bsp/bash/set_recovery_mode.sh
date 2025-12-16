#!/usr/bin/expect -f
#exp_internal 1

set env(TERM) xterm

set RECOVERY_MODE [lindex $argv 0]
if {![string length $RECOVERY_MODE]} {
	exit;
} else {
    set RECOVERY_MODE [lindex $argv 0]
}

set SERIAL_DEVICE [lindex $argv 1]
if {![string length $SERIAL_DEVICE]} {
    set SERIAL_DEVICE "/dev/ttyUSB2"  ;# 如果没有传参，使用默认值
} else {
    set SERIAL_DEVICE /dev/ttyUSB[lindex $argv 1]
}

set timeout 1

# 启动 minicom
spawn sudo minicom -C minicom_log.txt -D $SERIAL_DEVICE

expect {
    "password for" { send "123456\r" }  ;# 输入密码
    "*\[sudo\]*bsp*密码*" { send "123456\r" }
    timeout { puts ">>> 没有密码提示，继续执行" }
}

# 等待 minicom 提示符
expect "Welcome to minicom"

sleep 0.5
set timeout 1

# 输入指令（例如：重启设备）
send "tegrarecovery ${RECOVERY_MODE}\r"
send "tegrareset\r"

# 结束脚本
expect eof
