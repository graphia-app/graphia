#! /bin/bash

COMPILER=$(basename ${CC} | sed -e 's/-.*//g')
BUILD_DIR="build/${COMPILER}"

cd ${BUILD_DIR}

QML_DIRS=$(find ../../source -name "*.qml" | xargs -n1 dirname | \
  sort | uniq | sed -e 's/\(^.*$\)/-qmldir=\1/')

# Make an AppImage
LINUXDEPLOYQT=$(which linuxdeployqt)
if [ ! -z ${LINUXDEPLOYQT} ]
then
  ${LINUXDEPLOYQT} \
    AppDir/usr/share/applications/${PRODUCT_NAME}.desktop \
    ${QML_DIRS} \
    -appimage -no-copy-copyright-files -no-strip \
    -executable=AppDir/usr/bin/CrashReporter \
    -executable=AppDir/usr/bin/MessageBox \
    -extra-plugins=platformthemes/libqgtk3.so,imageformats/libqsvg.so,iconengines/libqsvgicon.so
else
  echo linuxdeployqt could not be found, please install \
    it from https://github.com/probonopd/linuxdeployqt
fi
