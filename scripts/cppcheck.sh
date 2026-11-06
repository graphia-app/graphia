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

SOURCE_DIR=$(pwd)
cd ${BUILD_DIR}

. variables.sh

if [ -z ${CPPCHECK} ]
then
  CPPCHECK="cppcheck"
fi

${CPPCHECK} --version
${CPPCHECK} --check-config ${INCLUDE_DIRS} ${DEFINES} ${CPP_FILES}
${CPPCHECK} --enable=all --xml \
  --check-level=exhaustive \
  --suppress=*:*/source/thirdparty/* \
  --suppress=missingIncludeSystem \
  --suppress=missingInclude \
  --inline-suppr \
  -i ${SOURCE_DIR}/source/tests/ \
  --library=qt \
  ${INCLUDE_DIRS} ${DEFINES} \
  ${CPP_FILES} 2>&1 | tee cppcheck-${VERSION}.log
