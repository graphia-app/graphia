#! /bin/bash
#
# Copyright © 2013-2025 Tim Angus
# Copyright © 2013-2025 Tom Freeman
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

# Run the static analysis jobs from .github/workflows/build.yml locally: the
# Clang Debug build that produces compile_commands.json, then each analyser,
# leaving their logs in build/clang/. Run from the root of the source tree.
#
# Qt is found via QT_ROOT_DIR (as set by install-qt-action), or failing that in
# a few usual install locations. The analysers can be overridden in the same way
# as the individual scripts: CLANG_TIDY, CPPCHECK, CLAZY, QMLLINT, IWYU_SOURCE.
# Unless CLAZY is set, the clazy commit the CI uses is built once and cached in
# ~/.cache/graphia-static-analysis. If DEFECTS_DIR points at a checkout of
# graphia-app/defects, the logs are summarised at the end, as the log-analysis
# job does.

ALL_TOOLS="clang-tidy cppcheck clazy qmllint iwyu"

usage()
{
  echo "Usage: $(basename $0) [--no-build] [${ALL_TOOLS// /|} ...]"
  echo "  --no-build  reuse the existing build/clang instead of rebuilding it"
  echo "  With no tools given, all of them are run."
  exit 1
}

WORKFLOW=".github/workflows/build.yml"

if [ ! -e ${WORKFLOW} ]
then
  echo "${WORKFLOW} not found; run from the root of the source tree"
  exit 1
fi

workflowEnv()
{
  sed -n "s/^  $1: *//p" ${WORKFLOW}
}

QT_VERSION=$(workflowEnv QT_VERSION)
CLANG_VERSION=$(workflowEnv CLANG_VERSION)

BUILD=1
TOOLS=()

for ARGUMENT in "$@"
do
  case ${ARGUMENT} in
    --no-build) BUILD=0 ;;
    -h|--help) usage ;;
    *)
      [[ " ${ALL_TOOLS} " == *" ${ARGUMENT} "* ]] || usage
      TOOLS+=(${ARGUMENT})
      ;;
  esac
done

[ ${#TOOLS[@]} -eq 0 ] && TOOLS=(${ALL_TOOLS})

if [ -z "${QT_ROOT_DIR}" ]
then
  for QT_DIR in ${HOME}/Qt ${HOME}/sdks/Qt /opt/Qt
  do
    if [ -d "${QT_DIR}/${QT_VERSION}/gcc_64" ]
    then
      QT_ROOT_DIR="${QT_DIR}/${QT_VERSION}/gcc_64"
      break
    fi
  done
fi

if [ ! -d "${QT_ROOT_DIR}" ]
then
  echo "Qt ${QT_VERSION} not found; set QT_ROOT_DIR to e.g. ~/Qt/${QT_VERSION}/gcc_64"
  exit 1
fi

echo "Qt: ${QT_ROOT_DIR}"
echo "Clang: ${CLANG_VERSION}"
echo "Tools: ${TOOLS[*]}"

export CMAKE_PREFIX_PATH="${QT_ROOT_DIR}${CMAKE_PREFIX_PATH:+:${CMAKE_PREFIX_PATH}}"
export PATH="${QT_ROOT_DIR}/bin:${PATH}"
export CC=clang-${CLANG_VERSION}
export CXX=clang++-${CLANG_VERSION}
export BUILD_DIR=build/clang/

if [ ${BUILD} -eq 1 ]
then
  BUILD_TYPE=Debug UNITY_BUILD=OFF scripts/linux-build.sh || exit $?
  scripts/parse-compile_commands-json.sh || exit $?
elif [ ! -e ${BUILD_DIR}/compile_commands.json ]
then
  echo "No existing build in ${BUILD_DIR}; run without --no-build first"
  exit 1
fi

# Each analyser writes its log to ${BUILD_DIR}; old ones would be confusing
for TOOL in "${TOOLS[@]}"
do
  rm -f ${BUILD_DIR}/${TOOL}-*.log
done

SKIPPED=()

requireCommand()
{
  if ! command -v "$1" > /dev/null
  then
    echo "$1 not found, skipping ${TOOL}"
    SKIPPED+=(${TOOL})
    return 1
  fi
}

for TOOL in "${TOOLS[@]}"
do
  echo "===== ${TOOL}"

  case ${TOOL} in
    clang-tidy)
      export CLANG_TIDY=${CLANG_TIDY:-clang-tidy-${CLANG_VERSION}}
      requireCommand ${CLANG_TIDY} && scripts/clang-tidy.sh
      ;;

    cppcheck)
      export CPPCHECK=${CPPCHECK:-cppcheck}
      requireCommand ${CPPCHECK} && scripts/cppcheck.sh
      ;;

    clazy)
      # Released versions of clazy either crash or reject some of the checks,
      # so by default build the same commit as the CI does
      if [ -z "${CLAZY}" ]
      then
        CLAZY_REF=$(sed -n '/repository: KDE\/clazy/,/ref:/s/^ *ref: *//p' ${WORKFLOW})
        CLAZY_PREFIX="${XDG_CACHE_HOME:-${HOME}/.cache}/graphia-static-analysis/clazy-${CLAZY_REF}"

        if [ ! -x "${CLAZY_PREFIX}/bin/clazy" ]
        then
          requireCommand llvm-config-${CLANG_VERSION} || continue

          CLAZY_SOURCE=$(mktemp -d)
          (
            git clone --quiet https://github.com/KDE/clazy.git ${CLAZY_SOURCE} &&
            cd ${CLAZY_SOURCE} && git checkout --quiet ${CLAZY_REF} &&
            sed -i 's/target_precompile_headers/#target_precompile_headers/' CMakeLists.txt &&
            cmake -DLLVM_CONFIG_EXECUTABLE=$(command -v llvm-config-${CLANG_VERSION}) \
              -DCMAKE_INSTALL_PREFIX=${CLAZY_PREFIX} -DCMAKE_BUILD_TYPE=Release -G Ninja &&
            cmake --build . && cmake --build . --target install
          )
          CLAZY_STATUS=$?
          rm -rf ${CLAZY_SOURCE}

          if [ ${CLAZY_STATUS} -ne 0 ]
          then
            echo "Building clazy ${CLAZY_REF} failed, skipping ${TOOL}"
            SKIPPED+=(${TOOL})
            continue
          fi
        fi

        CLAZY="${CLAZY_PREFIX}/bin/clazy"
      fi

      export CLAZY
      requireCommand ${CLAZY} && scripts/clazy.sh
      ;;

    qmllint)
      export QMLLINT=${QMLLINT:-qmllint}
      requireCommand ${QMLLINT} && scripts/qmllint.sh
      ;;

    iwyu)
      requireCommand include-what-you-use || continue

      # iwyu.sh expects iwyu_tool.py, which some distributions install without
      # the extension
      if ! command -v iwyu_tool.py > /dev/null
      then
        requireCommand iwyu_tool || continue
        SHIM_DIR=$(mktemp -d)
        ln -s $(command -v iwyu_tool) ${SHIM_DIR}/iwyu_tool.py
        export PATH="${SHIM_DIR}:${PATH}"
      fi

      # The CI uses the mapping file from the IWYU source tree
      if [ -z "${IWYU_SOURCE}" ]
      then
        IWYU_SOURCE=$(dirname $(find /usr/share /usr/local/share \
          -name qt5_11.imp -print -quit 2>/dev/null) 2>/dev/null)
      fi

      if [ ! -e "${IWYU_SOURCE}/qt5_11.imp" ]
      then
        echo "qt5_11.imp not found; set IWYU_SOURCE, skipping ${TOOL}"
        SKIPPED+=(${TOOL})
        continue
      fi

      IWYU_SOURCE=${IWYU_SOURCE} scripts/iwyu.sh
      ;;
  esac
done

[ -n "${SHIM_DIR}" ] && rm -rf ${SHIM_DIR}

if [ -n "${DEFECTS_DIR}" ]
then
  echo "===== Summary"

  FILTERS=(".*\/thirdparty\/.*" ".*\/Qt.*" "^\/usr\/.*" ".*\/build\/.*" "^$(pwd)\/")
  FILTER_ARGUMENTS=()
  for FILTER in "${FILTERS[@]}"
  do
    FILTER_ARGUMENTS+=(--filter "${FILTER}")
  done

  ${DEFECTS_DIR}/compiler-logs-to-table.pl --summary --markdown "${FILTER_ARGUMENTS[@]}" ${BUILD_DIR}/*.log
fi

echo "Logs:"
ls -1 ${BUILD_DIR}/*.log

if [ ${#SKIPPED[@]} -gt 0 ]
then
  echo "Skipped: ${SKIPPED[*]}"
  exit 1
fi
