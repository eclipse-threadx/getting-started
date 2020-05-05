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

extern NX_SECURE_TLS_ECC _nx_secure_tls_ecc_info;

/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_secure_tls_find_curve_method                    PORTABLE C      */
/*                                                          5.12          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Express Logic, Inc.                                 */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function finds the curve method for the specified named curve  */
/*    ID.                                                                 */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    tls_session                           TLS control block             */
/*    named_curve                           Named curve ID                */
/*    curve_method                          Pointer to hold the curve     */
/*                                            method                      */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
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
UINT _nx_secure_tls_find_curve_method(struct NX_SECURE_TLS_SESSION_STRUCT *tls_session,
                                      USHORT named_curve, VOID **curve_method)
{
USHORT i;
NX_SECURE_TLS_ECC *ecc_info;

    if (tls_session == NX_NULL)
    {
        ecc_info = &_nx_secure_tls_ecc_info;
    }
    else
    {
        ecc_info = &(tls_session -> nx_secure_tls_ecc);
    }

    *curve_method = NX_NULL;
    /* Find out the curve method for the named curve. */
    for (i = 0; i < ecc_info -> nx_secure_tls_ecc_supported_groups_count; i++)
    {
        if (named_curve == ecc_info -> nx_secure_tls_ecc_supported_groups[i])
        {
            *curve_method = ((NX_CRYPTO_METHOD **)ecc_info -> nx_secure_tls_ecc_curves)[i];
            break;
        }
    }

    if(*curve_method == NX_NULL)
    {
        return(NX_CRYTPO_MISSING_ECC_CURVE);
    }

    return(NX_SUCCESS);
}


