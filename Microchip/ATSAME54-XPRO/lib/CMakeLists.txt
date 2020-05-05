include(CPM)

# define the cpu architecture for threadx
set(THREADX_ARCH "cortex_m4")
set(THREADX_TOOLCHAIN "gnu")

# threadX
#set(TX_USER_FILE "${CMAKE_CURRENT_LIST_DIR}/threadx/tx_user.h" CACHE STRING "")
#CPMAddPackage(
#    NAME threadx
#    GIT_REPOSITORY https://github.com/azurertos/threadx.git #git@github.com:azurertos/threadx.git
#    GIT_TAG master
#)

# threadX
set(TX_USER_FILE "${CMAKE_CURRENT_LIST_DIR}/threadx/tx_user.h" CACHE STRING "")
CPMAddPackage(
    NAME threadx
    # contains my custom tx_initialise_low_level.S for the SYSTEM_CLOCK override
    GIT_REPOSITORY https://github.com/ryanwinter/threadx.git
    GIT_TAG AECLOUD2
)

# netX-duo
set(NXD_USE_SNTP true)
set(NXD_USE_DHCP true)
set(NXD_USE_DNS true)
set(NXD_USE_MQTT true)
set(NX_USER_FILE "${CMAKE_CURRENT_LIST_DIR}/netx_duo/nx_user.h" CACHE STRING "")
CPMAddPackage(
    NAME netxduo
    GIT_REPOSITORY https://github.com/azurertos/netx-duo.git #git@github.com:azurertos/netx-duo.git
    GIT_TAG master
)

add_subdirectory(netx_driver)
add_subdirectory(atmel_start)