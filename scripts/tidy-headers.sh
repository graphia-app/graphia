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

if [ "$#" -ne 1 ]; then
    echo "Error: Invoke with compile_commands.json." >&2
    echo "Usage: $0 <compile_commands.json>" >&2
    exit 1
fi

COMPILE_COMMANDS="$1"
BUILD_DIR=$(dirname ${COMPILE_COMMANDS})

if [ ! -f "${COMPILE_COMMANDS}" ]; then
    echo "Error: File '${COMPILE_COMMANDS}' does not exist or is not a regular file." >&2
    exit 1
fi

if ! jq empty "${COMPILE_COMMANDS}" 2>/dev/null; then
    echo "Error: '${COMPILE_COMMANDS}' is not parseable by jq (invalid JSON)." >&2
    exit 1
fi

TEMP_FILE=$(mktemp)

cat ${COMPILE_COMMANDS} | \
    jq "map(select(.file | test(\"qrc_|mocs_compilation|thirdparty|${BUILD_DIR}\") | not))" > \
    ${TEMP_FILE}

CPP_FILES=$(cat ${TEMP_FILE} | jq '.[].file' | sed -e 's/"//g')

CONFIG=" \
{ \
    Checks: '-*,misc-include-cleaner', \
    CheckOptions: \
    { \
        IgnoreHeaders: \"\", \
        DeduplicateFindings: true, \
        UnusedIncludes: true, \
        MissingIncludes: true, \
    } \
}"

echo ${CPP_FILES} | xargs -n1 clang-tidy -p ${BUILD_DIR} -config="${CONFIG}"