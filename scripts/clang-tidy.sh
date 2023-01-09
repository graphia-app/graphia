#! /bin/bash
#
# Copyright © 2013-2023 Graphia Technologies Ltd.
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

if [ -z ${CLANGTIDY} ]
then
    CLANGTIDY="clang-tidy"
fi

${CLANGTIDY} --version
${CLANGTIDY} -dump-config

parallel -k -n1 -P$(nproc --all) \
  "echo [{#}/{= \$_=total_jobs() =}] && ${CLANGTIDY} -quiet -p . {}" \
  ::: ${CPP_FILES} 2>&1 | tee clang-tidy-${VERSION}.log
