#! /bin/bash

SCRIPT=$(readlink -f $0)
SCRIPT_DIR=$(dirname ${SCRIPT})

for SIZE in 1024 512 256 128 64 48 32 16
do
  rsvg -w ${SIZE} -h ${SIZE} Icon.svg Icon${SIZE}x${SIZE}.png
done

function iconFilesOfSize()
{
  PNGS=
  for PNG in $(ls ${SCRIPT_DIR}/*.png);
  do
    VALID_SIZES="$1"
    SIZE=$(convert ${PNG} -print "%w\n" /dev/null)

    if [[ ${VALID_SIZES} =~ ${SIZE} ]];
    then
      PNGS="${PNGS} ${PNG}"
    fi
  done

  echo ${PNGS}
}

icotool -c $(iconFilesOfSize "256 128 64 48 32 16") -o ${SCRIPT_DIR}/Icon.ico
png2icns ${SCRIPT_DIR}/Icon.icns $(iconFilesOfSize "1024 512 256 128 32 16")
