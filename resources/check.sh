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

