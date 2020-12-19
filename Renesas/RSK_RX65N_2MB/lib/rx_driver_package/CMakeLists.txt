# Copyright (c) Microsoft Corporation.
# Licensed under the MIT License.

set(SOURCES
    src/smc_gen/Config_PORT/Config_PORT.c
    src/smc_gen/Config_PORT/Config_PORT_user.c
    src/smc_gen/Config_SCI8/Config_SCI8.c
    src/smc_gen/Config_SCI8/Config_SCI8_user.c

    src/smc_gen/general/r_cg_hardware_setup.c
    src/smc_gen/general/r_smc_cgc.c
    src/smc_gen/general/r_smc_cgc_user.c
    src/smc_gen/general/r_smc_interrupt.c

    src/smc_gen/r_bsp/board/generic_rx65n/hwsetup.c

    src/smc_gen/r_bsp/mcu/all/dbsct.c
    src/smc_gen/r_bsp/mcu/all/lowlvl.c
    src/smc_gen/r_bsp/mcu/all/lowsrc.c
    src/smc_gen/r_bsp/mcu/all/mcu_locks.c
    src/smc_gen/r_bsp/mcu/all/r_bsp_common.c
    src/smc_gen/r_bsp/mcu/all/r_bsp_cpu.c
    src/smc_gen/r_bsp/mcu/all/r_bsp_interrupts.c
    src/smc_gen/r_bsp/mcu/all/r_bsp_locking.c
    src/smc_gen/r_bsp/mcu/all/r_bsp_mcu_startup.c
    src/smc_gen/r_bsp/mcu/all/r_bsp_software_interrupt.c
    src/smc_gen/r_bsp/mcu/all/r_rx_intrinsic_functions.c
    src/smc_gen/r_bsp/mcu/all/reset_program.S
    src/smc_gen/r_bsp/mcu/all/resetprg.c
    src/smc_gen/r_bsp/mcu/all/sbrk.c

    src/smc_gen/r_bsp/mcu/rx65n/mcu_clocks.c
    src/smc_gen/r_bsp/mcu/rx65n/mcu_init.c
    src/smc_gen/r_bsp/mcu/rx65n/mcu_interrupts.c
    src/smc_gen/r_bsp/mcu/rx65n/mcu_mapped_interrupts.c
    src/smc_gen/r_bsp/mcu/rx65n/vecttbl.c

    src/smc_gen/r_cmt_rx/src/r_cmt_rx.c

    src/smc_gen/r_ether_rx/src/phy/phy.c
    src/smc_gen/r_ether_rx/src/r_ether_rx.c
    src/smc_gen/r_ether_rx/src/targets/rx65n/r_ether_setting_rx65n.c

    src/smc_gen/r_pincfg/r_ether_rx_pinset.c
    src/smc_gen/r_pincfg/Pin.c
)

set(TARGET rx_driver_package)

add_library(${TARGET} OBJECT
    ${SOURCES}
)

target_include_directories(${TARGET}
    PUBLIC
        src/smc_gen/general
        src/smc_gen/Config_SCI8
        src/smc_gen/Config_PORT
        src/smc_gen/r_bsp
        src/smc_gen/r_cmt_rx
        src/smc_gen/r_config
        src/smc_gen/r_ether_rx
        src/smc_gen/r_pincfg
)
