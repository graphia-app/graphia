#! /bin/bash

. scripts/defaults.sh

QMAKE_SPEC=$(qmake -query QMAKE_SPEC)
BEAR=$(which bear)
BUILD_DIR="build/${QMAKE_SPEC}"
TOP_BUILD_DIR=$(echo ${BUILD_DIR} | cut -d "/" -f1)

rm -rf ${TOP_BUILD_DIR}
mkdir -p ${BUILD_DIR}

(
  cd ${BUILD_DIR}
  qmake -version || exit $?
  qmake ../../GraphTool.pro || exit $?

  if [ ! -z "${BEAR}" ] && [ ${QMAKE_SPEC} = "linux-clang" ]
  then
    rm -f compile_command.json
    bear make -O -j2 || exit $?
  else
    make -O -j2 || exit $?
  fi

  # This just removes the intermediate build products
  make clean || exit $?
)

# To get breakpad dump_syms
(
  mkdir -p breakpad-build
  cd breakpad-build
  ../source/thirdparty/breakpad/configure
  make -O -j2
)

breakpad-build/src/tools/linux/dump_syms/dump_syms \
  ${BUILD_DIR}/${PRODUCT_NAME} > \
  ${BUILD_DIR}/${PRODUCT_NAME}.sym || exit $?

for PLUGIN in $(find ${BUILD_DIR}/plugins -iname "*.so")
do
  breakpad-build/src/tools/linux/dump_syms/dump_syms \
    ${PLUGIN} > \
    ${PLUGIN}.sym || exit $?
done
