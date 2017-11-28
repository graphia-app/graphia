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
  jq '.[].file' | grep -vE "qrc_|mocs_compilation|thirdparty" | \
  sed -e 's/"//g')

echo "Files to be analysed:"
echo ${CPP_FILES}

# cppcheck
cppcheck --version
cppcheck --enable=all --xml --xml-version=2 \
  --library=scripts/cppcheck.cfg ${CPP_FILES} 2> cppcheck.xml

# clang-tidy
CHECKS="-checks=*,\
-readability-braces-around-statements,\
-readability-identifier-naming,\
-readability-named-parameter,\
-llvm-*,\
llvm-namespace-comment,\
-google-*,\
google-explicit-constructor,\
google-runtime-int,\
google-runtime-member-string-references,\
-cppcoreguidelines-pro-type-vararg,\
-clang-analyzer-alpha.deadcode.UnreachableCode"

echo "clang-tidy"
clang-tidy --version
clang-tidy -list-checks ${CHECKS}
parallel -n1 -P${NUM_CORES} \
  clang-tidy -p ${BUILD_DIR} -q \
  -header-filter="^.*source\/(app|shared|plugins).*$" ${CHECKS} {} \
  ::: ${CPP_FILES}

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
parallel -n1 -P${NUM_CORES} \
  clazy-standalone -p ${BUILD_DIR}/compile_commands.json ${CHECKS} {} \
  ::: ${CPP_FILES}

# qmllint
qmllint --version
find source/app \
  source/shared \
  source/plugins \
  source/crashreporter \
  -type f -iname "*.qml" | \
  xargs -n1 -P${NUM_CORES} -q qmllint
