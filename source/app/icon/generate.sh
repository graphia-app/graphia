#! /bin/bash

SCRIPT=$(readlink -f $0)
SCRIPT_DIR=$(dirname ${SCRIPT})
ICON_FILE=$1

if [[ -z ${ICON_FILE} ]]
then
  echo Missing icon file parameter!
  exit 1
fi

if [ ! -e ${ICON_FILE} ]
then
  echo ${ICON_FILE} does not exist!
  exit 1
fi

BASE_NAME="${ICON_FILE%.*}"

for SIZE in 1024 512 256 128 64 48 32 16
do
  rsvg -w ${SIZE} -h ${SIZE} ${BASE_NAME}.svg ${BASE_NAME}${SIZE}x${SIZE}.png
done

function iconFilesOfSize()
{
  PNGS=
  for PNG in $(ls ${SCRIPT_DIR}/${BASE_NAME}*.png);
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

icotool -c $(iconFilesOfSize "256 128 64 48 32 16") -o ${SCRIPT_DIR}/${BASE_NAME}.ico
png2icns ${SCRIPT_DIR}/${BASE_NAME}.icns $(iconFilesOfSize "1024 512 256 128 32 16")
