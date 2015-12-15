#! /bin/sh

cd ${0%/*}

PREAMBLE="<RCC>\n\
\t<qresource prefix=\"/icons\">"

POSTAMBLE=" \t</qresource>\n\
</RCC>"

THEME_FILES=$(find * -name "*.theme")
ICON_NAMES=$(grep "iconName:" ../ui/qml/*.qml | \
  sed -e "s/[^\"]*\"\([^\"]*\)\"/\1 /g")

echo ${PREAMBLE} > icons.qrc

for THEME_FILE in ${THEME_FILES};
do
  echo "\t\t<file>${THEME_FILE}</file>" >> icons.qrc
done

for ICON_NAME in ${ICON_NAMES};
do
  ICON_FILES=$(find * -iname "${ICON_NAME}.*")
  for ICON_FILE in ${ICON_FILES};
  do
    echo "\t\t<file>${ICON_FILE}</file>" >> icons.qrc
  done
done

echo ${POSTAMBLE} >> icons.qrc
