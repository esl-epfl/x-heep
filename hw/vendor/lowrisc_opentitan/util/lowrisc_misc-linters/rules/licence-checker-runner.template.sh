#!/usr/bin/env bash
# Copyright lowRISC contributors.
# Licensed under the Apache License, Version 2.0, see LICENSE for details.
# SPDX-License-Identifier: Apache-2.0

# Remember the runfiles dir since this is what paths like "@@LICENCE_CHECKER@@"
# and "@@CONFIG@@" are relative to once we `cd` elsewhere.
RUNFILES_DIR="$PWD"

WORKSPACE="@@WORKSPACE@@"

if [[ ! -z "${WORKSPACE}" ]]; then
    REPO="$(dirname "$(realpath ${WORKSPACE})")"
    cd ${REPO} || exit 1
elif [[ ! -z "${BUILD_WORKSPACE_DIRECTORY+is_set}" ]]; then
    cd ${BUILD_WORKSPACE_DIRECTORY} || exit 1
else
    echo "Neither WORKSPACE nor BUILD_WORKSPACE_DIRECTORY were set."
    echo "If this is a test rule, add 'workspace = \"//:WORKSPACE\"' to your rule."
    exit 1
fi

"${RUNFILES_DIR}/@@LICENCE_CHECKER@@" --config="${RUNFILES_DIR}/@@CONFIG@@" "$@"
