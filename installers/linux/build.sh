#! /bin/bash

NUM_CORES=$(nproc --all)
COMPILER=$(basename ${CC} | sed -e 's/-.*//g')
BUILD_DIR="build/${COMPILER}"

QML_DIRS=$(find source -name "*.qml" | xargs -n1 realpath | \
  xargs -n1 dirname | sort | uniq | sed -e 's/\(^.*$\)/-qmldir=\1/')

mkdir -p ${BUILD_DIR}/AppDir/usr/share/${PRODUCT_NAME}
cp -r source/app/examples \
  ${BUILD_DIR}/AppDir/usr/share/${PRODUCT_NAME}

# Make an AppImage
LINUXDEPLOYQT=$(which linuxdeployqt)
if [ ! -z ${LINUXDEPLOYQT} ]
then
  (
    cd ${BUILD_DIR}

    ${LINUXDEPLOYQT} \
      AppDir/usr/share/applications/${PRODUCT_NAME}.desktop \
      ${QML_DIRS} \
      -appimage -no-copy-copyright-files -no-strip \
      -executable=AppDir/usr/bin/CrashReporter \
      -executable=AppDir/usr/bin/MessageBox \
      -extra-plugins=platformthemes/libqgtk3.so,imageformats/libqsvg.so,iconengines/libqsvgicon.so \
      || exit $?
  )
else
  echo linuxdeployqt could not be found, please install \
    it from https://github.com/probonopd/linuxdeployqt
  exit 1
fi

# To get breakpad dump_syms
(
  mkdir -p breakpad-build
  cd breakpad-build
  ../source/thirdparty/breakpad/configure
  make -O -j${NUM_CORES}
)

breakpad-build/src/tools/linux/dump_syms/dump_syms \
  ${BUILD_DIR}/AppDir/usr/bin/${PRODUCT_NAME} > \
  ${BUILD_DIR}/${PRODUCT_NAME}.sym || exit $?
breakpad-build/src/tools/linux/dump_syms/dump_syms \
  ${BUILD_DIR}/AppDir/usr/lib/libthirdparty.so > \
  ${BUILD_DIR}/libthirdparty.so.sym || exit $?

for PLUGIN in $(find ${BUILD_DIR}/AppDir/usr/lib/${PRODUCT_NAME}/plugins -iname "*.so")
do
  breakpad-build/src/tools/linux/dump_syms/dump_syms \
    ${PLUGIN} > \
    ${PLUGIN}.sym || exit $?
done
