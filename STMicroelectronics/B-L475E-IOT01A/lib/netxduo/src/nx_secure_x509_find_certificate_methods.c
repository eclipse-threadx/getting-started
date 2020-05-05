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
#include "nx_secure_x509.h"

/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_secure_x509_find_certificate_methods            PORTABLE C      */
/*                                                           5.12         */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Express Logic, Inc.                                 */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function finds crypto methods specified in a certificate.      */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    cert                                  Pointer to X509 certificate   */
/*    signature_algorithm                   Id for signature method       */
/*    crypto_methods                        Return matching table entry   */
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
/*    _nx_secure_x509_certificate_verify    Verify a certificate          */
/*    _nx_secure_x509_crl_verify            Verify revocation list        */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  06-09-2017     Timothy Stapko           Initial Version 5.10          */
/*  12-15-2017     Timothy Stapko           Modified comment(s),          */
/*                                            updated API to take an ID   */
/*                                            for the algorithm desired,  */
/*                                            resulting in version 5.11   */
/*  08-15-2019     Timothy Stapko           Modified comment(s),          */
/*                                            resulting in version 5.12   */
/*                                                                        */
/**************************************************************************/
UINT _nx_secure_x509_find_certificate_methods(NX_SECURE_X509_CERT *cert, USHORT signature_algorithm,
                                              NX_SECURE_X509_CRYPTO **crypto_methods)
{
SHORT index;

    /* The number of ciphersuites is very small so a linear search should be fine. */
    for (index = 0; index < cert -> nx_secure_x509_cipher_table_size; ++index)
    {
        /* See if the ciphersuite is supported. */
        if (cert -> nx_secure_x509_cipher_table[index].nx_secure_x509_crypto_identifier == signature_algorithm)
        {
            *crypto_methods = &cert -> nx_secure_x509_cipher_table[index];
            return(NX_SUCCESS);
        }
    }

    /* No entry found, crypto routines unknown. */
    *crypto_methods = NX_NULL;
    return(NX_SECURE_TLS_UNKNOWN_CERT_SIG_ALGORITHM);
}

