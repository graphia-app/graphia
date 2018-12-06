#! /bin/bash

function htmlify
{
  echo "$1" | perl -pe 's/( {2,})/"&nbsp;" x length($1)/eg' | perl -pe 's/\n/<br>\n/g'
}

function stripLeadingWhitespace
{
  RE="s/^\s{$2}//"
  echo "$1" | perl -pe ${RE}
}

function appendLicenseFromUrl
{
  echo "<h3>$1</h3>" >> OSS.html
  LICENSE=$(curl -s $2)
  htmlify "${LICENSE}" >> OSS.html
}

function appendLicenseFromFile
{
  echo "<h3>$1</h3>" >> OSS.html
  LICENSE=$(tr -d '\r' < $2)
  htmlify "${LICENSE}" >> OSS.html
}

function appendLicenseFromHeader
{
  echo "<h3>$1</h3>" >> OSS.html
  HEADER=$(cat $2)
  DECOMMENTED=$(echo "${HEADER}" | perl -pe 's/^\s*\/\///g')
  SED_PATTERN="/$3/,/$4/p"
  CLIPPED=$(echo "$DECOMMENTED" | sed -n ${SED_PATTERN})

  if [[ $5 ]]
  then
    CLIPPED=$(stripLeadingWhitespace "${CLIPPED}" $5)
  fi

  htmlify "${CLIPPED}" >> OSS.html
}

# Clear the file
> OSS.html

appendLicenseFromHeader Blaze ../../../thirdparty/blaze/blaze/Blaze.h \
  "^.*Copyright.*" ".*DAMAGE\.$" 2

appendLicenseFromUrl Boost https://www.boost.org/LICENSE_1_0.txt

appendLicenseFromFile Breakpad ../../../thirdparty/breakpad/LICENSE

appendLicenseFromFile Crypto++ ../../../thirdparty/cryptopp/License.txt

appendLicenseFromUrl csv-parser \
  https://raw.githubusercontent.com/AriaFallah/csv-parser/master/LICENSE

appendLicenseFromUrl HDF5 https://support.hdfgroup.org/ftp/HDF5/releases/COPYING

appendLicenseFromUrl json https://raw.githubusercontent.com/nlohmann/json/develop/LICENSE.MIT

appendLicenseFromUrl Matio https://raw.githubusercontent.com/tbeu/matio/master/COPYING

appendLicenseFromUrl SortFilterProxyModel \
  https://raw.githubusercontent.com/oKcerG/SortFilterProxyModel/master/LICENSE

appendLicenseFromHeader utfcpp ../../../thirdparty/utfcpp/utf8.h \
  "^Permission.*" ".*SOFTWARE\.$"

appendLicenseFromHeader valgrind ../../../thirdparty/valgrind/valgrind.h \
  "^.*Copyright.*" ".*DAMAGE\.$" 3

appendLicenseFromHeader zlib ../../../thirdparty/zlib/zlib.h \
  "^.*Copyright.*" ".*distribution\.$" 2
