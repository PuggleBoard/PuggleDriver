#!/bin/bash -e

for i in {0..9999}
    do
        echo `printf %04d ${i%.*}`
        echo `printf %04d ${i%.*}` >/dev/pruss7seg
    done

