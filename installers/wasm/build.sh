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

DEPLOY_DIR=${BUILD_DIR}/${PRODUCT_NAME}-${VERSION}

mkdir -p ${DEPLOY_DIR}

cp ${BUILD_DIR}/${PRODUCT_NAME}.html \
    ${BUILD_DIR}/${PRODUCT_NAME}.js \
    ${BUILD_DIR}/${PRODUCT_NAME}.wasm \
    ${BUILD_DIR}/${PRODUCT_NAME}.worker.js \
    ${BUILD_DIR}/qtloader.js \
    ${BUILD_DIR}/qtlogo.svg \
    ${DEPLOY_DIR} || exit $?

tar cvfz ${DEPLOY_DIR}.tar.gz ${DEPLOY_DIR} || exit $?
