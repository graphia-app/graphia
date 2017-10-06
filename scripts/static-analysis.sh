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

# cppcheck
cppcheck --version
find source/app \
  source/shared \
  source/plugins \
  source/crashreporter \
  -type f -iname "*.cpp" -not -iname "moc_*" -not -iname "qrc_*" | \
  xargs cppcheck --enable=all \
  --xml --xml-version=2 \
  --library=scripts/cppcheck.cfg 2> cppcheck.xml

# clang-tidy (this works better when a compile_command.json has been created by bear)
CHECKS="-checks=*,\
-*readability*,\
-llvm-*,\
-google-*,\
-clang-analyzer-alpha.deadcode.UnreachableCode"

clang-tidy --version
clang-tidy -p build/linux-clang -list-checks ${CHECKS}
find source/app \
  source/shared \
  source/plugins \
  source/crashreporter \
  -type f -iname "*.cpp" -not -iname "moc_*" -not -iname "qrc_*" | \
  xargs -n1 -P${NUM_CORES} clang-tidy ${CHECKS}

# clazy
CHECKS="-checks=level1,\
no-rule-of-two-soft,\
no-non-pod-global-static"

clazy-standalone --version
find source/app \
  source/shared \
  source/plugins \
  source/crashreporter \
  -type f -iname "*.cpp" -not -iname "moc_*" -not -iname "qrc_*" | \
  xargs -n1 -P${NUM_CORES} clazy-standalone ${CHECKS}

# qmllint
qmllint --version
find source/app \
  source/shared \
  source/plugins \
  source/crashreporter \
  -type f -iname "*.qml" | \
  xargs qmllint
