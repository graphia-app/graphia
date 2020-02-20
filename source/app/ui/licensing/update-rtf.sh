#! /bin/bash
#
# Copyright © 2013-2020 Graphia Technologies Ltd.
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

# The HTML(ish) copy of the EULA is considered the master version, but the
# Windows installer also needs to display it, which doesn't support HTML, hence
# this hack-tastic script to produce something vaguely resembling an RTF file

DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null && pwd )"

IN="${DIR}/EULA.html"
OUT="${DIR}/EULA.rtf"

echo -e "{\\\\rtf\n" > ${OUT}
cat ${IN} | sed -e 's/<b>/\\b /g' \
  -e 's/<\/b>/\\b0/g' \
  -e 's/<br>/ \\line/g' >> ${OUT}
echo -e "}" >> ${OUT}
