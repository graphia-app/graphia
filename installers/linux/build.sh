#! /bin/bash
#
# Copyright © 2013-2025 Tim Angus
# Copyright © 2013-2025 Tom Freeman
#
# This file is part of Graphia.
#
# Graphia is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# Graphia is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with Graphia.  If not, see <http://www.gnu.org/licenses/>.
#

NUM_CORES=$(nproc --all)
COMPILER=$(basename ${CC} | sed -e 's/-.*//g')
BUILD_DIR="build/${COMPILER}"

. ${BUILD_DIR}/variables.sh

export QML_SOURCES_PATHS="${PWD}/source"
export DEPLOY_PLATFORM_THEMES=1 # For platformthemes/libqgtk3.so
export DEPLOY_GTK_VERSION=3
export LD_LIBRARY_PATH="AppDir/usr/lib${LD_LIBRARY_PATH:+:$LD_LIBRARY_PATH}"

mkdir -p ${BUILD_DIR}/AppDir/usr/share/${PRODUCT_NAME}
cp -r source/app/examples \
  ${BUILD_DIR}/AppDir/usr/share/${PRODUCT_NAME}

curl -L \
    -O https://github.com/linuxdeploy/linuxdeploy/releases/download/continuous/linuxdeploy-x86_64.AppImage \
    -O https://github.com/linuxdeploy/linuxdeploy-plugin-qt/releases/download/continuous/linuxdeploy-plugin-qt-x86_64.AppImage \
    -O https://github.com/linuxdeploy/linuxdeploy-plugin-checkrt/releases/download/continuous/linuxdeploy-plugin-checkrt-x86_64.sh \
    -O https://raw.githubusercontent.com/linuxdeploy/linuxdeploy-plugin-gtk/master/linuxdeploy-plugin-gtk.sh || exit $?
chmod +x linuxdeploy*

LINUXDEPLOY=${PWD}/linuxdeploy-x86_64.AppImage

# Make an AppImage
(
  cd ${BUILD_DIR}

  LIBRARIES=$(find AppDir/usr/lib -type f -iname "*.so" | \
    sed -e 's/\(^.*$\)/--library \1/')

  ${LINUXDEPLOY} \
    --desktop-file AppDir/usr/share/applications/${PRODUCT_NAME}.desktop \
    ${LIBRARIES} \
    --executable AppDir/usr/bin/Graphia \
    --appdir AppDir \
    --plugin qt \
    --plugin checkrt \
    --plugin gtk \
    || exit $?

  # Manually exclude libnss since linuxdeploy --exclude-library doesn't, for whatever reason
  # See also: https://github.com/probonopd/linuxdeployqt/issues/35
  rm AppDir/usr/lib/libnss*

  ${LINUXDEPLOY} \
    --desktop-file AppDir/usr/share/applications/${PRODUCT_NAME}.desktop \
    --appdir AppDir \
    --output appimage \
    --exclude-library "*libnss*" \
    || exit $?

  for APPIMAGE_FILENAME in $(find -iname "*.AppImage")
  do
    # Remove everything after the dash
    SIMPLIFIED_FILENAME=$(echo ${APPIMAGE_FILENAME} | \
      perl -pe 's/([^-]+)(?:[-\.\w]+)+(\.\w+)/$1$2/g')

    echo "${APPIMAGE_FILENAME} -> ${SIMPLIFIED_FILENAME}"
    mv ${APPIMAGE_FILENAME} ${SIMPLIFIED_FILENAME}
  done

  # tar up the AppImage in a file that includes the version in its name
  tar cvfz ${PRODUCT_NAME}-${VERSION}.tar.gz *.AppImage || exit $?

  rm *.AppImage
) || exit $?

# To get breakpad dump_syms
(
  mkdir -p breakpad-build
  cd breakpad-build
  ../source/thirdparty/breakpad/configure CXXFLAGS="-Wno-deprecated"
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
