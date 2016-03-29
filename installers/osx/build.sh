#! /bin/bash

security unlock-keychain -p ${SIGN_BUILD_USER_PASSWORD}
macdeployqt ${PRODUCT_NAME}.app -qmldir=source/ui/qml \
    -executable=${PRODUCT_NAME}.app/Contents/MacOS/${PRODUCT_NAME} \
    -codesign="${SIGN_APPLE_KEYCHAIN_ID}"

rm -rf build
mkdir build
cp -a ${PRODUCT_NAME}.app build
