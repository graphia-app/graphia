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
BEAR=$(which bear)
BUILD_DIR="build/${CC}"
TOP_BUILD_DIR=$(echo ${BUILD_DIR} | cut -d "/" -f1)

${CXX} --version
echo "NUM_CORES: ${NUM_CORES}"

rm -rf ${BUILD_DIR}
mkdir -p ${BUILD_DIR}

(
  cd ${BUILD_DIR}
  cmake --version || exit $?
  cmake -DCMAKE_BUILD_TYPE=Release -GNinja ../.. || exit $?

  if [ ! -z "${BEAR}" ] && [ ${CC} = "clang" ]
  then
    BUILD_LOG=${WORKSPACE}/compile_commands.json

    rm -f ${BUILD_LOG}
    echo "Building with ${BEAR} ${BUILD_LOG}"
    ${BEAR} -o ${BUILD_LOG} cmake --build . --target all || exit $?
  else
    cmake --build . --target all || exit $?
  fi

  # This just removes the intermediate build products
  #FIXME make clean || exit $?
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
