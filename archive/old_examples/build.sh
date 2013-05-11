#!/bin/bash -e

make | egrep --color -i '[1-9] error|'

