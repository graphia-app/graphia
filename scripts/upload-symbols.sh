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

if [ -z "${SYM_UPLOAD_URL}" ]
then
  echo "No symbol upload url supplied, skipping..."
  exit 0
fi

BUILD_DIR="build"

SYM_FILES=$(find ${BUILD_DIR} -iname "*.sym")
ARCHIVE="symbols.tar.gz"

tar cvfz ${ARCHIVE} ${SYM_FILES} || exit 1
curl -T ${ARCHIVE} -X POST -H 'Transfer-Encoding: chunked' -H 'Expect:' "${SYM_UPLOAD_URL}"
rm -f ${ARCHIVE}

# Upload Qt pdbs, if we can find them
if [ ! -z "${Qt5_Dir}" ]
then
  PDB_FILES=$(find ${Qt5_Dir} -iname "*.pdb")

  if [ ! -z "${PDB_FILES}" ]
  then
    PDB_ARCHIVE="pdbs.tar.gz"

    tar cvfz ${PDB_ARCHIVE} ${PDB_FILES} || exit 1
    curl -T ${PDB_ARCHIVE} -X POST -H 'Transfer-Encoding: chunked' -H 'Expect:' "${SYM_UPLOAD_URL}"
    rm -f ${PDB_ARCHIVE}
  fi
fi
