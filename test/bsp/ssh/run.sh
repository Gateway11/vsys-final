#!/usr/bin/env bash

ssh -J xuwy@10.106.224.114 <username>@10.106.250.32
ssh xuwy@10.106.224.114 "tar cz -C /home/xuwy/<username> flash.sh" | ssh bsp@192.168.1.200 "tar xz -C /home/bsp/<username>"
ssh xuwy@10.106.224.114 "cat /home/xuwy/<username>/board_a22_a.tar.gz" | ssh bsp@192.168.1.200 "cat > /home/bsp/<username>/board_a22_a.tar.gz"

scp -J zhangcy52@192.168.1.100,xuwy@10.106.224.114 record.wav <username>@10.106.250.32:/drives/c/Users/<username>/record.wav
