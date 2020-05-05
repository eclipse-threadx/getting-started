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
/*    _nx_secure_tls_ecc_generate_premaster_secret        PORTABLE C      */
/*                                                          5.12          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Express Logic, Inc.                                 */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function generates the Pre-Master Secret when ECDH key         */
/*    exchange is used.                                                   */
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
/*    _nx_secure_x509_remote_endpoint_certificate_get                     */
/*                                          Get cert for remote host      */
/*    _nx_secure_tls_find_curve_method      Find named curve used in cert */
/*    [nx_crypto_operation]                 Public-key ciphers            */
/*    [nx_crypto_init]                      Initialize Crypto Method      */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  08-15-2019     Timothy Stapko           Initial Version 5.12          */
/*                                                                        */
/**************************************************************************/
UINT _nx_secure_tls_ecc_generate_premaster_secret(struct NX_SECURE_TLS_SESSION_STRUCT *tls_session)
{
UINT                                  status;
NX_SECURE_X509_CERT                  *server_certificate;
NX_CRYPTO_METHOD                     *curve_method_cert;
NX_CRYPTO_METHOD                     *ecdh_method;
NX_SECURE_EC_PUBLIC_KEY              *ec_pubkey;
VOID                                 *handler = NX_NULL;
NX_CRYPTO_EXTENDED_OUTPUT             extended_output;

    if (tls_session -> nx_secure_tls_session_ciphersuite == NX_NULL)
    {

        /* Likely internal error since at this point ciphersuite negotiation was theoretically completed. */
        return(NX_SECURE_TLS_UNKNOWN_CIPHERSUITE);
    }

    if (tls_session -> nx_secure_tls_session_ciphersuite -> nx_secure_tls_public_cipher -> nx_crypto_algorithm == NX_CRYPTO_KEY_EXCHANGE_ECDHE)
    {

        return(NX_SECURE_TLS_SUCCESS);
    }
    else if (tls_session -> nx_secure_tls_session_ciphersuite -> nx_secure_tls_public_cipher -> nx_crypto_algorithm == NX_CRYPTO_KEY_EXCHANGE_ECDH)
    {
        /* Get reference to remote server certificate so we can find out the named curve. */
        status = _nx_secure_x509_remote_endpoint_certificate_get(&tls_session -> nx_secure_tls_credentials.nx_secure_tls_certificate_store,
                                                                 &server_certificate);
        if (status || server_certificate == NX_NULL)
        {
            /* No certificate found, error! */
            return(NX_SECURE_TLS_CERTIFICATE_NOT_FOUND);
        }

        ec_pubkey = (NX_SECURE_EC_PUBLIC_KEY *)&server_certificate -> nx_secure_x509_public_key;

        /* Find out which named curve the remote certificate is using. */
        status = _nx_secure_tls_find_curve_method(tls_session, (USHORT)(ec_pubkey -> nx_secure_ec_named_curve), (VOID **)&curve_method_cert);

        if(status != NX_SUCCESS)
        {
            return(status);
        }

        if (curve_method_cert == NX_NULL)
        {
            /* The remote certificate is using an unsupported curve. */
            return(NX_SECURE_TLS_UNSUPPORTED_ECC_CURVE);
        }

        ecdh_method = tls_session -> nx_secure_tls_session_ciphersuite -> nx_secure_tls_public_cipher;
        if (ecdh_method -> nx_crypto_operation == NX_NULL)
        {
            return(NX_SECURE_TLS_MISSING_CRYPTO_ROUTINE);
        }

        if (ecdh_method -> nx_crypto_init != NX_NULL)
        {
            status = ecdh_method -> nx_crypto_init(ecdh_method,
                                          NX_NULL,
                                          0,
                                          &handler,
                                          tls_session -> nx_secure_public_cipher_metadata_area,
                                          tls_session -> nx_secure_public_cipher_metadata_size);
            if(status != NX_SUCCESS)
            {
                return(status);
            }
        }

        status = ecdh_method -> nx_crypto_operation(NX_CRYPTO_EC_CURVE_SET, handler,
                                                    ecdh_method, NX_NULL, 0,
                                                    (UCHAR *)curve_method_cert, sizeof(NX_CRYPTO_METHOD *), NX_NULL,
                                                    NX_NULL, 0,
                                                    tls_session -> nx_secure_public_cipher_metadata_area,
                                                    tls_session -> nx_secure_public_cipher_metadata_size,
                                                    NX_NULL, NX_NULL);
        if (status != NX_CRYPTO_SUCCESS)
        {
            return(status);
        }

        /* Store public key in the nx_secure_tls_new_key_material_data. */
        extended_output.nx_crypto_extended_output_data = &tls_session -> nx_secure_tls_key_material.nx_secure_tls_new_key_material_data[1];
        extended_output.nx_crypto_extended_output_length_in_byte = sizeof(tls_session -> nx_secure_tls_key_material.nx_secure_tls_new_key_material_data) - 1;
        extended_output.nx_crypto_extended_output_actual_size = 0;
        status = ecdh_method -> nx_crypto_operation(NX_CRYPTO_DH_SETUP, handler,
                                                    ecdh_method, NX_NULL, 0,
                                                    NX_NULL, 0, NX_NULL,
                                                    (UCHAR *)&extended_output,
                                                    sizeof(extended_output),
                                                    tls_session -> nx_secure_public_cipher_metadata_area,
                                                    tls_session -> nx_secure_public_cipher_metadata_size,
                                                    NX_NULL, NX_NULL);
        if (status != NX_CRYPTO_SUCCESS)
        {
            return(status);
        }

        tls_session -> nx_secure_tls_key_material.nx_secure_tls_new_key_material_data[0] = (UCHAR)extended_output.nx_crypto_extended_output_actual_size;

        extended_output.nx_crypto_extended_output_data = tls_session -> nx_secure_tls_key_material.nx_secure_tls_pre_master_secret;
        extended_output.nx_crypto_extended_output_length_in_byte = sizeof(tls_session -> nx_secure_tls_key_material.nx_secure_tls_pre_master_secret);
        extended_output.nx_crypto_extended_output_actual_size = 0;
        status = ecdh_method -> nx_crypto_operation(NX_CRYPTO_DH_CALCULATE, handler,
                                                    ecdh_method, NX_NULL, 0,
                                                    (UCHAR *)ec_pubkey -> nx_secure_ec_public_key,
                                                    ec_pubkey -> nx_secure_ec_public_key_length, NX_NULL,
                                                    (UCHAR *)&extended_output,
                                                    sizeof(extended_output),
                                                    tls_session -> nx_secure_public_cipher_metadata_area,
                                                    tls_session -> nx_secure_public_cipher_metadata_size,
                                                    NX_NULL, NX_NULL);
        if (status != NX_CRYPTO_SUCCESS)
        {
            return(status);
        }

        tls_session -> nx_secure_tls_key_material.nx_secure_tls_pre_master_secret_size = extended_output.nx_crypto_extended_output_actual_size;

        if (ecdh_method -> nx_crypto_cleanup)
        {
            status = ecdh_method -> nx_crypto_cleanup(tls_session -> nx_secure_public_cipher_metadata_area);
            if(status != NX_SUCCESS)
            {
                return(status);
            }
        }

        return(NX_SECURE_TLS_SUCCESS);
    }

    return(NX_CONTINUE);
}

