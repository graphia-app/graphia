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

# Install LLVM from apt.llvm.org, then the given packages

if [ -z "$1" ]
then
  echo "Usage: $(basename $0) llvm-version [package...]"
  exit 1
fi

VERSION=$1
shift

LLVM_SH=$(mktemp)
wget -O ${LLVM_SH} https://apt.llvm.org/llvm.sh || exit $?
sudo bash ${LLVM_SH} ${VERSION} || exit $?
rm -f ${LLVM_SH}

sudo apt-get install -y "$@"
