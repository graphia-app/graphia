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

# Unpacks the AppImage built by build.sh, checks it reaches its main window, then cleans up
# Needs an X display (e.g. xvfb-run) and xdotool

COMPILER=$(basename ${CC} | sed -e 's/-.*//g')
BUILD_DIR="build/${COMPILER}"

. ${BUILD_DIR}/variables.sh

ARCHIVE="${PWD}/${BUILD_DIR}/${PRODUCT_NAME}-${VERSION}.tar.gz"
TEST_DIR=$(mktemp -d)
STARTUP_TIMEOUT_SECONDS=120

# Includes anything the executable spawns, e.g. QtWebEngineProcess or CrashReporter
stopInstalledProcesses()
{
  pkill -KILL -f "${TEST_DIR}/" 2> /dev/null

  while pgrep -f "${TEST_DIR}/" > /dev/null
  do
    sleep 0.1
  done
}

cleanUp()
{
  echo "------ cleaning up"
  stopInstalledProcesses
  rm -rf "${TEST_DIR}"
}

trap cleanUp EXIT

visibleWindowTitles()
{
  for WINDOW in $(xdotool search --onlyvisible --name '.*' 2> /dev/null)
  do
    xdotool getwindowname ${WINDOW} 2> /dev/null
  done | sort -u | sed -e 's/^/"/' -e 's/$/"/' | paste -sd ',' | sed -e 's/,/, /g'
}

echo "------ unpacking ${ARCHIVE} to ${TEST_DIR}"
tar -xzf "${ARCHIVE}" -C "${TEST_DIR}" || exit $?

# Extracting rather than mounting avoids needing FUSE
(cd "${TEST_DIR}" && "./${PRODUCT_NAME}.AppImage" --appimage-extract > /dev/null) || exit $?

echo "------ starting ${PRODUCT_NAME}"
"${TEST_DIR}/squashfs-root/AppRun" --dontUpdate --skipWelcome \
  > "${TEST_DIR}/stdout.txt" 2> "${TEST_DIR}/stderr.txt" &
PID=$!

FAILURE=""
DEADLINE=$((SECONDS + STARTUP_TIMEOUT_SECONDS))

while true
do
  if ! kill -0 ${PID} 2> /dev/null
  then
    # e.g. 127 means a library couldn't be found
    wait ${PID}
    FAILURE="exited with code $?"
    break
  fi

  # Error message boxes are also windows, so check the title rather than just its presence;
  # the main window's is the product name, after the open tab's (e.g. the first run tutorial)
  if xdotool search --onlyvisible --name "^(.+ - )?${PRODUCT_NAME}\$" > /dev/null 2>&1
  then
    break
  fi

  if [ ${SECONDS} -ge ${DEADLINE} ]
  then
    TITLES=$(visibleWindowTitles)

    if [ -n "${TITLES}" ]
    then
      FAILURE="showed windows titled ${TITLES} instead of its main window"
    else
      FAILURE="showed no main window within ${STARTUP_TIMEOUT_SECONDS}s"
    fi

    break
  fi

  sleep 0.5
done

if [ -n "${FAILURE}" ]
then
  echo "${PRODUCT_NAME} failed to start: ${FAILURE}"
  OUTPUT=$(cat "${TEST_DIR}/stdout.txt" "${TEST_DIR}/stderr.txt")

  if [ -n "${OUTPUT}" ]
  then
    echo "Its output was:"
    echo "${OUTPUT}"
  fi

  exit 1
fi

echo "${PRODUCT_NAME} started successfully"
