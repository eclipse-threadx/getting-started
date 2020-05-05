/**************************************************************************/
/*                                                                        */
/*            Copyright (c) 1996-2019 by Express Logic Inc.               */
/*                                                                        */
/*  This software is copyrighted by and is the sole property of Express   */
/*  Logic, Inc.  All rights, title, ownership, or other interests         */
/*  in the software remain the property of Express Logic, Inc.  This      */
/*  software may only be used in accordance with the corresponding        */
/*  license agreement.  Any unauthorized use, duplication, transmission,  */
/*  distribution, or disclosure of this software is expressly forbidden.  */
/*                                                                        */
/*  This Copyright notice may not be removed or modified without prior    */
/*  written consent of Express Logic, Inc.                                */
/*                                                                        */
/*  Express Logic, Inc. reserves the right to modify this software        */
/*  without notice.                                                       */
/*                                                                        */
/*  Express Logic, Inc.                     info@expresslogic.com         */
/*  11423 West Bernardo Court               http://www.expresslogic.com   */
/*  San Diego, CA  92127                                                  */
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
/*                                                           5.12         */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Express Logic, Inc.                                 */
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
/*  06-09-2017     Timothy Stapko           Initial Version 5.10          */
/*  12-15-2017     Timothy Stapko           Modified comment(s),          */
/*                                            optimized the logic,        */
/*                                            resulting in version 5.11   */
/*  08-15-2019     Timothy Stapko           Modified comment(s), and      */
/*                                            added flexibility of using  */
/*                                            macros instead of direct C  */
/*                                            library function calls,     */
/*                                            resulting in version 5.12   */
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

    return(NX_SECURE_TLS_SUCCESS);
}

