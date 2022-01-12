#! /bin/bash
#
# Copyright © 2013-2022 Graphia Technologies Ltd.
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

. scripts/defaults.sh

for ARGUMENT in "$@"
do
  echo -n "Sourcing ${ARGUMENT}"
  if [ -e ${ARGUMENT} ]
  then
    echo "..."
    . ${ARGUMENT}
  else
    echo "...doesn't exist"
  fi
done

NUM_CORES=$(nproc --all)
COMPILER=$(basename ${CC} | sed -e 's/-.*//g')
BUILD_DIR="build/${COMPILER}"

${CXX} --version
echo "NUM_CORES: ${NUM_CORES}"

rm -rf ${BUILD_DIR}
mkdir -p ${BUILD_DIR}

(
  cd ${BUILD_DIR}
  cmake --version || exit $?
  cmake -DCMAKE_UNITY_BUILD=${UNITY_BUILD} -DCMAKE_EXPORT_COMPILE_COMMANDS=ON \
    -DCMAKE_INSTALL_PREFIX=AppDir/usr \
    -DCMAKE_BUILD_TYPE=${BUILD_TYPE} -GNinja ../.. || exit $?
  cat variables.sh
  . variables.sh
  cmake --build . --target install 2>&1 | tee compiler-${VERSION}.log
  [[ "${PIPESTATUS[0]}" -eq 0 ]] || exit ${PIPESTATUS[0]}
)
