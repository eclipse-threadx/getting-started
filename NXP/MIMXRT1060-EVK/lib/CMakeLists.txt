include(CPM)

# define the cpu architecture for threadx
set(THREADX_ARCH "cortex_m7")
set(THREADX_TOOLCHAIN "gnu")

# threadX
set(TX_USER_FILE "${CMAKE_CURRENT_LIST_DIR}/threadx/tx_user.h" CACHE STRING "")
CPMAddPackage(
    NAME threadx
    GIT_REPOSITORY https://github.com/ryanwinter/threadx.git
    GIT_TAG MIMXRT1060
)

# netX-duo
set(NX_USER_FILE "${CMAKE_CURRENT_LIST_DIR}/netxduo/nx_user.h" CACHE STRING "")
CPMAddPackage(
    NAME netxduo
    GIT_REPOSITORY https://github.com/azurertos/netx-duo.git
    GIT_TAG master
)

# filex
#set(FX_USER_FILE "${CMAKE_CURRENT_LIST_DIR}/filex/fx_user.h" CACHE STRING "")
CPMAddPackage(
    NAME filex
    GIT_REPOSITORY https://github.com/azurertos/filex.git
    GIT_TAG master
)

add_subdirectory(netx_driver)
add_subdirectory(MIMXRT1060-evk)
