#! /bin/bash

. scripts/defaults.sh

rm -rf build
mkdir -p build

GCC_TREAT_WARNINGS_AS_ERRORS=NO xcodebuild -project \
  source/thirdparty/breakpad/src/tools/mac/dump_syms/dump_syms.xcodeproj
make distclean
qmake -version || exit $?
qmake GraphTool.pro || exit $?
make clean || exit $?
make -j2 || exit $?

installers/osx/build.sh || exit $?

source/thirdparty/breakpad/src/tools/mac/dump_syms/build/Release/dump_syms \
  ${PRODUCT_NAME}.app/Contents/MacOS/${PRODUCT_NAME} > \
  ${PRODUCT_NAME}.sym

for PLUGIN in $(find plugins -name "*.dylib")
do
  source/thirdparty/breakpad/src/tools/mac/dump_syms/build/Release/dump_syms \
    ${PLUGIN} > \
    ${PLUGIN}.sym

done

cp -a ${PRODUCT_NAME}.sym build/
cp -a plugins/*.sym build/
