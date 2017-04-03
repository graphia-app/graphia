#! /bin/bash

. scripts/defaults.sh

BUILD_DIR="build"

rm -rf ${BUILD_DIR}
mkdir -p ${BUILD_DIR}

GCC_TREAT_WARNINGS_AS_ERRORS=NO xcodebuild -project \
  source/thirdparty/breakpad/src/tools/mac/dump_syms/dump_syms.xcodeproj

(
  cd ${BUILD_DIR}
  qmake -version || exit $?
  qmake ../GraphTool.pro || exit $?
  make -j2 || exit $?

  # This just removes the intermediate build products
  make clean || exit $?

  ../installers/osx/build.sh || exit $?
)

source/thirdparty/breakpad/src/tools/mac/dump_syms/build/Release/dump_syms \
  ${BUILD_DIR}/${PRODUCT_NAME}.app/Contents/MacOS/${PRODUCT_NAME} > \
  ${BUILD_DIR}/${PRODUCT_NAME}.sym

for PLUGIN in $(find ${BUILD_DIR}/plugins -name "*.dylib")
do
  source/thirdparty/breakpad/src/tools/mac/dump_syms/build/Release/dump_syms \
    ${PLUGIN} > \
    ${PLUGIN}.sym
done
