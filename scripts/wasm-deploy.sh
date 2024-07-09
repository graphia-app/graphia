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

set -e

. scripts/defaults.sh

WEB_REPOSITORY=$1

if [ ! -d "${WEB_REPOSITORY}/.git" ]
then
    echo Supply repository as first argument.
    exit 1
fi

# This mysterious js module somehow works around the requirement for COOP headers
npm install -g coi-serviceworker

TARBALL=$(readlink -f build/*.tar.gz)
BASENAME=${TARBALL##*/}
BASENAME=${BASENAME%.tar.gz}

cd ${WEB_REPOSITORY}

git config user.email ""
git config user.name "WebAssembly Deployer"

# Remove existing content
git rm *

# GitHub pages needs this file to indicate DNS resolution
echo web.graphia.app > CNAME

# Extract the new build
tar --strip-components=1 -xzf ${TARBALL}

# Patch in coi-serviceworker.js
cp /usr/local/lib/node_modules/coi-serviceworker/coi-serviceworker.js .
sed -i '/<head>/a <script src="coi-serviceworker.js"></script>' index.html

# Patch in setting CORS_PROXY
sed -i '/qt: {/a environment: {"CORS_PROXY": "https://corsproxy.graphia.app"},' index.html

# Commit
git add *
git commit -m "${BASENAME}"
git push
