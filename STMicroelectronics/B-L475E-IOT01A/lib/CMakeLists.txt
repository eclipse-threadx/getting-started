include(CPM)

# define the cpu architecture for threadx
set(THREADX_ARCH "cortex_m4")
set(THREADX_TOOLCHAIN "gnu")

# threadX
set(TX_USER_FILE "${CMAKE_CURRENT_LIST_DIR}/threadx/tx_user.h" CACHE STRING "")
CPMAddPackage(
    NAME threadx
#    GIT_REPOSITORY https://github.com/azurertos/threadx.git
#    GIT_TAG master
    # contains my custom tx_initialise_low_level.S for the SYSTEM_CLOCK override
    GIT_REPOSITORY https://github.com/ryanwinter/threadx.git
    GIT_TAG STM32L475
)

add_subdirectory(netxduo)

add_subdirectory(stm32cubel4)
add_subdirectory(netx_driver)
