/*
 * Copyright (c) 2016, Freescale Semiconductor, Inc.
 * Copyright 2016-2020 NXP
 * All rights reserved.
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _RTE_DEVICE_H
#define _RTE_DEVICE_H

#include "pin_mux.h"

/* UART Select, UART0 - UART5. */
/* User needs to provide the implementation of LPUARTX_GetFreq/LPUARTX_InitPins/LPUARTX_DeinitPins for the enabled
 * LPUART instance. */
#define RTE_USART1        1
#define RTE_USART1_DMA_EN 1
#define RTE_USART2        0
#define RTE_USART2_DMA_EN 0
#define RTE_USART3        0
#define RTE_USART3_DMA_EN 0
#define RTE_USART4        0
#define RTE_USART4_DMA_EN 0
#define RTE_USART5        0
#define RTE_USART5_DMA_EN 0
#define RTE_USART6        0
#define RTE_USART6_DMA_EN 0
#define RTE_USART7        0
#define RTE_USART7_DMA_EN 0
#define RTE_USART8        0
#define RTE_USART8_DMA_EN 0

/* UART configuration. */
#define RTE_USART1_PIN_INIT           LPUART1_InitPins
#define RTE_USART1_PIN_DEINIT         LPUART1_DeinitPins
#define RTE_USART1_DMA_TX_CH          0
#define RTE_USART1_DMA_TX_PERI_SEL    (uint8_t) kDmaRequestMuxLPUART1Tx
#define RTE_USART1_DMA_TX_DMAMUX_BASE DMAMUX
#define RTE_USART1_DMA_TX_DMA_BASE    DMA0
#define RTE_USART1_DMA_RX_CH          1
#define RTE_USART1_DMA_RX_PERI_SEL    (uint8_t) kDmaRequestMuxLPUART1Rx
#define RTE_USART1_DMA_RX_DMAMUX_BASE DMAMUX
#define RTE_USART1_DMA_RX_DMA_BASE    DMA0

#define RTE_USART2_PIN_INIT           LPUART2_InitPins
#define RTE_USART2_PIN_DEINIT         LPUART2_DeinitPins
#define RTE_USART2_DMA_TX_CH          2
#define RTE_USART2_DMA_TX_PERI_SEL    (uint8_t) kDmaRequestMuxLPUART2Tx
#define RTE_USART2_DMA_TX_DMAMUX_BASE DMAMUX
#define RTE_USART2_DMA_TX_DMA_BASE    DMA0
#define RTE_USART2_DMA_RX_CH          3
#define RTE_USART2_DMA_RX_PERI_SEL    (uint8_t) kDmaRequestMuxLPUART2Rx
#define RTE_USART2_DMA_RX_DMAMUX_BASE DMAMUX
#define RTE_USART2_DMA_RX_DMA_BASE    DMA0

#define RTE_USART3_PIN_INIT           LPUART3_InitPins
#define RTE_USART3_PIN_DEINIT         LPUART3_DeinitPins
#define RTE_USART3_DMA_TX_CH          4
#define RTE_USART3_DMA_TX_PERI_SEL    (uint8_t) kDmaRequestMuxLPUART3Tx
#define RTE_USART3_DMA_TX_DMAMUX_BASE DMAMUX
#define RTE_USART3_DMA_TX_DMA_BASE    DMA0
#define RTE_USART3_DMA_RX_CH          5
#define RTE_USART3_DMA_RX_PERI_SEL    (uint8_t) kDmaRequestMuxLPUART3Rx
#define RTE_USART3_DMA_RX_DMAMUX_BASE DMAMUX
#define RTE_USART3_DMA_RX_DMA_BASE    DMA0

#define RTE_USART4_PIN_INIT           LPUART4_InitPins
#define RTE_USART4_PIN_DEINIT         LPUART4_DeinitPins
#define RTE_USART4_DMA_TX_CH          6
#define RTE_USART4_DMA_TX_PERI_SEL    (uint8_t) kDmaRequestMuxLPUART4Tx
#define RTE_USART4_DMA_TX_DMAMUX_BASE DMAMUX
#define RTE_USART4_DMA_TX_DMA_BASE    DMA0
#define RTE_USART4_DMA_RX_CH          7
#define RTE_USART4_DMA_RX_PERI_SEL    (uint8_t) kDmaRequestMuxLPUART4Rx
#define RTE_USART4_DMA_RX_DMAMUX_BASE DMAMUX
#define RTE_USART4_DMA_RX_DMA_BASE    DMA0

#define RTE_USART5_PIN_INIT           LPUART5_InitPins
#define RTE_USART5_PIN_DEINIT         LPUART5_DeinitPins
#define RTE_USART5_DMA_TX_CH          8
#define RTE_USART5_DMA_TX_PERI_SEL    (uint8_t) kDmaRequestMuxLPUART5Tx
#define RTE_USART5_DMA_TX_DMAMUX_BASE DMAMUX
#define RTE_USART5_DMA_TX_DMA_BASE    DMA0
#define RTE_USART5_DMA_RX_CH          9
#define RTE_USART5_DMA_RX_PERI_SEL    (uint8_t) kDmaRequestMuxLPUART5Rx
#define RTE_USART5_DMA_RX_DMAMUX_BASE DMAMUX
#define RTE_USART5_DMA_RX_DMA_BASE    DMA0

#define RTE_USART6_PIN_INIT           LPUART6_InitPins
#define RTE_USART6_PIN_DEINIT         LPUART6_DeinitPins
#define RTE_USART6_DMA_TX_CH          10
#define RTE_USART6_DMA_TX_PERI_SEL    (uint8_t) kDmaRequestMuxLPUART6Tx
#define RTE_USART6_DMA_TX_DMAMUX_BASE DMAMUX
#define RTE_USART6_DMA_TX_DMA_BASE    DMA0
#define RTE_USART6_DMA_RX_CH          11
#define RTE_USART6_DMA_RX_PERI_SEL    (uint8_t) kDmaRequestMuxLPUART6Rx
#define RTE_USART6_DMA_RX_DMAMUX_BASE DMAMUX
#define RTE_USART6_DMA_RX_DMA_BASE    DMA0

#define RTE_USART7_PIN_INIT           LPUART7_InitPins
#define RTE_USART7_PIN_DEINIT         LPUART7_DeinitPins
#define RTE_USART7_DMA_TX_CH          12
#define RTE_USART7_DMA_TX_PERI_SEL    (uint8_t) kDmaRequestMuxLPUART7Tx
#define RTE_USART7_DMA_TX_DMAMUX_BASE DMAMUX
#define RTE_USART7_DMA_TX_DMA_BASE    DMA0
#define RTE_USART7_DMA_RX_CH          13
#define RTE_USART7_DMA_RX_PERI_SEL    (uint8_t) kDmaRequestMuxLPUART7Rx
#define RTE_USART7_DMA_RX_DMAMUX_BASE DMAMUX
#define RTE_USART7_DMA_RX_DMA_BASE    DMA0

#define RTE_USART8_PIN_INIT           LPUART8_InitPins
#define RTE_USART8_PIN_DEINIT         LPUART8_DeinitPins
#define RTE_USART8_DMA_TX_CH          14
#define RTE_USART8_DMA_TX_PERI_SEL    (uint8_t) kDmaRequestMuxLPUART8Tx
#define RTE_USART8_DMA_TX_DMAMUX_BASE DMAMUX
#define RTE_USART8_DMA_TX_DMA_BASE    DMA0
#define RTE_USART8_DMA_RX_CH          15
#define RTE_USART8_DMA_RX_PERI_SEL    (uint8_t) kDmaRequestMuxLPUART8Rx
#define RTE_USART8_DMA_RX_DMAMUX_BASE DMAMUX
#define RTE_USART8_DMA_RX_DMA_BASE    DMA0

/* ENET configuration. */
#define RTE_ENET             1
#define RTE_ENET_PHY_ADDRESS 2
#define RTE_ENET_MII         0
#define RTE_ENET_RMII        1

#endif /* _RTE_DEVICE_H */
