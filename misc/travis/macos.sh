#! /usr/bin/env sh
#
# Copyright © 2013-2020 Graphia Technologies Ltd.
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

export PATH=/usr/local/opt/qt5/bin:$PATH

CERTIFICATE_P12_FILE=certificate.p12

echo ${APPLE_CERTIFICATE_P12_BASE64} | base64 --decode > ${CERTIFICATE_P12_FILE}

security create-keychain -p ${APPLE_KEYCHAIN_PASSWORD} build.keychain
security default-keychain -s build.keychain
security unlock-keychain -p ${APPLE_KEYCHAIN_PASSWORD} build.keychain
security import ${CERTIFICATE_P12_FILE} -k build.keychain \
    -P ${APPLE_CERTIFICATE_PASSWORD} -T /usr/bin/codesign
security set-key-partition-list -S apple-tool:,apple: -s -k ${APPLE_KEYCHAIN_PASSWORD} build.keychain

rm -rf *.p12

scripts/macos-build.sh
installers/macos/build.sh
