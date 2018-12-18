#! /bin/bash

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
