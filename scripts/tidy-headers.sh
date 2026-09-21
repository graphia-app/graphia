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

if [ "$#" -lt 1 ]; then
    echo "Error: Missing arguments." >&2
    echo "Usage: $0 <compile_commands.json> [file1.cpp file2.cpp ...]" >&2
    exit 1
fi

COMPILE_COMMANDS="$1"
shift

if [ ! -f "${COMPILE_COMMANDS}" ]; then
    echo "Error: File '${COMPILE_COMMANDS}' does not exist or is not a regular file." >&2
    exit 1
fi

jq empty "$COMPILE_COMMANDS" 2>/dev/null
if [ $? -ne 0 ]; then
    echo "Error: '${COMPILE_COMMANDS}' is not a valid JSON file." >&2
    exit 1
fi

BUILD_DIR=$(dirname "${COMPILE_COMMANDS}")

if [ "$#" -eq 0 ]; then
    mapfile -t SOURCE_FILES < <(jq -r \
        "map(select(.file | test(\"qrc_|mocs_compilation|thirdparty|${BUILD_DIR}\") | not)) | .[].file" \
        "${COMPILE_COMMANDS}")
else
    SOURCE_FILES=("$@")
fi

for FILE in "${SOURCE_FILES[@]}"; do
    if [ ! -f "$FILE" ]; then
        echo "Error: File '$FILE' does not exist." >&2
        exit 1
    fi
done

CONFIG='{
    "Checks": "-*,misc-include-cleaner",
    "CheckOptions": {
        "misc-include-cleaner.IgnoreHeaders": "",
        "misc-include-cleaner.DeduplicateFindings": "true",
        "misc-include-cleaner.UnusedIncludes": "true",
        "misc-include-cleaner.MissingIncludes": "false"
    }
}'

CLANG_TIDY=$(compgen -c clang-tidy | grep -E '^[^0-9]+[0-9]+$' | sort -V | tail -1)
CLANG_TIDY=${CLANG_TIDY:-clang-tidy}

if command -v "$CLANG_TIDY" >/dev/null 2>&1 && "$CLANG_TIDY" --version >/dev/null 2>&1; then
    echo "Using $CLANG_TIDY"
else
    echo "Error: No working clang-tidy found." >&2
    exit 1
fi

SOURCE_DIR="${SOURCE_FILES[0]%/*}"

for FILE in "${SOURCE_FILES[@]}"; do
    while [[ "$FILE" != "${SOURCE_DIR}"/* ]]; do
        SOURCE_DIR="${SOURCE_DIR%/*}"
    done
done

# Find all the headers too
mapfile -t -O "${#SOURCE_FILES[@]}" SOURCE_FILES < <( \
    find $SOURCE_DIR -type f -iname "*.h" ! -path "${SOURCE_DIR}/thirdparty/*")

for FILE in "${SOURCE_FILES[@]}"; do
    echo ${FILE}
    $CLANG_TIDY -p ${BUILD_DIR} -config="${CONFIG}" ${FILE} -fix
done