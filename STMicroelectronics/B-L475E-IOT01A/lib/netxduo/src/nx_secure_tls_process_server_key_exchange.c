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
/*    _nx_secure_tls_process_server_key_exchange           PORTABLE C     */
/*                                                           5.12         */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Express Logic, Inc.                                 */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function processes an incoming ServerKeyExchange message,      */
/*    which is sent by the remote TLS Server host when certain            */
/*    ciphersuites (e.g. those using Diffie-Hellman) are used.            */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    tls_session                           TLS control block             */
/*    packet_buffer                         Pointer to message data       */
/*    message_length                        Length of message data (bytes)*/
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    [nx_crypto_operation]                 Crypto operation              */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    _nx_secure_dtls_client_handshake      DTLS client state machine     */
/*    _nx_secure_tls_client_handshake       TLS client state machine      */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  06-09-2017     Timothy Stapko           Initial Version 5.10          */
/*  12-15-2017     Timothy Stapko           Modified comment(s),          */
/*                                            supported ECJPAKE,          */
/*                                            resulting in version 5.11   */
/*  08-15-2019     Timothy Stapko           Modified comment(s)           */
/*                                            added operation method for  */
/*                                            elliptic curve cryptography,*/
/*                                            passed crypto handle into   */
/*                                            crypto internal functions,  */
/*                                            added extension hook,       */
/*                                            fixed endian issue, updated */
/*                                            error return codes,         */
/*                                            removed cipher suite lookup,*/
/*                                            resulting in version 5.12   */
/*                                                                        */
/**************************************************************************/
UINT _nx_secure_tls_process_server_key_exchange(NX_SECURE_TLS_SESSION *tls_session,
                                                UCHAR *packet_buffer, UINT message_length)
{

#ifdef NX_SECURE_ENABLE_PSK_CIPHERSUITES
USHORT                                length;
#endif /* NX_SECURE_ENABLE_PSK_CIPHERSUITES */
#if defined(NX_SECURE_ENABLE_PSK_CIPHERSUITES) || defined(NX_SECURE_ENABLE_ECJPAKE_CIPHERSUITE)
UINT                                  status;
const NX_SECURE_TLS_CIPHERSUITE_INFO *ciphersuite;
#endif /* defined(NX_SECURE_ENABLE_PSK_CIPHERSUITES) || defined(NX_SECURE_ENABLE_ECJPAKE_CIPHERSUITE) */

    NX_PARAMETER_NOT_USED(tls_session);
    NX_PARAMETER_NOT_USED(packet_buffer);
    NX_PARAMETER_NOT_USED(message_length);

    NX_SECURE_PROCESS_SERVER_KEY_EXCHANGE_EXTENSION

#if defined(NX_SECURE_ENABLE_PSK_CIPHERSUITES) || defined(NX_SECURE_ENABLE_ECJPAKE_CIPHERSUITE)
    /* Figure out which ciphersuite we are using. */
    ciphersuite = tls_session -> nx_secure_tls_session_ciphersuite;
    if (ciphersuite == NX_NULL)
    {

        /* Likely internal error since at this point ciphersuite negotiation was theoretically completed. */
        return(NX_SECURE_TLS_UNKNOWN_CIPHERSUITE);
    }
#endif /* defined(NX_SECURE_ENABLE_PSK_CIPHERSUITES) || defined(NX_SECURE_ENABLE_ECJPAKE_CIPHERSUITE) */

#ifdef NX_SECURE_ENABLE_PSK_CIPHERSUITES
    /* Check for PSK ciphersuites. */
    if (ciphersuite -> nx_secure_tls_public_auth -> nx_crypto_algorithm == NX_CRYPTO_KEY_EXCHANGE_PSK)
    {
        /* Get identity hint length. */
        length = (USHORT)((packet_buffer[0] << 8) + (USHORT)packet_buffer[1]);
        packet_buffer += 2;

        if (length > message_length || length > NX_SECURE_TLS_MAX_PSK_ID_SIZE)
        {
            /* The payload is larger than the header indicated. */
            return(NX_SECURE_TLS_INCORRECT_MESSAGE_LENGTH);
        }

        /* Extract the identity hint and save in the TLS Session. Then when pre-master is generated
           in client_handshake, we can do the right thing... */
        NX_SECURE_MEMCPY(tls_session -> nx_secure_tls_credentials.nx_secure_tls_remote_psk_id, &packet_buffer[0], length);
        tls_session -> nx_secure_tls_credentials.nx_secure_tls_remote_psk_id_size = length;
    }
#endif

#ifdef NX_SECURE_ENABLE_ECJPAKE_CIPHERSUITE
    /* Check for ECJ-PAKE ciphersuites. */
    if (ciphersuite -> nx_secure_tls_public_auth -> nx_crypto_algorithm == NX_CRYPTO_KEY_EXCHANGE_ECJPAKE)
    {
        /* Make sure curve type is named_curve (3). */
        if (packet_buffer[0] != 3)
        {
            return(NX_SECURE_TLS_UNSUPPORTED_ECC_FORMAT);
        }

        /* Only secp256r1 (23) is supported.  */
        if (packet_buffer[1] != 0 || packet_buffer[2] != 23)
        {
            return(NX_SECURE_TLS_UNSUPPORTED_ECC_CURVE);
        }

        packet_buffer += 3;
        tls_session -> nx_secure_tls_key_material.nx_secure_tls_pre_master_secret_size = 32;

        status = ciphersuite -> nx_secure_tls_public_auth -> nx_crypto_operation(NX_CRYPTO_ECJPAKE_SERVER_KEY_EXCHANGE_PROCESS,
                                                                                 tls_session -> nx_secure_public_auth_handler,
                                                                                 ciphersuite -> nx_secure_tls_public_auth,
                                                                                 NX_NULL, 0,
                                                                                 packet_buffer,
                                                                                 message_length,
                                                                                 NX_NULL,
                                                                                 tls_session -> nx_secure_tls_key_material.nx_secure_tls_pre_master_secret,
                                                                                 tls_session -> nx_secure_tls_key_material.nx_secure_tls_pre_master_secret_size,
                                                                                 tls_session -> nx_secure_public_auth_metadata_area,
                                                                                 tls_session -> nx_secure_public_auth_metadata_size,
                                                                                 NX_NULL, NX_NULL);
        if (status)
        {
            return(status);
        }

        if (ciphersuite -> nx_secure_tls_public_auth -> nx_crypto_cleanup)
        {
            status = ciphersuite -> nx_secure_tls_public_auth -> nx_crypto_cleanup(tls_session -> nx_secure_public_auth_metadata_area);

            if (status)
            {
                return(status);
            }
        }
    }
#endif

    return(NX_SECURE_TLS_SUCCESS);
}

