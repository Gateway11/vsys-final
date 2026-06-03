$SwitchTool = "..\automotive_switch_config_win_v4.18.0000_ENGINEERING_VERSION\AutomotiveSwitchV4.exe"
$SecureImageTool = "..\create_secure_image_v3.04.0001\create_secure_image_v3.exe"

$Version = "v1.0"
$CompanyName = "eabot"
$DirName = "SwitchConfig_${CompanyName}_$(Get-Date -Format 'yyyyMMdd')_${Version}"
New-Item -ItemType Directory -Path $DirName -Force

cp ..\88Q5152_flash_7_5_base.bin .
$CfgPrefix = "5192A-cpu0-swicth-cfg"
& $SwitchTool -switchcfg="${CfgPrefix}-${CompanyName}.xml" -productioncfg="${CfgPrefix}.bin" -targetid=6
& $SecureImageTool -imagefile="88Q5152_flash_7_5_base.bin" -partitionoffset=0x100000 -partitionsize=0x0F0000 -cfgimagefile="${CfgPrefix}.bin" -acceptanycfg
& $SecureImageTool -imagefile="88Q5152_flash_7_5_base.bin" -partitionoffset=0x010000 -partitionsize=0x0F0000 -cfgimagefile="${CfgPrefix}.bin" -acceptanycfg
mv 88Q5152_flash_7_5_base.bin ${DirName}\5192A-cpu0-${Version}.bin
pause

cp ..\88Q5152_flash_7_5_base.bin .
$CfgPrefix = "5192A-cpu1-swicth-cfg"
& $SwitchTool -switchcfg="${CfgPrefix}-${CompanyName}.xml" -productioncfg="${CfgPrefix}.bin" -targetid=7
& $SecureImageTool -imagefile="88Q5152_flash_7_5_base.bin" -partitionoffset=0x100000 -partitionsize=0x0F0000 -cfgimagefile="${CfgPrefix}.bin" -acceptanycfg
& $SecureImageTool -imagefile="88Q5152_flash_7_5_base.bin" -partitionoffset=0x010000 -partitionsize=0x0F0000 -cfgimagefile="${CfgPrefix}.bin" -acceptanycfg
mv 88Q5152_flash_7_5_base.bin ${DirName}\5192A-cpu1-${Version}.bin

cp ..\88Q5152_flash_7_5_base.bin .
$CfgPrefix = "5192B-cpu0-swicth-cfg"
& $SwitchTool -switchcfg="${CfgPrefix}-${CompanyName}.xml" -productioncfg="${CfgPrefix}.bin" -targetid=8
& $SecureImageTool -imagefile="88Q5152_flash_7_5_base.bin" -partitionoffset=0x100000 -partitionsize=0x0F0000 -cfgimagefile="${CfgPrefix}.bin" -acceptanycfg
& $SecureImageTool -imagefile="88Q5152_flash_7_5_base.bin" -partitionoffset=0x010000 -partitionsize=0x0F0000 -cfgimagefile="${CfgPrefix}.bin" -acceptanycfg
mv 88Q5152_flash_7_5_base.bin ${DirName}\5192B-cpu0-${Version}.bin

cp ..\88Q5152_flash_7_5_base.bin .
$CfgPrefix = "5192B-cpu1-swicth-cfg"
& $SwitchTool -switchcfg="${CfgPrefix}-${CompanyName}.xml" -productioncfg="${CfgPrefix}.bin" -targetid=9
& $SecureImageTool -imagefile="88Q5152_flash_7_5_base.bin" -partitionoffset=0x100000 -partitionsize=0x0F0000 -cfgimagefile="${CfgPrefix}.bin" -acceptanycfg
& $SecureImageTool -imagefile="88Q5152_flash_7_5_base.bin" -partitionoffset=0x010000 -partitionsize=0x0F0000 -cfgimagefile="${CfgPrefix}.bin" -acceptanycfg
mv 88Q5152_flash_7_5_base.bin ${DirName}\5192B-cpu1-${Version}.bin
