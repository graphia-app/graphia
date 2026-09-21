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

if [ -z ${CLANG_TIDY} ]
then
    CLANG_TIDY=$(compgen -c clang-tidy | grep -E '^[^0-9]+[0-9]+$' | sort -V | tail -1)
    CLANG_TIDY=${CLANG_TIDY:-clang-tidy}
fi

${CLANG_TIDY} --version
${CLANG_TIDY} --dump-config

parallel -k -n1 -P$(nproc --all) \
  "echo [{#}/{= \$_=total_jobs() =}] && ${CLANG_TIDY} -quiet -p . {}" \
  ::: ${CPP_FILES} 2>&1 | tee clang-tidy-${VERSION}.log
