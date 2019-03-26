#! /bin/bash

. scripts/defaults.sh

NUM_CORES=$(sysctl -n hw.ncpu)
BUILD_DIR="build"

rm -rf ${BUILD_DIR}
mkdir -p ${BUILD_DIR}

# https://github.com/detroit-labs/safe-xcode-select
# If this is installed, use it to select the default Xcode,
# in case something else has changed it
SAFE_XCODE_SELECT=$(which safe-xcode-select)
if [ ! -z "${SAFE_XCODE_SELECT}" ]
then
  # Use Xcode 9 install, to workaround Qt's current limitations
  # with "Dark mode" on newer MacOS releases
  ${SAFE_XCODE_SELECT} /Applications/Xcode.9.app
  echo "Switched Xcode version to:"
  xcodebuild -version
fi

GCC_TREAT_WARNINGS_AS_ERRORS=NO xcodebuild -project \
  source/thirdparty/breakpad/src/tools/mac/dump_syms/dump_syms.xcodeproj

(
  cd ${BUILD_DIR}
  cmake --version || exit $?
  cmake -DUNITY_BUILD=ON -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_OSX_DEPLOYMENT_TARGET=10.12 \
    -GNinja .. || exit $?
  cmake --build . --target all || exit $?
  cmake --build . --target all 2>&1 | tee compiler.log
  [[ "${PIPESTATUS[0]}" -eq 0 ]] || exit ${PIPESTATUS[0]}
)

function makeSymFile
{
  SOURCE=$1
  TARGET=$2

  dsymutil ${SOURCE} -flat -o ${TARGET}.dsym || exit $?
  source/thirdparty/breakpad/src/tools/mac/dump_syms/build/Release/dump_syms \
    ${TARGET}.dsym > ${TARGET} 2> /dev/null || exit $?
  rm ${TARGET}.dsym

  # Remove .sym.dsym from the MODULE name
  sed -e '1s/\.dsym$//' -e '1s/\.sym$//' -i '' ${TARGET}
}

makeSymFile ${BUILD_DIR}/${PRODUCT_NAME}.app/Contents/MacOS/${PRODUCT_NAME} \
  ${BUILD_DIR}/${PRODUCT_NAME}.sym
makeSymFile ${BUILD_DIR}/source/thirdparty/libthirdparty.dylib \
  ${BUILD_DIR}/libthirdparty.dylib.sym

for PLUGIN in $(find ${BUILD_DIR}/plugins -name "*.dylib")
do
  makeSymFile ${PLUGIN} ${PLUGIN}.sym || exit $?
done

(
  cd ${BUILD_DIR}

  # Clean intermediate build products
  grep "^rule.*\(_COMPILER_\|_STATIC_LIBRARY_\)" rules.ninja | \
    cut -d' ' -f2 | xargs -n1 ninja -t clean -r
)
