# Copyright (c) Microsoft Corporation.
# Licensed under the MIT License.

#!/bin/sh

sudo apt-get update
sudo apt-get install apt-transport-https ca-certificates gnupg software-properties-common wget 

wget -O - https://apt.kitware.com/keys/kitware-archive-latest.asc 2>/dev/null | sudo apt-key add -

sudo apt-add-repository 'deb https://apt.kitware.com/ubuntu/ bionic main'

sudo apt-get update
sudo apt-get install  gcc-arm-none-eabi ninja-build cmake 
