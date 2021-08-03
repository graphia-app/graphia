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

. scripts/defaults.sh

NUM_CORES=$(sysctl -n hw.ncpu)
BUILD_DIR="build"

rm -rf ${BUILD_DIR}
mkdir -p ${BUILD_DIR}

# https://github.com/detroit-labs/safe-xcode-select
# If this is installed, use it to select the default Xcode,
# in case something else has changed it
SAFE_XCODE_SELECT=$(which safe-xcode-select)
if [ ! -z "${SAFE_XCODE_SELECT}" ]
then
  ${SAFE_XCODE_SELECT} /Applications/Xcode.app
  echo "Switched Xcode version to:"
  xcodebuild -version
fi

GCC_TREAT_WARNINGS_AS_ERRORS=NO xcodebuild -project \
  source/thirdparty/breakpad/src/tools/mac/dump_syms/dump_syms.xcodeproj
PATH=$PATH:source/thirdparty/breakpad/src/tools/mac/dump_syms/build/Release

(
  cd ${BUILD_DIR}
  cmake --version || exit $?
  cmake -DCMAKE_UNITY_BUILD=${UNITY_BUILD} -DCMAKE_BUILD_TYPE=${BUILD_TYPE} \
    -DCMAKE_OSX_DEPLOYMENT_TARGET=10.13 \
    -GNinja .. || exit $?
  cat variables.sh
  . variables.sh
  cmake --build . --target all 2>&1 | tee compiler-${VERSION}.log
  [[ "${PIPESTATUS[0]}" -eq 0 ]] || exit ${PIPESTATUS[0]}
)

function makeSymFileFromDsym
{
  SOURCE=$1
  TARGET=$2

  echo "${TARGET}"

  dump_syms ${SOURCE} > ${TARGET} 2> /dev/null || exit $?

  # Remove .sym.dsym from the MODULE name
  sed -e '1s/\.dsym$//' -e '1s/\.sym$//' -i '' ${TARGET}
}

function makeSymFileFromObject
{
  SOURCE=$1
  TARGET=$2

  dsymutil ${SOURCE} -flat -o ${TARGET}.dsym || exit $?
  makeSymFileFromDsym ${TARGET}.dsym ${TARGET}
  rm ${TARGET}.dsym
}

makeSymFileFromObject ${BUILD_DIR}/${PRODUCT_NAME}.app/Contents/MacOS/${PRODUCT_NAME} \
  ${BUILD_DIR}/${PRODUCT_NAME}.sym || exit $?
makeSymFileFromObject ${BUILD_DIR}/source/thirdparty/libthirdparty.dylib \
  ${BUILD_DIR}/libthirdparty.dylib.sym || exit $?

for PLUGIN in $(find ${BUILD_DIR}/plugins -iname "*.dylib")
do
  makeSymFileFromObject ${PLUGIN} ${PLUGIN}.sym || exit $?
done

# Generate symbols for Qt installation
if [[ ! -z "${Qt5_DIR}" ]]
then
  for DSYM_FILE in $(find ${Qt5_DIR} -iname "*.dsym")
  do
    OBJECT_FILE="${DSYM_FILE%.*}"
    SYM_FILE="$BUILD_DIR/Qt${OBJECT_FILE#${Qt5_DIR}}.sym"
    DIR_NAME=$(dirname ${SYM_FILE})
    mkdir -p ${DIR_NAME}
    makeSymFileFromDsym ${DSYM_FILE} ${SYM_FILE}
  done
fi
