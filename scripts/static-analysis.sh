#! /bin/bash

OPTIND=1
VERBOSE=0

while getopts "vs:" OPTION
do
  case "${OPTION}" in
    v)
      VERBOSE=1
      ;;
    s)
      echo -n "Sourcing ${OPTARG}"
      if [ -e ${OPTARG} ]
      then
        echo "..."
        . ${OPTARG}
      else
        echo "...doesn't exist"
      fi
      ;;
  esac
done

shift $((OPTIND-1))
[ "$1" = "--" ] && shift

NUM_CORES=$(nproc --all)

if [ -z ${BUILD_DIR} ]
then
  COMPILER=$(basename ${CC} | sed -e 's/-.*//g')
  BUILD_DIR="build/${COMPILER}"
fi

if [ ! -z "$@" ]
then
  CPP_FILES=$@
else
  CPP_FILES=$(cat ${BUILD_DIR}/compile_commands.json | \
    jq '.[].file' | grep -vE "qrc_|mocs_compilation|thirdparty" | \
    sed -e 's/"//g')
fi

if [ "${VERBOSE}" != 0 ]
then
  echo "Files to be analysed:"
  echo ${CPP_FILES}
fi

# cppcheck
cppcheck --version
cppcheck --enable=all --xml --xml-version=2 \
  --library=scripts/cppcheck.cfg ${CPP_FILES} 2> cppcheck.xml

# clang-tidy
if [ "${VERBOSE}" != 0 ]
then
  echo "clang-tidy"
  clang-tidy --version
  clang-tidy -dump-config
fi

parallel -n1 -P${NUM_CORES} -q \
  clang-tidy -p ${BUILD_DIR} {} \
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
connect-non-signal,\
lambda-in-connect,\
lambda-unique-connection,\
thread-with-slots,\
connect-not-normalized,\
overriden-signal,\
virtual-signal,\
incorrect-emit,\
connect-by-name,\
qproperty-without-notify,\
no-rule-of-two-soft,\
no-qenums,\
no-non-pod-global-static,\
no-connect-3arg-lambda,\
no-const-signal-or-slot,\
global-const-char-pointer,\
implicit-casts,\
missing-qobject-macro,\
missing-typeinfo,\
returning-void-expression,\
virtual-call-ctor,\
assert-with-side-effects,\
bogus-dynamic-cast,\
detaching-member,\
thread-with-slots"

if [ "${VERBOSE}" != 0 ]
then
  echo "clazy"
  clazy-standalone --version
fi

parallel -n1 -P${NUM_CORES} \
  clazy-standalone -p ${BUILD_DIR}/compile_commands.json ${CHECKS} {} \
  ::: ${CPP_FILES}

#FIXME disable qmllint until it catches up with the latest QML standard
QT_VERSION=$(qmake --version | sed -n 2p | sed -e 's/Using Qt version \([^ ]*\).*$/\1/')
if [ "${QT_VERSION}" == "`echo -e "${QT_VERSION}\n5.10.0" | sort -rV | head -n1`" ]
then
  echo "Check if qmllint works, following Qt upgrade"
  exit 1
fi

# qmllint
#qmllint --version
#find source/app \
#  source/shared \
#  source/plugins \
#  source/crashreporter \
#  -type f -iname "*.qml" | \
#  xargs -n1 -P${NUM_CORES} qmllint
