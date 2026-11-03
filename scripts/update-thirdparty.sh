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

# Usage: update-thirdparty.sh [library[@version]]...
#
# Replaces each library in source/thirdparty with its latest upstream release,
# or the given tag or commit, keeps the files we own, then applies
# patches/*.diff. With no arguments it updates every library.

set -euo pipefail

ROOT=$(git -C "$(dirname "$0")" rev-parse --show-toplevel)
WORK=$(mktemp -d)
trap 'rm -rf "$WORK"' EXIT

latestTag() # url regex
{
    git ls-remote --tags --refs "$1" | sed 's|.*refs/tags/||' | \
        grep -E "$2" | sort -V | tail -1
}

fetch() # url ref destination
{
    git init -q "$3"
    git -C "$3" fetch -q --depth 1 "$1" "$2"
    git -C "$3" -c advice.detachedHead=false checkout -q FETCH_HEAD
    rm -rf "$3/.git"
}

# Each library function answers:
#   latest              the newest release tag
#   stage <ref> <dir>   lay that version out in <dir>
#   own                 globs of the files Graphia adds
#   notes               what the script can't do for it

lexy()
{
    local url=https://github.com/foonathan/lexy.git
    case $1 in
        latest) latestTag $url '^v[0-9.]+$' ;;
        stage) fetch $url "$2" "$3" ;;
    esac
}

libraries=(lexy)

# The repository stores thirdparty text with LF endings, which the patches assume
normaliseLineEndings() # directory
{
    find "$1" -type f -exec grep -qI $'\r' {} \; -exec sed -i 's/\r$//' {} +
}

keepOwnFiles() # library from to
{
    (
        cd "$2"
        shopt -s nullglob
        local file
        for file in $($1 own) patches; do
            if [[ -e $file ]]; then cp -a --parents "$file" "$3/"; fi
        done
    )
}

applyPatches() # directory
{
    local patch
    for patch in "$1"/patches/*.diff; do
        [[ -e $patch ]] || continue

        if git -C "$ROOT" apply --check "$patch" 2> /dev/null; then
            git -C "$ROOT" apply "$patch"
            echo "  applied ${patch##*/}"
        elif git -C "$ROOT" apply --reverse --check "$patch" 2> /dev/null; then
            echo "  ${patch##*/} is upstream now; delete it"
        else
            echo "  ${patch##*/} FAILED"
            git -C "$ROOT" apply --reject "$patch" 2>&1 | grep -E '^error:' | sed 's/^/    /' || true
        fi
    done
}

reportSourceChanges() # directory
{
    git -C "$ROOT" status --porcelain --untracked-files=all -- "$1" | \
        grep -E '^(\?\?| D)' | grep -E '\.(c|cc|cpp)$' | \
        sed -e 's/^?? /  new source /' -e 's/^ D /  removed source /' || true
}

update() # library version
{
    local library=$1 version=$2
    local directory=$ROOT/source/thirdparty/$library
    local stage=$WORK/$library

    if [[ -n $(git -C "$ROOT" status --porcelain -- "$directory" \
        ":(exclude)$directory/patches") ]]; then
        echo "$library has uncommitted changes; skipping it"
        return
    fi

    echo "$library → $version"
    mkdir "$stage"
    $library stage "$version" "$stage"
    rm -rf "$WORK/upstream"
    normaliseLineEndings "$stage"
    keepOwnFiles "$library" "$directory" "$stage"

    rm -rf "$directory"
    mv "$stage" "$directory"

    applyPatches "$directory"
    reportSourceChanges "$directory"

    local notes
    notes=$($library notes)
    [[ -n $notes ]] && echo "  $notes"
    return 0
}

(( $# )) || set -- "${libraries[@]}"

for argument in "$@"; do
    library=${argument%%@*}

    if [[ ! " ${libraries[*]} " =~ " $library " ]]; then
        echo "Unknown library $library; choose from: ${libraries[*]}"
        exit 1
    fi

    version=${argument#"$library"}
    version=${version#@}
    [[ -n $version ]] || version=$($library latest)

    update "$library" "$version"
done
