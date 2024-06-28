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

BUILD_DIR="build/wasm"

. ${BUILD_DIR}/variables.sh

DEPLOY_NAME=${PRODUCT_NAME}-WebAssembly-${VERSION}
DEPLOY_DIR=${BUILD_DIR}/${DEPLOY_NAME}

mkdir -p ${DEPLOY_DIR}

cp ${BUILD_DIR}/*.html \
    ${BUILD_DIR}/*.js \
    ${BUILD_DIR}/*.css \
    ${BUILD_DIR}/*.wasm \
    ${BUILD_DIR}/*.svg \
    ${BUILD_DIR}/*.ico \
    ${DEPLOY_DIR} || exit $?

tar cvfz ${DEPLOY_DIR}.tar.gz -C ${BUILD_DIR} ${DEPLOY_NAME} || exit $?
