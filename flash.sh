#!/bin/sh

HEXFILE="Check.hex"

if [ -z ${HEXFILE+x} ]
then
    echo -n "ERROR: variable HEXFILE unset, uncomment exactly _one_ of the lines at top of the script"
    exit 0
fi

openocd -f board/stm32f0discovery.cfg -f ./openocd/stm32f0-openocd-hex.cfg -c "stm_flash `pwd`/build/$HEXFILE" -c shutdown

exit 1
