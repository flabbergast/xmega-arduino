#!/bin/bash -x
#echo RR$1RR RR$2RR RR$3RR RR$4RR RR$5RR RR$6RR RR$7RR
dfu-programmer $1 erase
dfu-programmer $@
dfu-programmer $1 reset
exit 0
