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

#include "nx_secure_tls_ecc.h"
#include "nx_secure_tls.h"

NX_SECURE_TLS_ECC _nx_secure_tls_ecc_info;

/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_secure_tls_ecc_initialize                       PORTABLE C      */
/*                                                          5.12          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Express Logic, Inc.                                 */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function initializes supported curve lists for TLS.            */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    tls_session                           TLS control block             */
/*    supported_groups                      List of supported groups      */
/*    supported_group_count                 Number of supported groups    */
/*    curves                                List of curve methods         */
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
/*    Application Code                                                    */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  08-15-2019     Timothy Stapko           Initial Version 5.12          */
/*                                                                        */
/**************************************************************************/
UINT _nx_secure_tls_ecc_initialize(struct NX_SECURE_TLS_SESSION_STRUCT *tls_session,
                                   const USHORT *supported_groups, USHORT supported_group_count,
                                   const VOID *curves)
{

    tls_session -> nx_secure_tls_ecc.nx_secure_tls_ecc_supported_groups = supported_groups;
    tls_session -> nx_secure_tls_ecc.nx_secure_tls_ecc_supported_groups_count = supported_group_count;
    tls_session -> nx_secure_tls_ecc.nx_secure_tls_ecc_curves = curves;

    _nx_secure_tls_ecc_info.nx_secure_tls_ecc_supported_groups = supported_groups;
    _nx_secure_tls_ecc_info.nx_secure_tls_ecc_supported_groups_count = supported_group_count;
    _nx_secure_tls_ecc_info.nx_secure_tls_ecc_curves = (NX_CRYPTO_METHOD **)curves;

    return(NX_SUCCESS);
}


