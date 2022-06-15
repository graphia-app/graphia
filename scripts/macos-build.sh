#! /bin/bash
#
# Copyright © 2013-2022 Graphia Technologies Ltd.
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

# Hackily copy the Qt.labs.platform versions of the menu components over
# the top of the implementation that will actually be deployed; the correct
# way to do this is at the CMake level using qt_add_qml_module to dynamically
# create the relevant .qrc and qmldir files, but that's a chunk of work that
# there is no time for at the moment (FIXME)
for FILE in PlatformMenuBar PlatformMenuItem PlatformMenu PlatformMenuSeparator
do
  cp source/shared/ui/qml/app/graphia/Shared/Controls/${FILE}_labs.qml \
    source/shared/ui/qml/app/graphia/Shared/Controls/${FILE}.qml
done

(
  cd ${BUILD_DIR}
  cmake --version || exit $?
  cmake -DCMAKE_UNITY_BUILD=${UNITY_BUILD} -DCMAKE_BUILD_TYPE=${BUILD_TYPE} \
    -DCMAKE_OSX_DEPLOYMENT_TARGET=10.14 \
    -GNinja .. || exit $?
  cat variables.sh
  . variables.sh
  cmake --build . --target all 2>&1 | tee compiler-${VERSION}.log
  [[ "${PIPESTATUS[0]}" -eq 0 ]] || exit ${PIPESTATUS[0]}
)

function makeSymFile
{
  SOURCE=$1
  TARGET=$2

  dsymutil ${SOURCE} -flat -o ${TARGET}.dsym || exit $?
  source/thirdparty/breakpad/src/tools/mac/dump_syms/build/Release/dump_syms \
    ${TARGET}.dsym > ${TARGET} 2> /dev/null || exit $?
  rm ${TARGET}.dsym

  # Remove .sym.dsym from the MODULE name
  sed -e '1s/\.dsym$//' -e '1s/\.sym$//' -i '' ${TARGET}
}

makeSymFile ${BUILD_DIR}/${PRODUCT_NAME}.app/Contents/MacOS/${PRODUCT_NAME} \
  ${BUILD_DIR}/${PRODUCT_NAME}.sym
makeSymFile ${BUILD_DIR}/source/thirdparty/libthirdparty.dylib \
  ${BUILD_DIR}/libthirdparty.dylib.sym

for PLUGIN in $(find ${BUILD_DIR}/plugins -name "*.dylib")
do
  makeSymFile ${PLUGIN} ${PLUGIN}.sym || exit $?
done
