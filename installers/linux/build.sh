#! /bin/bash
#
# Copyright © 2013-2021 Graphia Technologies Ltd.
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

QML_DIRS=$(find source -name "*.qml" | xargs -n1 realpath | \
  xargs -n1 dirname | sort | uniq | sed -e 's/\(^.*$\)/-qmldir=\1/')

mkdir -p ${BUILD_DIR}/AppDir/usr/share/${PRODUCT_NAME}
cp -r source/app/examples \
  ${BUILD_DIR}/AppDir/usr/share/${PRODUCT_NAME}

# Make an AppImage
LINUXDEPLOYQT=$(which linuxdeployqt)

# Look in current directory if not installed in the system
if [ -z ${LINUXDEPLOYQT} ]
then
  if [ -f "${PWD}/linuxdeployqt" ]
  then
    LINUXDEPLOYQT="${PWD}/linuxdeployqt"
  fi
fi

if [ ! -z ${LINUXDEPLOYQT} ]
then
  (
    cd ${BUILD_DIR}

    # Below -exclude-libs line is a work around for https://github.com/probonopd/linuxdeployqt/issues/35

    ${LINUXDEPLOYQT} \
      AppDir/usr/share/applications/${PRODUCT_NAME}.desktop \
      ${QML_DIRS} \
      -appimage -no-copy-copyright-files -no-strip \
      -executable=AppDir/usr/bin/CrashReporter \
      -executable=AppDir/usr/bin/MessageBox \
      -executable=AppDir/usr/bin/Updater \
      -extra-plugins=platformthemes/libqgtk3.so,imageformats/libqsvg.so,iconengines/libqsvgicon.so \
      -exclude-libs=libnss3.so,libnssutil3.so \
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
else
  echo linuxdeployqt could not be found, please install \
    it from https://github.com/probonopd/linuxdeployqt
  exit 1
fi

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
