#! /bin/bash

. scripts/defaults.sh

NUM_CORES=$(sysctl -n hw.ncpu)
BUILD_DIR="build"

rm -rf ${BUILD_DIR}
mkdir -p ${BUILD_DIR}

GCC_TREAT_WARNINGS_AS_ERRORS=NO xcodebuild -project \
  source/thirdparty/breakpad/src/tools/mac/dump_syms/dump_syms.xcodeproj

(
  cd ${BUILD_DIR}
  qmake -version || exit $?
  qmake ../GraphTool.pro || exit $?
  make -j${NUM_CORES} || exit $?
)

function makeSymFile
{
  SOURCE=$1
  TARGET=$2

  dsymutil ${SOURCE} -flat -o ${TARGET}.dsym || exit $?
  source/thirdparty/breakpad/src/tools/mac/dump_syms/build/Release/dump_syms \
    ${TARGET}.dsym > ${TARGET} || exit $?
  rm ${TARGET}.dsym

  # Remove .sym.dsym from the MODULE name
  sed -e '1s/\.dsym$//' -e '1s/\.sym$//' -i '' ${TARGET}
}

makeSymFile ${BUILD_DIR}/${PRODUCT_NAME}.app/Contents/MacOS/${PRODUCT_NAME} \
  ${BUILD_DIR}/${PRODUCT_NAME}.sym

for PLUGIN in $(find ${BUILD_DIR}/plugins -name "*.dylib")
do
  makeSymFile ${PLUGIN} ${PLUGIN}.sym || exit $?
done

(
  cd ${BUILD_DIR}

  # This just removes the intermediate build products
  make clean || exit $?
)
