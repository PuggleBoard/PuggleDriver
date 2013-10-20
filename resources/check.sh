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
set -x
set -e
export PINS=/sys/kernel/debug/pinctrl/44e10800.pinmux/pins
cat $PINS | grep 8a0
cat $PINS | grep 8a4
cat $PINS | grep 8a8
cat $PINS | grep 8ac
cat $PINS | grep 8b0
cat $PINS | grep 8b4
cat $PINS | grep 8b8
cat $PINS | grep 8bc
cat $PINS | grep 8e0
cat $PINS | grep 8e4
cat $PINS | grep 8e8
cat $PINS | grep 8ec

