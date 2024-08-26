adb disconnect 192.168.60.70
adb disconnect 192.168.60.252

@echo off
adb wait-for-device
adb shell touch /data/nfc
if "%errorlevel%"=="1" (
    adb -s a31089dc root & adb -s a31089dc remount
)

ping -n 1 192.168.60.%1
if "%errorlevel%"=="1" (
    echo cmd wifi connect-network "moto X40_5928" wpa3 12345678 > tempfile.txt
    powershell -Command Get-Content .\tempfile.txt| adb -s a31089dc shell
    powershell -Command Get-Content .\tempfile.txt| adb -s 7131e20b shell
)
adb connect 192.168.60.%1

echo log=$(ps -A ^| grep audio.service ^| awk '{print $2}'); kill -9 $log > tempfile.txt
if "%2"=="1" (
    powershell -Command Get-Content .\tempfile.txt| adb -s 192.168.60.%1 shell
    timeout /T 1
)

set NOW_TIME=%date:~0,4%-%date:~5,2%-%date:~8,2%-%time:~0,2%-%time:~3,2%
set NOW_TIME=%NOW_TIME: =0%
MD %NOW_TIME% & CD %NOW_TIME%
adb -s 192.168.60.%1 shell logcat -c
adb -s 192.168.60.%1 shell logcat -c
start "" cmd /c "adb -s 192.168.60.%1 shell logcat | tee log"
rem adb shell diag_mdlog
rem diag_socket_log -a 192.168.60.13 -c 1 &

if "%1"=="252" (
    adb -s 0000000123456789 root & adb -s 0000000123456789 remount
    adb -s 0000000123456789 shell tinyplay /data/48k_16bit_2ch.wav -D 0 -d 2
)
