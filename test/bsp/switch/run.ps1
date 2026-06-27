#.\run.ps1 -TargetChip 5192 -Version "v1.5" -ProductName "acme"
param(
    [ValidateSet('5152', '5192')]
    [string]$TargetChip = '5192',
    [string]$Version = 'v1.0',
    [string]$ProductName = 'eabot'
)

$SwitchTool = ".\AutomotiveSwitchV4.exe"
$SecureImageTool = "..\create_secure_image_v3.04.0001\create_secure_image_v3.exe"

$BaseImage = "88Q5152_flash_7_5_base.bin"
$DirName = "SwitchConfig_${ProductName}_$(Get-Date -Format 'yyyyMMdd')_${Version}"
New-Item -ItemType Directory -Path $DirName -Force

$configs = @(
    @{ChipCpu = "88Q5152_flash_7_5_base.bin"; CfgPrefix = "88Q5152-switch-cfg"; TargetId = 7},
    @{ChipCpu = "5192A-cpu0"; CfgPrefix = "5192A-cpu0-switch-cfg"; TargetId = 6},
    @{ChipCpu = "5192A-cpu1"; CfgPrefix = "5192A-cpu1-switch-cfg"; TargetId = 7},
    @{ChipCpu = "5192B-cpu0"; CfgPrefix = "5192B-cpu0-switch-cfg"; TargetId = 8},
    @{ChipCpu = "5192B-cpu1"; CfgPrefix = "5192B-cpu1-switch-cfg"; TargetId = 9}
)

foreach ($config in $configs) {
    if ($config.ChipCpu -notlike "*$TargetChip*") { continue }
    cp ..\${BaseImage} .
    & $SwitchTool -switchcfg="..\eabot\$($config.CfgPrefix)-${ProductName}.xml" -productioncfg="$($config.CfgPrefix).bin" -targetid="$($config.TargetId)"
    & $SecureImageTool -imagefile="$BaseImage" -partitionoffset=0x100000 -partitionsize=0x0F0000 -cfgimagefile="$($config.CfgPrefix).bin" -acceptanycfg
    & $SecureImageTool -imagefile="$BaseImage" -partitionoffset=0x010000 -partitionsize=0x0F0000 -cfgimagefile="$($config.CfgPrefix).bin" -acceptanycfg
    rm "$($config.CfgPrefix).bin"
    mv ${BaseImage} "${DirName}\$($config.ChipCpu)-${Version}.bin" -Force
}
