$SwitchTool = "..\automotive_switch_config_win_v4.18.0000_ENGINEERING_VERSION\AutomotiveSwitchV4.exe"
$SecureImageTool = "..\create_secure_image_v3.04.0001\create_secure_image_v3.exe"

$Version = "v1.0"
$CompanyName = "eabot"
$BaseImage = "88Q5152_flash_7_5_base.bin"
$DirName = "SwitchConfig_${CompanyName}_$(Get-Date -Format 'yyyyMMdd')_${Version}"
New-Item -ItemType Directory -Path $DirName -Force

cp ..\${BaseImage} .
$CfgPrefix = "5192A-cpu0-swicth-cfg"
& $SwitchTool -switchcfg="${CfgPrefix}-${CompanyName}.xml" -productioncfg="${CfgPrefix}.bin" -targetid=6
& $SecureImageTool -imagefile="${BaseImage}" -partitionoffset=0x100000 -partitionsize=0x0F0000 -cfgimagefile="${CfgPrefix}.bin" -acceptanycfg
& $SecureImageTool -imagefile="${BaseImage}" -partitionoffset=0x010000 -partitionsize=0x0F0000 -cfgimagefile="${CfgPrefix}.bin" -acceptanycfg
mv ${BaseImage} ${DirName}\5192A-cpu0-${Version}.bin
pause

cp ..\${BaseImage} .
$CfgPrefix = "5192A-cpu1-swicth-cfg"
& $SwitchTool -switchcfg="${CfgPrefix}-${CompanyName}.xml" -productioncfg="${CfgPrefix}.bin" -targetid=7
& $SecureImageTool -imagefile="${BaseImage}" -partitionoffset=0x100000 -partitionsize=0x0F0000 -cfgimagefile="${CfgPrefix}.bin" -acceptanycfg
& $SecureImageTool -imagefile="${BaseImage}" -partitionoffset=0x010000 -partitionsize=0x0F0000 -cfgimagefile="${CfgPrefix}.bin" -acceptanycfg
mv ${BaseImage} ${DirName}\5192A-cpu1-${Version}.bin

cp ..\${BaseImage} .
$CfgPrefix = "5192B-cpu0-swicth-cfg"
& $SwitchTool -switchcfg="${CfgPrefix}-${CompanyName}.xml" -productioncfg="${CfgPrefix}.bin" -targetid=8
& $SecureImageTool -imagefile="${BaseImage}" -partitionoffset=0x100000 -partitionsize=0x0F0000 -cfgimagefile="${CfgPrefix}.bin" -acceptanycfg
& $SecureImageTool -imagefile="${BaseImage}" -partitionoffset=0x010000 -partitionsize=0x0F0000 -cfgimagefile="${CfgPrefix}.bin" -acceptanycfg
mv ${BaseImage} ${DirName}\5192B-cpu0-${Version}.bin

cp ..\${BaseImage} .
$CfgPrefix = "5192B-cpu1-swicth-cfg"
& $SwitchTool -switchcfg="${CfgPrefix}-${CompanyName}.xml" -productioncfg="${CfgPrefix}.bin" -targetid=9
& $SecureImageTool -imagefile="${BaseImage}" -partitionoffset=0x100000 -partitionsize=0x0F0000 -cfgimagefile="${CfgPrefix}.bin" -acceptanycfg
& $SecureImageTool -imagefile="${BaseImage}" -partitionoffset=0x010000 -partitionsize=0x0F0000 -cfgimagefile="${CfgPrefix}.bin" -acceptanycfg
mv ${BaseImage} ${DirName}\5192B-cpu1-${Version}.bin

#$configs = @(
#    @{ChipCpu = "5192A-cpu0"; CfgPrefix = "5192A-cpu0-swicth-cfg"; TargetId = 6},
#    @{ChipCpu = "5192A-cpu1"; CfgPrefix = "5192A-cpu1-swicth-cfg"; TargetId = 7},
#    @{ChipCpu = "5192B-cpu0"; CfgPrefix = "5192B-cpu0-swicth-cfg"; TargetId = 8},
#    @{ChipCpu = "5192B-cpu1"; CfgPrefix = "5192B-cpu1-swicth-cfg"; TargetId = 9}
#)
#
## 循环处理每个配置
#foreach ($config in $configs) {
#    cp ..\${BaseImage} .
#    & $SwitchTool -switchcfg="$($config.CfgPrefix)-${CompanyName}.xml" -productioncfg="$($config.CfgPrefix).bin" -targetid=$($config.TargetId)
#    & $SecureImageTool -imagefile="$BaseImage" -partitionoffset=0x100000 -partitionsize=0x0F0000 -cfgimagefile="$($config.CfgPrefix).bin" -acceptanycfg
#    & $SecureImageTool -imagefile="$BaseImage" -partitionoffset=0x010000 -partitionsize=0x0F0000 -cfgimagefile="$($config.CfgPrefix).bin" -acceptanycfg
#    mv ${BaseImage} "${DirName}\$($config.ChipCpu)-${Version}.bin"
#}
