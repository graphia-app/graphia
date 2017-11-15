#! /bin/bash

for ARGUMENT in "$@"
do
  echo -n "Sourcing ${ARGUMENT}"
  if [ -e ${ARGUMENT} ]
  then
    echo "..."
    . ${ARGUMENT}
  else
    echo "...doesn't exist"
  fi
done

NUM_CORES=$(nproc --all)
COMPILER=$(basename ${CC} | sed -e 's/-.*//g')
BUILD_DIR="build/${COMPILER}"

CPP_FILES=$(cat ${BUILD_DIR}/compile_commands.json | \
  jq '.[].file' | grep -vE "qrc_|_automoc|thirdparty")

# cppcheck
cppcheck --version
echo ${CPP_FILES} | xargs cppcheck --enable=all \
  --xml --xml-version=2 --library=scripts/cppcheck.cfg 2> cppcheck.xml

# clang-tidy (this works better when a compile_command.json has been created by bear)
CHECKS="-checks=*,\
-*readability*,\
-llvm-*,\
-google-*,\
-clang-analyzer-alpha.deadcode.UnreachableCode"

echo "clang-tidy"
clang-tidy --version
clang-tidy -p build/linux-clang -list-checks ${CHECKS}
echo ${CPP_FILES} | xargs -n1 -P${NUM_CORES} clang-tidy ${CHECKS}

# clazy
CHECKS="-checks=level1,\
base-class-event,\
container-inside-loop,\
global-const-char-pointer,\
implicit-casts,\
missing-typeinfo,\
qstring-allocations,\
reserve-candidates,\
no-rule-of-two-soft,\
no-qenums,\
no-non-pod-global-static"

echo "clazy"
clazy-standalone --version
echo ${CPP_FILES} | xargs -n1 -P${NUM_CORES} clazy-standalone ${CHECKS}

# qmllint
qmllint --version
find source/app \
  source/shared \
  source/plugins \
  source/crashreporter \
  -type f -iname "*.qml" | \
  xargs qmllint
