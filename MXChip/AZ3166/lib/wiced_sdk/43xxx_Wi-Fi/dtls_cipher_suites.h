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

#pragma once

#include <stdint.h>
#include "cipher_suites.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifdef BESL
//#ifndef WICED_USE_CUSTOM_DTLS_CIPHER_SUITES

    /* Default supported key schemes */
    #define USE_DTLS_PSK_KEYSCHEME
    #define USE_DTLS_ECDHE_ECDSA_KEYSCHEME

    /* Default supported MACs */
    #define USE_DTLS_AES_128_CCM_8_MAC

    /* Default supported ciphers */
    #define USE_DTLS_AES_128_CCM_8_CIPHER

//#endif /* #ifndef WICED_USE_CUSTOM_CIPHER_SUITES */


/* Note - The following code was generated using the conv_cipher_suite.pl script */
/* See https://www.iana.org/assignments/tls-parameters/tls-parameters.xhtml#tls-parameters-4 */


#if defined( USE_DTLS_NULL_KEYSCHEME ) && defined( USE_DTLS_NULL_CIPHER ) && defined( USE_DTLS_NULL_MAC )
extern const cipher_suite_t DTLS_NULL_WITH_NULL_NULL;
#endif /* if defined( USE_DTLS_NULL_KEYSCHEME ) && defined( USE_DTLS_NULL_CIPHER ) && defined( USE_DTLS_NULL_MAC ) */

#if defined( USE_DTLS_RSA_KEYSCHEME ) && defined( USE_DTLS_NULL_CIPHER ) && defined( USE_DTLS_MD5_MAC )
extern const cipher_suite_t DTLS_RSA_WITH_NULL_MD5;
#endif /* if defined( USE_DTLS_RSA_KEYSCHEME ) && defined( USE_DTLS_NULL_CIPHER ) && defined( USE_DTLS_MD5_MAC ) */

#if defined( USE_DTLS_RSA_KEYSCHEME ) && defined( USE_DTLS_NULL_CIPHER ) && defined( USE_DTLS_SHA_MAC )
extern const cipher_suite_t DTLS_RSA_WITH_NULL_SHA;
#endif /* if defined( USE_DTLS_RSA_KEYSCHEME ) && defined( USE_DTLS_NULL_CIPHER ) && defined( USE_DTLS_SHA_MAC ) */

#if defined( USE_DTLS_RSA_EXPORT_KEYSCHEME ) && defined( USE_DTLS_RC4_40_CIPHER ) && defined( USE_DTLS_MD5_MAC )
extern const cipher_suite_t DTLS_RSA_EXPORT_WITH_RC4_40_MD5;
#endif /* if defined( USE_DTLS_RSA_EXPORT_KEYSCHEME ) && defined( USE_DTLS_RC4_40_CIPHER ) && defined( USE_DTLS_MD5_MAC ) */

#if defined( USE_DTLS_RSA_KEYSCHEME ) && defined( USE_DTLS_RC4_128_CIPHER ) && defined( USE_DTLS_MD5_MAC )
extern const cipher_suite_t DTLS_RSA_WITH_RC4_128_MD5;
#endif /* if defined( USE_DTLS_RSA_KEYSCHEME ) && defined( USE_DTLS_RC4_128_CIPHER ) && defined( USE_DTLS_MD5_MAC ) */

#if defined( USE_DTLS_RSA_KEYSCHEME ) && defined( USE_DTLS_RC4_128_CIPHER ) && defined( USE_DTLS_SHA_MAC )
extern const cipher_suite_t DTLS_RSA_WITH_RC4_128_SHA;
#endif /* if defined( USE_DTLS_RSA_KEYSCHEME ) && defined( USE_DTLS_RC4_128_CIPHER ) && defined( USE_DTLS_SHA_MAC ) */

#if defined( USE_DTLS_RSA_EXPORT_KEYSCHEME ) && defined( USE_DTLS_RC2_CBC_40_CIPHER ) && defined( USE_DTLS_MD5_MAC )
extern const cipher_suite_t DTLS_RSA_EXPORT_WITH_RC2_CBC_40_MD5;
#endif /* if defined( USE_DTLS_RSA_EXPORT_KEYSCHEME ) && defined( USE_DTLS_RC2_CBC_40_CIPHER ) && defined( USE_DTLS_MD5_MAC ) */

#if defined( USE_DTLS_RSA_KEYSCHEME ) && defined( USE_DTLS_IDEA_CBC_CIPHER ) && defined( USE_DTLS_SHA_MAC )
extern const cipher_suite_t DTLS_RSA_WITH_IDEA_CBC_SHA;
#endif /* if defined( USE_DTLS_RSA_KEYSCHEME ) && defined( USE_DTLS_IDEA_CBC_CIPHER ) && defined( USE_DTLS_SHA_MAC ) */

#if defined( USE_DTLS_RSA_EXPORT_KEYSCHEME ) && defined( USE_DTLS_DES40_CBC_CIPHER ) && defined( USE_DTLS_SHA_MAC )
extern const cipher_suite_t DTLS_RSA_EXPORT_WITH_DES40_CBC_SHA;
#endif /* if defined( USE_DTLS_RSA_EXPORT_KEYSCHEME ) && defined( USE_DTLS_DES40_CBC_CIPHER ) && defined( USE_DTLS_SHA_MAC ) */

#if defined( USE_DTLS_RSA_KEYSCHEME ) && defined( USE_DTLS_DES_CBC_CIPHER ) && defined( USE_DTLS_SHA_MAC )
extern const cipher_suite_t DTLS_RSA_WITH_DES_CBC_SHA;
#endif /* if defined( USE_DTLS_RSA_KEYSCHEME ) && defined( USE_DTLS_DES_CBC_CIPHER ) && defined( USE_DTLS_SHA_MAC ) */

#if defined( USE_DTLS_RSA_KEYSCHEME ) && defined( USE_DTLS_3DES_EDE_CBC_CIPHER ) && defined( USE_DTLS_SHA_MAC )
extern const cipher_suite_t DTLS_RSA_WITH_3DES_EDE_CBC_SHA;
#endif /* if defined( USE_DTLS_RSA_KEYSCHEME ) && defined( USE_DTLS_3DES_EDE_CBC_CIPHER ) && defined( USE_DTLS_SHA_MAC ) */

#if defined( USE_DTLS_DH_DSS_EXPORT_KEYSCHEME ) && defined( USE_DTLS_DES40_CBC_CIPHER ) && defined( USE_DTLS_SHA_MAC )
extern const cipher_suite_t DTLS_DH_DSS_EXPORT_WITH_DES40_CBC_SHA;
#endif /* if defined( USE_DTLS_DH_DSS_EXPORT_KEYSCHEME ) && defined( USE_DTLS_DES40_CBC_CIPHER ) && defined( USE_DTLS_SHA_MAC ) */

#if defined( USE_DTLS_DH_DSS_KEYSCHEME ) && defined( USE_DTLS_DES_CBC_CIPHER ) && defined( USE_DTLS_SHA_MAC )
extern const cipher_suite_t DTLS_DH_DSS_WITH_DES_CBC_SHA;
#endif /* if defined( USE_DTLS_DH_DSS_KEYSCHEME ) && defined( USE_DTLS_DES_CBC_CIPHER ) && defined( USE_DTLS_SHA_MAC ) */

#if defined( USE_DTLS_DH_DSS_KEYSCHEME ) && defined( USE_DTLS_3DES_EDE_CBC_CIPHER ) && defined( USE_DTLS_SHA_MAC )
extern const cipher_suite_t DTLS_DH_DSS_WITH_3DES_EDE_CBC_SHA;
#endif /* if defined( USE_DTLS_DH_DSS_KEYSCHEME ) && defined( USE_DTLS_3DES_EDE_CBC_CIPHER ) && defined( USE_DTLS_SHA_MAC ) */

#if defined( USE_DTLS_DH_RSA_EXPORT_KEYSCHEME ) && defined( USE_DTLS_DES40_CBC_CIPHER ) && defined( USE_DTLS_SHA_MAC )
extern const cipher_suite_t DTLS_DH_RSA_EXPORT_WITH_DES40_CBC_SHA;
#endif /* if defined( USE_DTLS_DH_RSA_EXPORT_KEYSCHEME ) && defined( USE_DTLS_DES40_CBC_CIPHER ) && defined( USE_DTLS_SHA_MAC ) */

#if defined( USE_DTLS_DH_RSA_KEYSCHEME ) && defined( USE_DTLS_DES_CBC_CIPHER ) && defined( USE_DTLS_SHA_MAC )
extern const cipher_suite_t DTLS_DH_RSA_WITH_DES_CBC_SHA;
#endif /* if defined( USE_DTLS_DH_RSA_KEYSCHEME ) && defined( USE_DTLS_DES_CBC_CIPHER ) && defined( USE_DTLS_SHA_MAC ) */

#if defined( USE_DTLS_DH_RSA_KEYSCHEME ) && defined( USE_DTLS_3DES_EDE_CBC_CIPHER ) && defined( USE_DTLS_SHA_MAC )
extern const cipher_suite_t DTLS_DH_RSA_WITH_3DES_EDE_CBC_SHA;
#endif /* if defined( USE_DTLS_DH_RSA_KEYSCHEME ) && defined( USE_DTLS_3DES_EDE_CBC_CIPHER ) && defined( USE_DTLS_SHA_MAC ) */

#if defined( USE_DTLS_DHE_DSS_EXPORT_KEYSCHEME ) && defined( USE_DTLS_DES40_CBC_CIPHER ) && defined( USE_DTLS_SHA_MAC )
extern const cipher_suite_t DTLS_DHE_DSS_EXPORT_WITH_DES40_CBC_SHA;
#endif /* if defined( USE_DTLS_DHE_DSS_EXPORT_KEYSCHEME ) && defined( USE_DTLS_DES40_CBC_CIPHER ) && defined( USE_DTLS_SHA_MAC ) */

#if defined( USE_DTLS_DHE_DSS_KEYSCHEME ) && defined( USE_DTLS_DES_CBC_CIPHER ) && defined( USE_DTLS_SHA_MAC )
extern const cipher_suite_t DTLS_DHE_DSS_WITH_DES_CBC_SHA;
#endif /* if defined( USE_DTLS_DHE_DSS_KEYSCHEME ) && defined( USE_DTLS_DES_CBC_CIPHER ) && defined( USE_DTLS_SHA_MAC ) */

#if defined( USE_DTLS_DHE_DSS_KEYSCHEME ) && defined( USE_DTLS_3DES_EDE_CBC_CIPHER ) && defined( USE_DTLS_SHA_MAC )
extern const cipher_suite_t DTLS_DHE_DSS_WITH_3DES_EDE_CBC_SHA;
#endif /* if defined( USE_DTLS_DHE_DSS_KEYSCHEME ) && defined( USE_DTLS_3DES_EDE_CBC_CIPHER ) && defined( USE_DTLS_SHA_MAC ) */

#if defined( USE_DTLS_DHE_RSA_EXPORT_KEYSCHEME ) && defined( USE_DTLS_DES40_CBC_CIPHER ) && defined( USE_DTLS_SHA_MAC )
extern const cipher_suite_t DTLS_DHE_RSA_EXPORT_WITH_DES40_CBC_SHA;
#endif /* if defined( USE_DTLS_DHE_RSA_EXPORT_KEYSCHEME ) && defined( USE_DTLS_DES40_CBC_CIPHER ) && defined( USE_DTLS_SHA_MAC ) */

#if defined( USE_DTLS_DHE_RSA_KEYSCHEME ) && defined( USE_DTLS_DES_CBC_CIPHER ) && defined( USE_DTLS_SHA_MAC )
extern const cipher_suite_t DTLS_DHE_RSA_WITH_DES_CBC_SHA;
#endif /* if defined( USE_DTLS_DHE_RSA_KEYSCHEME ) && defined( USE_DTLS_DES_CBC_CIPHER ) && defined( USE_DTLS_SHA_MAC ) */

#if defined( USE_DTLS_DHE_RSA_KEYSCHEME ) && defined( USE_DTLS_3DES_EDE_CBC_CIPHER ) && defined( USE_DTLS_SHA_MAC )
extern const cipher_suite_t DTLS_DHE_RSA_WITH_3DES_EDE_CBC_SHA;
#endif /* if defined( USE_DTLS_DHE_RSA_KEYSCHEME ) && defined( USE_DTLS_3DES_EDE_CBC_CIPHER ) && defined( USE_DTLS_SHA_MAC ) */

#if defined( USE_DTLS_DH_anon_EXPORT_KEYSCHEME ) && defined( USE_DTLS_RC4_40_CIPHER ) && defined( USE_DTLS_MD5_MAC )
extern const cipher_suite_t DTLS_DH_anon_EXPORT_WITH_RC4_40_MD5;
#endif /* if defined( USE_DTLS_DH_anon_EXPORT_KEYSCHEME ) && defined( USE_DTLS_RC4_40_CIPHER ) && defined( USE_DTLS_MD5_MAC ) */

#if defined( USE_DTLS_DH_anon_KEYSCHEME ) && defined( USE_DTLS_RC4_128_CIPHER ) && defined( USE_DTLS_MD5_MAC )
extern const cipher_suite_t DTLS_DH_anon_WITH_RC4_128_MD5;
#endif /* if defined( USE_DTLS_DH_anon_KEYSCHEME ) && defined( USE_DTLS_RC4_128_CIPHER ) && defined( USE_DTLS_MD5_MAC ) */

#if defined( USE_DTLS_DH_anon_EXPORT_KEYSCHEME ) && defined( USE_DTLS_DES40_CBC_CIPHER ) && defined( USE_DTLS_SHA_MAC )
extern const cipher_suite_t DTLS_DH_anon_EXPORT_WITH_DES40_CBC_SHA;
#endif /* if defined( USE_DTLS_DH_anon_EXPORT_KEYSCHEME ) && defined( USE_DTLS_DES40_CBC_CIPHER ) && defined( USE_DTLS_SHA_MAC ) */

#if defined( USE_DTLS_DH_anon_KEYSCHEME ) && defined( USE_DTLS_DES_CBC_CIPHER ) && defined( USE_DTLS_SHA_MAC )
extern const cipher_suite_t DTLS_DH_anon_WITH_DES_CBC_SHA;
#endif /* if defined( USE_DTLS_DH_anon_KEYSCHEME ) && defined( USE_DTLS_DES_CBC_CIPHER ) && defined( USE_DTLS_SHA_MAC ) */

#if defined( USE_DTLS_DH_anon_KEYSCHEME ) && defined( USE_DTLS_3DES_EDE_CBC_CIPHER ) && defined( USE_DTLS_SHA_MAC )
extern const cipher_suite_t DTLS_DH_anon_WITH_3DES_EDE_CBC_SHA;
#endif /* if defined( USE_DTLS_DH_anon_KEYSCHEME ) && defined( USE_DTLS_3DES_EDE_CBC_CIPHER ) && defined( USE_DTLS_SHA_MAC ) */

#if defined( USE_DTLS_KRB5_KEYSCHEME ) && defined( USE_DTLS_DES_CBC_CIPHER ) && defined( USE_DTLS_SHA_MAC )
extern const cipher_suite_t DTLS_KRB5_WITH_DES_CBC_SHA;
#endif /* if defined( USE_DTLS_KRB5_KEYSCHEME ) && defined( USE_DTLS_DES_CBC_CIPHER ) && defined( USE_DTLS_SHA_MAC ) */

#if defined( USE_DTLS_KRB5_KEYSCHEME ) && defined( USE_DTLS_3DES_EDE_CBC_CIPHER ) && defined( USE_DTLS_SHA_MAC )
extern const cipher_suite_t DTLS_KRB5_WITH_3DES_EDE_CBC_SHA;
#endif /* if defined( USE_DTLS_KRB5_KEYSCHEME ) && defined( USE_DTLS_3DES_EDE_CBC_CIPHER ) && defined( USE_DTLS_SHA_MAC ) */

#if defined( USE_DTLS_KRB5_KEYSCHEME ) && defined( USE_DTLS_RC4_128_CIPHER ) && defined( USE_DTLS_SHA_MAC )
extern const cipher_suite_t DTLS_KRB5_WITH_RC4_128_SHA;
#endif /* if defined( USE_DTLS_KRB5_KEYSCHEME ) && defined( USE_DTLS_RC4_128_CIPHER ) && defined( USE_DTLS_SHA_MAC ) */

#if defined( USE_DTLS_KRB5_KEYSCHEME ) && defined( USE_DTLS_IDEA_CBC_CIPHER ) && defined( USE_DTLS_SHA_MAC )
extern const cipher_suite_t DTLS_KRB5_WITH_IDEA_CBC_SHA;
#endif /* if defined( USE_DTLS_KRB5_KEYSCHEME ) && defined( USE_DTLS_IDEA_CBC_CIPHER ) && defined( USE_DTLS_SHA_MAC ) */

#if defined( USE_DTLS_KRB5_KEYSCHEME ) && defined( USE_DTLS_DES_CBC_CIPHER ) && defined( USE_DTLS_MD5_MAC )
extern const cipher_suite_t DTLS_KRB5_WITH_DES_CBC_MD5;
#endif /* if defined( USE_DTLS_KRB5_KEYSCHEME ) && defined( USE_DTLS_DES_CBC_CIPHER ) && defined( USE_DTLS_MD5_MAC ) */

#if defined( USE_DTLS_KRB5_KEYSCHEME ) && defined( USE_DTLS_3DES_EDE_CBC_CIPHER ) && defined( USE_DTLS_MD5_MAC )
extern const cipher_suite_t DTLS_KRB5_WITH_3DES_EDE_CBC_MD5;
#endif /* if defined( USE_DTLS_KRB5_KEYSCHEME ) && defined( USE_DTLS_3DES_EDE_CBC_CIPHER ) && defined( USE_DTLS_MD5_MAC ) */

#if defined( USE_DTLS_KRB5_KEYSCHEME ) && defined( USE_DTLS_RC4_128_CIPHER ) && defined( USE_DTLS_MD5_MAC )
extern const cipher_suite_t DTLS_KRB5_WITH_RC4_128_MD5;
#endif /* if defined( USE_DTLS_KRB5_KEYSCHEME ) && defined( USE_DTLS_RC4_128_CIPHER ) && defined( USE_DTLS_MD5_MAC ) */

#if defined( USE_DTLS_KRB5_KEYSCHEME ) && defined( USE_DTLS_IDEA_CBC_CIPHER ) && defined( USE_DTLS_MD5_MAC )
extern const cipher_suite_t DTLS_KRB5_WITH_IDEA_CBC_MD5;
#endif /* if defined( USE_DTLS_KRB5_KEYSCHEME ) && defined( USE_DTLS_IDEA_CBC_CIPHER ) && defined( USE_DTLS_MD5_MAC ) */

#if defined( USE_DTLS_KRB5_EXPORT_KEYSCHEME ) && defined( USE_DTLS_DES_CBC_40_CIPHER ) && defined( USE_DTLS_SHA_MAC )
extern const cipher_suite_t DTLS_KRB5_EXPORT_WITH_DES_CBC_40_SHA;
#endif /* if defined( USE_DTLS_KRB5_EXPORT_KEYSCHEME ) && defined( USE_DTLS_DES_CBC_40_CIPHER ) && defined( USE_DTLS_SHA_MAC ) */

#if defined( USE_DTLS_KRB5_EXPORT_KEYSCHEME ) && defined( USE_DTLS_RC2_CBC_40_CIPHER ) && defined( USE_DTLS_SHA_MAC )
extern const cipher_suite_t DTLS_KRB5_EXPORT_WITH_RC2_CBC_40_SHA;
#endif /* if defined( USE_DTLS_KRB5_EXPORT_KEYSCHEME ) && defined( USE_DTLS_RC2_CBC_40_CIPHER ) && defined( USE_DTLS_SHA_MAC ) */

#if defined( USE_DTLS_KRB5_EXPORT_KEYSCHEME ) && defined( USE_DTLS_RC4_40_CIPHER ) && defined( USE_DTLS_SHA_MAC )
extern const cipher_suite_t DTLS_KRB5_EXPORT_WITH_RC4_40_SHA;
#endif /* if defined( USE_DTLS_KRB5_EXPORT_KEYSCHEME ) && defined( USE_DTLS_RC4_40_CIPHER ) && defined( USE_DTLS_SHA_MAC ) */

#if defined( USE_DTLS_KRB5_EXPORT_KEYSCHEME ) && defined( USE_DTLS_DES_CBC_40_CIPHER ) && defined( USE_DTLS_MD5_MAC )
extern const cipher_suite_t DTLS_KRB5_EXPORT_WITH_DES_CBC_40_MD5;
#endif /* if defined( USE_DTLS_KRB5_EXPORT_KEYSCHEME ) && defined( USE_DTLS_DES_CBC_40_CIPHER ) && defined( USE_DTLS_MD5_MAC ) */

#if defined( USE_DTLS_KRB5_EXPORT_KEYSCHEME ) && defined( USE_DTLS_RC2_CBC_40_CIPHER ) && defined( USE_DTLS_MD5_MAC )
extern const cipher_suite_t DTLS_KRB5_EXPORT_WITH_RC2_CBC_40_MD5;
#endif /* if defined( USE_DTLS_KRB5_EXPORT_KEYSCHEME ) && defined( USE_DTLS_RC2_CBC_40_CIPHER ) && defined( USE_DTLS_MD5_MAC ) */

#if defined( USE_DTLS_KRB5_EXPORT_KEYSCHEME ) && defined( USE_DTLS_RC4_40_CIPHER ) && defined( USE_DTLS_MD5_MAC )
extern const cipher_suite_t DTLS_KRB5_EXPORT_WITH_RC4_40_MD5;
#endif /* if defined( USE_DTLS_KRB5_EXPORT_KEYSCHEME ) && defined( USE_DTLS_RC4_40_CIPHER ) && defined( USE_DTLS_MD5_MAC ) */

#if defined( USE_DTLS_PSK_KEYSCHEME ) && defined( USE_DTLS_NULL_CIPHER ) && defined( USE_DTLS_SHA_MAC )
extern const cipher_suite_t DTLS_PSK_WITH_NULL_SHA;
#endif /* if defined( USE_DTLS_PSK_KEYSCHEME ) && defined( USE_DTLS_NULL_CIPHER ) && defined( USE_DTLS_SHA_MAC ) */

#if defined( USE_DTLS_DHE_PSK_KEYSCHEME ) && defined( USE_DTLS_NULL_CIPHER ) && defined( USE_DTLS_SHA_MAC )
extern const cipher_suite_t DTLS_DHE_PSK_WITH_NULL_SHA;
#endif /* if defined( USE_DTLS_DHE_PSK_KEYSCHEME ) && defined( USE_DTLS_NULL_CIPHER ) && defined( USE_DTLS_SHA_MAC ) */

#if defined( USE_DTLS_RSA_PSK_KEYSCHEME ) && defined( USE_DTLS_NULL_CIPHER ) && defined( USE_DTLS_SHA_MAC )
extern const cipher_suite_t DTLS_RSA_PSK_WITH_NULL_SHA;
#endif /* if defined( USE_DTLS_RSA_PSK_KEYSCHEME ) && defined( USE_DTLS_NULL_CIPHER ) && defined( USE_DTLS_SHA_MAC ) */

#if defined( USE_DTLS_RSA_KEYSCHEME ) && defined( USE_DTLS_AES_128_CBC_CIPHER ) && defined( USE_DTLS_SHA_MAC )
extern const cipher_suite_t DTLS_RSA_WITH_AES_128_CBC_SHA;
#endif /* if defined( USE_DTLS_RSA_KEYSCHEME ) && defined( USE_DTLS_AES_128_CBC_CIPHER ) && defined( USE_DTLS_SHA_MAC ) */

#if defined( USE_DTLS_DH_DSS_KEYSCHEME ) && defined( USE_DTLS_AES_128_CBC_CIPHER ) && defined( USE_DTLS_SHA_MAC )
extern const cipher_suite_t DTLS_DH_DSS_WITH_AES_128_CBC_SHA;
#endif /* if defined( USE_DTLS_DH_DSS_KEYSCHEME ) && defined( USE_DTLS_AES_128_CBC_CIPHER ) && defined( USE_DTLS_SHA_MAC ) */

#if defined( USE_DTLS_DH_RSA_KEYSCHEME ) && defined( USE_DTLS_AES_128_CBC_CIPHER ) && defined( USE_DTLS_SHA_MAC )
extern const cipher_suite_t DTLS_DH_RSA_WITH_AES_128_CBC_SHA;
#endif /* if defined( USE_DTLS_DH_RSA_KEYSCHEME ) && defined( USE_DTLS_AES_128_CBC_CIPHER ) && defined( USE_DTLS_SHA_MAC ) */

#if defined( USE_DTLS_DHE_DSS_KEYSCHEME ) && defined( USE_DTLS_AES_128_CBC_CIPHER ) && defined( USE_DTLS_SHA_MAC )
extern const cipher_suite_t DTLS_DHE_DSS_WITH_AES_128_CBC_SHA;
#endif /* if defined( USE_DTLS_DHE_DSS_KEYSCHEME ) && defined( USE_DTLS_AES_128_CBC_CIPHER ) && defined( USE_DTLS_SHA_MAC ) */

#if defined( USE_DTLS_DHE_RSA_KEYSCHEME ) && defined( USE_DTLS_AES_128_CBC_CIPHER ) && defined( USE_DTLS_SHA_MAC )
extern const cipher_suite_t DTLS_DHE_RSA_WITH_AES_128_CBC_SHA;
#endif /* if defined( USE_DTLS_DHE_RSA_KEYSCHEME ) && defined( USE_DTLS_AES_128_CBC_CIPHER ) && defined( USE_DTLS_SHA_MAC ) */

#if defined( USE_DTLS_DH_anon_KEYSCHEME ) && defined( USE_DTLS_AES_128_CBC_CIPHER ) && defined( USE_DTLS_SHA_MAC )
extern const cipher_suite_t DTLS_DH_anon_WITH_AES_128_CBC_SHA;
#endif /* if defined( USE_DTLS_DH_anon_KEYSCHEME ) && defined( USE_DTLS_AES_128_CBC_CIPHER ) && defined( USE_DTLS_SHA_MAC ) */

#if defined( USE_DTLS_RSA_KEYSCHEME ) && defined( USE_DTLS_AES_256_CBC_CIPHER ) && defined( USE_DTLS_SHA_MAC )
extern const cipher_suite_t DTLS_RSA_WITH_AES_256_CBC_SHA;
#endif /* if defined( USE_DTLS_RSA_KEYSCHEME ) && defined( USE_DTLS_AES_256_CBC_CIPHER ) && defined( USE_DTLS_SHA_MAC ) */

#if defined( USE_DTLS_DH_DSS_KEYSCHEME ) && defined( USE_DTLS_AES_256_CBC_CIPHER ) && defined( USE_DTLS_SHA_MAC )
extern const cipher_suite_t DTLS_DH_DSS_WITH_AES_256_CBC_SHA;
#endif /* if defined( USE_DTLS_DH_DSS_KEYSCHEME ) && defined( USE_DTLS_AES_256_CBC_CIPHER ) && defined( USE_DTLS_SHA_MAC ) */

#if defined( USE_DTLS_DH_RSA_KEYSCHEME ) && defined( USE_DTLS_AES_256_CBC_CIPHER ) && defined( USE_DTLS_SHA_MAC )
extern const cipher_suite_t DTLS_DH_RSA_WITH_AES_256_CBC_SHA;
#endif /* if defined( USE_DTLS_DH_RSA_KEYSCHEME ) && defined( USE_DTLS_AES_256_CBC_CIPHER ) && defined( USE_DTLS_SHA_MAC ) */

#if defined( USE_DTLS_DHE_DSS_KEYSCHEME ) && defined( USE_DTLS_AES_256_CBC_CIPHER ) && defined( USE_DTLS_SHA_MAC )
extern const cipher_suite_t DTLS_DHE_DSS_WITH_AES_256_CBC_SHA;
#endif /* if defined( USE_DTLS_DHE_DSS_KEYSCHEME ) && defined( USE_DTLS_AES_256_CBC_CIPHER ) && defined( USE_DTLS_SHA_MAC ) */

#if defined( USE_DTLS_DHE_RSA_KEYSCHEME ) && defined( USE_DTLS_AES_256_CBC_CIPHER ) && defined( USE_DTLS_SHA_MAC )
extern const cipher_suite_t DTLS_DHE_RSA_WITH_AES_256_CBC_SHA;
#endif /* if defined( USE_DTLS_DHE_RSA_KEYSCHEME ) && defined( USE_DTLS_AES_256_CBC_CIPHER ) && defined( USE_DTLS_SHA_MAC ) */

#if defined( USE_DTLS_DH_anon_KEYSCHEME ) && defined( USE_DTLS_AES_256_CBC_CIPHER ) && defined( USE_DTLS_SHA_MAC )
extern const cipher_suite_t DTLS_DH_anon_WITH_AES_256_CBC_SHA;
#endif /* if defined( USE_DTLS_DH_anon_KEYSCHEME ) && defined( USE_DTLS_AES_256_CBC_CIPHER ) && defined( USE_DTLS_SHA_MAC ) */

#if defined( USE_DTLS_RSA_KEYSCHEME ) && defined( USE_DTLS_NULL_CIPHER ) && defined( USE_DTLS_SHA256_MAC )
extern const cipher_suite_t DTLS_RSA_WITH_NULL_SHA256;
#endif /* if defined( USE_DTLS_RSA_KEYSCHEME ) && defined( USE_DTLS_NULL_CIPHER ) && defined( USE_DTLS_SHA256_MAC ) */

#if defined( USE_DTLS_RSA_KEYSCHEME ) && defined( USE_DTLS_AES_128_CBC_CIPHER ) && defined( USE_DTLS_SHA256_MAC )
extern const cipher_suite_t DTLS_RSA_WITH_AES_128_CBC_SHA256;
#endif /* if defined( USE_DTLS_RSA_KEYSCHEME ) && defined( USE_DTLS_AES_128_CBC_CIPHER ) && defined( USE_DTLS_SHA256_MAC ) */

#if defined( USE_DTLS_RSA_KEYSCHEME ) && defined( USE_DTLS_AES_256_CBC_CIPHER ) && defined( USE_DTLS_SHA256_MAC )
extern const cipher_suite_t DTLS_RSA_WITH_AES_256_CBC_SHA256;
#endif /* if defined( USE_DTLS_RSA_KEYSCHEME ) && defined( USE_DTLS_AES_256_CBC_CIPHER ) && defined( USE_DTLS_SHA256_MAC ) */

#if defined( USE_DTLS_DH_DSS_KEYSCHEME ) && defined( USE_DTLS_AES_128_CBC_CIPHER ) && defined( USE_DTLS_SHA256_MAC )
extern const cipher_suite_t DTLS_DH_DSS_WITH_AES_128_CBC_SHA256;
#endif /* if defined( USE_DTLS_DH_DSS_KEYSCHEME ) && defined( USE_DTLS_AES_128_CBC_CIPHER ) && defined( USE_DTLS_SHA256_MAC ) */

#if defined( USE_DTLS_DH_RSA_KEYSCHEME ) && defined( USE_DTLS_AES_128_CBC_CIPHER ) && defined( USE_DTLS_SHA256_MAC )
extern const cipher_suite_t DTLS_DH_RSA_WITH_AES_128_CBC_SHA256;
#endif /* if defined( USE_DTLS_DH_RSA_KEYSCHEME ) && defined( USE_DTLS_AES_128_CBC_CIPHER ) && defined( USE_DTLS_SHA256_MAC ) */

#if defined( USE_DTLS_DHE_DSS_KEYSCHEME ) && defined( USE_DTLS_AES_128_CBC_CIPHER ) && defined( USE_DTLS_SHA256_MAC )
extern const cipher_suite_t DTLS_DHE_DSS_WITH_AES_128_CBC_SHA256;
#endif /* if defined( USE_DTLS_DHE_DSS_KEYSCHEME ) && defined( USE_DTLS_AES_128_CBC_CIPHER ) && defined( USE_DTLS_SHA256_MAC ) */

#if defined( USE_DTLS_RSA_KEYSCHEME ) && defined( USE_DTLS_CAMELLIA_128_CBC_CIPHER ) && defined( USE_DTLS_SHA_MAC )
extern const cipher_suite_t DTLS_RSA_WITH_CAMELLIA_128_CBC_SHA;
#endif /* if defined( USE_DTLS_RSA_KEYSCHEME ) && defined( USE_DTLS_CAMELLIA_128_CBC_CIPHER ) && defined( USE_DTLS_SHA_MAC ) */

#if defined( USE_DTLS_DH_DSS_KEYSCHEME ) && defined( USE_DTLS_CAMELLIA_128_CBC_CIPHER ) && defined( USE_DTLS_SHA_MAC )
extern const cipher_suite_t DTLS_DH_DSS_WITH_CAMELLIA_128_CBC_SHA;
#endif /* if defined( USE_DTLS_DH_DSS_KEYSCHEME ) && defined( USE_DTLS_CAMELLIA_128_CBC_CIPHER ) && defined( USE_DTLS_SHA_MAC ) */

#if defined( USE_DTLS_DH_RSA_KEYSCHEME ) && defined( USE_DTLS_CAMELLIA_128_CBC_CIPHER ) && defined( USE_DTLS_SHA_MAC )
extern const cipher_suite_t DTLS_DH_RSA_WITH_CAMELLIA_128_CBC_SHA;
#endif /* if defined( USE_DTLS_DH_RSA_KEYSCHEME ) && defined( USE_DTLS_CAMELLIA_128_CBC_CIPHER ) && defined( USE_DTLS_SHA_MAC ) */

#if defined( USE_DTLS_DHE_DSS_KEYSCHEME ) && defined( USE_DTLS_CAMELLIA_128_CBC_CIPHER ) && defined( USE_DTLS_SHA_MAC )
extern const cipher_suite_t DTLS_DHE_DSS_WITH_CAMELLIA_128_CBC_SHA;
#endif /* if defined( USE_DTLS_DHE_DSS_KEYSCHEME ) && defined( USE_DTLS_CAMELLIA_128_CBC_CIPHER ) && defined( USE_DTLS_SHA_MAC ) */

#if defined( USE_DTLS_DHE_RSA_KEYSCHEME ) && defined( USE_DTLS_CAMELLIA_128_CBC_CIPHER ) && defined( USE_DTLS_SHA_MAC )
extern const cipher_suite_t DTLS_DHE_RSA_WITH_CAMELLIA_128_CBC_SHA;
#endif /* if defined( USE_DTLS_DHE_RSA_KEYSCHEME ) && defined( USE_DTLS_CAMELLIA_128_CBC_CIPHER ) && defined( USE_DTLS_SHA_MAC ) */

#if defined( USE_DTLS_DH_anon_KEYSCHEME ) && defined( USE_DTLS_CAMELLIA_128_CBC_CIPHER ) && defined( USE_DTLS_SHA_MAC )
extern const cipher_suite_t DTLS_DH_anon_WITH_CAMELLIA_128_CBC_SHA;
#endif /* if defined( USE_DTLS_DH_anon_KEYSCHEME ) && defined( USE_DTLS_CAMELLIA_128_CBC_CIPHER ) && defined( USE_DTLS_SHA_MAC ) */

#if defined( USE_DTLS_DHE_RSA_KEYSCHEME ) && defined( USE_DTLS_AES_128_CBC_CIPHER ) && defined( USE_DTLS_SHA256_MAC )
extern const cipher_suite_t DTLS_DHE_RSA_WITH_AES_128_CBC_SHA256;
#endif /* if defined( USE_DTLS_DHE_RSA_KEYSCHEME ) && defined( USE_DTLS_AES_128_CBC_CIPHER ) && defined( USE_DTLS_SHA256_MAC ) */

#if defined( USE_DTLS_DH_DSS_KEYSCHEME ) && defined( USE_DTLS_AES_256_CBC_CIPHER ) && defined( USE_DTLS_SHA256_MAC )
extern const cipher_suite_t DTLS_DH_DSS_WITH_AES_256_CBC_SHA256;
#endif /* if defined( USE_DTLS_DH_DSS_KEYSCHEME ) && defined( USE_DTLS_AES_256_CBC_CIPHER ) && defined( USE_DTLS_SHA256_MAC ) */

#if defined( USE_DTLS_DH_RSA_KEYSCHEME ) && defined( USE_DTLS_AES_256_CBC_CIPHER ) && defined( USE_DTLS_SHA256_MAC )
extern const cipher_suite_t DTLS_DH_RSA_WITH_AES_256_CBC_SHA256;
#endif /* if defined( USE_DTLS_DH_RSA_KEYSCHEME ) && defined( USE_DTLS_AES_256_CBC_CIPHER ) && defined( USE_DTLS_SHA256_MAC ) */

#if defined( USE_DTLS_DHE_DSS_KEYSCHEME ) && defined( USE_DTLS_AES_256_CBC_CIPHER ) && defined( USE_DTLS_SHA256_MAC )
extern const cipher_suite_t DTLS_DHE_DSS_WITH_AES_256_CBC_SHA256;
#endif /* if defined( USE_DTLS_DHE_DSS_KEYSCHEME ) && defined( USE_DTLS_AES_256_CBC_CIPHER ) && defined( USE_DTLS_SHA256_MAC ) */

#if defined( USE_DTLS_DHE_RSA_KEYSCHEME ) && defined( USE_DTLS_AES_256_CBC_CIPHER ) && defined( USE_DTLS_SHA256_MAC )
extern const cipher_suite_t DTLS_DHE_RSA_WITH_AES_256_CBC_SHA256;
#endif /* if defined( USE_DTLS_DHE_RSA_KEYSCHEME ) && defined( USE_DTLS_AES_256_CBC_CIPHER ) && defined( USE_DTLS_SHA256_MAC ) */

#if defined( USE_DTLS_DH_anon_KEYSCHEME ) && defined( USE_DTLS_AES_128_CBC_CIPHER ) && defined( USE_DTLS_SHA256_MAC )
extern const cipher_suite_t DTLS_DH_anon_WITH_AES_128_CBC_SHA256;
#endif /* if defined( USE_DTLS_DH_anon_KEYSCHEME ) && defined( USE_DTLS_AES_128_CBC_CIPHER ) && defined( USE_DTLS_SHA256_MAC ) */

#if defined( USE_DTLS_DH_anon_KEYSCHEME ) && defined( USE_DTLS_AES_256_CBC_CIPHER ) && defined( USE_DTLS_SHA256_MAC )
extern const cipher_suite_t DTLS_DH_anon_WITH_AES_256_CBC_SHA256;
#endif /* if defined( USE_DTLS_DH_anon_KEYSCHEME ) && defined( USE_DTLS_AES_256_CBC_CIPHER ) && defined( USE_DTLS_SHA256_MAC ) */

#if defined( USE_DTLS_RSA_KEYSCHEME ) && defined( USE_DTLS_CAMELLIA_256_CBC_CIPHER ) && defined( USE_DTLS_SHA_MAC )
extern const cipher_suite_t DTLS_RSA_WITH_CAMELLIA_256_CBC_SHA;
#endif /* if defined( USE_DTLS_RSA_KEYSCHEME ) && defined( USE_DTLS_CAMELLIA_256_CBC_CIPHER ) && defined( USE_DTLS_SHA_MAC ) */

#if defined( USE_DTLS_DH_DSS_KEYSCHEME ) && defined( USE_DTLS_CAMELLIA_256_CBC_CIPHER ) && defined( USE_DTLS_SHA_MAC )
extern const cipher_suite_t DTLS_DH_DSS_WITH_CAMELLIA_256_CBC_SHA;
#endif /* if defined( USE_DTLS_DH_DSS_KEYSCHEME ) && defined( USE_DTLS_CAMELLIA_256_CBC_CIPHER ) && defined( USE_DTLS_SHA_MAC ) */

#if defined( USE_DTLS_DH_RSA_KEYSCHEME ) && defined( USE_DTLS_CAMELLIA_256_CBC_CIPHER ) && defined( USE_DTLS_SHA_MAC )
extern const cipher_suite_t DTLS_DH_RSA_WITH_CAMELLIA_256_CBC_SHA;
#endif /* if defined( USE_DTLS_DH_RSA_KEYSCHEME ) && defined( USE_DTLS_CAMELLIA_256_CBC_CIPHER ) && defined( USE_DTLS_SHA_MAC ) */

#if defined( USE_DTLS_DHE_DSS_KEYSCHEME ) && defined( USE_DTLS_CAMELLIA_256_CBC_CIPHER ) && defined( USE_DTLS_SHA_MAC )
extern const cipher_suite_t DTLS_DHE_DSS_WITH_CAMELLIA_256_CBC_SHA;
#endif /* if defined( USE_DTLS_DHE_DSS_KEYSCHEME ) && defined( USE_DTLS_CAMELLIA_256_CBC_CIPHER ) && defined( USE_DTLS_SHA_MAC ) */

#if defined( USE_DTLS_DHE_RSA_KEYSCHEME ) && defined( USE_DTLS_CAMELLIA_256_CBC_CIPHER ) && defined( USE_DTLS_SHA_MAC )
extern const cipher_suite_t DTLS_DHE_RSA_WITH_CAMELLIA_256_CBC_SHA;
#endif /* if defined( USE_DTLS_DHE_RSA_KEYSCHEME ) && defined( USE_DTLS_CAMELLIA_256_CBC_CIPHER ) && defined( USE_DTLS_SHA_MAC ) */

#if defined( USE_DTLS_DH_anon_KEYSCHEME ) && defined( USE_DTLS_CAMELLIA_256_CBC_CIPHER ) && defined( USE_DTLS_SHA_MAC )
extern const cipher_suite_t DTLS_DH_anon_WITH_CAMELLIA_256_CBC_SHA;
#endif /* if defined( USE_DTLS_DH_anon_KEYSCHEME ) && defined( USE_DTLS_CAMELLIA_256_CBC_CIPHER ) && defined( USE_DTLS_SHA_MAC ) */

#if defined( USE_DTLS_PSK_KEYSCHEME ) && defined( USE_DTLS_RC4_128_CIPHER ) && defined( USE_DTLS_SHA_MAC )
extern const cipher_suite_t DTLS_PSK_WITH_RC4_128_SHA;
#endif /* if defined( USE_DTLS_PSK_KEYSCHEME ) && defined( USE_DTLS_RC4_128_CIPHER ) && defined( USE_DTLS_SHA_MAC ) */

#if defined( USE_DTLS_PSK_KEYSCHEME ) && defined( USE_DTLS_3DES_EDE_CBC_CIPHER ) && defined( USE_DTLS_SHA_MAC )
extern const cipher_suite_t DTLS_PSK_WITH_3DES_EDE_CBC_SHA;
#endif /* if defined( USE_DTLS_PSK_KEYSCHEME ) && defined( USE_DTLS_3DES_EDE_CBC_CIPHER ) && defined( USE_DTLS_SHA_MAC ) */

#if defined( USE_DTLS_PSK_KEYSCHEME ) && defined( USE_DTLS_AES_128_CBC_CIPHER ) && defined( USE_DTLS_SHA_MAC )
extern const cipher_suite_t DTLS_PSK_WITH_AES_128_CBC_SHA;
#endif /* if defined( USE_DTLS_PSK_KEYSCHEME ) && defined( USE_DTLS_AES_128_CBC_CIPHER ) && defined( USE_DTLS_SHA_MAC ) */

#if defined( USE_DTLS_PSK_KEYSCHEME ) && defined( USE_DTLS_AES_256_CBC_CIPHER ) && defined( USE_DTLS_SHA_MAC )
extern const cipher_suite_t DTLS_PSK_WITH_AES_256_CBC_SHA;
#endif /* if defined( USE_DTLS_PSK_KEYSCHEME ) && defined( USE_DTLS_AES_256_CBC_CIPHER ) && defined( USE_DTLS_SHA_MAC ) */

#if defined( USE_DTLS_DHE_PSK_KEYSCHEME ) && defined( USE_DTLS_RC4_128_CIPHER ) && defined( USE_DTLS_SHA_MAC )
extern const cipher_suite_t DTLS_DHE_PSK_WITH_RC4_128_SHA;
#endif /* if defined( USE_DTLS_DHE_PSK_KEYSCHEME ) && defined( USE_DTLS_RC4_128_CIPHER ) && defined( USE_DTLS_SHA_MAC ) */

#if defined( USE_DTLS_DHE_PSK_KEYSCHEME ) && defined( USE_DTLS_3DES_EDE_CBC_CIPHER ) && defined( USE_DTLS_SHA_MAC )
extern const cipher_suite_t DTLS_DHE_PSK_WITH_3DES_EDE_CBC_SHA;
#endif /* if defined( USE_DTLS_DHE_PSK_KEYSCHEME ) && defined( USE_DTLS_3DES_EDE_CBC_CIPHER ) && defined( USE_DTLS_SHA_MAC ) */

#if defined( USE_DTLS_DHE_PSK_KEYSCHEME ) && defined( USE_DTLS_AES_128_CBC_CIPHER ) && defined( USE_DTLS_SHA_MAC )
extern const cipher_suite_t DTLS_DHE_PSK_WITH_AES_128_CBC_SHA;
#endif /* if defined( USE_DTLS_DHE_PSK_KEYSCHEME ) && defined( USE_DTLS_AES_128_CBC_CIPHER ) && defined( USE_DTLS_SHA_MAC ) */

#if defined( USE_DTLS_DHE_PSK_KEYSCHEME ) && defined( USE_DTLS_AES_256_CBC_CIPHER ) && defined( USE_DTLS_SHA_MAC )
extern const cipher_suite_t DTLS_DHE_PSK_WITH_AES_256_CBC_SHA;
#endif /* if defined( USE_DTLS_DHE_PSK_KEYSCHEME ) && defined( USE_DTLS_AES_256_CBC_CIPHER ) && defined( USE_DTLS_SHA_MAC ) */

#if defined( USE_DTLS_RSA_PSK_KEYSCHEME ) && defined( USE_DTLS_RC4_128_CIPHER ) && defined( USE_DTLS_SHA_MAC )
extern const cipher_suite_t DTLS_RSA_PSK_WITH_RC4_128_SHA;
#endif /* if defined( USE_DTLS_RSA_PSK_KEYSCHEME ) && defined( USE_DTLS_RC4_128_CIPHER ) && defined( USE_DTLS_SHA_MAC ) */

#if defined( USE_DTLS_RSA_PSK_KEYSCHEME ) && defined( USE_DTLS_3DES_EDE_CBC_CIPHER ) && defined( USE_DTLS_SHA_MAC )
extern const cipher_suite_t DTLS_RSA_PSK_WITH_3DES_EDE_CBC_SHA;
#endif /* if defined( USE_DTLS_RSA_PSK_KEYSCHEME ) && defined( USE_DTLS_3DES_EDE_CBC_CIPHER ) && defined( USE_DTLS_SHA_MAC ) */

#if defined( USE_DTLS_RSA_PSK_KEYSCHEME ) && defined( USE_DTLS_AES_128_CBC_CIPHER ) && defined( USE_DTLS_SHA_MAC )
extern const cipher_suite_t DTLS_RSA_PSK_WITH_AES_128_CBC_SHA;
#endif /* if defined( USE_DTLS_RSA_PSK_KEYSCHEME ) && defined( USE_DTLS_AES_128_CBC_CIPHER ) && defined( USE_DTLS_SHA_MAC ) */

#if defined( USE_DTLS_RSA_PSK_KEYSCHEME ) && defined( USE_DTLS_AES_256_CBC_CIPHER ) && defined( USE_DTLS_SHA_MAC )
extern const cipher_suite_t DTLS_RSA_PSK_WITH_AES_256_CBC_SHA;
#endif /* if defined( USE_DTLS_RSA_PSK_KEYSCHEME ) && defined( USE_DTLS_AES_256_CBC_CIPHER ) && defined( USE_DTLS_SHA_MAC ) */

#if defined( USE_DTLS_RSA_KEYSCHEME ) && defined( USE_DTLS_SEED_CBC_CIPHER ) && defined( USE_DTLS_SHA_MAC )
extern const cipher_suite_t DTLS_RSA_WITH_SEED_CBC_SHA;
#endif /* if defined( USE_DTLS_RSA_KEYSCHEME ) && defined( USE_DTLS_SEED_CBC_CIPHER ) && defined( USE_DTLS_SHA_MAC ) */

#if defined( USE_DTLS_DH_DSS_KEYSCHEME ) && defined( USE_DTLS_SEED_CBC_CIPHER ) && defined( USE_DTLS_SHA_MAC )
extern const cipher_suite_t DTLS_DH_DSS_WITH_SEED_CBC_SHA;
#endif /* if defined( USE_DTLS_DH_DSS_KEYSCHEME ) && defined( USE_DTLS_SEED_CBC_CIPHER ) && defined( USE_DTLS_SHA_MAC ) */

#if defined( USE_DTLS_DH_RSA_KEYSCHEME ) && defined( USE_DTLS_SEED_CBC_CIPHER ) && defined( USE_DTLS_SHA_MAC )
extern const cipher_suite_t DTLS_DH_RSA_WITH_SEED_CBC_SHA;
#endif /* if defined( USE_DTLS_DH_RSA_KEYSCHEME ) && defined( USE_DTLS_SEED_CBC_CIPHER ) && defined( USE_DTLS_SHA_MAC ) */

#if defined( USE_DTLS_DHE_DSS_KEYSCHEME ) && defined( USE_DTLS_SEED_CBC_CIPHER ) && defined( USE_DTLS_SHA_MAC )
extern const cipher_suite_t DTLS_DHE_DSS_WITH_SEED_CBC_SHA;
#endif /* if defined( USE_DTLS_DHE_DSS_KEYSCHEME ) && defined( USE_DTLS_SEED_CBC_CIPHER ) && defined( USE_DTLS_SHA_MAC ) */

#if defined( USE_DTLS_DHE_RSA_KEYSCHEME ) && defined( USE_DTLS_SEED_CBC_CIPHER ) && defined( USE_DTLS_SHA_MAC )
extern const cipher_suite_t DTLS_DHE_RSA_WITH_SEED_CBC_SHA;
#endif /* if defined( USE_DTLS_DHE_RSA_KEYSCHEME ) && defined( USE_DTLS_SEED_CBC_CIPHER ) && defined( USE_DTLS_SHA_MAC ) */

#if defined( USE_DTLS_DH_anon_KEYSCHEME ) && defined( USE_DTLS_SEED_CBC_CIPHER ) && defined( USE_DTLS_SHA_MAC )
extern const cipher_suite_t DTLS_DH_anon_WITH_SEED_CBC_SHA;
#endif /* if defined( USE_DTLS_DH_anon_KEYSCHEME ) && defined( USE_DTLS_SEED_CBC_CIPHER ) && defined( USE_DTLS_SHA_MAC ) */

#if defined( USE_DTLS_RSA_KEYSCHEME ) && defined( USE_DTLS_AES_128_GCM_CIPHER ) && defined( USE_DTLS_SHA256_MAC )
extern const cipher_suite_t DTLS_RSA_WITH_AES_128_GCM_SHA256;
#endif /* if defined( USE_DTLS_RSA_KEYSCHEME ) && defined( USE_DTLS_AES_128_GCM_CIPHER ) && defined( USE_DTLS_SHA256_MAC ) */

#if defined( USE_DTLS_RSA_KEYSCHEME ) && defined( USE_DTLS_AES_256_GCM_CIPHER ) && defined( USE_DTLS_SHA384_MAC )
extern const cipher_suite_t DTLS_RSA_WITH_AES_256_GCM_SHA384;
#endif /* if defined( USE_DTLS_RSA_KEYSCHEME ) && defined( USE_DTLS_AES_256_GCM_CIPHER ) && defined( USE_DTLS_SHA384_MAC ) */

#if defined( USE_DTLS_DHE_RSA_KEYSCHEME ) && defined( USE_DTLS_AES_128_GCM_CIPHER ) && defined( USE_DTLS_SHA256_MAC )
extern const cipher_suite_t DTLS_DHE_RSA_WITH_AES_128_GCM_SHA256;
#endif /* if defined( USE_DTLS_DHE_RSA_KEYSCHEME ) && defined( USE_DTLS_AES_128_GCM_CIPHER ) && defined( USE_DTLS_SHA256_MAC ) */

#if defined( USE_DTLS_DHE_RSA_KEYSCHEME ) && defined( USE_DTLS_AES_256_GCM_CIPHER ) && defined( USE_DTLS_SHA384_MAC )
extern const cipher_suite_t DTLS_DHE_RSA_WITH_AES_256_GCM_SHA384;
#endif /* if defined( USE_DTLS_DHE_RSA_KEYSCHEME ) && defined( USE_DTLS_AES_256_GCM_CIPHER ) && defined( USE_DTLS_SHA384_MAC ) */

#if defined( USE_DTLS_DH_RSA_KEYSCHEME ) && defined( USE_DTLS_AES_128_GCM_CIPHER ) && defined( USE_DTLS_SHA256_MAC )
extern const cipher_suite_t DTLS_DH_RSA_WITH_AES_128_GCM_SHA256;
#endif /* if defined( USE_DTLS_DH_RSA_KEYSCHEME ) && defined( USE_DTLS_AES_128_GCM_CIPHER ) && defined( USE_DTLS_SHA256_MAC ) */

#if defined( USE_DTLS_DH_RSA_KEYSCHEME ) && defined( USE_DTLS_AES_256_GCM_CIPHER ) && defined( USE_DTLS_SHA384_MAC )
extern const cipher_suite_t DTLS_DH_RSA_WITH_AES_256_GCM_SHA384;
#endif /* if defined( USE_DTLS_DH_RSA_KEYSCHEME ) && defined( USE_DTLS_AES_256_GCM_CIPHER ) && defined( USE_DTLS_SHA384_MAC ) */

#if defined( USE_DTLS_DHE_DSS_KEYSCHEME ) && defined( USE_DTLS_AES_128_GCM_CIPHER ) && defined( USE_DTLS_SHA256_MAC )
extern const cipher_suite_t DTLS_DHE_DSS_WITH_AES_128_GCM_SHA256;
#endif /* if defined( USE_DTLS_DHE_DSS_KEYSCHEME ) && defined( USE_DTLS_AES_128_GCM_CIPHER ) && defined( USE_DTLS_SHA256_MAC ) */

#if defined( USE_DTLS_DHE_DSS_KEYSCHEME ) && defined( USE_DTLS_AES_256_GCM_CIPHER ) && defined( USE_DTLS_SHA384_MAC )
extern const cipher_suite_t DTLS_DHE_DSS_WITH_AES_256_GCM_SHA384;
#endif /* if defined( USE_DTLS_DHE_DSS_KEYSCHEME ) && defined( USE_DTLS_AES_256_GCM_CIPHER ) && defined( USE_DTLS_SHA384_MAC ) */

#if defined( USE_DTLS_DH_DSS_KEYSCHEME ) && defined( USE_DTLS_AES_128_GCM_CIPHER ) && defined( USE_DTLS_SHA256_MAC )
extern const cipher_suite_t DTLS_DH_DSS_WITH_AES_128_GCM_SHA256;
#endif /* if defined( USE_DTLS_DH_DSS_KEYSCHEME ) && defined( USE_DTLS_AES_128_GCM_CIPHER ) && defined( USE_DTLS_SHA256_MAC ) */

#if defined( USE_DTLS_DH_DSS_KEYSCHEME ) && defined( USE_DTLS_AES_256_GCM_CIPHER ) && defined( USE_DTLS_SHA384_MAC )
extern const cipher_suite_t DTLS_DH_DSS_WITH_AES_256_GCM_SHA384;
#endif /* if defined( USE_DTLS_DH_DSS_KEYSCHEME ) && defined( USE_DTLS_AES_256_GCM_CIPHER ) && defined( USE_DTLS_SHA384_MAC ) */

#if defined( USE_DTLS_DH_anon_KEYSCHEME ) && defined( USE_DTLS_AES_128_GCM_CIPHER ) && defined( USE_DTLS_SHA256_MAC )
extern const cipher_suite_t DTLS_DH_anon_WITH_AES_128_GCM_SHA256;
#endif /* if defined( USE_DTLS_DH_anon_KEYSCHEME ) && defined( USE_DTLS_AES_128_GCM_CIPHER ) && defined( USE_DTLS_SHA256_MAC ) */

#if defined( USE_DTLS_DH_anon_KEYSCHEME ) && defined( USE_DTLS_AES_256_GCM_CIPHER ) && defined( USE_DTLS_SHA384_MAC )
extern const cipher_suite_t DTLS_DH_anon_WITH_AES_256_GCM_SHA384;
#endif /* if defined( USE_DTLS_DH_anon_KEYSCHEME ) && defined( USE_DTLS_AES_256_GCM_CIPHER ) && defined( USE_DTLS_SHA384_MAC ) */

#if defined( USE_DTLS_PSK_KEYSCHEME ) && defined( USE_DTLS_AES_128_GCM_CIPHER ) && defined( USE_DTLS_SHA256_MAC )
extern const cipher_suite_t DTLS_PSK_WITH_AES_128_GCM_SHA256;
#endif /* if defined( USE_DTLS_PSK_KEYSCHEME ) && defined( USE_DTLS_AES_128_GCM_CIPHER ) && defined( USE_DTLS_SHA256_MAC ) */

#if defined( USE_DTLS_PSK_KEYSCHEME ) && defined( USE_DTLS_AES_256_GCM_CIPHER ) && defined( USE_DTLS_SHA384_MAC )
extern const cipher_suite_t DTLS_PSK_WITH_AES_256_GCM_SHA384;
#endif /* if defined( USE_DTLS_PSK_KEYSCHEME ) && defined( USE_DTLS_AES_256_GCM_CIPHER ) && defined( USE_DTLS_SHA384_MAC ) */

#if defined( USE_DTLS_DHE_PSK_KEYSCHEME ) && defined( USE_DTLS_AES_128_GCM_CIPHER ) && defined( USE_DTLS_SHA256_MAC )
extern const cipher_suite_t DTLS_DHE_PSK_WITH_AES_128_GCM_SHA256;
#endif /* if defined( USE_DTLS_DHE_PSK_KEYSCHEME ) && defined( USE_DTLS_AES_128_GCM_CIPHER ) && defined( USE_DTLS_SHA256_MAC ) */

#if defined( USE_DTLS_DHE_PSK_KEYSCHEME ) && defined( USE_DTLS_AES_256_GCM_CIPHER ) && defined( USE_DTLS_SHA384_MAC )
extern const cipher_suite_t DTLS_DHE_PSK_WITH_AES_256_GCM_SHA384;
#endif /* if defined( USE_DTLS_DHE_PSK_KEYSCHEME ) && defined( USE_DTLS_AES_256_GCM_CIPHER ) && defined( USE_DTLS_SHA384_MAC ) */

#if defined( USE_DTLS_RSA_PSK_KEYSCHEME ) && defined( USE_DTLS_AES_128_GCM_CIPHER ) && defined( USE_DTLS_SHA256_MAC )
extern const cipher_suite_t DTLS_RSA_PSK_WITH_AES_128_GCM_SHA256;
#endif /* if defined( USE_DTLS_RSA_PSK_KEYSCHEME ) && defined( USE_DTLS_AES_128_GCM_CIPHER ) && defined( USE_DTLS_SHA256_MAC ) */

#if defined( USE_DTLS_RSA_PSK_KEYSCHEME ) && defined( USE_DTLS_AES_256_GCM_CIPHER ) && defined( USE_DTLS_SHA384_MAC )
extern const cipher_suite_t DTLS_RSA_PSK_WITH_AES_256_GCM_SHA384;
#endif /* if defined( USE_DTLS_RSA_PSK_KEYSCHEME ) && defined( USE_DTLS_AES_256_GCM_CIPHER ) && defined( USE_DTLS_SHA384_MAC ) */

#if defined( USE_DTLS_PSK_KEYSCHEME ) && defined( USE_DTLS_AES_128_CBC_CIPHER ) && defined( USE_DTLS_SHA256_MAC )
extern const cipher_suite_t DTLS_PSK_WITH_AES_128_CBC_SHA256;
#endif /* if defined( USE_DTLS_PSK_KEYSCHEME ) && defined( USE_DTLS_AES_128_CBC_CIPHER ) && defined( USE_DTLS_SHA256_MAC ) */

#if defined( USE_DTLS_PSK_KEYSCHEME ) && defined( USE_DTLS_AES_256_CBC_CIPHER ) && defined( USE_DTLS_SHA384_MAC )
extern const cipher_suite_t DTLS_PSK_WITH_AES_256_CBC_SHA384;
#endif /* if defined( USE_DTLS_PSK_KEYSCHEME ) && defined( USE_DTLS_AES_256_CBC_CIPHER ) && defined( USE_DTLS_SHA384_MAC ) */

#if defined( USE_DTLS_PSK_KEYSCHEME ) && defined( USE_DTLS_NULL_CIPHER ) && defined( USE_DTLS_SHA256_MAC )
extern const cipher_suite_t DTLS_PSK_WITH_NULL_SHA256;
#endif /* if defined( USE_DTLS_PSK_KEYSCHEME ) && defined( USE_DTLS_NULL_CIPHER ) && defined( USE_DTLS_SHA256_MAC ) */

#if defined( USE_DTLS_PSK_KEYSCHEME ) && defined( USE_DTLS_NULL_CIPHER ) && defined( USE_DTLS_SHA384_MAC )
extern const cipher_suite_t DTLS_PSK_WITH_NULL_SHA384;
#endif /* if defined( USE_DTLS_PSK_KEYSCHEME ) && defined( USE_DTLS_NULL_CIPHER ) && defined( USE_DTLS_SHA384_MAC ) */

#if defined( USE_DTLS_DHE_PSK_KEYSCHEME ) && defined( USE_DTLS_AES_128_CBC_CIPHER ) && defined( USE_DTLS_SHA256_MAC )
extern const cipher_suite_t DTLS_DHE_PSK_WITH_AES_128_CBC_SHA256;
#endif /* if defined( USE_DTLS_DHE_PSK_KEYSCHEME ) && defined( USE_DTLS_AES_128_CBC_CIPHER ) && defined( USE_DTLS_SHA256_MAC ) */

#if defined( USE_DTLS_DHE_PSK_KEYSCHEME ) && defined( USE_DTLS_AES_256_CBC_CIPHER ) && defined( USE_DTLS_SHA384_MAC )
extern const cipher_suite_t DTLS_DHE_PSK_WITH_AES_256_CBC_SHA384;
#endif /* if defined( USE_DTLS_DHE_PSK_KEYSCHEME ) && defined( USE_DTLS_AES_256_CBC_CIPHER ) && defined( USE_DTLS_SHA384_MAC ) */

#if defined( USE_DTLS_DHE_PSK_KEYSCHEME ) && defined( USE_DTLS_NULL_CIPHER ) && defined( USE_DTLS_SHA256_MAC )
extern const cipher_suite_t DTLS_DHE_PSK_WITH_NULL_SHA256;
#endif /* if defined( USE_DTLS_DHE_PSK_KEYSCHEME ) && defined( USE_DTLS_NULL_CIPHER ) && defined( USE_DTLS_SHA256_MAC ) */

#if defined( USE_DTLS_DHE_PSK_KEYSCHEME ) && defined( USE_DTLS_NULL_CIPHER ) && defined( USE_DTLS_SHA384_MAC )
extern const cipher_suite_t DTLS_DHE_PSK_WITH_NULL_SHA384;
#endif /* if defined( USE_DTLS_DHE_PSK_KEYSCHEME ) && defined( USE_DTLS_NULL_CIPHER ) && defined( USE_DTLS_SHA384_MAC ) */

#if defined( USE_DTLS_RSA_PSK_KEYSCHEME ) && defined( USE_DTLS_AES_128_CBC_CIPHER ) && defined( USE_DTLS_SHA256_MAC )
extern const cipher_suite_t DTLS_RSA_PSK_WITH_AES_128_CBC_SHA256;
#endif /* if defined( USE_DTLS_RSA_PSK_KEYSCHEME ) && defined( USE_DTLS_AES_128_CBC_CIPHER ) && defined( USE_DTLS_SHA256_MAC ) */

#if defined( USE_DTLS_RSA_PSK_KEYSCHEME ) && defined( USE_DTLS_AES_256_CBC_CIPHER ) && defined( USE_DTLS_SHA384_MAC )
extern const cipher_suite_t DTLS_RSA_PSK_WITH_AES_256_CBC_SHA384;
#endif /* if defined( USE_DTLS_RSA_PSK_KEYSCHEME ) && defined( USE_DTLS_AES_256_CBC_CIPHER ) && defined( USE_DTLS_SHA384_MAC ) */

#if defined( USE_DTLS_RSA_PSK_KEYSCHEME ) && defined( USE_DTLS_NULL_CIPHER ) && defined( USE_DTLS_SHA256_MAC )
extern const cipher_suite_t DTLS_RSA_PSK_WITH_NULL_SHA256;
#endif /* if defined( USE_DTLS_RSA_PSK_KEYSCHEME ) && defined( USE_DTLS_NULL_CIPHER ) && defined( USE_DTLS_SHA256_MAC ) */

#if defined( USE_DTLS_RSA_PSK_KEYSCHEME ) && defined( USE_DTLS_NULL_CIPHER ) && defined( USE_DTLS_SHA384_MAC )
extern const cipher_suite_t DTLS_RSA_PSK_WITH_NULL_SHA384;
#endif /* if defined( USE_DTLS_RSA_PSK_KEYSCHEME ) && defined( USE_DTLS_NULL_CIPHER ) && defined( USE_DTLS_SHA384_MAC ) */

#if defined( USE_DTLS_RSA_KEYSCHEME ) && defined( USE_DTLS_CAMELLIA_128_CBC_CIPHER ) && defined( USE_DTLS_SHA256_MAC )
extern const cipher_suite_t DTLS_RSA_WITH_CAMELLIA_128_CBC_SHA256;
#endif /* if defined( USE_DTLS_RSA_KEYSCHEME ) && defined( USE_DTLS_CAMELLIA_128_CBC_CIPHER ) && defined( USE_DTLS_SHA256_MAC ) */

#if defined( USE_DTLS_DH_DSS_KEYSCHEME ) && defined( USE_DTLS_CAMELLIA_128_CBC_CIPHER ) && defined( USE_DTLS_SHA256_MAC )
extern const cipher_suite_t DTLS_DH_DSS_WITH_CAMELLIA_128_CBC_SHA256;
#endif /* if defined( USE_DTLS_DH_DSS_KEYSCHEME ) && defined( USE_DTLS_CAMELLIA_128_CBC_CIPHER ) && defined( USE_DTLS_SHA256_MAC ) */

#if defined( USE_DTLS_DH_RSA_KEYSCHEME ) && defined( USE_DTLS_CAMELLIA_128_CBC_CIPHER ) && defined( USE_DTLS_SHA256_MAC )
extern const cipher_suite_t DTLS_DH_RSA_WITH_CAMELLIA_128_CBC_SHA256;
#endif /* if defined( USE_DTLS_DH_RSA_KEYSCHEME ) && defined( USE_DTLS_CAMELLIA_128_CBC_CIPHER ) && defined( USE_DTLS_SHA256_MAC ) */

#if defined( USE_DTLS_DHE_DSS_KEYSCHEME ) && defined( USE_DTLS_CAMELLIA_128_CBC_CIPHER ) && defined( USE_DTLS_SHA256_MAC )
extern const cipher_suite_t DTLS_DHE_DSS_WITH_CAMELLIA_128_CBC_SHA256;
#endif /* if defined( USE_DTLS_DHE_DSS_KEYSCHEME ) && defined( USE_DTLS_CAMELLIA_128_CBC_CIPHER ) && defined( USE_DTLS_SHA256_MAC ) */

#if defined( USE_DTLS_DHE_RSA_KEYSCHEME ) && defined( USE_DTLS_CAMELLIA_128_CBC_CIPHER ) && defined( USE_DTLS_SHA256_MAC )
extern const cipher_suite_t DTLS_DHE_RSA_WITH_CAMELLIA_128_CBC_SHA256;
#endif /* if defined( USE_DTLS_DHE_RSA_KEYSCHEME ) && defined( USE_DTLS_CAMELLIA_128_CBC_CIPHER ) && defined( USE_DTLS_SHA256_MAC ) */

#if defined( USE_DTLS_DH_anon_KEYSCHEME ) && defined( USE_DTLS_CAMELLIA_128_CBC_CIPHER ) && defined( USE_DTLS_SHA256_MAC )
extern const cipher_suite_t DTLS_DH_anon_WITH_CAMELLIA_128_CBC_SHA256;
#endif /* if defined( USE_DTLS_DH_anon_KEYSCHEME ) && defined( USE_DTLS_CAMELLIA_128_CBC_CIPHER ) && defined( USE_DTLS_SHA256_MAC ) */

#if defined( USE_DTLS_RSA_KEYSCHEME ) && defined( USE_DTLS_CAMELLIA_256_CBC_CIPHER ) && defined( USE_DTLS_SHA256_MAC )
extern const cipher_suite_t DTLS_RSA_WITH_CAMELLIA_256_CBC_SHA256;
#endif /* if defined( USE_DTLS_RSA_KEYSCHEME ) && defined( USE_DTLS_CAMELLIA_256_CBC_CIPHER ) && defined( USE_DTLS_SHA256_MAC ) */

#if defined( USE_DTLS_DH_DSS_KEYSCHEME ) && defined( USE_DTLS_CAMELLIA_256_CBC_CIPHER ) && defined( USE_DTLS_SHA256_MAC )
extern const cipher_suite_t DTLS_DH_DSS_WITH_CAMELLIA_256_CBC_SHA256;
#endif /* if defined( USE_DTLS_DH_DSS_KEYSCHEME ) && defined( USE_DTLS_CAMELLIA_256_CBC_CIPHER ) && defined( USE_DTLS_SHA256_MAC ) */

#if defined( USE_DTLS_DH_RSA_KEYSCHEME ) && defined( USE_DTLS_CAMELLIA_256_CBC_CIPHER ) && defined( USE_DTLS_SHA256_MAC )
extern const cipher_suite_t DTLS_DH_RSA_WITH_CAMELLIA_256_CBC_SHA256;
#endif /* if defined( USE_DTLS_DH_RSA_KEYSCHEME ) && defined( USE_DTLS_CAMELLIA_256_CBC_CIPHER ) && defined( USE_DTLS_SHA256_MAC ) */

#if defined( USE_DTLS_DHE_DSS_KEYSCHEME ) && defined( USE_DTLS_CAMELLIA_256_CBC_CIPHER ) && defined( USE_DTLS_SHA256_MAC )
extern const cipher_suite_t DTLS_DHE_DSS_WITH_CAMELLIA_256_CBC_SHA256;
#endif /* if defined( USE_DTLS_DHE_DSS_KEYSCHEME ) && defined( USE_DTLS_CAMELLIA_256_CBC_CIPHER ) && defined( USE_DTLS_SHA256_MAC ) */

#if defined( USE_DTLS_DHE_RSA_KEYSCHEME ) && defined( USE_DTLS_CAMELLIA_256_CBC_CIPHER ) && defined( USE_DTLS_SHA256_MAC )
extern const cipher_suite_t DTLS_DHE_RSA_WITH_CAMELLIA_256_CBC_SHA256;
#endif /* if defined( USE_DTLS_DHE_RSA_KEYSCHEME ) && defined( USE_DTLS_CAMELLIA_256_CBC_CIPHER ) && defined( USE_DTLS_SHA256_MAC ) */

#if defined( USE_DTLS_DH_anon_KEYSCHEME ) && defined( USE_DTLS_CAMELLIA_256_CBC_CIPHER ) && defined( USE_DTLS_SHA256_MAC )
extern const cipher_suite_t DTLS_DH_anon_WITH_CAMELLIA_256_CBC_SHA256;
#endif /* if defined( USE_DTLS_DH_anon_KEYSCHEME ) && defined( USE_DTLS_CAMELLIA_256_CBC_CIPHER ) && defined( USE_DTLS_SHA256_MAC ) */

#if defined( USE_DTLS_ECDH_ECDSA_KEYSCHEME ) && defined( USE_DTLS_NULL_CIPHER ) && defined( USE_DTLS_SHA_MAC )
extern const cipher_suite_t DTLS_ECDH_ECDSA_WITH_NULL_SHA;
#endif /* if defined( USE_DTLS_ECDH_ECDSA_KEYSCHEME ) && defined( USE_DTLS_NULL_CIPHER ) && defined( USE_DTLS_SHA_MAC ) */

#if defined( USE_DTLS_ECDH_ECDSA_KEYSCHEME ) && defined( USE_DTLS_RC4_128_CIPHER ) && defined( USE_DTLS_SHA_MAC )
extern const cipher_suite_t DTLS_ECDH_ECDSA_WITH_RC4_128_SHA;
#endif /* if defined( USE_DTLS_ECDH_ECDSA_KEYSCHEME ) && defined( USE_DTLS_RC4_128_CIPHER ) && defined( USE_DTLS_SHA_MAC ) */

#if defined( USE_DTLS_ECDH_ECDSA_KEYSCHEME ) && defined( USE_DTLS_3DES_EDE_CBC_CIPHER ) && defined( USE_DTLS_SHA_MAC )
extern const cipher_suite_t DTLS_ECDH_ECDSA_WITH_3DES_EDE_CBC_SHA;
#endif /* if defined( USE_DTLS_ECDH_ECDSA_KEYSCHEME ) && defined( USE_DTLS_3DES_EDE_CBC_CIPHER ) && defined( USE_DTLS_SHA_MAC ) */

#if defined( USE_DTLS_ECDH_ECDSA_KEYSCHEME ) && defined( USE_DTLS_AES_128_CBC_CIPHER ) && defined( USE_DTLS_SHA_MAC )
extern const cipher_suite_t DTLS_ECDH_ECDSA_WITH_AES_128_CBC_SHA;
#endif /* if defined( USE_DTLS_ECDH_ECDSA_KEYSCHEME ) && defined( USE_DTLS_AES_128_CBC_CIPHER ) && defined( USE_DTLS_SHA_MAC ) */

#if defined( USE_DTLS_ECDH_ECDSA_KEYSCHEME ) && defined( USE_DTLS_AES_256_CBC_CIPHER ) && defined( USE_DTLS_SHA_MAC )
extern const cipher_suite_t DTLS_ECDH_ECDSA_WITH_AES_256_CBC_SHA;
#endif /* if defined( USE_DTLS_ECDH_ECDSA_KEYSCHEME ) && defined( USE_DTLS_AES_256_CBC_CIPHER ) && defined( USE_DTLS_SHA_MAC ) */

#if defined( USE_DTLS_ECDHE_ECDSA_KEYSCHEME ) && defined( USE_DTLS_NULL_CIPHER ) && defined( USE_DTLS_SHA_MAC )
extern const cipher_suite_t DTLS_ECDHE_ECDSA_WITH_NULL_SHA;
#endif /* if defined( USE_DTLS_ECDHE_ECDSA_KEYSCHEME ) && defined( USE_DTLS_NULL_CIPHER ) && defined( USE_DTLS_SHA_MAC ) */

#if defined( USE_DTLS_ECDHE_ECDSA_KEYSCHEME ) && defined( USE_DTLS_RC4_128_CIPHER ) && defined( USE_DTLS_SHA_MAC )
extern const cipher_suite_t DTLS_ECDHE_ECDSA_WITH_RC4_128_SHA;
#endif /* if defined( USE_DTLS_ECDHE_ECDSA_KEYSCHEME ) && defined( USE_DTLS_RC4_128_CIPHER ) && defined( USE_DTLS_SHA_MAC ) */

#if defined( USE_DTLS_ECDHE_ECDSA_KEYSCHEME ) && defined( USE_DTLS_3DES_EDE_CBC_CIPHER ) && defined( USE_DTLS_SHA_MAC )
extern const cipher_suite_t DTLS_ECDHE_ECDSA_WITH_3DES_EDE_CBC_SHA;
#endif /* if defined( USE_DTLS_ECDHE_ECDSA_KEYSCHEME ) && defined( USE_DTLS_3DES_EDE_CBC_CIPHER ) && defined( USE_DTLS_SHA_MAC ) */

#if defined( USE_DTLS_ECDHE_ECDSA_KEYSCHEME ) && defined( USE_DTLS_AES_128_CBC_CIPHER ) && defined( USE_DTLS_SHA_MAC )
extern const cipher_suite_t DTLS_ECDHE_ECDSA_WITH_AES_128_CBC_SHA;
#endif /* if defined( USE_DTLS_ECDHE_ECDSA_KEYSCHEME ) && defined( USE_DTLS_AES_128_CBC_CIPHER ) && defined( USE_DTLS_SHA_MAC ) */

#if defined( USE_DTLS_ECDHE_ECDSA_KEYSCHEME ) && defined( USE_DTLS_AES_256_CBC_CIPHER ) && defined( USE_DTLS_SHA_MAC )
extern const cipher_suite_t DTLS_ECDHE_ECDSA_WITH_AES_256_CBC_SHA;
#endif /* if defined( USE_DTLS_ECDHE_ECDSA_KEYSCHEME ) && defined( USE_DTLS_AES_256_CBC_CIPHER ) && defined( USE_DTLS_SHA_MAC ) */

#if defined( USE_DTLS_ECDH_RSA_KEYSCHEME ) && defined( USE_DTLS_NULL_CIPHER ) && defined( USE_DTLS_SHA_MAC )
extern const cipher_suite_t DTLS_ECDH_RSA_WITH_NULL_SHA;
#endif /* if defined( USE_DTLS_ECDH_RSA_KEYSCHEME ) && defined( USE_DTLS_NULL_CIPHER ) && defined( USE_DTLS_SHA_MAC ) */

#if defined( USE_DTLS_ECDH_RSA_KEYSCHEME ) && defined( USE_DTLS_RC4_128_CIPHER ) && defined( USE_DTLS_SHA_MAC )
extern const cipher_suite_t DTLS_ECDH_RSA_WITH_RC4_128_SHA;
#endif /* if defined( USE_DTLS_ECDH_RSA_KEYSCHEME ) && defined( USE_DTLS_RC4_128_CIPHER ) && defined( USE_DTLS_SHA_MAC ) */

#if defined( USE_DTLS_ECDH_RSA_KEYSCHEME ) && defined( USE_DTLS_3DES_EDE_CBC_CIPHER ) && defined( USE_DTLS_SHA_MAC )
extern const cipher_suite_t DTLS_ECDH_RSA_WITH_3DES_EDE_CBC_SHA;
#endif /* if defined( USE_DTLS_ECDH_RSA_KEYSCHEME ) && defined( USE_DTLS_3DES_EDE_CBC_CIPHER ) && defined( USE_DTLS_SHA_MAC ) */

#if defined( USE_DTLS_ECDH_RSA_KEYSCHEME ) && defined( USE_DTLS_AES_128_CBC_CIPHER ) && defined( USE_DTLS_SHA_MAC )
extern const cipher_suite_t DTLS_ECDH_RSA_WITH_AES_128_CBC_SHA;
#endif /* if defined( USE_DTLS_ECDH_RSA_KEYSCHEME ) && defined( USE_DTLS_AES_128_CBC_CIPHER ) && defined( USE_DTLS_SHA_MAC ) */

#if defined( USE_DTLS_ECDH_RSA_KEYSCHEME ) && defined( USE_DTLS_AES_256_CBC_CIPHER ) && defined( USE_DTLS_SHA_MAC )
extern const cipher_suite_t DTLS_ECDH_RSA_WITH_AES_256_CBC_SHA;
#endif /* if defined( USE_DTLS_ECDH_RSA_KEYSCHEME ) && defined( USE_DTLS_AES_256_CBC_CIPHER ) && defined( USE_DTLS_SHA_MAC ) */

#if defined( USE_DTLS_ECDHE_RSA_KEYSCHEME ) && defined( USE_DTLS_NULL_CIPHER ) && defined( USE_DTLS_SHA_MAC )
extern const cipher_suite_t DTLS_ECDHE_RSA_WITH_NULL_SHA;
#endif /* if defined( USE_DTLS_ECDHE_RSA_KEYSCHEME ) && defined( USE_DTLS_NULL_CIPHER ) && defined( USE_DTLS_SHA_MAC ) */

#if defined( USE_DTLS_ECDHE_RSA_KEYSCHEME ) && defined( USE_DTLS_RC4_128_CIPHER ) && defined( USE_DTLS_SHA_MAC )
extern const cipher_suite_t DTLS_ECDHE_RSA_WITH_RC4_128_SHA;
#endif /* if defined( USE_DTLS_ECDHE_RSA_KEYSCHEME ) && defined( USE_DTLS_RC4_128_CIPHER ) && defined( USE_DTLS_SHA_MAC ) */

#if defined( USE_DTLS_ECDHE_RSA_KEYSCHEME ) && defined( USE_DTLS_3DES_EDE_CBC_CIPHER ) && defined( USE_DTLS_SHA_MAC )
extern const cipher_suite_t DTLS_ECDHE_RSA_WITH_3DES_EDE_CBC_SHA;
#endif /* if defined( USE_DTLS_ECDHE_RSA_KEYSCHEME ) && defined( USE_DTLS_3DES_EDE_CBC_CIPHER ) && defined( USE_DTLS_SHA_MAC ) */

#if defined( USE_DTLS_ECDHE_RSA_KEYSCHEME ) && defined( USE_DTLS_AES_128_CBC_CIPHER ) && defined( USE_DTLS_SHA_MAC )
extern const cipher_suite_t DTLS_ECDHE_RSA_WITH_AES_128_CBC_SHA;
#endif /* if defined( USE_DTLS_ECDHE_RSA_KEYSCHEME ) && defined( USE_DTLS_AES_128_CBC_CIPHER ) && defined( USE_DTLS_SHA_MAC ) */

#if defined( USE_DTLS_ECDHE_RSA_KEYSCHEME ) && defined( USE_DTLS_AES_256_CBC_CIPHER ) && defined( USE_DTLS_SHA_MAC )
extern const cipher_suite_t DTLS_ECDHE_RSA_WITH_AES_256_CBC_SHA;
#endif /* if defined( USE_DTLS_ECDHE_RSA_KEYSCHEME ) && defined( USE_DTLS_AES_256_CBC_CIPHER ) && defined( USE_DTLS_SHA_MAC ) */

#if defined( USE_DTLS_ECDH_anon_KEYSCHEME ) && defined( USE_DTLS_NULL_CIPHER ) && defined( USE_DTLS_SHA_MAC )
extern const cipher_suite_t DTLS_ECDH_anon_WITH_NULL_SHA;
#endif /* if defined( USE_DTLS_ECDH_anon_KEYSCHEME ) && defined( USE_DTLS_NULL_CIPHER ) && defined( USE_DTLS_SHA_MAC ) */

#if defined( USE_DTLS_ECDH_anon_KEYSCHEME ) && defined( USE_DTLS_RC4_128_CIPHER ) && defined( USE_DTLS_SHA_MAC )
extern const cipher_suite_t DTLS_ECDH_anon_WITH_RC4_128_SHA;
#endif /* if defined( USE_DTLS_ECDH_anon_KEYSCHEME ) && defined( USE_DTLS_RC4_128_CIPHER ) && defined( USE_DTLS_SHA_MAC ) */

#if defined( USE_DTLS_ECDH_anon_KEYSCHEME ) && defined( USE_DTLS_3DES_EDE_CBC_CIPHER ) && defined( USE_DTLS_SHA_MAC )
extern const cipher_suite_t DTLS_ECDH_anon_WITH_3DES_EDE_CBC_SHA;
#endif /* if defined( USE_DTLS_ECDH_anon_KEYSCHEME ) && defined( USE_DTLS_3DES_EDE_CBC_CIPHER ) && defined( USE_DTLS_SHA_MAC ) */

#if defined( USE_DTLS_ECDH_anon_KEYSCHEME ) && defined( USE_DTLS_AES_128_CBC_CIPHER ) && defined( USE_DTLS_SHA_MAC )
extern const cipher_suite_t DTLS_ECDH_anon_WITH_AES_128_CBC_SHA;
#endif /* if defined( USE_DTLS_ECDH_anon_KEYSCHEME ) && defined( USE_DTLS_AES_128_CBC_CIPHER ) && defined( USE_DTLS_SHA_MAC ) */

#if defined( USE_DTLS_ECDH_anon_KEYSCHEME ) && defined( USE_DTLS_AES_256_CBC_CIPHER ) && defined( USE_DTLS_SHA_MAC )
extern const cipher_suite_t DTLS_ECDH_anon_WITH_AES_256_CBC_SHA;
#endif /* if defined( USE_DTLS_ECDH_anon_KEYSCHEME ) && defined( USE_DTLS_AES_256_CBC_CIPHER ) && defined( USE_DTLS_SHA_MAC ) */

#if defined( USE_DTLS_SRP_SHA_KEYSCHEME ) && defined( USE_DTLS_3DES_EDE_CBC_CIPHER ) && defined( USE_DTLS_SHA_MAC )
extern const cipher_suite_t DTLS_SRP_SHA_WITH_3DES_EDE_CBC_SHA;
#endif /* if defined( USE_DTLS_SRP_SHA_KEYSCHEME ) && defined( USE_DTLS_3DES_EDE_CBC_CIPHER ) && defined( USE_DTLS_SHA_MAC ) */

#if defined( USE_DTLS_SRP_SHA_RSA_KEYSCHEME ) && defined( USE_DTLS_3DES_EDE_CBC_CIPHER ) && defined( USE_DTLS_SHA_MAC )
extern const cipher_suite_t DTLS_SRP_SHA_RSA_WITH_3DES_EDE_CBC_SHA;
#endif /* if defined( USE_DTLS_SRP_SHA_RSA_KEYSCHEME ) && defined( USE_DTLS_3DES_EDE_CBC_CIPHER ) && defined( USE_DTLS_SHA_MAC ) */

#if defined( USE_DTLS_SRP_SHA_DSS_KEYSCHEME ) && defined( USE_DTLS_3DES_EDE_CBC_CIPHER ) && defined( USE_DTLS_SHA_MAC )
extern const cipher_suite_t DTLS_SRP_SHA_DSS_WITH_3DES_EDE_CBC_SHA;
#endif /* if defined( USE_DTLS_SRP_SHA_DSS_KEYSCHEME ) && defined( USE_DTLS_3DES_EDE_CBC_CIPHER ) && defined( USE_DTLS_SHA_MAC ) */

#if defined( USE_DTLS_SRP_SHA_KEYSCHEME ) && defined( USE_DTLS_AES_128_CBC_CIPHER ) && defined( USE_DTLS_SHA_MAC )
extern const cipher_suite_t DTLS_SRP_SHA_WITH_AES_128_CBC_SHA;
#endif /* if defined( USE_DTLS_SRP_SHA_KEYSCHEME ) && defined( USE_DTLS_AES_128_CBC_CIPHER ) && defined( USE_DTLS_SHA_MAC ) */

#if defined( USE_DTLS_SRP_SHA_RSA_KEYSCHEME ) && defined( USE_DTLS_AES_128_CBC_CIPHER ) && defined( USE_DTLS_SHA_MAC )
extern const cipher_suite_t DTLS_SRP_SHA_RSA_WITH_AES_128_CBC_SHA;
#endif /* if defined( USE_DTLS_SRP_SHA_RSA_KEYSCHEME ) && defined( USE_DTLS_AES_128_CBC_CIPHER ) && defined( USE_DTLS_SHA_MAC ) */

#if defined( USE_DTLS_SRP_SHA_DSS_KEYSCHEME ) && defined( USE_DTLS_AES_128_CBC_CIPHER ) && defined( USE_DTLS_SHA_MAC )
extern const cipher_suite_t DTLS_SRP_SHA_DSS_WITH_AES_128_CBC_SHA;
#endif /* if defined( USE_DTLS_SRP_SHA_DSS_KEYSCHEME ) && defined( USE_DTLS_AES_128_CBC_CIPHER ) && defined( USE_DTLS_SHA_MAC ) */

#if defined( USE_DTLS_SRP_SHA_KEYSCHEME ) && defined( USE_DTLS_AES_256_CBC_CIPHER ) && defined( USE_DTLS_SHA_MAC )
extern const cipher_suite_t DTLS_SRP_SHA_WITH_AES_256_CBC_SHA;
#endif /* if defined( USE_DTLS_SRP_SHA_KEYSCHEME ) && defined( USE_DTLS_AES_256_CBC_CIPHER ) && defined( USE_DTLS_SHA_MAC ) */

#if defined( USE_DTLS_SRP_SHA_RSA_KEYSCHEME ) && defined( USE_DTLS_AES_256_CBC_CIPHER ) && defined( USE_DTLS_SHA_MAC )
extern const cipher_suite_t DTLS_SRP_SHA_RSA_WITH_AES_256_CBC_SHA;
#endif /* if defined( USE_DTLS_SRP_SHA_RSA_KEYSCHEME ) && defined( USE_DTLS_AES_256_CBC_CIPHER ) && defined( USE_DTLS_SHA_MAC ) */

#if defined( USE_DTLS_SRP_SHA_DSS_KEYSCHEME ) && defined( USE_DTLS_AES_256_CBC_CIPHER ) && defined( USE_DTLS_SHA_MAC )
extern const cipher_suite_t DTLS_SRP_SHA_DSS_WITH_AES_256_CBC_SHA;
#endif /* if defined( USE_DTLS_SRP_SHA_DSS_KEYSCHEME ) && defined( USE_DTLS_AES_256_CBC_CIPHER ) && defined( USE_DTLS_SHA_MAC ) */

#if defined( USE_DTLS_ECDHE_ECDSA_KEYSCHEME ) && defined( USE_DTLS_AES_128_CBC_CIPHER ) && defined( USE_DTLS_SHA256_MAC )
extern const cipher_suite_t DTLS_ECDHE_ECDSA_WITH_AES_128_CBC_SHA256;
#endif /* if defined( USE_DTLS_ECDHE_ECDSA_KEYSCHEME ) && defined( USE_DTLS_AES_128_CBC_CIPHER ) && defined( USE_DTLS_SHA256_MAC ) */

#if defined( USE_DTLS_ECDHE_ECDSA_KEYSCHEME ) && defined( USE_DTLS_AES_256_CBC_CIPHER ) && defined( USE_DTLS_SHA384_MAC )
extern const cipher_suite_t DTLS_ECDHE_ECDSA_WITH_AES_256_CBC_SHA384;
#endif /* if defined( USE_DTLS_ECDHE_ECDSA_KEYSCHEME ) && defined( USE_DTLS_AES_256_CBC_CIPHER ) && defined( USE_DTLS_SHA384_MAC ) */

#if defined( USE_DTLS_ECDH_ECDSA_KEYSCHEME ) && defined( USE_DTLS_AES_128_CBC_CIPHER ) && defined( USE_DTLS_SHA256_MAC )
extern const cipher_suite_t DTLS_ECDH_ECDSA_WITH_AES_128_CBC_SHA256;
#endif /* if defined( USE_DTLS_ECDH_ECDSA_KEYSCHEME ) && defined( USE_DTLS_AES_128_CBC_CIPHER ) && defined( USE_DTLS_SHA256_MAC ) */

#if defined( USE_DTLS_ECDH_ECDSA_KEYSCHEME ) && defined( USE_DTLS_AES_256_CBC_CIPHER ) && defined( USE_DTLS_SHA384_MAC )
extern const cipher_suite_t DTLS_ECDH_ECDSA_WITH_AES_256_CBC_SHA384;
#endif /* if defined( USE_DTLS_ECDH_ECDSA_KEYSCHEME ) && defined( USE_DTLS_AES_256_CBC_CIPHER ) && defined( USE_DTLS_SHA384_MAC ) */

#if defined( USE_DTLS_ECDHE_RSA_KEYSCHEME ) && defined( USE_DTLS_AES_128_CBC_CIPHER ) && defined( USE_DTLS_SHA256_MAC )
extern const cipher_suite_t DTLS_ECDHE_RSA_WITH_AES_128_CBC_SHA256;
#endif /* if defined( USE_DTLS_ECDHE_RSA_KEYSCHEME ) && defined( USE_DTLS_AES_128_CBC_CIPHER ) && defined( USE_DTLS_SHA256_MAC ) */

#if defined( USE_DTLS_ECDHE_RSA_KEYSCHEME ) && defined( USE_DTLS_AES_256_CBC_CIPHER ) && defined( USE_DTLS_SHA384_MAC )
extern const cipher_suite_t DTLS_ECDHE_RSA_WITH_AES_256_CBC_SHA384;
#endif /* if defined( USE_DTLS_ECDHE_RSA_KEYSCHEME ) && defined( USE_DTLS_AES_256_CBC_CIPHER ) && defined( USE_DTLS_SHA384_MAC ) */

#if defined( USE_DTLS_ECDH_RSA_KEYSCHEME ) && defined( USE_DTLS_AES_128_CBC_CIPHER ) && defined( USE_DTLS_SHA256_MAC )
extern const cipher_suite_t DTLS_ECDH_RSA_WITH_AES_128_CBC_SHA256;
#endif /* if defined( USE_DTLS_ECDH_RSA_KEYSCHEME ) && defined( USE_DTLS_AES_128_CBC_CIPHER ) && defined( USE_DTLS_SHA256_MAC ) */

#if defined( USE_DTLS_ECDH_RSA_KEYSCHEME ) && defined( USE_DTLS_AES_256_CBC_CIPHER ) && defined( USE_DTLS_SHA384_MAC )
extern const cipher_suite_t DTLS_ECDH_RSA_WITH_AES_256_CBC_SHA384;
#endif /* if defined( USE_DTLS_ECDH_RSA_KEYSCHEME ) && defined( USE_DTLS_AES_256_CBC_CIPHER ) && defined( USE_DTLS_SHA384_MAC ) */

#if defined( USE_DTLS_ECDHE_ECDSA_KEYSCHEME ) && defined( USE_DTLS_AES_128_GCM_CIPHER ) && defined( USE_DTLS_SHA256_MAC )
extern const cipher_suite_t DTLS_ECDHE_ECDSA_WITH_AES_128_GCM_SHA256;
#endif /* if defined( USE_DTLS_ECDHE_ECDSA_KEYSCHEME ) && defined( USE_DTLS_AES_128_GCM_CIPHER ) && defined( USE_DTLS_SHA256_MAC ) */

#if defined( USE_DTLS_ECDHE_ECDSA_KEYSCHEME ) && defined( USE_DTLS_AES_256_GCM_CIPHER ) && defined( USE_DTLS_SHA384_MAC )
extern const cipher_suite_t DTLS_ECDHE_ECDSA_WITH_AES_256_GCM_SHA384;
#endif /* if defined( USE_DTLS_ECDHE_ECDSA_KEYSCHEME ) && defined( USE_DTLS_AES_256_GCM_CIPHER ) && defined( USE_DTLS_SHA384_MAC ) */

#if defined( USE_DTLS_ECDH_ECDSA_KEYSCHEME ) && defined( USE_DTLS_AES_128_GCM_CIPHER ) && defined( USE_DTLS_SHA256_MAC )
extern const cipher_suite_t DTLS_ECDH_ECDSA_WITH_AES_128_GCM_SHA256;
#endif /* if defined( USE_DTLS_ECDH_ECDSA_KEYSCHEME ) && defined( USE_DTLS_AES_128_GCM_CIPHER ) && defined( USE_DTLS_SHA256_MAC ) */

#if defined( USE_DTLS_ECDH_ECDSA_KEYSCHEME ) && defined( USE_DTLS_AES_256_GCM_CIPHER ) && defined( USE_DTLS_SHA384_MAC )
extern const cipher_suite_t DTLS_ECDH_ECDSA_WITH_AES_256_GCM_SHA384;
#endif /* if defined( USE_DTLS_ECDH_ECDSA_KEYSCHEME ) && defined( USE_DTLS_AES_256_GCM_CIPHER ) && defined( USE_DTLS_SHA384_MAC ) */

#if defined( USE_DTLS_ECDHE_RSA_KEYSCHEME ) && defined( USE_DTLS_AES_128_GCM_CIPHER ) && defined( USE_DTLS_SHA256_MAC )
extern const cipher_suite_t DTLS_ECDHE_RSA_WITH_AES_128_GCM_SHA256;
#endif /* if defined( USE_DTLS_ECDHE_RSA_KEYSCHEME ) && defined( USE_DTLS_AES_128_GCM_CIPHER ) && defined( USE_DTLS_SHA256_MAC ) */

#if defined( USE_DTLS_ECDHE_RSA_KEYSCHEME ) && defined( USE_DTLS_AES_256_GCM_CIPHER ) && defined( USE_DTLS_SHA384_MAC )
extern const cipher_suite_t DTLS_ECDHE_RSA_WITH_AES_256_GCM_SHA384;
#endif /* if defined( USE_DTLS_ECDHE_RSA_KEYSCHEME ) && defined( USE_DTLS_AES_256_GCM_CIPHER ) && defined( USE_DTLS_SHA384_MAC ) */

#if defined( USE_DTLS_ECDH_RSA_KEYSCHEME ) && defined( USE_DTLS_AES_128_GCM_CIPHER ) && defined( USE_DTLS_SHA256_MAC )
extern const cipher_suite_t DTLS_ECDH_RSA_WITH_AES_128_GCM_SHA256;
#endif /* if defined( USE_DTLS_ECDH_RSA_KEYSCHEME ) && defined( USE_DTLS_AES_128_GCM_CIPHER ) && defined( USE_DTLS_SHA256_MAC ) */

#if defined( USE_DTLS_ECDH_RSA_KEYSCHEME ) && defined( USE_DTLS_AES_256_GCM_CIPHER ) && defined( USE_DTLS_SHA384_MAC )
extern const cipher_suite_t DTLS_ECDH_RSA_WITH_AES_256_GCM_SHA384;
#endif /* if defined( USE_DTLS_ECDH_RSA_KEYSCHEME ) && defined( USE_DTLS_AES_256_GCM_CIPHER ) && defined( USE_DTLS_SHA384_MAC ) */

#if defined( USE_DTLS_ECDHE_PSK_KEYSCHEME ) && defined( USE_DTLS_RC4_128_CIPHER ) && defined( USE_DTLS_SHA_MAC )
extern const cipher_suite_t DTLS_ECDHE_PSK_WITH_RC4_128_SHA;
#endif /* if defined( USE_DTLS_ECDHE_PSK_KEYSCHEME ) && defined( USE_DTLS_RC4_128_CIPHER ) && defined( USE_DTLS_SHA_MAC ) */

#if defined( USE_DTLS_ECDHE_PSK_KEYSCHEME ) && defined( USE_DTLS_3DES_EDE_CBC_CIPHER ) && defined( USE_DTLS_SHA_MAC )
extern const cipher_suite_t DTLS_ECDHE_PSK_WITH_3DES_EDE_CBC_SHA;
#endif /* if defined( USE_DTLS_ECDHE_PSK_KEYSCHEME ) && defined( USE_DTLS_3DES_EDE_CBC_CIPHER ) && defined( USE_DTLS_SHA_MAC ) */

#if defined( USE_DTLS_ECDHE_PSK_KEYSCHEME ) && defined( USE_DTLS_AES_128_CBC_CIPHER ) && defined( USE_DTLS_SHA_MAC )
extern const cipher_suite_t DTLS_ECDHE_PSK_WITH_AES_128_CBC_SHA;
#endif /* if defined( USE_DTLS_ECDHE_PSK_KEYSCHEME ) && defined( USE_DTLS_AES_128_CBC_CIPHER ) && defined( USE_DTLS_SHA_MAC ) */

#if defined( USE_DTLS_ECDHE_PSK_KEYSCHEME ) && defined( USE_DTLS_AES_256_CBC_CIPHER ) && defined( USE_DTLS_SHA_MAC )
extern const cipher_suite_t DTLS_ECDHE_PSK_WITH_AES_256_CBC_SHA;
#endif /* if defined( USE_DTLS_ECDHE_PSK_KEYSCHEME ) && defined( USE_DTLS_AES_256_CBC_CIPHER ) && defined( USE_DTLS_SHA_MAC ) */

#if defined( USE_DTLS_ECDHE_PSK_KEYSCHEME ) && defined( USE_DTLS_AES_128_CBC_CIPHER ) && defined( USE_DTLS_SHA256_MAC )
extern const cipher_suite_t DTLS_ECDHE_PSK_WITH_AES_128_CBC_SHA256;
#endif /* if defined( USE_DTLS_ECDHE_PSK_KEYSCHEME ) && defined( USE_DTLS_AES_128_CBC_CIPHER ) && defined( USE_DTLS_SHA256_MAC ) */

#if defined( USE_DTLS_ECDHE_PSK_KEYSCHEME ) && defined( USE_DTLS_AES_256_CBC_CIPHER ) && defined( USE_DTLS_SHA384_MAC )
extern const cipher_suite_t DTLS_ECDHE_PSK_WITH_AES_256_CBC_SHA384;
#endif /* if defined( USE_DTLS_ECDHE_PSK_KEYSCHEME ) && defined( USE_DTLS_AES_256_CBC_CIPHER ) && defined( USE_DTLS_SHA384_MAC ) */

#if defined( USE_DTLS_ECDHE_PSK_KEYSCHEME ) && defined( USE_DTLS_NULL_CIPHER ) && defined( USE_DTLS_SHA_MAC )
extern const cipher_suite_t DTLS_ECDHE_PSK_WITH_NULL_SHA;
#endif /* if defined( USE_DTLS_ECDHE_PSK_KEYSCHEME ) && defined( USE_DTLS_NULL_CIPHER ) && defined( USE_DTLS_SHA_MAC ) */

#if defined( USE_DTLS_ECDHE_PSK_KEYSCHEME ) && defined( USE_DTLS_NULL_CIPHER ) && defined( USE_DTLS_SHA256_MAC )
extern const cipher_suite_t DTLS_ECDHE_PSK_WITH_NULL_SHA256;
#endif /* if defined( USE_DTLS_ECDHE_PSK_KEYSCHEME ) && defined( USE_DTLS_NULL_CIPHER ) && defined( USE_DTLS_SHA256_MAC ) */

#if defined( USE_DTLS_ECDHE_PSK_KEYSCHEME ) && defined( USE_DTLS_NULL_CIPHER ) && defined( USE_DTLS_SHA384_MAC )
extern const cipher_suite_t DTLS_ECDHE_PSK_WITH_NULL_SHA384;
#endif /* if defined( USE_DTLS_ECDHE_PSK_KEYSCHEME ) && defined( USE_DTLS_NULL_CIPHER ) && defined( USE_DTLS_SHA384_MAC ) */

#if defined( USE_DTLS_RSA_KEYSCHEME ) && defined( USE_DTLS_ARIA_128_CBC_CIPHER ) && defined( USE_DTLS_SHA256_MAC )
extern const cipher_suite_t DTLS_RSA_WITH_ARIA_128_CBC_SHA256;
#endif /* if defined( USE_DTLS_RSA_KEYSCHEME ) && defined( USE_DTLS_ARIA_128_CBC_CIPHER ) && defined( USE_DTLS_SHA256_MAC ) */

#if defined( USE_DTLS_RSA_KEYSCHEME ) && defined( USE_DTLS_ARIA_256_CBC_CIPHER ) && defined( USE_DTLS_SHA384_MAC )
extern const cipher_suite_t DTLS_RSA_WITH_ARIA_256_CBC_SHA384;
#endif /* if defined( USE_DTLS_RSA_KEYSCHEME ) && defined( USE_DTLS_ARIA_256_CBC_CIPHER ) && defined( USE_DTLS_SHA384_MAC ) */

#if defined( USE_DTLS_DH_DSS_KEYSCHEME ) && defined( USE_DTLS_ARIA_128_CBC_CIPHER ) && defined( USE_DTLS_SHA256_MAC )
extern const cipher_suite_t DTLS_DH_DSS_WITH_ARIA_128_CBC_SHA256;
#endif /* if defined( USE_DTLS_DH_DSS_KEYSCHEME ) && defined( USE_DTLS_ARIA_128_CBC_CIPHER ) && defined( USE_DTLS_SHA256_MAC ) */

#if defined( USE_DTLS_DH_DSS_KEYSCHEME ) && defined( USE_DTLS_ARIA_256_CBC_CIPHER ) && defined( USE_DTLS_SHA384_MAC )
extern const cipher_suite_t DTLS_DH_DSS_WITH_ARIA_256_CBC_SHA384;
#endif /* if defined( USE_DTLS_DH_DSS_KEYSCHEME ) && defined( USE_DTLS_ARIA_256_CBC_CIPHER ) && defined( USE_DTLS_SHA384_MAC ) */

#if defined( USE_DTLS_DH_RSA_KEYSCHEME ) && defined( USE_DTLS_ARIA_128_CBC_CIPHER ) && defined( USE_DTLS_SHA256_MAC )
extern const cipher_suite_t DTLS_DH_RSA_WITH_ARIA_128_CBC_SHA256;
#endif /* if defined( USE_DTLS_DH_RSA_KEYSCHEME ) && defined( USE_DTLS_ARIA_128_CBC_CIPHER ) && defined( USE_DTLS_SHA256_MAC ) */

#if defined( USE_DTLS_DH_RSA_KEYSCHEME ) && defined( USE_DTLS_ARIA_256_CBC_CIPHER ) && defined( USE_DTLS_SHA384_MAC )
extern const cipher_suite_t DTLS_DH_RSA_WITH_ARIA_256_CBC_SHA384;
#endif /* if defined( USE_DTLS_DH_RSA_KEYSCHEME ) && defined( USE_DTLS_ARIA_256_CBC_CIPHER ) && defined( USE_DTLS_SHA384_MAC ) */

#if defined( USE_DTLS_DHE_DSS_KEYSCHEME ) && defined( USE_DTLS_ARIA_128_CBC_CIPHER ) && defined( USE_DTLS_SHA256_MAC )
extern const cipher_suite_t DTLS_DHE_DSS_WITH_ARIA_128_CBC_SHA256;
#endif /* if defined( USE_DTLS_DHE_DSS_KEYSCHEME ) && defined( USE_DTLS_ARIA_128_CBC_CIPHER ) && defined( USE_DTLS_SHA256_MAC ) */

#if defined( USE_DTLS_DHE_DSS_KEYSCHEME ) && defined( USE_DTLS_ARIA_256_CBC_CIPHER ) && defined( USE_DTLS_SHA384_MAC )
extern const cipher_suite_t DTLS_DHE_DSS_WITH_ARIA_256_CBC_SHA384;
#endif /* if defined( USE_DTLS_DHE_DSS_KEYSCHEME ) && defined( USE_DTLS_ARIA_256_CBC_CIPHER ) && defined( USE_DTLS_SHA384_MAC ) */

#if defined( USE_DTLS_DHE_RSA_KEYSCHEME ) && defined( USE_DTLS_ARIA_128_CBC_CIPHER ) && defined( USE_DTLS_SHA256_MAC )
extern const cipher_suite_t DTLS_DHE_RSA_WITH_ARIA_128_CBC_SHA256;
#endif /* if defined( USE_DTLS_DHE_RSA_KEYSCHEME ) && defined( USE_DTLS_ARIA_128_CBC_CIPHER ) && defined( USE_DTLS_SHA256_MAC ) */

#if defined( USE_DTLS_DHE_RSA_KEYSCHEME ) && defined( USE_DTLS_ARIA_256_CBC_CIPHER ) && defined( USE_DTLS_SHA384_MAC )
extern const cipher_suite_t DTLS_DHE_RSA_WITH_ARIA_256_CBC_SHA384;
#endif /* if defined( USE_DTLS_DHE_RSA_KEYSCHEME ) && defined( USE_DTLS_ARIA_256_CBC_CIPHER ) && defined( USE_DTLS_SHA384_MAC ) */

#if defined( USE_DTLS_DH_anon_KEYSCHEME ) && defined( USE_DTLS_ARIA_128_CBC_CIPHER ) && defined( USE_DTLS_SHA256_MAC )
extern const cipher_suite_t DTLS_DH_anon_WITH_ARIA_128_CBC_SHA256;
#endif /* if defined( USE_DTLS_DH_anon_KEYSCHEME ) && defined( USE_DTLS_ARIA_128_CBC_CIPHER ) && defined( USE_DTLS_SHA256_MAC ) */

#if defined( USE_DTLS_DH_anon_KEYSCHEME ) && defined( USE_DTLS_ARIA_256_CBC_CIPHER ) && defined( USE_DTLS_SHA384_MAC )
extern const cipher_suite_t DTLS_DH_anon_WITH_ARIA_256_CBC_SHA384;
#endif /* if defined( USE_DTLS_DH_anon_KEYSCHEME ) && defined( USE_DTLS_ARIA_256_CBC_CIPHER ) && defined( USE_DTLS_SHA384_MAC ) */

#if defined( USE_DTLS_ECDHE_ECDSA_KEYSCHEME ) && defined( USE_DTLS_ARIA_128_CBC_CIPHER ) && defined( USE_DTLS_SHA256_MAC )
extern const cipher_suite_t DTLS_ECDHE_ECDSA_WITH_ARIA_128_CBC_SHA256;
#endif /* if defined( USE_DTLS_ECDHE_ECDSA_KEYSCHEME ) && defined( USE_DTLS_ARIA_128_CBC_CIPHER ) && defined( USE_DTLS_SHA256_MAC ) */

#if defined( USE_DTLS_ECDHE_ECDSA_KEYSCHEME ) && defined( USE_DTLS_ARIA_256_CBC_CIPHER ) && defined( USE_DTLS_SHA384_MAC )
extern const cipher_suite_t DTLS_ECDHE_ECDSA_WITH_ARIA_256_CBC_SHA384;
#endif /* if defined( USE_DTLS_ECDHE_ECDSA_KEYSCHEME ) && defined( USE_DTLS_ARIA_256_CBC_CIPHER ) && defined( USE_DTLS_SHA384_MAC ) */

#if defined( USE_DTLS_ECDH_ECDSA_KEYSCHEME ) && defined( USE_DTLS_ARIA_128_CBC_CIPHER ) && defined( USE_DTLS_SHA256_MAC )
extern const cipher_suite_t DTLS_ECDH_ECDSA_WITH_ARIA_128_CBC_SHA256;
#endif /* if defined( USE_DTLS_ECDH_ECDSA_KEYSCHEME ) && defined( USE_DTLS_ARIA_128_CBC_CIPHER ) && defined( USE_DTLS_SHA256_MAC ) */

#if defined( USE_DTLS_ECDH_ECDSA_KEYSCHEME ) && defined( USE_DTLS_ARIA_256_CBC_CIPHER ) && defined( USE_DTLS_SHA384_MAC )
extern const cipher_suite_t DTLS_ECDH_ECDSA_WITH_ARIA_256_CBC_SHA384;
#endif /* if defined( USE_DTLS_ECDH_ECDSA_KEYSCHEME ) && defined( USE_DTLS_ARIA_256_CBC_CIPHER ) && defined( USE_DTLS_SHA384_MAC ) */

#if defined( USE_DTLS_ECDHE_RSA_KEYSCHEME ) && defined( USE_DTLS_ARIA_128_CBC_CIPHER ) && defined( USE_DTLS_SHA256_MAC )
extern const cipher_suite_t DTLS_ECDHE_RSA_WITH_ARIA_128_CBC_SHA256;
#endif /* if defined( USE_DTLS_ECDHE_RSA_KEYSCHEME ) && defined( USE_DTLS_ARIA_128_CBC_CIPHER ) && defined( USE_DTLS_SHA256_MAC ) */

#if defined( USE_DTLS_ECDHE_RSA_KEYSCHEME ) && defined( USE_DTLS_ARIA_256_CBC_CIPHER ) && defined( USE_DTLS_SHA384_MAC )
extern const cipher_suite_t DTLS_ECDHE_RSA_WITH_ARIA_256_CBC_SHA384;
#endif /* if defined( USE_DTLS_ECDHE_RSA_KEYSCHEME ) && defined( USE_DTLS_ARIA_256_CBC_CIPHER ) && defined( USE_DTLS_SHA384_MAC ) */

#if defined( USE_DTLS_ECDH_RSA_KEYSCHEME ) && defined( USE_DTLS_ARIA_128_CBC_CIPHER ) && defined( USE_DTLS_SHA256_MAC )
extern const cipher_suite_t DTLS_ECDH_RSA_WITH_ARIA_128_CBC_SHA256;
#endif /* if defined( USE_DTLS_ECDH_RSA_KEYSCHEME ) && defined( USE_DTLS_ARIA_128_CBC_CIPHER ) && defined( USE_DTLS_SHA256_MAC ) */

#if defined( USE_DTLS_ECDH_RSA_KEYSCHEME ) && defined( USE_DTLS_ARIA_256_CBC_CIPHER ) && defined( USE_DTLS_SHA384_MAC )
extern const cipher_suite_t DTLS_ECDH_RSA_WITH_ARIA_256_CBC_SHA384;
#endif /* if defined( USE_DTLS_ECDH_RSA_KEYSCHEME ) && defined( USE_DTLS_ARIA_256_CBC_CIPHER ) && defined( USE_DTLS_SHA384_MAC ) */

#if defined( USE_DTLS_RSA_KEYSCHEME ) && defined( USE_DTLS_ARIA_128_GCM_CIPHER ) && defined( USE_DTLS_SHA256_MAC )
extern const cipher_suite_t DTLS_RSA_WITH_ARIA_128_GCM_SHA256;
#endif /* if defined( USE_DTLS_RSA_KEYSCHEME ) && defined( USE_DTLS_ARIA_128_GCM_CIPHER ) && defined( USE_DTLS_SHA256_MAC ) */

#if defined( USE_DTLS_RSA_KEYSCHEME ) && defined( USE_DTLS_ARIA_256_GCM_CIPHER ) && defined( USE_DTLS_SHA384_MAC )
extern const cipher_suite_t DTLS_RSA_WITH_ARIA_256_GCM_SHA384;
#endif /* if defined( USE_DTLS_RSA_KEYSCHEME ) && defined( USE_DTLS_ARIA_256_GCM_CIPHER ) && defined( USE_DTLS_SHA384_MAC ) */

#if defined( USE_DTLS_DHE_RSA_KEYSCHEME ) && defined( USE_DTLS_ARIA_128_GCM_CIPHER ) && defined( USE_DTLS_SHA256_MAC )
extern const cipher_suite_t DTLS_DHE_RSA_WITH_ARIA_128_GCM_SHA256;
#endif /* if defined( USE_DTLS_DHE_RSA_KEYSCHEME ) && defined( USE_DTLS_ARIA_128_GCM_CIPHER ) && defined( USE_DTLS_SHA256_MAC ) */

#if defined( USE_DTLS_DHE_RSA_KEYSCHEME ) && defined( USE_DTLS_ARIA_256_GCM_CIPHER ) && defined( USE_DTLS_SHA384_MAC )
extern const cipher_suite_t DTLS_DHE_RSA_WITH_ARIA_256_GCM_SHA384;
#endif /* if defined( USE_DTLS_DHE_RSA_KEYSCHEME ) && defined( USE_DTLS_ARIA_256_GCM_CIPHER ) && defined( USE_DTLS_SHA384_MAC ) */

#if defined( USE_DTLS_DH_RSA_KEYSCHEME ) && defined( USE_DTLS_ARIA_128_GCM_CIPHER ) && defined( USE_DTLS_SHA256_MAC )
extern const cipher_suite_t DTLS_DH_RSA_WITH_ARIA_128_GCM_SHA256;
#endif /* if defined( USE_DTLS_DH_RSA_KEYSCHEME ) && defined( USE_DTLS_ARIA_128_GCM_CIPHER ) && defined( USE_DTLS_SHA256_MAC ) */

#if defined( USE_DTLS_DH_RSA_KEYSCHEME ) && defined( USE_DTLS_ARIA_256_GCM_CIPHER ) && defined( USE_DTLS_SHA384_MAC )
extern const cipher_suite_t DTLS_DH_RSA_WITH_ARIA_256_GCM_SHA384;
#endif /* if defined( USE_DTLS_DH_RSA_KEYSCHEME ) && defined( USE_DTLS_ARIA_256_GCM_CIPHER ) && defined( USE_DTLS_SHA384_MAC ) */

#if defined( USE_DTLS_DHE_DSS_KEYSCHEME ) && defined( USE_DTLS_ARIA_128_GCM_CIPHER ) && defined( USE_DTLS_SHA256_MAC )
extern const cipher_suite_t DTLS_DHE_DSS_WITH_ARIA_128_GCM_SHA256;
#endif /* if defined( USE_DTLS_DHE_DSS_KEYSCHEME ) && defined( USE_DTLS_ARIA_128_GCM_CIPHER ) && defined( USE_DTLS_SHA256_MAC ) */

#if defined( USE_DTLS_DHE_DSS_KEYSCHEME ) && defined( USE_DTLS_ARIA_256_GCM_CIPHER ) && defined( USE_DTLS_SHA384_MAC )
extern const cipher_suite_t DTLS_DHE_DSS_WITH_ARIA_256_GCM_SHA384;
#endif /* if defined( USE_DTLS_DHE_DSS_KEYSCHEME ) && defined( USE_DTLS_ARIA_256_GCM_CIPHER ) && defined( USE_DTLS_SHA384_MAC ) */

#if defined( USE_DTLS_DH_DSS_KEYSCHEME ) && defined( USE_DTLS_ARIA_128_GCM_CIPHER ) && defined( USE_DTLS_SHA256_MAC )
extern const cipher_suite_t DTLS_DH_DSS_WITH_ARIA_128_GCM_SHA256;
#endif /* if defined( USE_DTLS_DH_DSS_KEYSCHEME ) && defined( USE_DTLS_ARIA_128_GCM_CIPHER ) && defined( USE_DTLS_SHA256_MAC ) */

#if defined( USE_DTLS_DH_DSS_KEYSCHEME ) && defined( USE_DTLS_ARIA_256_GCM_CIPHER ) && defined( USE_DTLS_SHA384_MAC )
extern const cipher_suite_t DTLS_DH_DSS_WITH_ARIA_256_GCM_SHA384;
#endif /* if defined( USE_DTLS_DH_DSS_KEYSCHEME ) && defined( USE_DTLS_ARIA_256_GCM_CIPHER ) && defined( USE_DTLS_SHA384_MAC ) */

#if defined( USE_DTLS_DH_anon_KEYSCHEME ) && defined( USE_DTLS_ARIA_128_GCM_CIPHER ) && defined( USE_DTLS_SHA256_MAC )
extern const cipher_suite_t DTLS_DH_anon_WITH_ARIA_128_GCM_SHA256;
#endif /* if defined( USE_DTLS_DH_anon_KEYSCHEME ) && defined( USE_DTLS_ARIA_128_GCM_CIPHER ) && defined( USE_DTLS_SHA256_MAC ) */

#if defined( USE_DTLS_DH_anon_KEYSCHEME ) && defined( USE_DTLS_ARIA_256_GCM_CIPHER ) && defined( USE_DTLS_SHA384_MAC )
extern const cipher_suite_t DTLS_DH_anon_WITH_ARIA_256_GCM_SHA384;
#endif /* if defined( USE_DTLS_DH_anon_KEYSCHEME ) && defined( USE_DTLS_ARIA_256_GCM_CIPHER ) && defined( USE_DTLS_SHA384_MAC ) */

#if defined( USE_DTLS_ECDHE_ECDSA_KEYSCHEME ) && defined( USE_DTLS_ARIA_128_GCM_CIPHER ) && defined( USE_DTLS_SHA256_MAC )
extern const cipher_suite_t DTLS_ECDHE_ECDSA_WITH_ARIA_128_GCM_SHA256;
#endif /* if defined( USE_DTLS_ECDHE_ECDSA_KEYSCHEME ) && defined( USE_DTLS_ARIA_128_GCM_CIPHER ) && defined( USE_DTLS_SHA256_MAC ) */

#if defined( USE_DTLS_ECDHE_ECDSA_KEYSCHEME ) && defined( USE_DTLS_ARIA_256_GCM_CIPHER ) && defined( USE_DTLS_SHA384_MAC )
extern const cipher_suite_t DTLS_ECDHE_ECDSA_WITH_ARIA_256_GCM_SHA384;
#endif /* if defined( USE_DTLS_ECDHE_ECDSA_KEYSCHEME ) && defined( USE_DTLS_ARIA_256_GCM_CIPHER ) && defined( USE_DTLS_SHA384_MAC ) */

#if defined( USE_DTLS_ECDH_ECDSA_KEYSCHEME ) && defined( USE_DTLS_ARIA_128_GCM_CIPHER ) && defined( USE_DTLS_SHA256_MAC )
extern const cipher_suite_t DTLS_ECDH_ECDSA_WITH_ARIA_128_GCM_SHA256;
#endif /* if defined( USE_DTLS_ECDH_ECDSA_KEYSCHEME ) && defined( USE_DTLS_ARIA_128_GCM_CIPHER ) && defined( USE_DTLS_SHA256_MAC ) */

#if defined( USE_DTLS_ECDH_ECDSA_KEYSCHEME ) && defined( USE_DTLS_ARIA_256_GCM_CIPHER ) && defined( USE_DTLS_SHA384_MAC )
extern const cipher_suite_t DTLS_ECDH_ECDSA_WITH_ARIA_256_GCM_SHA384;
#endif /* if defined( USE_DTLS_ECDH_ECDSA_KEYSCHEME ) && defined( USE_DTLS_ARIA_256_GCM_CIPHER ) && defined( USE_DTLS_SHA384_MAC ) */

#if defined( USE_DTLS_ECDHE_RSA_KEYSCHEME ) && defined( USE_DTLS_ARIA_128_GCM_CIPHER ) && defined( USE_DTLS_SHA256_MAC )
extern const cipher_suite_t DTLS_ECDHE_RSA_WITH_ARIA_128_GCM_SHA256;
#endif /* if defined( USE_DTLS_ECDHE_RSA_KEYSCHEME ) && defined( USE_DTLS_ARIA_128_GCM_CIPHER ) && defined( USE_DTLS_SHA256_MAC ) */

#if defined( USE_DTLS_ECDHE_RSA_KEYSCHEME ) && defined( USE_DTLS_ARIA_256_GCM_CIPHER ) && defined( USE_DTLS_SHA384_MAC )
extern const cipher_suite_t DTLS_ECDHE_RSA_WITH_ARIA_256_GCM_SHA384;
#endif /* if defined( USE_DTLS_ECDHE_RSA_KEYSCHEME ) && defined( USE_DTLS_ARIA_256_GCM_CIPHER ) && defined( USE_DTLS_SHA384_MAC ) */

#if defined( USE_DTLS_ECDH_RSA_KEYSCHEME ) && defined( USE_DTLS_ARIA_128_GCM_CIPHER ) && defined( USE_DTLS_SHA256_MAC )
extern const cipher_suite_t DTLS_ECDH_RSA_WITH_ARIA_128_GCM_SHA256;
#endif /* if defined( USE_DTLS_ECDH_RSA_KEYSCHEME ) && defined( USE_DTLS_ARIA_128_GCM_CIPHER ) && defined( USE_DTLS_SHA256_MAC ) */

#if defined( USE_DTLS_ECDH_RSA_KEYSCHEME ) && defined( USE_DTLS_ARIA_256_GCM_CIPHER ) && defined( USE_DTLS_SHA384_MAC )
extern const cipher_suite_t DTLS_ECDH_RSA_WITH_ARIA_256_GCM_SHA384;
#endif /* if defined( USE_DTLS_ECDH_RSA_KEYSCHEME ) && defined( USE_DTLS_ARIA_256_GCM_CIPHER ) && defined( USE_DTLS_SHA384_MAC ) */

#if defined( USE_DTLS_PSK_KEYSCHEME ) && defined( USE_DTLS_ARIA_128_CBC_CIPHER ) && defined( USE_DTLS_SHA256_MAC )
extern const cipher_suite_t DTLS_PSK_WITH_ARIA_128_CBC_SHA256;
#endif /* if defined( USE_DTLS_PSK_KEYSCHEME ) && defined( USE_DTLS_ARIA_128_CBC_CIPHER ) && defined( USE_DTLS_SHA256_MAC ) */

#if defined( USE_DTLS_PSK_KEYSCHEME ) && defined( USE_DTLS_ARIA_256_CBC_CIPHER ) && defined( USE_DTLS_SHA384_MAC )
extern const cipher_suite_t DTLS_PSK_WITH_ARIA_256_CBC_SHA384;
#endif /* if defined( USE_DTLS_PSK_KEYSCHEME ) && defined( USE_DTLS_ARIA_256_CBC_CIPHER ) && defined( USE_DTLS_SHA384_MAC ) */

#if defined( USE_DTLS_DHE_PSK_KEYSCHEME ) && defined( USE_DTLS_ARIA_128_CBC_CIPHER ) && defined( USE_DTLS_SHA256_MAC )
extern const cipher_suite_t DTLS_DHE_PSK_WITH_ARIA_128_CBC_SHA256;
#endif /* if defined( USE_DTLS_DHE_PSK_KEYSCHEME ) && defined( USE_DTLS_ARIA_128_CBC_CIPHER ) && defined( USE_DTLS_SHA256_MAC ) */

#if defined( USE_DTLS_DHE_PSK_KEYSCHEME ) && defined( USE_DTLS_ARIA_256_CBC_CIPHER ) && defined( USE_DTLS_SHA384_MAC )
extern const cipher_suite_t DTLS_DHE_PSK_WITH_ARIA_256_CBC_SHA384;
#endif /* if defined( USE_DTLS_DHE_PSK_KEYSCHEME ) && defined( USE_DTLS_ARIA_256_CBC_CIPHER ) && defined( USE_DTLS_SHA384_MAC ) */

#if defined( USE_DTLS_RSA_PSK_KEYSCHEME ) && defined( USE_DTLS_ARIA_128_CBC_CIPHER ) && defined( USE_DTLS_SHA256_MAC )
extern const cipher_suite_t DTLS_RSA_PSK_WITH_ARIA_128_CBC_SHA256;
#endif /* if defined( USE_DTLS_RSA_PSK_KEYSCHEME ) && defined( USE_DTLS_ARIA_128_CBC_CIPHER ) && defined( USE_DTLS_SHA256_MAC ) */

#if defined( USE_DTLS_RSA_PSK_KEYSCHEME ) && defined( USE_DTLS_ARIA_256_CBC_CIPHER ) && defined( USE_DTLS_SHA384_MAC )
extern const cipher_suite_t DTLS_RSA_PSK_WITH_ARIA_256_CBC_SHA384;
#endif /* if defined( USE_DTLS_RSA_PSK_KEYSCHEME ) && defined( USE_DTLS_ARIA_256_CBC_CIPHER ) && defined( USE_DTLS_SHA384_MAC ) */

#if defined( USE_DTLS_PSK_KEYSCHEME ) && defined( USE_DTLS_ARIA_128_GCM_CIPHER ) && defined( USE_DTLS_SHA256_MAC )
extern const cipher_suite_t DTLS_PSK_WITH_ARIA_128_GCM_SHA256;
#endif /* if defined( USE_DTLS_PSK_KEYSCHEME ) && defined( USE_DTLS_ARIA_128_GCM_CIPHER ) && defined( USE_DTLS_SHA256_MAC ) */

#if defined( USE_DTLS_PSK_KEYSCHEME ) && defined( USE_DTLS_ARIA_256_GCM_CIPHER ) && defined( USE_DTLS_SHA384_MAC )
extern const cipher_suite_t DTLS_PSK_WITH_ARIA_256_GCM_SHA384;
#endif /* if defined( USE_DTLS_PSK_KEYSCHEME ) && defined( USE_DTLS_ARIA_256_GCM_CIPHER ) && defined( USE_DTLS_SHA384_MAC ) */

#if defined( USE_DTLS_DHE_PSK_KEYSCHEME ) && defined( USE_DTLS_ARIA_128_GCM_CIPHER ) && defined( USE_DTLS_SHA256_MAC )
extern const cipher_suite_t DTLS_DHE_PSK_WITH_ARIA_128_GCM_SHA256;
#endif /* if defined( USE_DTLS_DHE_PSK_KEYSCHEME ) && defined( USE_DTLS_ARIA_128_GCM_CIPHER ) && defined( USE_DTLS_SHA256_MAC ) */

#if defined( USE_DTLS_DHE_PSK_KEYSCHEME ) && defined( USE_DTLS_ARIA_256_GCM_CIPHER ) && defined( USE_DTLS_SHA384_MAC )
extern const cipher_suite_t DTLS_DHE_PSK_WITH_ARIA_256_GCM_SHA384;
#endif /* if defined( USE_DTLS_DHE_PSK_KEYSCHEME ) && defined( USE_DTLS_ARIA_256_GCM_CIPHER ) && defined( USE_DTLS_SHA384_MAC ) */

#if defined( USE_DTLS_RSA_PSK_KEYSCHEME ) && defined( USE_DTLS_ARIA_128_GCM_CIPHER ) && defined( USE_DTLS_SHA256_MAC )
extern const cipher_suite_t DTLS_RSA_PSK_WITH_ARIA_128_GCM_SHA256;
#endif /* if defined( USE_DTLS_RSA_PSK_KEYSCHEME ) && defined( USE_DTLS_ARIA_128_GCM_CIPHER ) && defined( USE_DTLS_SHA256_MAC ) */

#if defined( USE_DTLS_RSA_PSK_KEYSCHEME ) && defined( USE_DTLS_ARIA_256_GCM_CIPHER ) && defined( USE_DTLS_SHA384_MAC )
extern const cipher_suite_t DTLS_RSA_PSK_WITH_ARIA_256_GCM_SHA384;
#endif /* if defined( USE_DTLS_RSA_PSK_KEYSCHEME ) && defined( USE_DTLS_ARIA_256_GCM_CIPHER ) && defined( USE_DTLS_SHA384_MAC ) */

#if defined( USE_DTLS_ECDHE_PSK_KEYSCHEME ) && defined( USE_DTLS_ARIA_128_CBC_CIPHER ) && defined( USE_DTLS_SHA256_MAC )
extern const cipher_suite_t DTLS_ECDHE_PSK_WITH_ARIA_128_CBC_SHA256;
#endif /* if defined( USE_DTLS_ECDHE_PSK_KEYSCHEME ) && defined( USE_DTLS_ARIA_128_CBC_CIPHER ) && defined( USE_DTLS_SHA256_MAC ) */

#if defined( USE_DTLS_ECDHE_PSK_KEYSCHEME ) && defined( USE_DTLS_ARIA_256_CBC_CIPHER ) && defined( USE_DTLS_SHA384_MAC )
extern const cipher_suite_t DTLS_ECDHE_PSK_WITH_ARIA_256_CBC_SHA384;
#endif /* if defined( USE_DTLS_ECDHE_PSK_KEYSCHEME ) && defined( USE_DTLS_ARIA_256_CBC_CIPHER ) && defined( USE_DTLS_SHA384_MAC ) */

#if defined( USE_DTLS_ECDHE_ECDSA_KEYSCHEME ) && defined( USE_DTLS_CAMELLIA_128_CBC_CIPHER ) && defined( USE_DTLS_SHA256_MAC )
extern const cipher_suite_t DTLS_ECDHE_ECDSA_WITH_CAMELLIA_128_CBC_SHA256;
#endif /* if defined( USE_DTLS_ECDHE_ECDSA_KEYSCHEME ) && defined( USE_DTLS_CAMELLIA_128_CBC_CIPHER ) && defined( USE_DTLS_SHA256_MAC ) */

#if defined( USE_DTLS_ECDHE_ECDSA_KEYSCHEME ) && defined( USE_DTLS_CAMELLIA_256_CBC_CIPHER ) && defined( USE_DTLS_SHA384_MAC )
extern const cipher_suite_t DTLS_ECDHE_ECDSA_WITH_CAMELLIA_256_CBC_SHA384;
#endif /* if defined( USE_DTLS_ECDHE_ECDSA_KEYSCHEME ) && defined( USE_DTLS_CAMELLIA_256_CBC_CIPHER ) && defined( USE_DTLS_SHA384_MAC ) */

#if defined( USE_DTLS_ECDH_ECDSA_KEYSCHEME ) && defined( USE_DTLS_CAMELLIA_128_CBC_CIPHER ) && defined( USE_DTLS_SHA256_MAC )
extern const cipher_suite_t DTLS_ECDH_ECDSA_WITH_CAMELLIA_128_CBC_SHA256;
#endif /* if defined( USE_DTLS_ECDH_ECDSA_KEYSCHEME ) && defined( USE_DTLS_CAMELLIA_128_CBC_CIPHER ) && defined( USE_DTLS_SHA256_MAC ) */

#if defined( USE_DTLS_ECDH_ECDSA_KEYSCHEME ) && defined( USE_DTLS_CAMELLIA_256_CBC_CIPHER ) && defined( USE_DTLS_SHA384_MAC )
extern const cipher_suite_t DTLS_ECDH_ECDSA_WITH_CAMELLIA_256_CBC_SHA384;
#endif /* if defined( USE_DTLS_ECDH_ECDSA_KEYSCHEME ) && defined( USE_DTLS_CAMELLIA_256_CBC_CIPHER ) && defined( USE_DTLS_SHA384_MAC ) */

#if defined( USE_DTLS_ECDHE_RSA_KEYSCHEME ) && defined( USE_DTLS_CAMELLIA_128_CBC_CIPHER ) && defined( USE_DTLS_SHA256_MAC )
extern const cipher_suite_t DTLS_ECDHE_RSA_WITH_CAMELLIA_128_CBC_SHA256;
#endif /* if defined( USE_DTLS_ECDHE_RSA_KEYSCHEME ) && defined( USE_DTLS_CAMELLIA_128_CBC_CIPHER ) && defined( USE_DTLS_SHA256_MAC ) */

#if defined( USE_DTLS_ECDHE_RSA_KEYSCHEME ) && defined( USE_DTLS_CAMELLIA_256_CBC_CIPHER ) && defined( USE_DTLS_SHA384_MAC )
extern const cipher_suite_t DTLS_ECDHE_RSA_WITH_CAMELLIA_256_CBC_SHA384;
#endif /* if defined( USE_DTLS_ECDHE_RSA_KEYSCHEME ) && defined( USE_DTLS_CAMELLIA_256_CBC_CIPHER ) && defined( USE_DTLS_SHA384_MAC ) */

#if defined( USE_DTLS_ECDH_RSA_KEYSCHEME ) && defined( USE_DTLS_CAMELLIA_128_CBC_CIPHER ) && defined( USE_DTLS_SHA256_MAC )
extern const cipher_suite_t DTLS_ECDH_RSA_WITH_CAMELLIA_128_CBC_SHA256;
#endif /* if defined( USE_DTLS_ECDH_RSA_KEYSCHEME ) && defined( USE_DTLS_CAMELLIA_128_CBC_CIPHER ) && defined( USE_DTLS_SHA256_MAC ) */

#if defined( USE_DTLS_ECDH_RSA_KEYSCHEME ) && defined( USE_DTLS_CAMELLIA_256_CBC_CIPHER ) && defined( USE_DTLS_SHA384_MAC )
extern const cipher_suite_t DTLS_ECDH_RSA_WITH_CAMELLIA_256_CBC_SHA384;
#endif /* if defined( USE_DTLS_ECDH_RSA_KEYSCHEME ) && defined( USE_DTLS_CAMELLIA_256_CBC_CIPHER ) && defined( USE_DTLS_SHA384_MAC ) */

#if defined( USE_DTLS_RSA_KEYSCHEME ) && defined( USE_DTLS_CAMELLIA_128_GCM_CIPHER ) && defined( USE_DTLS_SHA256_MAC )
extern const cipher_suite_t DTLS_RSA_WITH_CAMELLIA_128_GCM_SHA256;
#endif /* if defined( USE_DTLS_RSA_KEYSCHEME ) && defined( USE_DTLS_CAMELLIA_128_GCM_CIPHER ) && defined( USE_DTLS_SHA256_MAC ) */

#if defined( USE_DTLS_RSA_KEYSCHEME ) && defined( USE_DTLS_CAMELLIA_256_GCM_CIPHER ) && defined( USE_DTLS_SHA384_MAC )
extern const cipher_suite_t DTLS_RSA_WITH_CAMELLIA_256_GCM_SHA384;
#endif /* if defined( USE_DTLS_RSA_KEYSCHEME ) && defined( USE_DTLS_CAMELLIA_256_GCM_CIPHER ) && defined( USE_DTLS_SHA384_MAC ) */

#if defined( USE_DTLS_DHE_RSA_KEYSCHEME ) && defined( USE_DTLS_CAMELLIA_128_GCM_CIPHER ) && defined( USE_DTLS_SHA256_MAC )
extern const cipher_suite_t DTLS_DHE_RSA_WITH_CAMELLIA_128_GCM_SHA256;
#endif /* if defined( USE_DTLS_DHE_RSA_KEYSCHEME ) && defined( USE_DTLS_CAMELLIA_128_GCM_CIPHER ) && defined( USE_DTLS_SHA256_MAC ) */

#if defined( USE_DTLS_DHE_RSA_KEYSCHEME ) && defined( USE_DTLS_CAMELLIA_256_GCM_CIPHER ) && defined( USE_DTLS_SHA384_MAC )
extern const cipher_suite_t DTLS_DHE_RSA_WITH_CAMELLIA_256_GCM_SHA384;
#endif /* if defined( USE_DTLS_DHE_RSA_KEYSCHEME ) && defined( USE_DTLS_CAMELLIA_256_GCM_CIPHER ) && defined( USE_DTLS_SHA384_MAC ) */

#if defined( USE_DTLS_DH_RSA_KEYSCHEME ) && defined( USE_DTLS_CAMELLIA_128_GCM_CIPHER ) && defined( USE_DTLS_SHA256_MAC )
extern const cipher_suite_t DTLS_DH_RSA_WITH_CAMELLIA_128_GCM_SHA256;
#endif /* if defined( USE_DTLS_DH_RSA_KEYSCHEME ) && defined( USE_DTLS_CAMELLIA_128_GCM_CIPHER ) && defined( USE_DTLS_SHA256_MAC ) */

#if defined( USE_DTLS_DH_RSA_KEYSCHEME ) && defined( USE_DTLS_CAMELLIA_256_GCM_CIPHER ) && defined( USE_DTLS_SHA384_MAC )
extern const cipher_suite_t DTLS_DH_RSA_WITH_CAMELLIA_256_GCM_SHA384;
#endif /* if defined( USE_DTLS_DH_RSA_KEYSCHEME ) && defined( USE_DTLS_CAMELLIA_256_GCM_CIPHER ) && defined( USE_DTLS_SHA384_MAC ) */

#if defined( USE_DTLS_DHE_DSS_KEYSCHEME ) && defined( USE_DTLS_CAMELLIA_128_GCM_CIPHER ) && defined( USE_DTLS_SHA256_MAC )
extern const cipher_suite_t DTLS_DHE_DSS_WITH_CAMELLIA_128_GCM_SHA256;
#endif /* if defined( USE_DTLS_DHE_DSS_KEYSCHEME ) && defined( USE_DTLS_CAMELLIA_128_GCM_CIPHER ) && defined( USE_DTLS_SHA256_MAC ) */

#if defined( USE_DTLS_DHE_DSS_KEYSCHEME ) && defined( USE_DTLS_CAMELLIA_256_GCM_CIPHER ) && defined( USE_DTLS_SHA384_MAC )
extern const cipher_suite_t DTLS_DHE_DSS_WITH_CAMELLIA_256_GCM_SHA384;
#endif /* if defined( USE_DTLS_DHE_DSS_KEYSCHEME ) && defined( USE_DTLS_CAMELLIA_256_GCM_CIPHER ) && defined( USE_DTLS_SHA384_MAC ) */

#if defined( USE_DTLS_DH_DSS_KEYSCHEME ) && defined( USE_DTLS_CAMELLIA_128_GCM_CIPHER ) && defined( USE_DTLS_SHA256_MAC )
extern const cipher_suite_t DTLS_DH_DSS_WITH_CAMELLIA_128_GCM_SHA256;
#endif /* if defined( USE_DTLS_DH_DSS_KEYSCHEME ) && defined( USE_DTLS_CAMELLIA_128_GCM_CIPHER ) && defined( USE_DTLS_SHA256_MAC ) */

#if defined( USE_DTLS_DH_DSS_KEYSCHEME ) && defined( USE_DTLS_CAMELLIA_256_GCM_CIPHER ) && defined( USE_DTLS_SHA384_MAC )
extern const cipher_suite_t DTLS_DH_DSS_WITH_CAMELLIA_256_GCM_SHA384;
#endif /* if defined( USE_DTLS_DH_DSS_KEYSCHEME ) && defined( USE_DTLS_CAMELLIA_256_GCM_CIPHER ) && defined( USE_DTLS_SHA384_MAC ) */

#if defined( USE_DTLS_DH_anon_KEYSCHEME ) && defined( USE_DTLS_CAMELLIA_128_GCM_CIPHER ) && defined( USE_DTLS_SHA256_MAC )
extern const cipher_suite_t DTLS_DH_anon_WITH_CAMELLIA_128_GCM_SHA256;
#endif /* if defined( USE_DTLS_DH_anon_KEYSCHEME ) && defined( USE_DTLS_CAMELLIA_128_GCM_CIPHER ) && defined( USE_DTLS_SHA256_MAC ) */

#if defined( USE_DTLS_DH_anon_KEYSCHEME ) && defined( USE_DTLS_CAMELLIA_256_GCM_CIPHER ) && defined( USE_DTLS_SHA384_MAC )
extern const cipher_suite_t DTLS_DH_anon_WITH_CAMELLIA_256_GCM_SHA384;
#endif /* if defined( USE_DTLS_DH_anon_KEYSCHEME ) && defined( USE_DTLS_CAMELLIA_256_GCM_CIPHER ) && defined( USE_DTLS_SHA384_MAC ) */

#if defined( USE_DTLS_ECDHE_ECDSA_KEYSCHEME ) && defined( USE_DTLS_CAMELLIA_128_GCM_CIPHER ) && defined( USE_DTLS_SHA256_MAC )
extern const cipher_suite_t DTLS_ECDHE_ECDSA_WITH_CAMELLIA_128_GCM_SHA256;
#endif /* if defined( USE_DTLS_ECDHE_ECDSA_KEYSCHEME ) && defined( USE_DTLS_CAMELLIA_128_GCM_CIPHER ) && defined( USE_DTLS_SHA256_MAC ) */

#if defined( USE_DTLS_ECDHE_ECDSA_KEYSCHEME ) && defined( USE_DTLS_CAMELLIA_256_GCM_CIPHER ) && defined( USE_DTLS_SHA384_MAC )
extern const cipher_suite_t DTLS_ECDHE_ECDSA_WITH_CAMELLIA_256_GCM_SHA384;
#endif /* if defined( USE_DTLS_ECDHE_ECDSA_KEYSCHEME ) && defined( USE_DTLS_CAMELLIA_256_GCM_CIPHER ) && defined( USE_DTLS_SHA384_MAC ) */

#if defined( USE_DTLS_ECDH_ECDSA_KEYSCHEME ) && defined( USE_DTLS_CAMELLIA_128_GCM_CIPHER ) && defined( USE_DTLS_SHA256_MAC )
extern const cipher_suite_t DTLS_ECDH_ECDSA_WITH_CAMELLIA_128_GCM_SHA256;
#endif /* if defined( USE_DTLS_ECDH_ECDSA_KEYSCHEME ) && defined( USE_DTLS_CAMELLIA_128_GCM_CIPHER ) && defined( USE_DTLS_SHA256_MAC ) */

#if defined( USE_DTLS_ECDH_ECDSA_KEYSCHEME ) && defined( USE_DTLS_CAMELLIA_256_GCM_CIPHER ) && defined( USE_DTLS_SHA384_MAC )
extern const cipher_suite_t DTLS_ECDH_ECDSA_WITH_CAMELLIA_256_GCM_SHA384;
#endif /* if defined( USE_DTLS_ECDH_ECDSA_KEYSCHEME ) && defined( USE_DTLS_CAMELLIA_256_GCM_CIPHER ) && defined( USE_DTLS_SHA384_MAC ) */

#if defined( USE_DTLS_ECDHE_RSA_KEYSCHEME ) && defined( USE_DTLS_CAMELLIA_128_GCM_CIPHER ) && defined( USE_DTLS_SHA256_MAC )
extern const cipher_suite_t DTLS_ECDHE_RSA_WITH_CAMELLIA_128_GCM_SHA256;
#endif /* if defined( USE_DTLS_ECDHE_RSA_KEYSCHEME ) && defined( USE_DTLS_CAMELLIA_128_GCM_CIPHER ) && defined( USE_DTLS_SHA256_MAC ) */

#if defined( USE_DTLS_ECDHE_RSA_KEYSCHEME ) && defined( USE_DTLS_CAMELLIA_256_GCM_CIPHER ) && defined( USE_DTLS_SHA384_MAC )
extern const cipher_suite_t DTLS_ECDHE_RSA_WITH_CAMELLIA_256_GCM_SHA384;
#endif /* if defined( USE_DTLS_ECDHE_RSA_KEYSCHEME ) && defined( USE_DTLS_CAMELLIA_256_GCM_CIPHER ) && defined( USE_DTLS_SHA384_MAC ) */

#if defined( USE_DTLS_ECDH_RSA_KEYSCHEME ) && defined( USE_DTLS_CAMELLIA_128_GCM_CIPHER ) && defined( USE_DTLS_SHA256_MAC )
extern const cipher_suite_t DTLS_ECDH_RSA_WITH_CAMELLIA_128_GCM_SHA256;
#endif /* if defined( USE_DTLS_ECDH_RSA_KEYSCHEME ) && defined( USE_DTLS_CAMELLIA_128_GCM_CIPHER ) && defined( USE_DTLS_SHA256_MAC ) */

#if defined( USE_DTLS_ECDH_RSA_KEYSCHEME ) && defined( USE_DTLS_CAMELLIA_256_GCM_CIPHER ) && defined( USE_DTLS_SHA384_MAC )
extern const cipher_suite_t DTLS_ECDH_RSA_WITH_CAMELLIA_256_GCM_SHA384;
#endif /* if defined( USE_DTLS_ECDH_RSA_KEYSCHEME ) && defined( USE_DTLS_CAMELLIA_256_GCM_CIPHER ) && defined( USE_DTLS_SHA384_MAC ) */

#if defined( USE_DTLS_PSK_KEYSCHEME ) && defined( USE_DTLS_CAMELLIA_128_GCM_CIPHER ) && defined( USE_DTLS_SHA256_MAC )
extern const cipher_suite_t DTLS_PSK_WITH_CAMELLIA_128_GCM_SHA256;
#endif /* if defined( USE_DTLS_PSK_KEYSCHEME ) && defined( USE_DTLS_CAMELLIA_128_GCM_CIPHER ) && defined( USE_DTLS_SHA256_MAC ) */

#if defined( USE_DTLS_PSK_KEYSCHEME ) && defined( USE_DTLS_CAMELLIA_256_GCM_CIPHER ) && defined( USE_DTLS_SHA384_MAC )
extern const cipher_suite_t DTLS_PSK_WITH_CAMELLIA_256_GCM_SHA384;
#endif /* if defined( USE_DTLS_PSK_KEYSCHEME ) && defined( USE_DTLS_CAMELLIA_256_GCM_CIPHER ) && defined( USE_DTLS_SHA384_MAC ) */

#if defined( USE_DTLS_DHE_PSK_KEYSCHEME ) && defined( USE_DTLS_CAMELLIA_128_GCM_CIPHER ) && defined( USE_DTLS_SHA256_MAC )
extern const cipher_suite_t DTLS_DHE_PSK_WITH_CAMELLIA_128_GCM_SHA256;
#endif /* if defined( USE_DTLS_DHE_PSK_KEYSCHEME ) && defined( USE_DTLS_CAMELLIA_128_GCM_CIPHER ) && defined( USE_DTLS_SHA256_MAC ) */

#if defined( USE_DTLS_DHE_PSK_KEYSCHEME ) && defined( USE_DTLS_CAMELLIA_256_GCM_CIPHER ) && defined( USE_DTLS_SHA384_MAC )
extern const cipher_suite_t DTLS_DHE_PSK_WITH_CAMELLIA_256_GCM_SHA384;
#endif /* if defined( USE_DTLS_DHE_PSK_KEYSCHEME ) && defined( USE_DTLS_CAMELLIA_256_GCM_CIPHER ) && defined( USE_DTLS_SHA384_MAC ) */

#if defined( USE_DTLS_RSA_PSK_KEYSCHEME ) && defined( USE_DTLS_CAMELLIA_128_GCM_CIPHER ) && defined( USE_DTLS_SHA256_MAC )
extern const cipher_suite_t DTLS_RSA_PSK_WITH_CAMELLIA_128_GCM_SHA256;
#endif /* if defined( USE_DTLS_RSA_PSK_KEYSCHEME ) && defined( USE_DTLS_CAMELLIA_128_GCM_CIPHER ) && defined( USE_DTLS_SHA256_MAC ) */

#if defined( USE_DTLS_RSA_PSK_KEYSCHEME ) && defined( USE_DTLS_CAMELLIA_256_GCM_CIPHER ) && defined( USE_DTLS_SHA384_MAC )
extern const cipher_suite_t DTLS_RSA_PSK_WITH_CAMELLIA_256_GCM_SHA384;
#endif /* if defined( USE_DTLS_RSA_PSK_KEYSCHEME ) && defined( USE_DTLS_CAMELLIA_256_GCM_CIPHER ) && defined( USE_DTLS_SHA384_MAC ) */

#if defined( USE_DTLS_PSK_KEYSCHEME ) && defined( USE_DTLS_CAMELLIA_128_CBC_CIPHER ) && defined( USE_DTLS_SHA256_MAC )
extern const cipher_suite_t DTLS_PSK_WITH_CAMELLIA_128_CBC_SHA256;
#endif /* if defined( USE_DTLS_PSK_KEYSCHEME ) && defined( USE_DTLS_CAMELLIA_128_CBC_CIPHER ) && defined( USE_DTLS_SHA256_MAC ) */

#if defined( USE_DTLS_PSK_KEYSCHEME ) && defined( USE_DTLS_CAMELLIA_256_CBC_CIPHER ) && defined( USE_DTLS_SHA384_MAC )
extern const cipher_suite_t DTLS_PSK_WITH_CAMELLIA_256_CBC_SHA384;
#endif /* if defined( USE_DTLS_PSK_KEYSCHEME ) && defined( USE_DTLS_CAMELLIA_256_CBC_CIPHER ) && defined( USE_DTLS_SHA384_MAC ) */

#if defined( USE_DTLS_DHE_PSK_KEYSCHEME ) && defined( USE_DTLS_CAMELLIA_128_CBC_CIPHER ) && defined( USE_DTLS_SHA256_MAC )
extern const cipher_suite_t DTLS_DHE_PSK_WITH_CAMELLIA_128_CBC_SHA256;
#endif /* if defined( USE_DTLS_DHE_PSK_KEYSCHEME ) && defined( USE_DTLS_CAMELLIA_128_CBC_CIPHER ) && defined( USE_DTLS_SHA256_MAC ) */

#if defined( USE_DTLS_DHE_PSK_KEYSCHEME ) && defined( USE_DTLS_CAMELLIA_256_CBC_CIPHER ) && defined( USE_DTLS_SHA384_MAC )
extern const cipher_suite_t DTLS_DHE_PSK_WITH_CAMELLIA_256_CBC_SHA384;
#endif /* if defined( USE_DTLS_DHE_PSK_KEYSCHEME ) && defined( USE_DTLS_CAMELLIA_256_CBC_CIPHER ) && defined( USE_DTLS_SHA384_MAC ) */

#if defined( USE_DTLS_RSA_PSK_KEYSCHEME ) && defined( USE_DTLS_CAMELLIA_128_CBC_CIPHER ) && defined( USE_DTLS_SHA256_MAC )
extern const cipher_suite_t DTLS_RSA_PSK_WITH_CAMELLIA_128_CBC_SHA256;
#endif /* if defined( USE_DTLS_RSA_PSK_KEYSCHEME ) && defined( USE_DTLS_CAMELLIA_128_CBC_CIPHER ) && defined( USE_DTLS_SHA256_MAC ) */

#if defined( USE_DTLS_RSA_PSK_KEYSCHEME ) && defined( USE_DTLS_CAMELLIA_256_CBC_CIPHER ) && defined( USE_DTLS_SHA384_MAC )
extern const cipher_suite_t DTLS_RSA_PSK_WITH_CAMELLIA_256_CBC_SHA384;
#endif /* if defined( USE_DTLS_RSA_PSK_KEYSCHEME ) && defined( USE_DTLS_CAMELLIA_256_CBC_CIPHER ) && defined( USE_DTLS_SHA384_MAC ) */

#if defined( USE_DTLS_ECDHE_PSK_KEYSCHEME ) && defined( USE_DTLS_CAMELLIA_128_CBC_CIPHER ) && defined( USE_DTLS_SHA256_MAC )
extern const cipher_suite_t DTLS_ECDHE_PSK_WITH_CAMELLIA_128_CBC_SHA256;
#endif /* if defined( USE_DTLS_ECDHE_PSK_KEYSCHEME ) && defined( USE_DTLS_CAMELLIA_128_CBC_CIPHER ) && defined( USE_DTLS_SHA256_MAC ) */

#if defined( USE_DTLS_ECDHE_PSK_KEYSCHEME ) && defined( USE_DTLS_CAMELLIA_256_CBC_CIPHER ) && defined( USE_DTLS_SHA384_MAC )
extern const cipher_suite_t DTLS_ECDHE_PSK_WITH_CAMELLIA_256_CBC_SHA384;
#endif /* if defined( USE_DTLS_ECDHE_PSK_KEYSCHEME ) && defined( USE_DTLS_CAMELLIA_256_CBC_CIPHER ) && defined( USE_DTLS_SHA384_MAC ) */

#if defined( USE_DTLS_RSA_KEYSCHEME ) && defined( USE_DTLS_AES_128_CCM_CIPHER ) && defined( USE_DTLS_AES_128_CCM_MAC )
extern const cipher_suite_t DTLS_RSA_WITH_AES_128_CCM;
#endif /* if defined( USE_DTLS_RSA_KEYSCHEME ) && defined( USE_DTLS_AES_128_CCM_CIPHER ) && defined( USE_DTLS_AES_128_CCM_MAC ) */

#if defined( USE_DTLS_RSA_KEYSCHEME ) && defined( USE_DTLS_AES_256_CCM_CIPHER ) && defined( USE_DTLS_AES_256_CCM_MAC )
extern const cipher_suite_t DTLS_RSA_WITH_AES_256_CCM;
#endif /* if defined( USE_DTLS_RSA_KEYSCHEME ) && defined( USE_DTLS_AES_256_CCM_CIPHER ) && defined( USE_DTLS_AES_256_CCM_MAC ) */

#if defined( USE_DTLS_DHE_RSA_KEYSCHEME ) && defined( USE_DTLS_AES_128_CCM_CIPHER ) && defined( USE_DTLS_AES_128_CCM_MAC )
extern const cipher_suite_t DTLS_DHE_RSA_WITH_AES_128_CCM;
#endif /* if defined( USE_DTLS_DHE_RSA_KEYSCHEME ) && defined( USE_DTLS_AES_128_CCM_CIPHER ) && defined( USE_DTLS_AES_128_CCM_MAC ) */

#if defined( USE_DTLS_DHE_RSA_KEYSCHEME ) && defined( USE_DTLS_AES_256_CCM_CIPHER ) && defined( USE_DTLS_AES_256_CCM_MAC )
extern const cipher_suite_t DTLS_DHE_RSA_WITH_AES_256_CCM;
#endif /* if defined( USE_DTLS_DHE_RSA_KEYSCHEME ) && defined( USE_DTLS_AES_256_CCM_CIPHER ) && defined( USE_DTLS_AES_256_CCM_MAC ) */

#if defined( USE_DTLS_RSA_KEYSCHEME ) && defined( USE_DTLS_AES_128_CCM_8_CIPHER ) && defined( USE_DTLS_AES_128_CCM_8_MAC )
extern const cipher_suite_t DTLS_RSA_WITH_AES_128_CCM_8;
#endif /* if defined( USE_DTLS_RSA_KEYSCHEME ) && defined( USE_DTLS_AES_128_CCM_8_CIPHER ) && defined( USE_DTLS_AES_128_CCM_8_MAC ) */

#if defined( USE_DTLS_RSA_KEYSCHEME ) && defined( USE_DTLS_AES_256_CCM_8_CIPHER ) && defined( USE_DTLS_AES_256_CCM_8_MAC )
extern const cipher_suite_t DTLS_RSA_WITH_AES_256_CCM_8;
#endif /* if defined( USE_DTLS_RSA_KEYSCHEME ) && defined( USE_DTLS_AES_256_CCM_8_CIPHER ) && defined( USE_DTLS_AES_256_CCM_8_MAC ) */

#if defined( USE_DTLS_DHE_RSA_KEYSCHEME ) && defined( USE_DTLS_AES_128_CCM_8_CIPHER ) && defined( USE_DTLS_AES_128_CCM_8_MAC )
extern const cipher_suite_t DTLS_DHE_RSA_WITH_AES_128_CCM_8;
#endif /* if defined( USE_DTLS_DHE_RSA_KEYSCHEME ) && defined( USE_DTLS_AES_128_CCM_8_CIPHER ) && defined( USE_DTLS_AES_128_CCM_8_MAC ) */

#if defined( USE_DTLS_DHE_RSA_KEYSCHEME ) && defined( USE_DTLS_AES_256_CCM_8_CIPHER ) && defined( USE_DTLS_AES_256_CCM_8_MAC )
extern const cipher_suite_t DTLS_DHE_RSA_WITH_AES_256_CCM_8;
#endif /* if defined( USE_DTLS_DHE_RSA_KEYSCHEME ) && defined( USE_DTLS_AES_256_CCM_8_CIPHER ) && defined( USE_DTLS_AES_256_CCM_8_MAC ) */

#if defined( USE_DTLS_PSK_KEYSCHEME ) && defined( USE_DTLS_AES_128_CCM_CIPHER ) && defined( USE_DTLS_AES_128_CCM_MAC )
extern const cipher_suite_t DTLS_PSK_WITH_AES_128_CCM;
#endif /* if defined( USE_DTLS_PSK_KEYSCHEME ) && defined( USE_DTLS_AES_128_CCM_CIPHER ) && defined( USE_DTLS_AES_128_CCM_MAC ) */

#if defined( USE_DTLS_PSK_KEYSCHEME ) && defined( USE_DTLS_AES_256_CCM_CIPHER ) && defined( USE_DTLS_AES_256_CCM_MAC )
extern const cipher_suite_t DTLS_PSK_WITH_AES_256_CCM;
#endif /* if defined( USE_DTLS_PSK_KEYSCHEME ) && defined( USE_DTLS_AES_256_CCM_CIPHER ) && defined( USE_DTLS_AES_256_CCM_MAC ) */

#if defined( USE_DTLS_DHE_PSK_KEYSCHEME ) && defined( USE_DTLS_AES_128_CCM_CIPHER ) && defined( USE_DTLS_AES_128_CCM_MAC )
extern const cipher_suite_t DTLS_DHE_PSK_WITH_AES_128_CCM;
#endif /* if defined( USE_DTLS_DHE_PSK_KEYSCHEME ) && defined( USE_DTLS_AES_128_CCM_CIPHER ) && defined( USE_DTLS_AES_128_CCM_MAC ) */

#if defined( USE_DTLS_DHE_PSK_KEYSCHEME ) && defined( USE_DTLS_AES_256_CCM_CIPHER ) && defined( USE_DTLS_AES_256_CCM_MAC )
extern const cipher_suite_t DTLS_DHE_PSK_WITH_AES_256_CCM;
#endif /* if defined( USE_DTLS_DHE_PSK_KEYSCHEME ) && defined( USE_DTLS_AES_256_CCM_CIPHER ) && defined( USE_DTLS_AES_256_CCM_MAC ) */

#if defined( USE_DTLS_PSK_KEYSCHEME ) && defined( USE_DTLS_AES_128_CCM_8_CIPHER ) && defined( USE_DTLS_AES_128_CCM_8_MAC )
extern const cipher_suite_t DTLS_PSK_WITH_AES_128_CCM_8;
#endif /* if defined( USE_DTLS_PSK_KEYSCHEME ) && defined( USE_DTLS_AES_128_CCM_8_CIPHER ) && defined( USE_DTLS_AES_128_CCM_8_MAC ) */

#if defined( USE_DTLS_PSK_KEYSCHEME ) && defined( USE_DTLS_AES_256_CCM_8_CIPHER ) && defined( USE_DTLS_AES_256_CCM_8_MAC )
extern const cipher_suite_t DTLS_PSK_WITH_AES_256_CCM_8;
#endif /* if defined( USE_DTLS_PSK_KEYSCHEME ) && defined( USE_DTLS_AES_256_CCM_8_CIPHER ) && defined( USE_DTLS_AES_256_CCM_8_MAC ) */

#if defined( USE_DTLS_DHE_PSK_KEYSCHEME ) && defined( USE_DTLS_AES_128_CCM_8_CIPHER ) && defined( USE_DTLS_AES_128_CCM_8_MAC )
extern const cipher_suite_t DTLS_DHE_PSK_WITH_AES_128_CCM_8;
#endif /* if defined( USE_DTLS_DHE_PSK_KEYSCHEME ) && defined( USE_DTLS_AES_128_CCM_8_CIPHER ) && defined( USE_DTLS_AES_128_CCM_8_MAC ) */

#if defined( USE_DTLS_DHE_PSK_KEYSCHEME ) && defined( USE_DTLS_AES_256_CCM_8_CIPHER ) && defined( USE_DTLS_AES_256_CCM_8_MAC )
extern const cipher_suite_t DTLS_DHE_PSK_WITH_AES_256_CCM_8;
#endif /* if defined( USE_DTLS_DHE_PSK_KEYSCHEME ) && defined( USE_DTLS_AES_256_CCM_8_CIPHER ) && defined( USE_DTLS_AES_256_CCM_8_MAC ) */

#if defined( USE_DTLS_ECDHE_ECDSA_KEYSCHEME ) && defined( USE_DTLS_AES_128_CCM_CIPHER ) && defined( USE_DTLS_AES_128_CCM_MAC )
extern const cipher_suite_t DTLS_ECDHE_ECDSA_WITH_AES_128_CCM;
#endif /* if defined( USE_DTLS_ECDHE_ECDSA_KEYSCHEME ) && defined( USE_DTLS_AES_128_CCM_CIPHER ) && defined( USE_DTLS_AES_128_CCM_MAC ) */

#if defined( USE_DTLS_ECDHE_ECDSA_KEYSCHEME ) && defined( USE_DTLS_AES_256_CCM_CIPHER ) && defined( USE_DTLS_AES_256_CCM_MAC )
extern const cipher_suite_t DTLS_ECDHE_ECDSA_WITH_AES_256_CCM;
#endif /* if defined( USE_DTLS_ECDHE_ECDSA_KEYSCHEME ) && defined( USE_DTLS_AES_256_CCM_CIPHER ) && defined( USE_DTLS_AES_256_CCM_MAC ) */

#if defined( USE_DTLS_ECDHE_ECDSA_KEYSCHEME ) && defined( USE_DTLS_AES_128_CCM_8_CIPHER ) && defined( USE_DTLS_AES_128_CCM_8_MAC )
extern const cipher_suite_t DTLS_ECDHE_ECDSA_WITH_AES_128_CCM_8;
#endif /* if defined( USE_DTLS_ECDHE_ECDSA_KEYSCHEME ) && defined( USE_DTLS_AES_128_CCM_8_CIPHER ) && defined( USE_DTLS_AES_128_CCM_8_MAC ) */

#if defined( USE_DTLS_ECDHE_ECDSA_KEYSCHEME ) && defined( USE_DTLS_AES_256_CCM_8_CIPHER ) && defined( USE_DTLS_AES_256_CCM_8_MAC )
extern const cipher_suite_t DTLS_ECDHE_ECDSA_WITH_AES_256_CCM_8;
#endif /* if defined( USE_DTLS_ECDHE_ECDSA_KEYSCHEME ) && defined( USE_DTLS_AES_256_CCM_8_CIPHER ) && defined( USE_DTLS_AES_256_CCM_8_MAC ) */

#if defined( USE_DTLS_ECDHE_RSA_KEYSCHEME ) && defined( USE_DTLS_CHACHA20_POLY1305_CIPHER ) && defined( USE_DTLS_SHA256_MAC )
extern const cipher_suite_t DTLS_ECDHE_RSA_WITH_CHACHA20_POLY1305_SHA256;
#endif /* if defined( USE_DTLS_ECDHE_RSA_KEYSCHEME ) && defined( USE_DTLS_CHACHA20_POLY1305_CIPHER ) && defined( USE_DTLS_SHA256_MAC ) */

#if defined( USE_DTLS_ECDHE_ECDSA_KEYSCHEME ) && defined( USE_DTLS_CHACHA20_POLY1305_CIPHER ) && defined( USE_DTLS_SHA256_MAC )
extern const cipher_suite_t DTLS_ECDHE_ECDSA_WITH_CHACHA20_POLY1305_SHA256;
#endif /* if defined( USE_DTLS_ECDHE_ECDSA_KEYSCHEME ) && defined( USE_DTLS_CHACHA20_POLY1305_CIPHER ) && defined( USE_DTLS_SHA256_MAC ) */

#if defined( USE_DTLS_DHE_RSA_KEYSCHEME ) && defined( USE_DTLS_CHACHA20_POLY1305_CIPHER ) && defined( USE_DTLS_SHA256_MAC )
extern const cipher_suite_t DTLS_DHE_RSA_WITH_CHACHA20_POLY1305_SHA256;
#endif /* if defined( USE_DTLS_DHE_RSA_KEYSCHEME ) && defined( USE_DTLS_CHACHA20_POLY1305_CIPHER ) && defined( USE_DTLS_SHA256_MAC ) */
#endif

#ifdef __cplusplus
} /*extern "C" */
#endif
