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

#!/bin/bash

if ! id | grep -q root; then
  echo "must be run as root"
	exit
fi

ntpdate -b -s -u pool.ntp.org
date
