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

BUILD_DIR="build/wasm"

emcc --version

rm -rf ${BUILD_DIR}
mkdir -p ${BUILD_DIR}

(
  cd ${BUILD_DIR}
  cmake --version || exit $?
  cmake -DCMAKE_UNITY_BUILD=${UNITY_BUILD} -DCMAKE_EXPORT_COMPILE_COMMANDS=ON \
    -DQT_HOST_PATH=${QT_ROOT_DIR}/../gcc_64 \
    -DCMAKE_TOOLCHAIN_FILE:FILEPATH=${QT_ROOT_DIR}/lib/cmake/Qt6/qt.toolchain.cmake \
    -DCMAKE_BUILD_TYPE=${BUILD_TYPE} -GNinja ../.. || exit $?
  cat variables.sh
  . variables.sh
  cmake --build . 2>&1 | tee compiler-${VERSION}.log
  [[ "${PIPESTATUS[0]}" -eq 0 ]] || exit ${PIPESTATUS[0]}
)

# Apply branding changes -- this will almost certainly break in future
cp misc/wasm-extras/* ${BUILD_DIR}
mv ${BUILD_DIR}/${PRODUCT_NAME}.html ${BUILD_DIR}/index.html
cp source/app/icon/Icon.svg ${BUILD_DIR}/qtlogo.svg
cp source/app/icon/Icon-favicon.ico ${BUILD_DIR}/favicon.ico

sed -i -e 's/Qt for WebAssembly: Graphia/Graphia for WebAssembly/' ${BUILD_DIR}/index.html
sed -i '/<head>/a <link rel="stylesheet" href="loader.css">' ${BUILD_DIR}/index.html
sed -i -e 's/Loading.../<div class="loader"><\/div>/' ${BUILD_DIR}/index.html
