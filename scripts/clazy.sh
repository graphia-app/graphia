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

${CLAZY} --version

parallel -k -n1 -P$(nproc --all) \
  ${CLAZY} --standalone -p compile_commands.json \
  --ignore-dirs="\"(\/usr|thirdparty)\"" \
  ${CHECKS} {} \
  ::: ${CPP_FILES} 2>&1 | tee clazy-${VERSION}.log
