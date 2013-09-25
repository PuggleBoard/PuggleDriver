#!/bin/sh

set -x
set -e
export SLOTS=/sys/devices/bone_capemgr.8/slots
export PINS=/sys/kernel/debug/pinctrl/44e10800.pinmux/pins
dtc -O dtb -o puggle_4ch-00A0.dtbo -b 0 -@ puggle_4ch.dts
cp puggle_4ch-00A0.dtbo /lib/firmware/
cat $SLOTS
echo puggle_4ch > $SLOTS
dmesg | tail
cat $SLOTS
cat $PINS | grep 0E0
