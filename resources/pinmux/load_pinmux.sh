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

echo "Compiling universal device tree overlays..."
dtc -I dts -O dtb -o cape-puggle-00A0.dtbo cape-puggle-00A0.dts
cp -vf cape-puggle-00A0.dtbo /lib/firmware/
echo "...done"
echo "Installing config-pin utility"
cp -vf config-pin /usr/bin/
echo "...done"

# Update to initrd
sudo update-initramfs -uk `uname -r` 

# Load overlay
sudo sh -c "echo 'cape-puggle' > /sys/devices/platform/bone_capemgr/slots"

# Configure pins
config-pin 9.28 spi
config-pin 9.42 spics
config-pin 9.31 spi
config-pin 9.29 spi
config-pin 9.30 spi
config-pin 9.24 pruin
config-pin 9.27 pruout
config-pin 9.17 spi
config-pin 9.22 spi
config-pin 9.21 spi
config-pin 9.18 spi

# Check configuration
config-pin -q 9.28
config-pin -q 9.42 
config-pin -q 9.31 
config-pin -q 9.29 
config-pin -q 9.30
config-pin -q 9.24
config-pin -q 9.27
config-pin -q 9.17
config-pin -q 9.22
config-pin -q 9.21
config-pin -q 9.18
