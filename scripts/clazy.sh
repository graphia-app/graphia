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

if [ -z ${BUILD_DIR} ]
then
    echo "BUILD_DIR not set"
    exit 1
fi

cd ${BUILD_DIR}

. variables.sh

if [ -z ${CLAZY} ]
then
    CLAZY="clazy"
fi

LEVEL0="\
level0\
"

LEVEL1="\
level1,\
no-connect-3arg-lambda,\
no-non-pod-global-static,\
no-rule-of-two-soft\
"

LEVEL2="\
level2,\
no-copyable-polymorphic,\
no-ctor-missing-parent-argument,\
no-function-args-by-ref,\
no-function-args-by-value,\
no-old-style-connect,\
no-rule-of-three\
"

MANUAL="\
assert-with-side-effects,\
container-inside-loop,\
detaching-member,\
heap-allocated-small-trivial-type,\
ifndef-define-typo,\
isempty-vs-count,\
jni-signatures,\
qhash-with-char-pointer-key,\
qproperty-type-mismatch,\
qrequiredresult-candidates,\
qstring-varargs,\
qt6-fwd-fixes,\
qt6-header-fixes,\
qt6-qhash-signature,\
qvariant-template-instantiation,\
raw-environment-function,\
reserve-candidates,\
signal-with-return-value,\
thread-with-slots,\
tr-non-literal,\
unexpected-flag-enumerator-value,\
unneeded-cast,\
use-arrow-operator-instead-of-data,\
use-chrono-in-qtimer\
"

# Not enabled manual checks
#qt-keywords,\
#qt4-qstring-from-array,\
#qt6-deprecated-api-fixes,\
#qt6-qlatin1stringchar-to-u,\

export CLAZY_EXTRA_OPTIONS="unneeded-cast-prefer-dynamic-cast-over-qobject"

CHECKS="-checks=\"${LEVEL0},${LEVEL1},${LEVEL2},${MANUAL}\""

${CLAZY} --version

parallel -k -n1 -P$(nproc --all) \
  ${CLAZY} --standalone -p compile_commands.json \
  --ignore-dirs="\"(\/usr|thirdparty)\"" \
  ${CHECKS} {} \
  ::: ${CPP_FILES} 2>&1 | tee clazy-${VERSION}.log
