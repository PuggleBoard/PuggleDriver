#!/bin/sh
set -x
set -e
export SLOTS=/sys/devices/bone_capemgr.8/slots
export PINS=/sys/kernel/debug/pinctrl/44e10800.pinmux/pins
dtc -O dtb -o puggle-4ch-00A0.dtbo -b 0 -@ puggle-4ch.dts
cp puggle-4ch-00A0.dtbo /lib/firmware/
cat $SLOTS
echo puggle-4ch > $SLOTS
#dmesg | tail
cat $SLOTS
cat $PINS | grep 884
cat $PINS | grep 8e0
cat $PINS | grep 8e4
cat $PINS | grep 8e8
cat $PINS | grep 8ec
cat $PINS | grep 880
