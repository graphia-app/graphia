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
  COMPILER=$(basename ${CC} | sed -e 's/-.*//g')
  BUILD_DIR="build/${COMPILER}"
fi

BUILD_DIR=$(realpath ${BUILD_DIR})

. ${BUILD_DIR}/variables.sh

# For some reason clang-tidy and clazy can't find various system headers, so we
# determine and add the system include dirs to compile_commands.json here
SYSTEM_INCLUDE_DIRS=$(${CXX} -E -xc++ - -v < /dev/null 2>&1 | \
    sed -n '/^#include </,/^End of search list\./p' | \
    sed -e s'/^\s\+/-isystem/' | sed '1d;$d' | tr '\n' ' ')

cat ${BUILD_DIR}/compile_commands.json | jq ".[].command += \" ${SYSTEM_INCLUDE_DIRS}\"" > \
    _compile_commands.json
mv _compile_commands.json ${BUILD_DIR}/compile_commands.json

cat ${BUILD_DIR}/compile_commands.json | \
    jq "map(select(.file | test(\"qrc_|mocs_compilation|thirdparty|${BUILD_DIR}\") | not))" > \
    _compile_commands.json
mv _compile_commands.json ${BUILD_DIR}/compile_commands.json

cp ${BUILD_DIR}/compile_commands.json ${BUILD_DIR}/compile_commands-${VERSION}.log

CPP_FILES=$(cat ${BUILD_DIR}/compile_commands.json | \
    jq '.[].file' | sed -e 's/"//g')

INCLUDE_DIRS=$(cat ${BUILD_DIR}/compile_commands.json | \
    jq '.[].command' | grep -oP '(?<=-I) *.*?(?= )' | \
    grep -vE "autogen" | \
    sort | uniq | \
    sed -e 's/\(.*\)/-I\1/')

SYSTEM_INCLUDE_DIRS=$(cat ${BUILD_DIR}/compile_commands.json | \
    jq '.[].command' | grep -oP '(?<=-isystem) *.*?(?= )' | \
    sort | uniq | \
    sed -e 's/\(.*\)/-I\1/')

DEFINES=$(cat ${BUILD_DIR}/compile_commands.json | \
    jq '.[].command' | grep -oP '(?<= -D) *.*?(?= -\D)' | \
    sort | uniq | \
    sed -e 's/\\//g' |
    sed -e 's/\(.*\)/-D\1/')

echo -e "export CPP_FILES=\"${CPP_FILES}\"\n" >> ${BUILD_DIR}/variables.sh
echo -e "export INCLUDE_DIRS=\"${INCLUDE_DIRS}\"\n" >> ${BUILD_DIR}/variables.sh
echo -e "export SYSTEM_INCLUDE_DIRS=\"${SYSTEM_INCLUDE_DIRS}\"\n" >> ${BUILD_DIR}/variables.sh
echo -e "export DEFINES=\"${DEFINES}\"\n" >> ${BUILD_DIR}/variables.sh

cat ${BUILD_DIR}/variables.sh
