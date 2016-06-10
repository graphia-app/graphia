#! /bin/bash

# OSX doesn't have readlink, hence this hackery
SCRIPT_DIR=$(cd "$(dirname "$0")"; pwd)

security unlock-keychain -p ${SIGN_BUILD_USER_PASSWORD}

mkdir -p ${PRODUCT_NAME}.app/Contents/PlugIns/
cp -r plugins/*.dylib ${PRODUCT_NAME}.app/PlugIns/

cp CrashReporter.app/Contents/MacOS/CrashReporter \
  ${PRODUCT_NAME}.app/Contents/MacOS/

macdeployqt ${PRODUCT_NAME}.app \
  -qmldir=source/app/ui/qml \
  -qmldir=source/crashreporter \
  -executable=${PRODUCT_NAME}.app/Contents/MacOS/${PRODUCT_NAME} \
  -executable=${PRODUCT_NAME}.app/Contents/MacOS/CrashReporter \
  -codesign="${SIGN_APPLE_KEYCHAIN_ID}"

# Need to sign again because macdeployqt won't sign the CrashReporter
echo "Resigning..."
codesign --verbose --sign "${SIGN_APPLE_KEYCHAIN_ID}" ${PRODUCT_NAME}.app
codesign --verbose --verify ${PRODUCT_NAME}.app || exit $?
echo "OK"

cat ${SCRIPT_DIR}/dmg.spec.json.template | sed \
  -e "s/_PRODUCT_NAME_/${PRODUCT_NAME}/g" \
  -e "s|_SCRIPT_DIR_|${SCRIPT_DIR}|g" > \
  dmg.spec.json
rm -f ${PRODUCT_NAME}-${VERSION}.dmg && appdmg dmg.spec.json \
  ${PRODUCT_NAME}-${VERSION}.dmg

cp -a ${PRODUCT_NAME}.app build
cp -a ${PRODUCT_NAME}-${VERSION}.dmg build
