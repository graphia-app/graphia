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

(
cd ${BUILD_DIR}
patch -p0 <<'EOF'
--- index.html
+++ index.html
@@ -1,6 +1,8 @@
 <!doctype html>
 <html lang="en-us">
   <head>
+    <script src="coi-serviceworker.js"></script>
+    <link rel="stylesheet" href="loader.css">
     <meta charset="utf-8">
     <meta http-equiv="Content-Type" content="text/html; charset=utf-8">

@@ -20,7 +22,7 @@
     <figure style="overflow:visible;" id="qtspinner">
       <center style="margin-top:1.5em; line-height:150%">
         <img src="qtlogo.svg" width="320" height="200" style="display:block"></img>
-        <strong>Qt for WebAssembly: Graphia</strong>
+        <strong>Graphia for WebAssembly</strong>
         <div id="qtstatus"></div>
         <noscript>JavaScript is disabled. Please enable JavaScript to use this application.</noscript>
       </center>
@@ -43,13 +45,17 @@

             try {
                 showUi(spinner);
-                status.innerHTML = 'Loading...';
+                status.classList.add('loader');

                 const instance = await qtLoad({
                     qt: {
                         onLoaded: () => showUi(screen),
                         onExit: exitData =>
                         {
+                            if(status.classList.contains('loader')) {
+                                status.classList.remove('loader');
+                            }
+
                             status.innerHTML = 'Application exit';
                             status.innerHTML +=
                                 exitData.code !== undefined ? ` with code ${exitData.code}` : '';
EOF
)
