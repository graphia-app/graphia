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

function htmlify
{
  echo "$1" | \
    perl -pe 's/( {2,})/"&nbsp;" x length($1)/eg' | \
    perl -pe 's/\n/<br>\n/g'
}

function stripLeadingWhitespace
{
  RE="s/^\s{$2}//"
  echo "$1" | perl -pe ${RE}
}

function appendOpenSSLLicense
{
  echo "<h3>OpenSSL</h3>" >> OSS.html
  OPENSSL_LICENSE=$(curl -s \
    https://www.openssl.org/source/license-openssl-ssleay.txt)
  OPENSSL_LICENSE=$(echo "${OPENSSL_LICENSE}" | tail -n +9)
  OPENSSL_LICENSE=$(echo "${OPENSSL_LICENSE}" | \
      perl -pe 's/^((\/\* )|( \*[ \/]?)|([ \t]+))//')
  htmlify "${OPENSSL_LICENSE}" >> OSS.html
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

function appendLicense
{
  echo "<h3>$1</h3>" >> OSS.html
  htmlify "$2" >> OSS.html
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

appendLicenseFromFile expat ../../../thirdparty/expat/COPYING

appendLicenseFromUrl HDF5 https://support.hdfgroup.org/ftp/HDF5/releases/COPYING

appendLicenseFromUrl json https://raw.githubusercontent.com/nlohmann/json/develop/LICENSE.MIT

appendLicenseFromUrl Matio https://raw.githubusercontent.com/tbeu/matio/master/COPYING

appendOpenSSLLicense

appendLicenseFromUrl SortFilterProxyModel \
  https://raw.githubusercontent.com/oKcerG/SortFilterProxyModel/master/LICENSE

appendLicense Tango \
  "Icons courtesy of the <a href=\"http://tango.freedesktop.org\">Tango Desktop Project</a>."

appendLicenseFromHeader utfcpp ../../../thirdparty/utfcpp/utf8.h \
  "^Permission.*" ".*SOFTWARE\.$"

appendLicenseFromHeader valgrind ../../../thirdparty/valgrind/valgrind.h \
  "^.*Copyright.*" ".*DAMAGE\.$" 3

appendLicenseFromFile "XLSX I/O" ../../../thirdparty/xlsxio/LICENSE.txt

appendLicenseFromHeader zlib ../../../thirdparty/zlib/zlib.h \
  "^.*Copyright.*" ".*distribution\.$" 2
