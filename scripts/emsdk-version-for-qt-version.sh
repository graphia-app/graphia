#! /bin/bash

if [ $# -eq 0 ]
then
    echo "Usage: $0 <Qt version>"
    exit 1
fi

qt_version=$1
url="https://raw.githubusercontent.com/qt/qtdoc/v$qt_version/doc/src/platforms/wasm.qdoc"
qt_major_minor=$(echo "$qt_version" | cut -d. -f1,2)
content=$(curl -s "$url")
pattern="Qt $qt_major_minor: ([0-9]+\.[0-9]+\.[0-9]+)"

if [[ $content =~ $pattern ]]
then
    emsdk_version="${BASH_REMATCH[1]}"
    echo $emsdk_version
    exit 0
fi

exit 1
