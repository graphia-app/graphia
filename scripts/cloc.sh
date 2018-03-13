#! /bin/bash

find source/app \
  source/shared \
  source/plugins \
  source/crashreporter \
  source/messagebox \
  -regex ".*\.\(cpp\|h\|qml\|js\|json\|sh\|pl\|py\|c\|bat\|frag\|vert\)" \
  -not -iname "moc_*" -not -iname "qrc_*" | \
  xargs cloc --read-lang-def=scripts/glshader.langdef --by-file \
    --xml --out=cloc.xml
