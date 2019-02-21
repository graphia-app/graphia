#! /bin/bash

BUILD_DIR=build

# OSX doesn't have readlink, hence this hackery
SCRIPT_DIR=$(cd "$(dirname "$0")"; pwd)

security unlock-keychain -p ${SIGN_BUILD_USER_PASSWORD}

cd ${BUILD_DIR}

mkdir -p ${PRODUCT_NAME}.app/Contents/PlugIns/
cp -r plugins/*.dylib ${PRODUCT_NAME}.app/Contents/PlugIns/

cp CrashReporter ${PRODUCT_NAME}.app/Contents/MacOS/
cp MessageBox ${PRODUCT_NAME}.app/Contents/MacOS/

mkdir -p ${PRODUCT_NAME}.app/Contents/Resources
cp -r ../source/app/examples \
  ${PRODUCT_NAME}.app/Contents/Resources

QML_DIRS=$(find ../source -name "*.qml" | xargs -n1 dirname | \
  sort | uniq | sed -e 's/\(^.*$\)/-qmldir=\1/')

# Any dylibs that we build ourselves will have an embedded RPATH that refers to
# the absolute Qt directory on the build machine, and the GateKeeper objects to
# this, so we manually remove them here
DYLIBS=$(find source -name "*.dylib")

for DYLIB in ${DYLIBS}
do
  RPATHS=$(otool -l ${DYLIB} | grep -A2 LC_RPATH | \
    grep "^ *path.*" | sed -e 's/^ *path \([^\(]*\).*$/\1/')

  for RPATH in ${RPATHS}
  do
    echo Removing RPATH ${RPATH} from ${DYLIB}...
    install_name_tool -delete_rpath ${RPATH} ${DYLIB}
  done
done

macdeployqt ${PRODUCT_NAME}.app \
  ${QML_DIRS} \
  -executable=${PRODUCT_NAME}.app/Contents/MacOS/${PRODUCT_NAME} \
  -executable=${PRODUCT_NAME}.app/Contents/MacOS/CrashReporter \
  -executable=${PRODUCT_NAME}.app/Contents/MacOS/MessageBox \
  -codesign="${SIGN_APPLE_KEYCHAIN_ID}"

# Need to sign again because macdeployqt won't sign the CrashReporter
echo "Resigning..."
codesign --verbose --force --sign "${SIGN_APPLE_KEYCHAIN_ID}" ${PRODUCT_NAME}.app
echo "Result $? Verifying..."
codesign --verbose --verify ${PRODUCT_NAME}.app || exit $?
echo "OK"

cat ${SCRIPT_DIR}/dmg.spec.json.template | sed \
  -e "s/_PRODUCT_NAME_/${PRODUCT_NAME}/g" \
  -e "s|_SCRIPT_DIR_|${SCRIPT_DIR}|g" > \
  dmg.spec.json
rm -f ${PRODUCT_NAME}-${VERSION}.dmg && appdmg dmg.spec.json \
  ${PRODUCT_NAME}-${VERSION}.dmg
