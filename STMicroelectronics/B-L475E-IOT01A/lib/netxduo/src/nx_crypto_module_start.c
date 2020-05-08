/**************************************************************************/
/*                                                                        */
/*       Copyright (c) Microsoft Corporation. All rights reserved.        */
/*                                                                        */
/*       This software is licensed under the Microsoft Software License   */
/*       Terms for Microsoft Azure RTOS. Full text of the license can be  */
/*       found in the LICENSE file at https://aka.ms/AzureRTOS_EULA       */
/*       and in the root directory of this software.                      */
/*                                                                        */
/**************************************************************************/


/**************************************************************************/
/**************************************************************************/
/**                                                                       */
/** NetX Crypto Component                                                 */
/**                                                                       */
/**   Crypto Module Initialization, startup, and helper routines          */
/**                                                                       */
/**************************************************************************/
/**************************************************************************/

/* NOTE: This file is used for FIPS certification on STM32F469 only. */

#define NX_CRYPTO_SOURCE_CODE

#include "nx_crypto.h"
#include "nx_crypto_hmac_sha2.h"

#ifdef NX_CRYPTO_FIPS

#define NX_CRYPTO_INTEGRITY_TEST_CHECK(status)                                      \
    if(status)                                                                      \
    {                                                                               \
        _nx_crypto_library_state |= NX_CRYPTO_LIBRARY_STATE_POST_FAILED;            \
    }

#define REG32(x) (*(volatile unsigned int *)(x))


/* Define RCC register.  */
#define STM32F4_RCC                         0x40023800
#define STM32F4_RCC_AHB2ENR                 REG32(STM32F4_RCC + 0x34)
#define STM32F4_RCC_AHB2ENR_RNGEN           0x00000040

#define STM32L4_RCC                         0x40021000
#define STM32L4_RCC_AHB2ENR                 REG32(STM32L4_RCC + 0x4C)
#define STM32L4_RCC_AHB2ENR_RNGEN           0x00040000

/* Define RNG registers.  */
#define STM32_RNG                           0x50060800
#define STM32_RNG_CR                        REG32(STM32_RNG + 0x00)
#define STM32_RNG_SR                        REG32(STM32_RNG + 0x04)
#define STM32_RNG_DR                        REG32(STM32_RNG + 0x08)

#define STM32_RNG_CR_RNGEN                  0x00000004
#define STM32_RNG_CR_IE                     0x00000008
#define STM32_RNG_CR_CED                    0x00000020

#define STM32_RNG_SR_DRDY                   0x00000001
#define STM32_RNG_SR_CECS                   0x00000002
#define STM32_RNG_SR_SECS                   0x00000004
#define STM32_RNG_SR_CEIS                   0x00000020
#define STM32_RNG_SR_SEIS                   0x00000040

#define DBGMCU_IDCODE                       REG32(0xE0042000)
#define DBGMCU_IDCODE_DEV_ID_MASK           0x00000FFF
#define DEV_ID_L47x                         0x415

const unsigned long long _nx_crypto_module_program_begin @ "NX_CRYPTO_PROGRAM_BEGIN" = 0ull;


const unsigned char _nx_crypto_module_hash_value[32] @ "NX_CRYPTO_PROGRAM_END" =
{
  
#if (__VER__ == 8030002)
/*  IAR 8.30.2 */
    0xe0, 0xa6, 0xf3, 0x19, 0x9f, 0x43, 0x02, 0x4f,
    0xe6, 0x69, 0xfb, 0xd3, 0x4c, 0x1b, 0xdd, 0xee,
    0x90, 0x3c, 0xff, 0xba, 0x16, 0x56, 0x51, 0x78,
    0x57, 0xe6, 0x47, 0xc2, 0xca, 0x83, 0xff, 0xc9
#else
/* IAR 8.22.2 */
    0xe8, 0xd9, 0xe8, 0xac, 0xd3, 0x7c, 0x54, 0x7e,
    0xd0, 0x72, 0x96, 0x38, 0xde, 0x45, 0x78, 0xff,
    0xe1, 0xe4, 0xb2, 0xfc, 0xfd, 0xa6, 0x11, 0xd7,
    0x0c, 0xda, 0xc9, 0xc9, 0xf2, 0x95, 0xd2, 0xa4
#endif
};

extern NX_CRYPTO_METHOD crypto_method_hmac_sha256;
extern const CHAR nx_crypto_hash_key[];
extern const UINT nx_crypto_hash_key_size;
static UCHAR nx_crypto_hmac_output[64];
static NX_CRYPTO_SHA256_HMAC nx_crypto_fips_hmac;

/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_crypto_hardware_rand_initialize                 PORTABLE C      */
/*                                                           6.0          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function initializes hardware random number generator.         */
/*    The following processors are supported:                             */
/*    STM32F407                                                           */
/*    STM32F417                                                           */
/*    STM32F429                                                           */
/*    STM32F439                                                           */
/*    STM32F469                                                           */
/*    STM32F479                                                           */
/*    STM32L475                                                           */
/*    STM32L476                                                           */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    None                                                                */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    None                                                                */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    None                                                                */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    _nx_crypto_method_self_test                                         */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  05-19-2020     Timothy Stapko           Initial Version 6.0           */
/*                                                                        */
/**************************************************************************/
NX_CRYPTO_KEEP void _nx_crypto_hardware_rand_initialize_stm32f4(void)
{
    
unsigned int device_id;

    /* Read device ID.  */
    device_id = DBGMCU_IDCODE & DBGMCU_IDCODE_DEV_ID_MASK;
    
    if (device_id == DEV_ID_L47x)
    {
        
        /* Enable clock for the RNG.  */
        STM32L4_RCC_AHB2ENR |= STM32L4_RCC_AHB2ENR_RNGEN;
    }
    else
    {

        /* Enable clock for the RNG.  */
        STM32F4_RCC_AHB2ENR |= STM32F4_RCC_AHB2ENR_RNGEN;
    }

    /* Enable the random number generator.  */
    STM32_RNG_CR = STM32_RNG_CR_RNGEN;
}

NX_CRYPTO_KEEP INT nx_crypto_hardware_rand(VOID)
{

    /* Wait for data ready.  */
    while((STM32_RNG_SR & STM32_RNG_SR_DRDY) == 0);

    /* Return the random number.  */
    return STM32_RNG_DR;
}

/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    __aeabi_memcpy                                      PORTABLE C      */
/*                                                           6.0          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function performs memory copy function for the FIPS 140-2      */
/*    compliance build.                                                   */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    dest                                  Pointer to the destination    */
/*                                            memory                      */
/*    value                                 value (in byte) to set to the */
/*                                            memory location             */
/*    size                                  Number of bytes to copy       */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    void *                                Pointer to the destination    */
/*                                            memory                      */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    None                                                                */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  05-19-2020     Timothy Stapko           Initial Version 6.0           */
/*                                                                        */
/**************************************************************************/
NX_CRYPTO_KEEP __nounwind __interwork __softfp __aapcs_core void __aeabi_memcpy (void *dest, const void *src, size_t size)
{
    char *from, *to;
    unsigned int i;

    from = (char*)src;
    to = (char*)dest;

    for(i = 0; i < size; i++)
    {
        to[i] = from[i];
    }
}


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    __aeabi_memcpy4                                     PORTABLE C      */
/*                                                           6.0          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function performs memory copy function for the FIPS 140-2      */
/*    compliance build.                                                   */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    dest                                  Pointer to the destination    */
/*                                            memory                      */
/*    value                                 value (in byte) to set to the */
/*                                            memory location             */
/*    size                                  Number of bytes to copy       */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    void *                                Pointer to the destination    */
/*                                            memory                      */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    None                                                                */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  05-19-2020     Timothy Stapko           Initial Version 6.0           */
/*                                                                        */
/**************************************************************************/
NX_CRYPTO_KEEP __nounwind __interwork __softfp __aapcs_core void __aeabi_memcpy4 (void *dest, const void *src, size_t size)
{
    char *from, *to;
    unsigned int i;

    from = (char*)src;
    to = (char*)dest;

    for(i = 0; i < size; i++)
    {
        to[i] = from[i];
    }
}


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    __aeabi_memcpy8                                     PORTABLE C      */
/*                                                           6.0          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function performs memory copy function for the FIPS 140-2      */
/*    compliance build.                                                   */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    dest                                  Pointer to the destination    */
/*                                            memory                      */
/*    value                                 value (in byte) to set to the */
/*                                            memory location             */
/*    size                                  Number of bytes to copy       */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    void *                                Pointer to the destination    */
/*                                            memory                      */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    None                                                                */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  05-19-2020     Timothy Stapko           Initial Version 6.0           */
/*                                                                        */
/**************************************************************************/
NX_CRYPTO_KEEP __nounwind __interwork __softfp __aapcs_core void __aeabi_memcpy8 (void *dest, const void *src, size_t size)
{
    char *from, *to;
    unsigned int i;

    from = (char*)src;
    to = (char*)dest;

    for(i = 0; i < size; i++)
    {
        to[i] = from[i];
    }
}

NX_CRYPTO_KEEP UINT _nx_crypto_integrity_test(void)
{
UINT status;

    status = _nx_crypto_method_hmac_sha256_operation(NX_CRYPTO_AUTHENTICATE,  /* op */
                                                     (VOID*)&nx_crypto_fips_hmac,
                                                     &crypto_method_hmac_sha256, /* Method */
                                                     (UCHAR*)nx_crypto_hash_key,
                                                     nx_crypto_hash_key_size,
                                                     (UCHAR*)&_nx_crypto_module_program_begin,
                                                     (ULONG)_nx_crypto_module_hash_value - (ULONG)&_nx_crypto_module_program_begin,
                                                     NX_CRYPTO_NULL, /* iv_ptr, not used */
                                                     nx_crypto_hmac_output, sizeof(nx_crypto_hmac_output),
                                                     &nx_crypto_fips_hmac, sizeof(nx_crypto_fips_hmac),
                                                     NX_CRYPTO_NULL, NX_CRYPTO_NULL);
    NX_CRYPTO_INTEGRITY_TEST_CHECK(status)

    status =  (UINT)NX_CRYPTO_MEMCMP(nx_crypto_hmac_output, _nx_crypto_module_hash_value, 32);
    NX_CRYPTO_INTEGRITY_TEST_CHECK(status);

    return(status);
}

#endif
