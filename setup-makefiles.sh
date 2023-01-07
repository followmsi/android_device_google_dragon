#!/bin/bash
#
# Copyright (C) 2017-2018 The Android Open Source Project
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#

set -e

DEVICE=dragon
VENDOR=google
INITIAL_COPYRIGHT_YEAR=2019

# Load extract_utils and do some sanity checks
MY_DIR="${BASH_SOURCE%/*}"
if [[ ! -d "$MY_DIR" ]]; then MY_DIR="$PWD"; fi

MK_ROOT="$MY_DIR"/../../..

HELPER="$MK_ROOT"/vendor/arrow/build/tools/extract_utils.sh
if [ ! -f "$HELPER" ]; then
    echo "Unable to find helper script at $HELPER"
    exit 1
fi
. "$HELPER"

# Initialize the helper
setup_vendor "$DEVICE" "$VENDOR" "$MK_ROOT" false

# Copyright headers and guards
write_headers "$DEVICE"

# The standard blobs
write_makefiles "$MY_DIR"/proprietary-files.txt true

# Deal with files that needs to be put in ramdisk
if [ -f proprietary-files-ramdisk.txt ]; then
    # These blobs also need to be put in vendor (for System-as-Root)
    write_makefiles "$MY_DIR"/proprietary-files-ramdisk.txt true

    # We will change PRODUCTMK to create a new file.
    # Save original value and restore when done.
    SAVED_PRODUCTMK=$PRODUCTMK
    export PRODUCTMK=${PRODUCTMK/%.mk/-ramdisk.mk}

    # Copyright headers but not guards
    write_makefile_header "$PRODUCTMK"

    # General copy routine
    parse_file_list proprietary-files-ramdisk.txt

    write_product_copy_files

    # The key: replace system/ in target with ramdisk/
    sed -i 's|:..TARGET_COPY_OUT_VENDOR.|:ramdisk/vendor|g' "$PRODUCTMK"

    # Include ramdisk file list
    printf "\n\$(call inherit-product,vendor/$VENDOR/$DEVICE/$(basename $PRODUCTMK))" >> $SAVED_PRODUCTMK

    # Restore file name
    export PRODUCTMK=$SAVED_PRODUCTMK
    unset SAVED_PRODUCTMK
fi

# Finish
write_footers

