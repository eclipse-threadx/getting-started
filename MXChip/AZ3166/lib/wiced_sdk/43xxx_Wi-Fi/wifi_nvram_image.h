/*
 * Copyright 2019, Cypress Semiconductor Corporation or a subsidiary of 
 * Cypress Semiconductor Corporation. All Rights Reserved.
 * 
 * This software, associated documentation and materials ("Software"),
 * is owned by Cypress Semiconductor Corporation
 * or one of its subsidiaries ("Cypress") and is protected by and subject to
 * worldwide patent protection (United States and foreign),
 * United States copyright laws and international treaty provisions.
 * Therefore, you may use this Software only as provided in the license
 * agreement accompanying the software package from which you
 * obtained this Software ("EULA").
 * If no EULA applies, Cypress hereby grants you a personal, non-exclusive,
 * non-transferable license to copy, modify, and compile the Software
 * source code solely for use in connection with Cypress's
 * integrated circuit products. Any reproduction, modification, translation,
 * compilation, or representation of this Software except as specified
 * above is prohibited without the express written permission of Cypress.
 *
 * Disclaimer: THIS SOFTWARE IS PROVIDED AS-IS, WITH NO WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING, BUT NOT LIMITED TO, NONINFRINGEMENT, IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE. Cypress
 * reserves the right to make changes to the Software without notice. Cypress
 * does not assume any liability arising out of the application or use of the
 * Software or any product or circuit described in the Software. Cypress does
 * not authorize its products for use in any products where a malfunction or
 * failure of the Cypress product may reasonably be expected to result in
 * significant property damage, injury or death ("High Risk Product"). By
 * including Cypress's product in a High Risk Product, the manufacturer
 * of such system or application assumes all risk of such use and in doing
 * so agrees to indemnify Cypress against all liability.
 */

/** @file
 *  NVRAM variables which define BCM43362 Parameters for the
 *  USI module used on the BCM943362WCD6_3 board
 *
 */

#ifndef INCLUDED_NVRAM_IMAGE_H_
#define INCLUDED_NVRAM_IMAGE_H_

#include <string.h>
#include <stdint.h>
#ifdef MODUSTOOLBOX
#include "generated_mac_address.txt"
#else
#include "../generated_mac_address.txt"
#endif

#ifdef __cplusplus
extern "C" {
#endif


/**
 * Character array of NVRAM image
 */

static const char wifi_nvram_image[] =
        "manfid=0x2d0"                                                       "\x00"
        "prodid=0x492"                                                       "\x00"
        "vendid=0x14e4"                                                      "\x00"
        "devid=0x4343"                                                       "\x00"
        "boardtype=0x05a0"                                                   "\x00"
        "boardrev=0x1301"                                                    "\x00" /*Board Revision is REV3.1*/
        "boardnum=777"                                                       "\x00"
        "xtalfreq=26000"                                                     "\x00"
        "boardflags=0xa00"                                                   "\x00"
        "sromrev=3"                                                          "\x00"
        "wl0id=0x431b"                                                       "\x00"
        NVRAM_GENERATED_MAC_ADDRESS                                          "\x00"
        "aa2g=3"                                                             "\x00"
        "ag0=2"                                                              "\x00"
        "maxp2ga0=68"                                                        "\x00"
        "ofdm2gpo=0x44444444"                                                "\x00"
        "mcs2gpo0=0x3333"                                                    "\x00"
        "mcs2gpo1=0x6333"                                                    "\x00"
        "pa0maxpwr=80"                                                       "\x00"
        "pa0b0=0x133E"                                                       "\x00"
        "pa0b1=0xFDBA"                                                       "\x00"
        "pa0b2=0xFF53"                                                       "\x00"
        "pa0itssit=62"                                                       "\x00"
        "pa1itssit=62"                                                       "\x00"
        "temp_based_dutycy_en=1"                                             "\x00"
        "tx_duty_cycle_ofdm=100"                                             "\x00"
        "tx_duty_cycle_cck=100"                                              "\x00"
        "tx_ofdm_temp_0=115"                                                 "\x00"
        "tx_cck_temp_0=115"                                                  "\x00"
        "tx_ofdm_dutycy_0=40"                                                "\x00"
        "tx_cck_dutycy_0=40"                                                 "\x00"
        "tx_ofdm_temp_1=255"                                                 "\x00"
        "tx_cck_temp_1=255"                                                  "\x00"
        "tx_ofdm_dutycy_1=40"                                                "\x00"
        "tx_cck_dutycy_1=40"                                                 "\x00"
        "tx_tone_power_index=40"                                             "\x00"
        "tx_tone_power_index.fab.3=48"                                       "\x00"
        "cckPwrOffset=0"                                                     "\x00"
        "ccode=0"                                                            "\x00"
        "rssismf2g=0xa"                                                      "\x00"
        "rssismc2g=0x3"                                                      "\x00"
        "rssisav2g=0x7"                                                      "\x00"
        "triso2g=0"                                                          "\x00"
        "noise_cal_enable_2g=0"                                              "\x00"
        "noise_cal_po_2g=0"                                                  "\x00"
        "noise_cal_po_2g.fab.3=-2"                                           "\x00"
        "swctrlmap_2g=0x050c050c,0x030a030a,0x030a030a,0x0,0x1ff"            "\x00"
        "temp_add=29767"                                                     "\x00"
        "temp_mult=425"                                                      "\x00"
        "temp_q=10"                                                          "\x00"
        "initxidx2g=45"                                                      "\x00"
        "tssitime=1"                                                         "\x00"
        "rfreg033=0x19"                                                      "\x00"
        "rfreg033_cck=0x1f"                                                  "\x00"
        "cckPwrIdxCorr=-8"                                                   "\x00"
        "spuravoid_enable2g=1"                                               "\x00"
        "edonthd=-65"                                                        "\x00"
        "edoffthd=-71"                                                       "\x00"
        "\x00\x00";

#else /* ifndef INCLUDED_NVRAM_IMAGE_H_ */

#error Wi-Fi NVRAM image included twice

#endif /* ifndef INCLUDED_NVRAM_IMAGE_H_ */

#ifdef __cplusplus
} /* extern "C" */
#endif
