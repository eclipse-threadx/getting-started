# "Generic" is used when cross compiling
SET(CMAKE_SYSTEM_NAME Generic)

# Optional, this one not so much
SET(CMAKE_SYSTEM_VERSION 1)

SET(RX_TOOLCHAIN_PATH "C:/Progra~2/Renesas/RX/3_2_0/bin")

# CMake variables for compiler, assembler, native build system
# Specify the C compiler
SET(CMAKE_C_COMPILER ${RX_TOOLCHAIN_PATH}/ccrx.exe)
SET(CMAKE_C_COMPILER_ID RXC)
SET(CMAKE_C_COMPILER_ID_RUN TRUE)

#force CXX compiler configuration
SET(CMAKE_C_COMPILER_FORCED TRUE)

# Specify the CPP compiler
SET(CMAKE_CXX_COMPILER ${RX_TOOLCHAIN_PATH}/ccrx.exe)
SET(CMAKE_CXX_COMPILER_ID RXC)
SET(CMAKE_CXX_COMPILER_ID_RUN TRUE)
SET(CMAKE_CXX_COMPILER_FORCED TRUE)

# Specify the ASM compiler
SET(CMAKE_ASM_COMPILER ${RX_TOOLCHAIN_PATH}/asrx.exe)
SET(CMAKE_ASM_COMPILER_ID GNU)
SET(CMAKE_ASM_COMPILER_ID_RUN TRUE)
SET(CMAKE_ASM_COMPILER_FORCED TRUE)

# Specify the linker
SET(CMAKE_C_LINK_EXECUTABLE ${RX_TOOLCHAIN_PATH}/rlink.exe)
SET(CMAKE_CXX_LINK_EXECUTABLE ${RX_TOOLCHAIN_PATH}/rlink.exe)

# use the repo version of ninja on Windows as there is no Ninja installer
SET(CMAKE_MAKE_PROGRAM ${CMAKE_CURRENT_LIST_DIR}/ninja CACHE STRING "Ninja location")
