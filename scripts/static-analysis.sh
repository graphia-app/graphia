#! /bin/bash
#
# Copyright © 2013-2021 Graphia Technologies Ltd.
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

OPTIND=1
VERBOSE=0

while getopts "vs:" OPTION
do
  case "${OPTION}" in
    v)
      VERBOSE=1
      ;;
    s)
      echo -n "Sourcing ${OPTARG}"
      if [ -e ${OPTARG} ]
      then
        echo "..."
        . ${OPTARG}
      else
        echo "...doesn't exist"
      fi
      ;;
  esac
done

shift $((OPTIND-1))
[ "$1" = "--" ] && shift

NUM_CORES=$(nproc --all)

if [ -z ${BUILD_DIR} ]
then
  COMPILER=$(basename ${CC} | sed -e 's/-.*//g')
  BUILD_DIR="build/${COMPILER}"
fi

. ${BUILD_DIR}/variables.sh

if [ ! -z "$@" ]
then
  CPP_FILES=$@
else
  CPP_FILES=$(cat ${BUILD_DIR}/compile_commands.json | \
    jq '.[].file' | grep -vE "qrc_|mocs_compilation|thirdparty" | \
    sed -e 's/"//g')

  INCLUDE_DIRS=$(cat ${BUILD_DIR}/compile_commands.json | \
    jq '.[].command' | grep -oP '(?<=-I) *.*?(?= )' | \
    grep -vE "thirdparty|autogen" | \
    sort | uniq | \
    sed -e 's/\(.*\)/-I\1/')

  DEFINES=$(cat ${BUILD_DIR}/compile_commands.json | \
    jq '.[].command' | grep -oP '(?<=-D) *.*?(?= -\D)' | \
    grep -vE "SOURCE_DIR.*" | sort | uniq | \
    sed -e 's/=.*\"/="dummyvalue"/g' |
    sed -e 's/\(.*\)/-D\1/')
fi

if [ "${VERBOSE}" != 0 ]
then
  echo "Files to be analysed:"
  echo ${CPP_FILES}
  echo "Included directories:"
  echo ${INCLUDE_DIRS}
  echo "Preprocessor defines:"
  echo ${DEFINES}
fi

if [ -z ${CPPCHECK} ]
then
  CPPCHECK="cppcheck"
fi

if [ -z ${CLANGTIDY} ]
then
  CLANGTIDY="clang-tidy"
fi

if [ -z ${CLAZY} ]
then
  CLAZY="clazy-standalone"
fi

if [ -z ${QMLLINT} ]
then
  QMLLINT="qmllint"
fi

# cppcheck
${CPPCHECK} --version
${CPPCHECK} -v --enable=all \
  --suppress=unusedFunction \
  --suppress=unusedPrivateFunction \
  --suppress=*:*/source/thirdparty/* \
  --inline-suppr \
  --library=scripts/cppcheck.cfg \
  ${INCLUDE_DIRS} ${DEFINES} \
  ${CPP_FILES} 2>&1 | tee ${BUILD_DIR}/cppcheck-${VERSION}.log

# clang-tidy
if [ "${VERBOSE}" != 0 ]
then
  echo "clang-tidy"
  ${CLANGTIDY} --version
  ${CLANGTIDY} -dump-config
fi

parallel -k -n1 -P${NUM_CORES} -q \
  ${CLANGTIDY} -quiet -p ${BUILD_DIR} {} \
  ::: ${CPP_FILES} 2>&1 | tee ${BUILD_DIR}/clang-tidy-${VERSION}.log

# clazy
CHECKS="-checks=level1,\
base-class-event,\
container-inside-loop,\
global-const-char-pointer,\
implicit-casts,\
missing-typeinfo,\
qstring-allocations,\
reserve-candidates,\
connect-non-signal,\
lambda-in-connect,\
lambda-unique-connection,\
thread-with-slots,\
connect-not-normalized,\
overridden-signal,\
virtual-signal,\
incorrect-emit,\
qproperty-without-notify,\
no-rule-of-two-soft,\
no-qenums,\
no-non-pod-global-static,\
no-connect-3arg-lambda,\
no-const-signal-or-slot,\
global-const-char-pointer,\
implicit-casts,\
missing-qobject-macro,\
missing-typeinfo,\
returning-void-expression,\
virtual-call-ctor,\
assert-with-side-effects,\
detaching-member,\
thread-with-slots,\
connect-by-name,\
skipped-base-method,\
fully-qualified-moc-types,\
qhash-with-char-pointer-key,\
wrong-qevent-cast,\
static-pmf,\
empty-qstringliteral"

if [ "${VERBOSE}" != 0 ]
then
  echo "clazy"
  ${CLAZY} --version
fi

parallel -k -n1 -P${NUM_CORES} \
  ${CLAZY} --standalone -p ${BUILD_DIR}/compile_commands.json \
  --ignore-dirs="\"(\/usr|thirdparty)\"" \
  ${CHECKS} {} \
  ::: ${CPP_FILES} 2>&1 | tee ${BUILD_DIR}/clazy-${VERSION}.log

# qmllint
${QMLLINT} --version
find source/app \
  source/shared \
  source/plugins \
  source/crashreporter \
  -type f -iname "*.qml" | \
  xargs -n1 -P${NUM_CORES} ${QMLLINT} 2>&1 | tee ${BUILD_DIR}/qmllint-${VERSION}.log
