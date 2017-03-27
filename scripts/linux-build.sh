#! /bin/bash

. defaults.sh

QMAKE_SPEC=$(qmake -query QMAKE_SPEC)
BEAR=$(which bear)

rm -rf build
mkdir -p build/plugins/

# To get breakpad dump_syms
mkdir -p breakpad-build
cd breakpad-build
../source/thirdparty/breakpad/configure
make -O -j2
cd ..

make distclean
qmake -version || exit $?
qmake GraphTool.pro || exit $?
make clean || exit $?

if [ ! -z "${BEAR}" && ${QMAKE_SPEC} == "linux-clang" ]
then
  rm -f compile_command.json
  bear make -O -j2 || exit $?
else
  make -O -j2 || exit $?
fi

cp ${PRODUCT_NAME} build/${PRODUCT_NAME}.${QMAKE_SPEC}
breakpad-build/src/tools/linux/dump_syms/dump_syms \
  build/${PRODUCT_NAME}.${QMAKE_SPEC} > \
  build/${PRODUCT_NAME}.${QMAKE_SPEC}.sym

for PLUGIN in $(find plugins -iname "*.so")
do
  cp ${PLUGIN} build/${PLUGIN}.${QMAKE_SPEC}
  breakpad-build/src/tools/linux/dump_syms/dump_syms \
    build/${PLUGIN}.${QMAKE_SPEC} > \
    build/${PLUGIN}.${QMAKE_SPEC}.sym

done
