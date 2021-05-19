#! /bin/bash
#
# Copyright © 2013-2021 Graphia Technologies Ltd.
#
# This file is part of Graphia.
#
# Graphia is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# Graphia is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with Graphia.  If not, see <http://www.gnu.org/licenses/>.
#

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
  inkscape -z -w ${SIZE} -h ${SIZE} ${BASE_NAME}.svg -e ${BASE_NAME}${SIZE}x${SIZE}.png
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
icotool -c ${SCRIPT_DIR}/${BASE_NAME}16x16.png -o ${SCRIPT_DIR}/${BASE_NAME}-favicon.ico
png2icns ${SCRIPT_DIR}/${BASE_NAME}.icns $(iconFilesOfSize "1024 512 256 128 32 16")
