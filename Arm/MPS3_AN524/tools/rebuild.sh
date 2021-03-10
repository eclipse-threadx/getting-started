# Copyright (c) 2021 Linaro Limited.
# Licensed under the MIT License.

#!/bin/bash

# Check if user wishes to auto update application image onto connected board
# after successful build.
HELP_STR="
Usage: rebuild.sh [-f] [-p=serial port] \n \
    Build Azure RTOS application image \n \
    Optional: \n \
    \t-f : Update generated image onto connected board \n \
    \t-p=serial port : MCC Serial port name, mandatory with -f option
"

flash_option=false
while getopts "hfp:" option; do
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

# Create our build folder if required and clear it
mkdir -p $BUILDDIR
rm -rf $BUILDDIR/*

# Generate the build system using Ninja
cmake -B"$BUILDDIR" -GNinja -DCMAKE_BUILD_TYPE=$BUILDTYPE $BASEDIR

# And then do the build
cmake --build $BUILDDIR

# Check if build succeeded
if [ $? -eq 0 ] && [ "$flash_option" = true ]; then
    python3 $SCRIPTDIR/flash.py $serial_port
fi
