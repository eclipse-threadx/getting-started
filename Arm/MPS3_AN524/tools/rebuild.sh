# Copyright (c) 2021 Linaro Limited.
# Licensed under the MIT License.

#!/bin/bash

# Check if user wishes to auto update application image onto connected board
# after successful build.
HELP_STR="
Usage: rebuild.sh [-h] [-q] [-f] [-p=serial port] \n \
    Build Azure RTOS application image \n \
    Optional: \n \
    \t-h : Show this message \n \
    \t-c : Clean the build directory before building \n \
    \t-q : Use QEMU for running application \n \
    \t-f : Update generated image onto connected board \n \
    \t-p=serial port : MCC Serial port name, mandatory with -f option
"

flash_option=false
while getopts "hcqfp:" option; do
    case ${option} in
        f )
            flash_option=true
            ;;
        p )
            if [ "$flash_option" = false ]; then
                echo "ERROR: -p option can only be used with -f option"
                exit 1
            fi
            serial_port=$OPTARG
            ;;
        q )
            use_qemu=true
            ;;
        c )
            clean_first=true
            ;;
        h )
            echo $HELP_STR
            exit 1
            ;;
        \? )
            echo $HELP_STR
            exit 1
    esac
done
# shift $((OPTIND -1))
if [ "$flash_option" = true ] && [ "$serial_port" = "" ]; then
    echo "ERROR: -p option is mandatory when -f option is used"
    exit 1
fi

# Use paths relative to this script's location
SCRIPT=$(realpath "$0")
SCRIPTDIR=$(dirname "$SCRIPT")
BASEDIR=$(dirname "$SCRIPTDIR")
BUILDTYPE=Debug

# echo $BASEDIR

# If you want to build into a different directory, change this variable
BUILDDIR="$BASEDIR/build"

# Ensure the bulid directory exists
mkdir -p $BUILDDIR
# When a clean build is specified, wipe away all chance at state
if [ "$clean_first" = true ]; then
    rm -rf $BUILDDIR/*
fi

# Generate the build system using Ninja
cmake -B"$BUILDDIR" -GNinja -DCMAKE_BUILD_TYPE=$BUILDTYPE $BASEDIR

# And then do the build
cmake --build $BUILDDIR

# Check if build succeeded and running on QEMU is requested
if [ $? -eq 0 ] && [ "$use_qemu" = true ]; then
    # Check if srec_cat is installed
    if ! command -v srec_cat &> /dev/null; then
        echo "srec_cat is required to merge TF-M and Azure RTOS example. Install it via \n \
$ sudo apt-get install srecord on Ubuntu or \n \
$ brew install srecord on OS X."

        exit 1
    fi
    # Before we launch QEMU, we need to merge TF-M and Azure RTOS example
    # application images
    srec_cat $BUILDDIR/tfm/bin/bl2.bin -Binary -offset 0x10000000 $BUILDDIR/tfm_s_ns_signed.bin -Binary -offset 0x10040000 -o $BUILDDIR/tfm_full.hex -Intel
    echo "Starting QEMU"
    qemu-system-arm -M mps3-an524 -M remap=QSPI -device loader,file=$BUILDDIR/tfm_full.hex -serial stdio
fi

# Check if build succeeded
if [ $? -eq 0 ] && [ "$flash_option" = true ]; then
    python3 $SCRIPTDIR/flash.py $serial_port
fi
