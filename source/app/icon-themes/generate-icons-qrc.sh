#! /bin/bash
#
# Copyright © 2013-2023 Graphia Technologies Ltd.
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

QML_CONTENT=$(find $(find ../../ -type d -name "qml") -name "*.qml" | \
  xargs perl -pe 's/\n/\$/g')

ICON_KEYS="iconName iconSource icon\.name"
for ICON_KEY in ${ICON_KEYS}
do
  # Extract the part after the colon (the values), then find quoted strings within that
  VALUES=$(echo ${QML_CONTENT} | \
    perl -ne 'print "$1\n" while /\s'"${ICON_KEY}"':[\s\$]*((\{([^{}]|(?2))*\})|([^\$]*))\$/gm' | \
    perl -pe 's/[^\"]+|([^\"]*\"([^\"]*)\"[^\"]*)/$2\n/g')
  ICON_NAMES="${ICON_NAMES} ${VALUES}"
done

ICON_NAMES=$(echo ${ICON_NAMES} | xargs -n1 | sort | uniq)

echo -e ${PREAMBLE} > icons.qrc

THEME_FILES=$(find * -name "*.theme")
for THEME_FILE in ${THEME_FILES}
do
  echo -e "\t\t<file>${THEME_FILE}</file>" >> icons.qrc
done

for ICON_NAME in ${ICON_NAMES}
do
  # Get the FQ file names (if they exist at all)
  ICON_FILES=$(find * -iname "${ICON_NAME}.*" | sort)
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
