rd /S /Q _build
cmake -B_build -GNinja -DCMAKE_TOOLCHAIN_FILE="../../cmake/arm-gcc-cortex-m4.cmake" -DCPM_SOURCE_CACHE="%USERPROFILE%/.cpm"
cmake --build _build