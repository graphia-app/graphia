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

# Merge the profiles the tests write to profiles/ in BUILD_DIR when run with
# LLVM_PROFILE_FILE, then report coverage to coverage.txt and coverage.lcov

if [ -z ${BUILD_DIR} ]
then
    echo "BUILD_DIR not set"
    exit 1
fi

cd ${BUILD_DIR}

if [ -z ${LLVM_PROFDATA} ]
then
    LLVM_PROFDATA="llvm-profdata"
fi

if [ -z ${LLVM_COV} ]
then
    LLVM_COV="llvm-cov"
fi

${LLVM_COV} --version

${LLVM_PROFDATA} merge -sparse profiles/*.profraw -o coverage.profdata || exit $?

OBJECTS=()
for TEST in source/tests/tst_*
do
    [ -f "${TEST}" ] && [ -x "${TEST}" ] || continue
    [ ${#OBJECTS[@]} -eq 0 ] && OBJECTS+=("${TEST}") || OBJECTS+=("-object" "${TEST}")
done

for PLUGIN in plugins/lib*.so
do
    [ -f "${PLUGIN}" ] && OBJECTS+=("-object" "${PLUGIN}")
done

for LIBRARY in source/tests/lib*.so
do
    [ -f "${LIBRARY}" ] && OBJECTS+=("-object" "${LIBRARY}")
done

IGNORE='(thirdparty|source/tests|_autogen|/\.rcc/|/\.qt/|^/usr/|qtenv|/Qt/)'
${LLVM_COV} report "${OBJECTS[@]}" -instr-profile=coverage.profdata \
    -ignore-filename-regex="${IGNORE}" > coverage.txt || exit $?
${LLVM_COV} export "${OBJECTS[@]}" -instr-profile=coverage.profdata \
    -ignore-filename-regex="${IGNORE}" -format=lcov > coverage.lcov
