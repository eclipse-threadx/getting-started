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
/*    _nx_secure_tls_generate_premaster_secret            PORTABLE C      */
/*                                                           5.12         */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Express Logic, Inc.                                 */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function generates the Pre-Master Secret for TLS Client        */
/*    instances. It is sent to the remote host and used as the seed for   */
/*    session key generation.                                             */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    tls_session                           TLS control block             */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _nx_secure_tls_psk_find               Find PSK from store           */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    _nx_secure_dtls_client_handshake      DTLS client state machine     */
/*    _nx_secure_tls_client_handshake       TLS client state machine      */
/*    _nx_secure_tls_process_client_key_exchange                          */
/*                                          Process ClientKeyExchange     */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  06-09-2017     Timothy Stapko           Initial Version 5.10          */
/*  12-15-2017     Timothy Stapko           Modified comment(s),          */
/*                                            supported ECJPAKE,          */
/*                                            resulting in version 5.11   */
/*  08-15-2019     Timothy Stapko           Modified comment(s), and      */
/*                                            added flexibility of using  */
/*                                            macros instead of direct C  */
/*                                            library function calls,     */
/*                                            added extension hook, change*/
/*                                            casting of NX_RAND output to*/
/*                                            to guaranteed 32-bit,       */
/*                                            improved packet length      */
/*                                            verification, removed       */
/*                                            cipher suite lookup,        */
/*                                            resulting in version 5.12   */
/*                                                                        */
/**************************************************************************/
UINT _nx_secure_tls_generate_premaster_secret(NX_SECURE_TLS_SESSION *tls_session)
{
UINT                                  *buffer_ptr;
UINT                                  i;
UINT                                  status = NX_SECURE_TLS_SUCCESS;
#ifdef NX_SECURE_ENABLE_PSK_CIPHERSUITES
UCHAR                                *psk_data;
UINT                                  psk_length;
UINT                                  index;
#endif

    NX_SECURE_GENERATE_PREMASTER_SECRET_EXTENSION

    if (tls_session -> nx_secure_tls_session_ciphersuite == NX_NULL)
    {

        /* Likely internal error since at this point ciphersuite negotiation was theoretically completed. */
        return(NX_SECURE_TLS_UNKNOWN_CIPHERSUITE);
    }

#ifdef NX_SECURE_ENABLE_PSK_CIPHERSUITES
    /* Check for PSK ciphersuites. */
    if (tls_session -> nx_secure_tls_session_ciphersuite -> nx_secure_tls_public_auth -> nx_crypto_algorithm == NX_CRYPTO_KEY_EXCHANGE_PSK)
    {
        /* Now, using the identity as a key, find the PSK in our PSK store. */
        if (tls_session -> nx_secure_tls_socket_type == NX_SECURE_TLS_SESSION_TYPE_SERVER)
        {
            /* Server just uses its PSK. */
            psk_data = tls_session -> nx_secure_tls_credentials.nx_secure_tls_psk_store[0].nx_secure_tls_psk_data;
            psk_length = tls_session -> nx_secure_tls_credentials.nx_secure_tls_psk_store[0].nx_secure_tls_psk_data_size;
        }
        else
        {
            /*  Client has to search for the PSK based on the identity hint. */
            status = _nx_secure_tls_psk_find(tls_session, &psk_data, &psk_length, tls_session -> nx_secure_tls_credentials.nx_secure_tls_remote_psk_id,
                                             tls_session -> nx_secure_tls_credentials.nx_secure_tls_remote_psk_id_size);

            if (status != NX_SUCCESS)
            {
                return(status);
            }
        }

        /* From RFC 4279:
           The premaster secret is formed as follows: if the PSK is N octets
           long, concatenate a uint16 with the value N, N zero octets, a second
           uint16 with the value N, and the PSK itself.
         |   2   |   <N>  |   2   |   <N>   |
         |   N   |    0   |   N   |   PSK   |
         */
        index = 0;

        if ((2 + psk_length + 2 + psk_length) > sizeof(tls_session -> nx_secure_tls_key_material.nx_secure_tls_pre_master_secret))
        {

            /* No more PSK space. */
            return(NX_SECURE_TLS_NO_MORE_PSK_SPACE);
        }

        tls_session -> nx_secure_tls_key_material.nx_secure_tls_pre_master_secret[0] = (UCHAR)(psk_length >> 8);
        tls_session -> nx_secure_tls_key_material.nx_secure_tls_pre_master_secret[1] = (UCHAR)psk_length;
        index += 2;

        NX_SECURE_MEMSET(&tls_session -> nx_secure_tls_key_material.nx_secure_tls_pre_master_secret[index], 0, psk_length);
        index += psk_length;

        tls_session -> nx_secure_tls_key_material.nx_secure_tls_pre_master_secret[index] = (UCHAR)(psk_length >> 8);
        tls_session -> nx_secure_tls_key_material.nx_secure_tls_pre_master_secret[index + 1] = (UCHAR)psk_length;
        index += 2;

        NX_SECURE_MEMCPY(&tls_session -> nx_secure_tls_key_material.nx_secure_tls_pre_master_secret[index], psk_data, psk_length);
        index += psk_length;

        /* Save the pre-master secret size for later use. */
        tls_session -> nx_secure_tls_key_material.nx_secure_tls_pre_master_secret_size = 2 + psk_length + 2 + psk_length;

        /* We are using PSK for our credentials and now that we have generated keys we can consider the
           remote host's credentials to have been received. */
        tls_session -> nx_secure_tls_received_remote_credentials = NX_TRUE;

        return(NX_SECURE_TLS_SUCCESS);
    }
#endif

#ifdef NX_SECURE_ENABLE_ECJPAKE_CIPHERSUITE
    /* When using ECJ-PAKE ciphersuite, pre-master secret is already generated. */
    if (tls_session -> nx_secure_tls_session_ciphersuite -> nx_secure_tls_public_auth -> nx_crypto_algorithm == NX_CRYPTO_KEY_EXCHANGE_ECJPAKE)
    {
        /* If using EC-JPAKE, credentials are passed differently - if we get here credentials should be OK. */
        tls_session -> nx_secure_tls_received_remote_credentials = NX_TRUE;

        return(NX_SECURE_TLS_SUCCESS);
    }
#endif

    /* Generate the Pre-Master Secret that is used to generate the key material
       used in the session. For TLS 1.1, the secret consists of two bytes
       representing the highest protocol version the client supports, followed
       by 46 random bytes. */
    buffer_ptr = (UINT *)tls_session -> nx_secure_tls_key_material.nx_secure_tls_pre_master_secret;

    /* Generate 48 bytes of random data, fill in the version afterwards. */
    for (i = 0; i < 12; i++)
    {
        /* Fill with 12 ULONG randoms, then fix first two bytes to protocol version after. */
        *(buffer_ptr + i) = (UINT)NX_RAND();
    }

    /* First two bytes are our client version. */
    buffer_ptr[0] = ((ULONG)tls_session -> nx_secure_tls_protocol_version << 16) | (buffer_ptr[0] & 0x0000FFFF);
    NX_CHANGE_ULONG_ENDIAN(buffer_ptr[0]);

    tls_session -> nx_secure_tls_key_material.nx_secure_tls_pre_master_secret_size = 48;

    return(status);
}

