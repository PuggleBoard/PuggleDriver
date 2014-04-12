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

if ! id | grep -q root; then
  echo "must be run as root"
	exit
fi

set -x
set -e
export SLOTS=/sys/devices/bone_capemgr.8/slots
dtc -O dtb -o PUGGLE-00A1.dtbo -b 0 -@ PUGGLE-00A1.dts
cp PUGGLE-00A1.dtbo /lib/firmware/
cat $SLOTS
echo PUGGLE:00A1 > $SLOTS
cat $SLOTS
