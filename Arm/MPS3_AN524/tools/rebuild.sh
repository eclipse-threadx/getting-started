# Copyright (c) 2021 Linaro Limited.
# Licensed under the MIT License.

#!/bin/bash

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
