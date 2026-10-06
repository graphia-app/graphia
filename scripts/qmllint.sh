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

if [ -z ${BUILD_DIR} ]
then
    echo "BUILD_DIR not set"
    exit 1
fi

cd ${BUILD_DIR}

. variables.sh

if [ -z ${QMLLINT} ]
then
    QMLLINT="qmllint"
fi

${QMLLINT} --version

# Some modules use others without declaring them as dependencies (e.g. plugins
# use the app's modules, which are only available together at runtime), so add
# the import path of every module in the build
IMPORT_DIRS=$(find $(pwd) -name qmldir -not -path "*/+*" | while read QMLDIR
do
    MODULE_PATH=$(sed -n 's/^module //p' ${QMLDIR} | tr '.' '/')
    dirname ${QMLDIR} | sed -e "s|/${MODULE_PATH}$||"
done | sort -u)

# qt_add_qml_module writes the arguments qmllint needs for each module (import
# paths, resources and the module's files) to .rcc/qmllint/<target>.rsp; lint
# each file separately, so that if qmllint crashes (which it has a tendency to
# do) the remaining files are still linted
OPTIONS_DIR=$(mktemp -d)

for RSP in $(find . -path "*/.rcc/qmllint/*.rsp" \
    -not -name "*_json.rsp" -not -name "*_module.rsp" | sort)
do
    OPTIONS="${OPTIONS_DIR}/$(basename ${RSP})"
    grep -v '\.qml$' ${RSP} > ${OPTIONS}

    for IMPORT_DIR in ${IMPORT_DIRS}
    do
        echo -e "-I\n${IMPORT_DIR}" >> ${OPTIONS}
    done

    for QML_FILE in $(grep '\.qml$' ${RSP})
    do
        ${QMLLINT} @${OPTIONS} ${QML_FILE}
    done
done 2>&1 | tee qmllint-${VERSION}.log

rm -rf ${OPTIONS_DIR}
