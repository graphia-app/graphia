#! /bin/bash

# OSX doesn't have readlink, hence this hackery
SCRIPT_DIR=$(cd "$(dirname "$0")"; pwd)

security unlock-keychain -p ${SIGN_BUILD_USER_PASSWORD}
macdeployqt ${PRODUCT_NAME}.app -qmldir=source/ui/qml \
  -executable=${PRODUCT_NAME}.app/Contents/MacOS/${PRODUCT_NAME} \
  -codesign="${SIGN_APPLE_KEYCHAIN_ID}"

cat ${SCRIPT_DIR}/dmg.spec.json.template | sed \
  -e "s/_PRODUCT_NAME_/${PRODUCT_NAME}/g" \
  -e "s|_SCRIPT_DIR_|${SCRIPT_DIR}|g" > \
  dmg.spec.json
rm -f ${PRODUCT_NAME}-${VERSION}.dmg && appdmg dmg.spec.json \
  ${PRODUCT_NAME}-${VERSION}.dmg

rm -rf build
mkdir build
cp -a ${PRODUCT_NAME}.app build
cp -a ${PRODUCT_NAME}-${VERSION}.dmg build
