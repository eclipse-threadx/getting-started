# Copyright (c) Microsoft Corporation.
# Licensed under the MIT License.

set(SOURCES
    src/Config_SCI8/Config_SCI8.c
    src/Config_SCI8/Config_SCI8_user.c

    src/general/r_cg_hardware_setup.c
    src/general/r_smc_cgc.c
    src/general/r_smc_cgc_user.c
    src/general/r_smc_interrupt.c

    src/r_bsp/board/generic_rx65n/hwsetup.c

    src/r_bsp/mcu/all/dbsct.c
    src/r_bsp/mcu/all/lowlvl.c
    src/r_bsp/mcu/all/lowsrc.c
    src/r_bsp/mcu/all/mcu_locks.c
    src/r_bsp/mcu/all/r_bsp_common.c
    src/r_bsp/mcu/all/r_bsp_cpu.c
    src/r_bsp/mcu/all/r_bsp_interrupts.c
    src/r_bsp/mcu/all/r_bsp_locking.c
    src/r_bsp/mcu/all/r_bsp_mcu_startup.c
    src/r_bsp/mcu/all/r_bsp_software_interrupt.c
    src/r_bsp/mcu/all/r_rx_intrinsic_functions.c
    src/r_bsp/mcu/all/reset_program.S
    src/r_bsp/mcu/all/resetprg.c
    src/r_bsp/mcu/all/sbrk.c

    src/r_bsp/mcu/rx65n/mcu_clocks.c
    src/r_bsp/mcu/rx65n/mcu_init.c
    src/r_bsp/mcu/rx65n/mcu_interrupts.c
    src/r_bsp/mcu/rx65n/mcu_mapped_interrupts.c
    src/r_bsp/mcu/rx65n/vecttbl.c

    src/r_cmt_rx/src/r_cmt_rx.c

    src/r_ether_rx/src/phy/phy.c
    src/r_ether_rx/src/r_ether_rx.c
    src/r_ether_rx/src/targets/rx65n/r_ether_setting_rx65n.c

    src/r_pincfg/r_ether_rx_pinset.c
    src/r_pincfg/Pin.c
)

set(TARGET rx_driver_package)

add_library(${TARGET} OBJECT
    ${SOURCES}
)

target_include_directories(${TARGET}
    PUBLIC
        src/r_bsp
        src/r_config
        src/r_cmt_rx
        src/Config_SCI8
        src/r_ether_rx
        src/general
        src/r_pincfg
)
