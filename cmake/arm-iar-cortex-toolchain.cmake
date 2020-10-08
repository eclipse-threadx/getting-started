set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_SYSTEM_PROCESSOR arm)

# Set the EW installation root directory
set(EW_ROOT_DIR "C:/Program Files (x86)/IAR Systems/Embedded Workbench 8.4/arm")

# Do some windows specific logic
if(WIN32)
    # use the repo version of ninja on Windows as there is no Ninja installer
    set(CMAKE_MAKE_PROGRAM ${CMAKE_CURRENT_LIST_DIR}/ninja CACHE STRING "Ninja location")
endif(WIN32)

# perform compiler test with the static library
set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)

# Set up the CMake variables for compiler and assembler
set(CMAKE_C_COMPILER "${EW_ROOT_DIR}/bin/iccarm.exe" "${CPU_FLAGS} --dlib_config normal")
set(CMAKE_CXX_COMPILER "${EW_ROOT_DIR}/bin/iccarm.exe" "${CPU_FLAGS} --dlib_config normal")
set(CMAKE_ASM_COMPILER "${EW_ROOT_DIR}/bin/iasmarm.exe" "${CPU_FLAGS}")
set(CMAKE_ELFTOOL "${EW_ROOT_DIR}/bin/ielftool.exe")

# Set up the CMake variables for the linker
set(CMAKE_C_LINK_FLAGS "--semihosting")
set(CMAKE_CXX_LINK_FLAGS "--semihosting")
