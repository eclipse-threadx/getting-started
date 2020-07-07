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
/** NetX Secure Component                                                 */
/**                                                                       */
/**    Transport Layer Security (TLS)                                     */
/**                                                                       */
/**************************************************************************/
/**************************************************************************/

#define NX_SECURE_SOURCE_CODE

#include "nx_secure_tls.h"

/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_secure_tls_key_material_init                    PORTABLE C      */
/*                                                           6.0          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function initializes the key material used by a TLS session.   */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    key_material                          Pointer to key material data  */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    None                                                                */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    _nx_secure_tls_session_reset          Clear out the session         */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  05-19-2020     Timothy Stapko           Initial Version 6.0           */
/*                                                                        */
/**************************************************************************/
UINT _nx_secure_tls_key_material_init(NX_SECURE_TLS_KEY_MATERIAL *key_material)
{
    /* Clear out key blocks individually - we may need to keep some key data
     * around in the future for things like session re-negotiation/resumption. */

    NX_SECURE_MEMSET(key_material -> nx_secure_tls_client_random, 0, NX_SECURE_TLS_RANDOM_SIZE);

    NX_SECURE_MEMSET(key_material -> nx_secure_tls_server_random, 0, NX_SECURE_TLS_RANDOM_SIZE);

    NX_SECURE_MEMSET(key_material -> nx_secure_tls_pre_master_secret, 0, NX_SECURE_TLS_PREMASTER_SIZE);
    key_material -> nx_secure_tls_pre_master_secret_size = 0;

    NX_SECURE_MEMSET(key_material -> nx_secure_tls_master_secret, 0, NX_SECURE_TLS_MASTER_SIZE);

    NX_SECURE_MEMSET(key_material -> nx_secure_tls_key_material_data, 0, NX_SECURE_TLS_KEY_MATERIAL_SIZE);
    NX_SECURE_MEMSET(key_material -> nx_secure_tls_new_key_material_data, 0, NX_SECURE_TLS_KEY_MATERIAL_SIZE);

#if(NX_SECURE_TLS_TLS_1_3_ENABLED)
    NX_SECURE_MEMSET(&key_material->nx_secure_tls_key_secrets, 0, sizeof(NX_SECURE_TLS_KEY_SECRETS));
    NX_SECURE_MEMSET(key_material->nx_secure_tls_ecc_key_data, 0, sizeof(key_material->nx_secure_tls_ecc_key_data));
    key_material->nx_secure_tls_handshake_cache_length = 0;
#endif


    return(NX_SECURE_TLS_SUCCESS);
}

