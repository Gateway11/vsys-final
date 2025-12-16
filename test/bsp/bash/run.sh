#!/usr/bin/env bash

ssh -J xuwy@10.106.224.114 <username>@10.106.250.32
ssh xuwy@10.106.224.114 "tar cz -C /home/xuwy/<username> flash.sh" | ssh bsp@192.168.1.200 "tar xz -C /home/bsp/<username>"
ssh xuwy@10.106.224.114 "cat /home/xuwy/<username>/board_a22_a.tar.gz" | ssh bsp@192.168.1.200 "cat > /home/bsp/<username>/board_a22_a.tar.gz"

scp -J zhangcy52@192.168.1.100,xuwy@10.106.224.114 record.wav <username>@10.106.250.32:/drives/c/Users/<username>/record.wav

function copy
{
    cat $1 | nc -q 0 10.106.224.114 5556
}

function copy2
{
    count=0
    tar -cf - -C $(dirname $1) $(basename $1) | xz -9 --extreme | base64 | split -b $((1000 * 1024)) - /tmp/part_
    for part in /tmp/part_*; do
	    xclip -sel clip < $part || cat $part | nc -q 0 10.106.224.114 5556
        read -t 20 -n1 -r -p "($((++count)) / $(ls /tmp/part_* | wc -l | tr -d ' ')) Press any key to continue..."
    done
    rm -f /tmp/part_* output.txt && echo "All parts processed and deleted."
}
