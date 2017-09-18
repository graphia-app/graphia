#! /bin/bash

. scripts/defaults.sh

for ARGUMENT in "$@"
do
  echo -n "Sourcing ${ARGUMENT}"
  if [ -e ${ARGUMENT} ]
  then
    echo "..."
    . ${ARGUMENT}
  else
    echo "...doesn't exist"
  fi
done

NUM_CORES=$(nproc --all)
QMAKE_SPEC=$(qmake -query QMAKE_SPEC)
COMPILER=$(echo ${QMAKE_SPEC} | sed -e 's/linux-//')
BEAR=$(which bear)
BUILD_DIR="build/${QMAKE_SPEC}"
TOP_BUILD_DIR=$(echo ${BUILD_DIR} | cut -d "/" -f1)

${COMPILER} --version
echo "NUM_CORES: ${NUM_CORES}"

rm -rf ${TOP_BUILD_DIR}
mkdir -p ${BUILD_DIR}

(
  cd ${BUILD_DIR}
  qmake -version || exit $?
  qmake ../../GraphTool.pro || exit $?

  if [ ! -z "${BEAR}" ] && [ ${QMAKE_SPEC} = "linux-clang" ]
  then
    BUILD_LOG=${WORKSPACE}/compile_commands.json

    rm -f ${BUILD_LOG}
    echo "Building with ${BEAR} ${BUILD_LOG}"
    ${BEAR} -o ${BUILD_LOG} make -O -j${NUM_CORES} || exit $?
  else
    make -O -j${NUM_CORES} || exit $?
  fi

  # This just removes the intermediate build products
  make clean || exit $?
)

# To get breakpad dump_syms
(
  mkdir -p breakpad-build
  cd breakpad-build
  ../source/thirdparty/breakpad/configure
  make -O -j${NUM_CORES}
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
