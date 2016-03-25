#! /bin/bash

TAG=$(git describe --always)
BRANCH=$(git rev-parse --abbrev-ref HEAD)

if [ "${BRANCH}" == "master" ];
then
    echo ${TAG}
else
    echo ${TAG}-${BRANCH}
fi
