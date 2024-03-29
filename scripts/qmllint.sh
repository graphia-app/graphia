#! /bin/bash
#
# Copyright © 2013-2024 Graphia Technologies Ltd.
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

if [ -z ${BUILD_DIR} ]
then
    echo "BUILD_DIR not set"
    exit 1
fi

SOURCE_DIR=$(pwd)
cd ${BUILD_DIR}

. variables.sh

INCLUDE_DIRS=$(find ${SOURCE_DIR}/source -type d -name "qml" | \
    xargs -n1 | sed -e 's/\(.*\)/-I \1/')
QML_DIRS=$(find ${SOURCE_DIR}/source -type f -name "qmldir" | \
    xargs dirname | sed -e 's/\(.*\)/--qmldirs \1/')

if [ -z ${QMLLINT} ]
then
    QMLLINT="qmllint"
fi

${QMLLINT} --version

# Generate a list of commands separately to avoid the problem where xargs stops
# if the thing it is executing crashes (which qmllint has a tendency to do)
COMMANDS=$(find \
    ${SOURCE_DIR}/source/app \
    ${SOURCE_DIR}/source/crashreporter \
    ${SOURCE_DIR}/source/messagebox \
    ${SOURCE_DIR}/source/plugins \
    ${SOURCE_DIR}/source/shared \
    ${SOURCE_DIR}/source/updater \
    -type f -iname "*.qml" | \
    xargs -n1 echo ${QMLLINT} ${INCLUDE_DIRS} ${QML_DIRS})

echo ${COMMANDS} | /bin/bash 2>&1 | tee qmllint-${VERSION}.log

