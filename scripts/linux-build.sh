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
    -DCMAKE_INSTALL_PREFIX=${BUILD_DIR}/AppDir/usr \
    -DCMAKE_BUILD_TYPE=Release -GNinja ../.. || exit $?
  cmake --build . --target install || exit $?

  # Clean intermediate build products
  grep "^rule.*\(_COMPILER_\|_STATIC_LIBRARY_\)" rules.ninja | \
    cut -d' ' -f2 | xargs -n1 ninja -t clean -r

  # Make an AppImage
  LINUXDEPLOYQT=$(which linuxdeployqt)
  if [ ! -z ${LINUXDEPLOYQT} ]
  then
    ${LINUXDEPLOYQT} \
      ${BUILD_DIR}/AppDir/usr/share/applications/${PRODUCT_NAME}.desktop \
      -appimage -no-copy-copyright-files -no-plugins -no-strip
  else
    echo linuxdeployqt could not be found, please install \
      it from https://github.com/probonopd/linuxdeployqt
  fi
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
