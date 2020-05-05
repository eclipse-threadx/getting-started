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


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_secure_tls_ecc_process_clienthello_extensions   PORTABLE C      */
/*                                                          5.12          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Express Logic, Inc.                                 */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function processes the supported groups extensions included in */
/*    an incoming ClientHello message from a remote host.                 */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    tls_session                           TLS control block             */
/*    exts                                  Parsed extensions             */
/*    num_extensions                        Number of extensions          */
/*    selected_curve                        Output selected ECDHE curve   */
/*    cert_curve                            Named curve of local cert     */
/*    cert_curve_supported                  Output whether curve used by  */
/*                                            local cert is supported     */
/*    ecdhe_signature_algorithm             Output signature algorithm    */
/*                                            for ECDHE key exchange      */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _nx_secure_tls_find_curve_method      Find named curve used by cert */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    _nx_secure_tls_process_clienthello    Process ClientHello           */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  08-15-2019     Timothy Stapko           Initial Version 5.12          */
/*                                                                        */
/**************************************************************************/
UINT _nx_secure_tls_ecc_process_clienthello_extensions(struct NX_SECURE_TLS_SESSION_STRUCT *tls_session,
                                                       struct NX_SECURE_TLS_HELLO_EXTENSION_STRUCT *exts,
                                                       UINT num_extensions,
                                                       UINT *selected_curve, USHORT cert_curve,
                                                       UINT *cert_curve_supported,
                                                       USHORT *ecdhe_signature_algorithm)
{
UINT   status = NX_SUCCESS;
UINT   i, j;
const UCHAR *groups;
USHORT group;
USHORT groups_len;
NX_CRYPTO_METHOD *curve_method;

#define TLS_SIG_EXTENSION_SHA1_RSA     0x0201
#define TLS_SIG_EXTENSION_SHA256_RSA   0x0401
#define TLS_SIG_EXTENSION_SHA1_ECDSA   0x0203
#define TLS_SIG_EXTENSION_SHA256_ECDSA 0x0403

    /* Select SHA256 as the signature hash. */
    *ecdhe_signature_algorithm = TLS_SIG_EXTENSION_SHA256_RSA;


    *cert_curve_supported = NX_FALSE;
    *selected_curve = 0;

    for (i = 0; i < num_extensions; i++)
    {
        switch (exts[i].nx_secure_tls_extension_id)
        {
        case NX_SECURE_TLS_EXTENSION_EC_GROUPS:
            groups = exts[i].nx_secure_tls_extension_data;
            groups_len = exts[i].nx_secure_tls_extension_data_length;
            for (j = 0; j < groups_len; j += 2)
            {
                group = (USHORT)((groups[j] << 8) + groups[j + 1]);

                if (group == cert_curve)
                {

                    /* The curve used in the server certificate is supported by the client. */
                    *cert_curve_supported = NX_TRUE;
                }

                if (*selected_curve == 0)
                {
                    status = _nx_secure_tls_find_curve_method(tls_session, group, (VOID **)&curve_method);

                    if (status == NX_CRYTPO_MISSING_ECC_CURVE)
                    {

                        /* Keep searching list. */
                        continue;
                    }

                    if (status == NX_SUCCESS)
                    {

                        /* Found shared named curve. */
                        *selected_curve = group;
                    }
                    else
                    {

                        /* status is not NX_CRYTPO_MISSING_ECC_CURVE or NX_SUCCESS, return error. */
                        return(status);
                    }
                }
            }
            break;

        case NX_SECURE_TLS_EXTENSION_SIGNATURE_ALGORITHMS:
            break;
        }
    }

    return(status);
}

