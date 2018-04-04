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
COMPILER=$(basename ${CC} | sed -e 's/-.*//g')
BUILD_DIR="build/${COMPILER}"

${CXX} --version
echo "NUM_CORES: ${NUM_CORES}"

rm -rf ${BUILD_DIR}
mkdir -p ${BUILD_DIR}

(
  cd ${BUILD_DIR}
  cmake --version || exit $?
  cmake -DUNITY_BUILD=ON -DCMAKE_EXPORT_COMPILE_COMMANDS=ON \
    -DCMAKE_INSTALL_PREFIX=AppDir/usr \
    -DCMAKE_BUILD_TYPE=Release -GNinja ../.. || exit $?
  cmake --build . --target install || exit $?

  # Clean intermediate build products
  grep "^rule.*\(_COMPILER_\|_STATIC_LIBRARY_\)" rules.ninja | \
    cut -d' ' -f2 | xargs -n1 ninja -t clean -r
)
