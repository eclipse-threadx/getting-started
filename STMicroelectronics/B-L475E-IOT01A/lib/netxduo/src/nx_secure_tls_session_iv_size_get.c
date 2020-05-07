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
/*    _nx_secure_tls_session_iv_size_get                  PORTABLE C      */
/*                                                           5.12         */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Express Logic, Inc.                                 */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function returns the size in bytes needed by the current       */
/*    TLS session cipher (e.g. AES) if the encrypted session is active.   */
/*    If the session is not encrypted (e.g. during initial handshake) or  */
/*    the cipher does not have an inlined IV, 0 is returned.              */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    tls_session                           TLS control block             */
/*    iv_size                               Size of IV in bytes           */
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
/*    _nx_secure_dtls_packet_allocate       Allocate internal DTLS packet */
/*    _nx_secure_dtls_send_record           Send the DTLS record          */
/*    _nx_secure_tls_packet_allocate        Allocate internal TLS packet  */
/*    _nx_secure_tls_send_record            Send TLS records              */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  12-15-2017     Timothy Stapko           Initial Version 5.11          */
/*  08-15-2019     Timothy Stapko           Modified comment(s),          */
/*                                            fixed IV size for TLS 1.0,  */
/*                                            removed cipher suite lookup,*/
/*                                            resulting in version 5.12   */
/*                                                                        */
/**************************************************************************/
UINT _nx_secure_tls_session_iv_size_get(NX_SECURE_TLS_SESSION *tls_session, USHORT *iv_size)
{
NX_CRYPTO_METHOD                     *session_cipher_method;


    /* If TLS session is active, allocate space for the IV that precedes the data in
       certain ciphersuites. */
    if (tls_session -> nx_secure_tls_local_session_active)
    {
        if (tls_session -> nx_secure_tls_session_ciphersuite == NX_NULL)
        {

            /* Likely internal error since at this point ciphersuite negotiation was theoretically completed. */
            return(NX_SECURE_TLS_UNKNOWN_CIPHERSUITE);
        }

        /* Select the encryption algorithm based on the ciphersuite. If the ciphersuite needs extra space
           for an IV or other data before the payload, this will return the number of bytes needed. */
        session_cipher_method = tls_session -> nx_secure_tls_session_ciphersuite -> nx_secure_tls_session_cipher;

        /* Check the crypto algorithm for any special processing. */
        switch (session_cipher_method -> nx_crypto_algorithm)
        {
        case NX_CRYPTO_ENCRYPTION_AES_CBC:
            /* TLS 1.0 does not use an explicit IV in CBC-mode ciphers, so don't include it
               in the record. */
            if (tls_session -> nx_secure_tls_protocol_version != NX_SECURE_TLS_VERSION_TLS_1_0)
            {
                /* Return size of cipher method IV. */
                *iv_size = (session_cipher_method -> nx_crypto_IV_size_in_bits >> 3);
            }
            else
            {
                *iv_size = 0;
            }
            break;
        default:
            /* Default, do nothing - only allocate space for ciphers that need it. */
            *iv_size = 0;
            break;
        }
    }
    else
    {
        /* Session is not active so we don't need to allocate space for an IV. */
        *iv_size = 0;
    }

    return(NX_SUCCESS);
}

