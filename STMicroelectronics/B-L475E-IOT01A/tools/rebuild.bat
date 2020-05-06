setlocal
cd /d %~dp0\..

IF EXIST build (rd /S /Q build)

cmake -Bbuild -GNinja -DCMAKE_TOOLCHAIN_FILE="../../cmake/arm-gcc-cortex-m4.cmake"
cmake --build build