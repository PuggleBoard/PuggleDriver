##
##	 -------------------------------------------------------------------------
##
##	 This file is part of the Puggle Data Conversion and Processing System
##	 Copyright (C) 2013 Puggle
##
##	 -------------------------------------------------------------------------
##
##	Written in 2013 by: Yogi Patel <yapatel@gatech.edu>
##
##	To the extent possible under law, the author(s) have dedicated all copyright
##	and related and neighboring rights to this software to the public domain
##	worldwide. This software is distributed without any warranty.
##
##	You should have received a copy of the CC Public Domain Dedication along with
##	this software. If not, see <http://creativecommons.org/licenses/by-sa/3.0/legalcode>.
##

#!/bin/sh

## Below is from CDSteinkhuehler's Beaglebone Universal IO repo
## to get around standard method of loading DTO for pinmux
echo "compiling universal device tree overlays..."
dtc -@ -I dts -O dtb -o /lib/firmware/cape-universal-00A0.dtbo cape-universal-00A0.dts
dtc -@ -I dts -O dtb -o /lib/firmware/cape-universaln-00A0.dtbo cape-universaln-00A0.dts
dtc -@ -I dts -O dtb -o /lib/firmware/cape-univ-emmc-00A0.dtbo cape-univ-emmc-00A0.dts
echo "done"
echo "installing config-pin utility"
cp -v config-pin /usr/bin/

echo cape-universal > /sys/devices/bone_capemgr.9/slots

config-pin 9.28 spi
config-pin 9.42 spics
config-pin 9.31 spi
config-pin 9.29 spi
config-pin 9.30 spi
config-pin 9.27 pruout
config-pin 9.24 pruin
config-pin 9.17 spi
config-pin 9.22 spi
config-pin 9.21 spi
config-pin 9.18 spi

#set -x
#set -e
#export SLOTS=/sys/devices/bone_capemgr.9/slots
#export PINS=/sys/kernel/debug/pinctrl/44e10800.pinmux/pins
#dtc -O dtb -o PUGGLEv3-00A0.dtbo -b 0 -@ PUGGLEv3.dts
#cp PUGGLEv3-00A0.dtbo /lib/firmware/
#cat $SLOTS
#echo PUGGLEv3 > $SLOTS
#cat $SLOTS
