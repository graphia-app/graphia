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

CERTIFICATE_P12_FILE=certificate.p12

if [ -n "${APPLE_CERTIFICATE_P12_BASE64}" ]
then
  echo ${APPLE_CERTIFICATE_P12_BASE64} | base64 --decode > ${CERTIFICATE_P12_FILE}

  echo "Creating keychain..."
  security create-keychain -p password.keychain build.keychain || exit $?
  security default-keychain -s build.keychain || exit $?
  security unlock-keychain -p password.keychain build.keychain || exit $?

  echo "Importing certificate into keychain..."
  security import ${CERTIFICATE_P12_FILE} -k build.keychain \
      -P ${APPLE_CERTIFICATE_PASSWORD} -T /usr/bin/codesign || exit $?
  security set-key-partition-list -S apple-tool:,apple: -s -k password.keychain build.keychain || exit $?

  rm -rf ${CERTIFICATE_P12_FILE}
  SIGNING_ENABLED=true
  echo Signing enabled...
fi

BUILD_DIR=build

. ${BUILD_DIR}/variables.sh

# MacOS doesn't have readlink, hence this hackery
SCRIPT_DIR=$(cd "$(dirname "$0")"; pwd)

if [ -n "${SIGNING_ENABLED}" ]
then
  security unlock-keychain -p password.keychain build.keychain
fi

cd ${BUILD_DIR}

mkdir -p ${PRODUCT_NAME}.app/Contents/PlugIns/
cp -r plugins/*.dylib ${PRODUCT_NAME}.app/Contents/PlugIns/

cp CrashReporter ${PRODUCT_NAME}.app/Contents/MacOS/
cp MessageBox ${PRODUCT_NAME}.app/Contents/MacOS/
cp Updater ${PRODUCT_NAME}.app/Contents/MacOS/

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
  -executable=${PRODUCT_NAME}.app/Contents/MacOS/Updater \
  -codesign="${APPLE_CERTIFICATE_ID}"

if [ -n "${SIGNING_ENABLED}" ]
then
  echo "Signing..."
  codesign --verbose --deep --force --options runtime --sign "${APPLE_CERTIFICATE_ID}" \
    ${PRODUCT_NAME}.app || exit $?

  cat <<EOF > QtWebEngineProcess.entitlements
<?xml version="1.0" encoding="UTF-8"?>
<!DOCTYPE plist PUBLIC "-//Apple//DTD PLIST 1.0//EN" "http://www.apple.com/DTDs/PropertyList-1.0.dtd">
<plist version="1.0">
<dict>
  <key>com.apple.security.cs.disable-executable-page-protection</key>
  <true/>
</dict>
</plist>
EOF

  echo "Signing QtWebEngine..."
  codesign --verbose --force --options runtime --sign "${APPLE_CERTIFICATE_ID}" \
    --entitlements QtWebEngineProcess.entitlements \
    ${PRODUCT_NAME}.app/Contents/Frameworks/QtWebEngineCore.framework/Helpers/QtWebEngineProcess.app/Contents/MacOS/QtWebEngineProcess || exit $?

  echo "Resigning main executable..."
  codesign --verbose --force --options runtime --sign "${APPLE_CERTIFICATE_ID}" \
    ${PRODUCT_NAME}.app/Contents/MacOS/${PRODUCT_NAME} || exit $?

  echo "Verifying..."
  codesign --verbose --verify ${PRODUCT_NAME}.app || exit $?
  echo "OK"
fi

cat ${SCRIPT_DIR}/dmg.spec.json.template | sed \
  -e "s/_PRODUCT_NAME_/${PRODUCT_NAME}/g" \
  -e "s|_SCRIPT_DIR_|${SCRIPT_DIR}|g" > \
  dmg.spec.json
rm -f ${PRODUCT_NAME}-${VERSION}.dmg && appdmg dmg.spec.json \
  ${PRODUCT_NAME}-${VERSION}.dmg || exit $?

if [ -z "${APPLE_NOTARIZATION_USERNAME}" ]
then
  echo "No notarization credentials supplied, skipping..."
  exit 0
fi

# Apple notarization
echo "Creating NotarizationProfile..."
xcrun notarytool store-credentials --apple-id "${APPLE_NOTARIZATION_USERNAME}" \
  --password "${APPLE_NOTARIZATION_PASSWORD}" \
  --team-id "${APPLE_TEAM_ID}" "NotarizationProfile" || exit $?

echo "Submitting notarization request..."
xcrun notarytool submit "${PRODUCT_NAME}-${VERSION}.dmg" \
  --keychain-profile "NotarizationProfile" --wait || exit $?

echo "Stapling..."
xcrun stapler staple "${PRODUCT_NAME}-${VERSION}.dmg" || exit $?
