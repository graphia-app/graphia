#! /bin/bash

DESCRIPTION=$(git describe --always)
TAG=$(git describe --abbrev=0)
COMMITS_SINCE_TAG=$(git rev-list ${TAG}..HEAD --count)
BRANCH=$(git rev-parse --abbrev-ref HEAD)

if [ "${BRANCH}" == "master" ] || [ "${COMMITS_SINCE_TAG}" == "0" ];
then
    echo ${DESCRIPTION}
else
    echo ${DESCRIPTION}-${BRANCH}
fi
