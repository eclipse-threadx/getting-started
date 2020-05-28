# Copyright (c) Microsoft Corporation.
# Licensed under the MIT License.

#!/bin/bash

# Use paths relative to this script's location
# SCRIPT=$(readlink -f "$0")
# BASEDIR=$(dirname "$SCRIPT")

# If you want to build into a different directory, change this variable
BUILDDIR="build"

# Create our build folder if required and clear it
rm -rf $BUILDDIR

# Generate the build system using Ninja
cmake -B"$BUILDDIR" -GNinja -DCMAKE_TOOLCHAIN_FILE=../../cmake/arm-gcc-cortex-m4.cmake

# And then do the build
# cmake --build "./$BUILDDIR" 2>&1 | tee ninja-build.log
cmake --build "./$BUILDDIR"