#! /bin/bash

SCRIPT=$(readlink -f $0)
SCRIPT_DIR=$(dirname ${SCRIPT})

icotool -c $(ls -S ${SCRIPT_DIR}/Icon*.png) -o ${SCRIPT_DIR}/Icon.ico

ICNS_PNGS=
for PNG in $(ls ${SCRIPT_DIR}/*.png);
do
  VALID_SIZES="1024 512 256 128 32 16"
  SIZE=$(convert ${PNG} -print "%w\n" /dev/null)

  if [[ ${VALID_SIZES} =~ ${SIZE} ]];
  then
    ICNS_PNGS="${ICNS_PNGS} ${PNG}"
  fi
done

png2icns ${SCRIPT_DIR}/Icon.icns ${ICNS_PNGS}
