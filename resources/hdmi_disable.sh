#!/bin/bash

echo "Mounting partition"
mount /dev/mmcblk0p1 /mnt/card
sed -i 's/$/ capemgr.disable_partno=BB-BONELT-HDMI,BB-BONELT-HDMIN/' /mnt/card/uEnv.txt
echo "Unmounting partition"
echo "Rebooting"
shutdown -r now
