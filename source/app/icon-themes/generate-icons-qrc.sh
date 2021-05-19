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

cd ${0%/*}

PREAMBLE="<RCC>\n\
\t<qresource prefix=\"/icons\">"

POSTAMBLE=" \t</qresource>\n\
</RCC>"

THEME_FILES=$(find * -name "*.theme")
ICON_NAMES=$(find $(find ../../ -type d -name "qml") -name "*.qml" | \
  xargs perl -pe 's/\n/\$/g' | \
  perl -ne 'print "$1\n" while /iconName:[\s\$]*((\{([^{}]|(?2))*\})|([^\$]*))\$/gm' | \
  perl -pe 's/[^\"]*\"([^\"]*)\"[^\"]*/$1\.\*\n/g' | \
  sort | uniq)

ICON_SOURCES=$(find $(find ../../ -type d -name "qml") -name "*.qml" | \
  xargs perl -pe 's/\n/\$/g' | \
  perl -ne 'print "$1\n" while /iconSource:[\s\$]*((\{([^{}]|(?2))*\})|([^\$]*))\$/gm' | \
  perl -pe 's/[^\"]*\".*\/([^\"]*)\"[^\"]*/$1\n/g' | \
  sort | uniq)

ICON_SOURCES="${ICON_NAMES} ${ICON_SOURCES}"

echo -e ${PREAMBLE} > icons.qrc

for THEME_FILE in ${THEME_FILES}
do
  echo -e "\t\t<file>${THEME_FILE}</file>" >> icons.qrc
done

for ICON_SOURCE in ${ICON_SOURCES}
do
  ICON_FILES=$(find * -iname "${ICON_SOURCE}" | sort)
  for ICON_FILE in ${ICON_FILES}
  do
    if [ -L ${ICON_FILE} ]
    then
      echo ${ICON_FILE} is symlink!
    fi

    echo -e "\t\t<file>${ICON_FILE}</file>" >> icons.qrc
  done
done

echo -e ${POSTAMBLE} >> icons.qrc
