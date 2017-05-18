#! /bin/sh

cd ${0%/*}

PREAMBLE="<RCC>\n\
\t<qresource prefix=\"/icons\">"

POSTAMBLE=" \t</qresource>\n\
</RCC>"

THEME_FILES=$(find * -name "*.theme")
ICON_NAMES=$(find $(find ../../ -type d -name "qml") -name "*.qml" | \
  xargs perl -pe 's/\n/\$/g' | \
  perl -ne 'print "$1\n" while /iconName:[\s\$]*(({([^{}]|(?2))*})|([^\$]*))\$/gm' | \
  perl -pe 's/[^\"]*\"([^\"]*)\"[^\"]*/$1\n/g' | \
  sort | uniq)

echo -e ${PREAMBLE} > icons.qrc

for THEME_FILE in ${THEME_FILES};
do
  echo -e "\t\t<file>${THEME_FILE}</file>" >> icons.qrc
done

for ICON_NAME in ${ICON_NAMES};
do
  ICON_FILES=$(find * -iname "${ICON_NAME}.*" | sort)
  for ICON_FILE in ${ICON_FILES};
  do
    if [ -L ${ICON_FILE} ]
    then
      echo ${ICON_FILE} is symlink!
    fi

    echo -e "\t\t<file>${ICON_FILE}</file>" >> icons.qrc
  done
done

echo -e ${POSTAMBLE} >> icons.qrc
