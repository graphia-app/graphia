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

# Installs the app from the disk image built by build.sh, checks it reaches its main window,
# then cleans up

BUILD_DIR=build

. ${BUILD_DIR}/variables.sh

DMG="${PWD}/${BUILD_DIR}/${PRODUCT_NAME}-${VERSION}.dmg"
TEST_DIR=$(mktemp -d)
MOUNT_POINT="${TEST_DIR}/mount"
APP="${TEST_DIR}/${PRODUCT_NAME}.app"
WINDOW_TITLES="${TEST_DIR}/window-titles"
STARTUP_TIMEOUT_SECONDS=120

# Includes anything the executable spawns, e.g. QtWebEngineProcess or CrashReporter
stopInstalledProcesses()
{
  pkill -KILL -f "${APP}/" 2> /dev/null

  while pgrep -f "${APP}/" > /dev/null
  do
    sleep 0.1
  done
}

cleanUp()
{
  echo "------ cleaning up"
  stopInstalledProcesses

  if [ -d "${MOUNT_POINT}" ]
  then
    hdiutil detach "${MOUNT_POINT}" -force > /dev/null 2>&1
  fi

  rm -rf "${TEST_DIR}"
}

trap cleanUp EXIT

# Prints the titles of a process's on screen windows, one per line; without the
# Screen Recording permission, macOS withholds the titles
cat > "${WINDOW_TITLES}.swift" <<'EOF'
import CoreGraphics

let pid = Int32(CommandLine.arguments[1])!
let windows = CGWindowListCopyWindowInfo([.optionOnScreenOnly], kCGNullWindowID) as? [[String: Any]] ?? []

for window in windows where window[kCGWindowOwnerPID as String] as? Int32 == pid
{
    print(window[kCGWindowName as String] as? String ?? "<title unavailable>")
}
EOF

swiftc -O -o "${WINDOW_TITLES}" "${WINDOW_TITLES}.swift" || exit $?

echo "------ installing ${DMG} to ${TEST_DIR}"
hdiutil attach "${DMG}" -nobrowse -readonly -mountpoint "${MOUNT_POINT}" > /dev/null || exit $?
ditto "${MOUNT_POINT}/${PRODUCT_NAME}.app" "${APP}" || exit $?
hdiutil detach "${MOUNT_POINT}" > /dev/null || exit $?

echo "------ starting ${PRODUCT_NAME}"
"${APP}/Contents/MacOS/${PRODUCT_NAME}" --dontUpdate --skipWelcome \
  > "${TEST_DIR}/stdout.txt" 2> "${TEST_DIR}/stderr.txt" &
PID=$!

FAILURE=""
DEADLINE=$((SECONDS + STARTUP_TIMEOUT_SECONDS))

while true
do
  if ! kill -0 ${PID} 2> /dev/null
  then
    wait ${PID}
    FAILURE="exited with code $?"
    break
  fi

  # Error message boxes are also windows, so check the title rather than just its presence;
  # the main window's is the product name, after the open tab's (e.g. the first run tutorial)
  if "${WINDOW_TITLES}" ${PID} | grep -qxE "(.+ - )?${PRODUCT_NAME}"
  then
    break
  fi

  if [ ${SECONDS} -ge ${DEADLINE} ]
  then
    TITLES=$("${WINDOW_TITLES}" ${PID} | sort -u | \
      sed -e 's/^/"/' -e 's/$/"/' | paste -sd ',' - | sed -e 's/,/, /g')

    if [[ "${TITLES}" == *"<title unavailable>"* ]]
    then
      FAILURE="showed windows whose titles macOS withheld (${TITLES}); the shell \
running this script needs the Screen Recording permission"
    elif [ -n "${TITLES}" ]
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
