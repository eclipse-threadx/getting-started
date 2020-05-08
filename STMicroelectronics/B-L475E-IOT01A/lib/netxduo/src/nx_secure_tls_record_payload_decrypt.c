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

#if (NX_SECURE_TLS_TLS_1_0_ENABLED)
static UCHAR save_iv[20]; /* Must be large enough to hold the block size for session ciphers! */
#endif

/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_secure_tls_record_payload_decrypt               PORTABLE C      */
/*                                                           6.0          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function decrypts the payload of an incoming TLS record using  */
/*    the session keys generated and ciphersuite determined during the    */
/*    TLS handshake.                                                      */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    tls_session                           TLS control block             */
/*    data                                  Pointer to message data       */
/*    length                                Length of message data (bytes)*/
/*    sequence_num                          Record sequence number        */
/*    record_type                           Record type                   */
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
/*    _nx_secure_dtls_process_record        Process DTLS record data      */
/*    _nx_secure_tls_process_record         Process TLS record data       */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  05-19-2020     Timothy Stapko           Initial Version 6.0           */
/*                                                                        */
/**************************************************************************/
UINT _nx_secure_tls_record_payload_decrypt(NX_SECURE_TLS_SESSION *tls_session, UCHAR *data,
                                           UINT *length,
                                           ULONG sequence_num[NX_SECURE_TLS_SEQUENCE_NUMBER_SIZE],
                                           UCHAR record_type)
{
UINT                                  status;
VOID                                 *crypto_method_metadata;
VOID                                 *handler = NX_NULL;
UCHAR                                *iv;
UCHAR                                 padding_length;
const NX_CRYPTO_METHOD               *session_cipher_method;
USHORT                                iv_size;
UCHAR                                *encrypted_data;
UCHAR                                *key = NX_NULL;
UINT                                  i;
#ifdef NX_SECURE_ENABLE_AEAD_CIPHER
UINT                                  icv_size;
UCHAR                                 additional_data[13];
UINT                                  additional_data_size;
UCHAR                                 nonce[13];
UINT                                  data_offset;
#else
    NX_PARAMETER_NOT_USED(sequence_num);
    NX_PARAMETER_NOT_USED(record_type);
#endif /* NX_SECURE_ENABLE_AEAD_CIPHER */


    if (tls_session -> nx_secure_tls_session_ciphersuite == NX_NULL)
    {

        /* Likely internal error since at this point ciphersuite negotiation was theoretically completed. */
        return(NX_SECURE_TLS_UNKNOWN_CIPHERSUITE);
    }

    /* Select the encryption algorithm based on the ciphersuite. Then, using the session keys and the chosen
       cipher, encrypt the data in place. */
    session_cipher_method = tls_session -> nx_secure_tls_session_ciphersuite -> nx_secure_tls_session_cipher;

    /* Get the size of the IV used by the session cipher. */
    iv_size = session_cipher_method -> nx_crypto_IV_size_in_bits >> 3;

    /* Select our proper data structures. */
    if (tls_session -> nx_secure_tls_socket_type == NX_SECURE_TLS_SESSION_TYPE_SERVER)
    {
        /* The socket is a TLS server, so use the *CLIENT* cipher to decrypt. */
        crypto_method_metadata = tls_session -> nx_secure_session_cipher_metadata_area_client;
        handler = tls_session -> nx_secure_session_cipher_handler_client;
        iv = tls_session -> nx_secure_tls_key_material.nx_secure_tls_client_iv;
        if (session_cipher_method -> nx_crypto_init == NX_NULL)
        {
            key = tls_session -> nx_secure_tls_key_material.nx_secure_tls_client_write_key;
        }
    }
    else
    {
        /* The socket is a TLS client, so use the *SERVER* cipher to decrypt. */
        crypto_method_metadata = tls_session -> nx_secure_session_cipher_metadata_area_server;
        handler = tls_session -> nx_secure_session_cipher_handler_server;
        iv = tls_session -> nx_secure_tls_key_material.nx_secure_tls_server_iv;
        if (session_cipher_method -> nx_crypto_init == NX_NULL)
        {
            key = tls_session -> nx_secure_tls_key_material.nx_secure_tls_server_write_key;
        }
    }

#ifdef NX_SECURE_ENABLE_AEAD_CIPHER
    if ((session_cipher_method -> nx_crypto_algorithm == NX_CRYPTO_ENCRYPTION_AES_CCM_8) ||
        (session_cipher_method -> nx_crypto_algorithm == NX_CRYPTO_ENCRYPTION_AES_CCM_12) ||
        (session_cipher_method -> nx_crypto_algorithm == NX_CRYPTO_ENCRYPTION_AES_CCM_16) ||
        (session_cipher_method -> nx_crypto_algorithm == NX_CRYPTO_ENCRYPTION_AES_GCM_16) ||
        NX_SECURE_AEAD_CIPHER_CHECK(session_cipher_method -> nx_crypto_algorithm))
    {
#if (NX_SECURE_TLS_TLS_1_3_ENABLED)
        /* TLS 1.3 AEAD uses a different Nonce construction from earlier versions. */
        if(tls_session->nx_secure_tls_1_3)
        {
            /*
             Each AEAD algorithm will specify a range of possible lengths for the
             per-record nonce, from N_MIN bytes to N_MAX bytes of input [RFC5116].
             The length of the TLS per-record nonce (iv_length) is set to the
             larger of 8 bytes and N_MIN for the AEAD algorithm (see [RFC5116],
             Section 4).  An AEAD algorithm where N_MAX is less than 8 bytes
             MUST NOT be used with TLS.  The per-record nonce for the AEAD
             construction is formed as follows:

             1.  The 64-bit record sequence number is encoded in network byte
                 order and padded to the left with zeros to iv_length.

             2.  The padded sequence number is XORed with either the static
                 client_write_iv or server_write_iv (depending on the role).

             The resulting quantity (of length iv_length) is used as the
             per-record nonce.

             Note: This is a different construction from that in TLS 1.2, which
             specified a partially explicit nonce.
             */

            icv_size = (session_cipher_method -> nx_crypto_ICV_size_in_bits >> 3);

            /* The length of the nonce is 12 bytes.  */
            nonce[0] = 12;

            /* Copy client_write_IV or server_write_IV.  */
            NX_SECURE_MEMCPY(&nonce[1], iv, 12);

            /* Correct the endianness of our sequence number and XOR with
             * the IV. Pad to the left with zeroes. */
            nonce[1]  = (UCHAR)(nonce[1] ^ 0);
            nonce[2]  = (UCHAR)(nonce[2] ^ 0);
            nonce[3]  = (UCHAR)(nonce[3] ^ 0);
            nonce[4]  = (UCHAR)(nonce[4] ^ 0);
            nonce[5]  = (UCHAR)(nonce[5] ^ (sequence_num[1] >> 24));
            nonce[6]  = (UCHAR)(nonce[6] ^ (sequence_num[1] >> 16));
            nonce[7]  = (UCHAR)(nonce[7] ^ (sequence_num[1] >> 8));
            nonce[8]  = (UCHAR)(nonce[8] ^ (sequence_num[1]));
            nonce[9]  = (UCHAR)(nonce[9] ^ (sequence_num[0] >> 24));
            nonce[10] = (UCHAR)(nonce[10] ^ (sequence_num[0] >> 16));
            nonce[11] = (UCHAR)(nonce[11] ^ (sequence_num[0] >> 8));
            nonce[12] = (UCHAR)(nonce[12] ^ (sequence_num[0]));

            /*  additional_data = record header
             *  record header = TLSCiphertext.opaque_type ||
                                TLSCiphertext.legacy_record_version ||
                                TLSCiphertext.length
             */
            additional_data[0] = record_type;
            additional_data[1] = (UCHAR)(0x03);
            additional_data[2] = (UCHAR)(0x03);
            additional_data[3] = (UCHAR)((*length) >> 8);
            additional_data[4] = (UCHAR)(*length);
            //additional_data[3] = (UCHAR)((*length - icv_size) >> 8);
            //additional_data[4] = (UCHAR)(*length - icv_size);

            
            /* We have 5 bytes of additional data. */
            additional_data_size = 5;
            
            /* Decrypt from beginning of data. */
            data_offset = 0;
        }
        else
#endif
        {
            /* AEAD ciphers structure:
                 struct {
                     opaque nonce_explicit[SecurityParameters.record_iv_length];
                     aead-ciphered struct {
                         opaque content[TLSCompressed.length];
                     };
                 } GenericAEADCipher;
             */

            /* The nonce of the CCM cipher is passed into crypto method using iv_ptr.
               struct {
                   uint32 client_write_IV; // low order 32-bits
                   uint64 seq_num;         // TLS sequence number
               } CCMClientNonce.
               struct {
                   uint32 server_write_IV; // low order 32-bits
                   uint64 seq_num; // TLS sequence number
               } CCMServerNonce.
             */


            icv_size = (session_cipher_method -> nx_crypto_ICV_size_in_bits >> 3);

            if (*length < (8 + icv_size))
            {
                return(NX_SECURE_TLS_AEAD_DECRYPT_FAIL);
            }

            /* Remove length of nonce_explicit.  */
            *length = *length - 8;

            /* The length of CCMClientNonce or CCMServerNonce is 12 bytes.  */
            nonce[0] = 12;

            /* Copy client_write_IV or server_write_IV.  */
            NX_SECURE_MEMCPY(&nonce[1], iv, 4);

            /* Correct the endianness of our sequence number before hashing. */
            additional_data[0] = (UCHAR)(sequence_num[1] >> 24);
            additional_data[1] = (UCHAR)(sequence_num[1] >> 16);
            additional_data[2] = (UCHAR)(sequence_num[1] >> 8);
            additional_data[3] = (UCHAR)(sequence_num[1]);
            additional_data[4] = (UCHAR)(sequence_num[0] >> 24);
            additional_data[5] = (UCHAR)(sequence_num[0] >> 16);
            additional_data[6] = (UCHAR)(sequence_num[0] >> 8);
            additional_data[7] = (UCHAR)(sequence_num[0]);

            /* Copy nonce_explicit from the data.  */
            NX_SECURE_MEMCPY(&nonce[5], data, 8);

            /*  additional_data = seq_num + TLSCompressed.type +
                            TLSCompressed.version + TLSCompressed.length;
             */
            additional_data[8]  = record_type;
            additional_data[9]  = (UCHAR)(tls_session -> nx_secure_tls_protocol_version >> 8);
            additional_data[10] = (UCHAR)(tls_session -> nx_secure_tls_protocol_version);
            additional_data[11] = (UCHAR)((*length - icv_size) >> 8);
            additional_data[12] = (UCHAR)(*length - icv_size);

            /* We have 13 bytes of additional data (8 bytes seq num + 5 bytes header). */
            additional_data_size = 13;

            /* Decrypt data following the nonce_explicit. */
            data_offset = 8;
        }

        if (*length < icv_size)
        {
            return(NX_SECURE_TLS_AEAD_DECRYPT_FAIL);
        }

        /* Decrypt the message payload using the session crypto method, and move the decrypted data to the beginning of the buffer. */
        if (session_cipher_method -> nx_crypto_operation != NX_NULL)
        {

            /* Set additional data pointer and length.  */
            status = session_cipher_method -> nx_crypto_operation(NX_CRYPTO_SET_ADDITIONAL_DATA,
                                                         handler,
                                                         (NX_CRYPTO_METHOD*)session_cipher_method,
                                                         NX_NULL,
                                                         NX_NULL,
                                                         additional_data,
                                                         additional_data_size,
                                                         NX_NULL,
                                                         NX_NULL,
                                                         NX_NULL,
                                                         crypto_method_metadata,
                                                         tls_session -> nx_secure_session_cipher_metadata_size,
                                                         NX_NULL, NX_NULL);

            if(status != NX_CRYPTO_SUCCESS)
            {
                return(status);
            }                                                     

            /* Decrypt the payload.  */
            status = session_cipher_method -> nx_crypto_operation(NX_CRYPTO_DECRYPT,
                                                                  handler,
                                                                  (NX_CRYPTO_METHOD*)session_cipher_method,
                                                                  key,
                                                                  session_cipher_method -> nx_crypto_key_size_in_bits,
                                                                  &data[data_offset],
                                                                  *length,
                                                                  nonce,
                                                                  &data[data_offset],
                                                                  *length,
                                                                  crypto_method_metadata,
                                                                  tls_session -> nx_secure_session_cipher_metadata_size,
                                                                  NX_NULL, NX_NULL);
#ifdef NX_SECURE_KEY_CLEAR
            NX_SECURE_MEMSET(additional_data, 0, sizeof(additional_data));
            NX_SECURE_MEMSET(nonce, 0, sizeof(nonce));
#endif /* NX_SECURE_KEY_CLEAR  */

            if (status == NX_CRYPTO_AUTHENTICATION_FAILED)
            {
                return(NX_SECURE_TLS_AEAD_DECRYPT_FAIL);
            }

            if (status != NX_SECURE_TLS_SUCCESS)
            {
                return(status);
            }
        }

        /* Remove ICV length of the CCM cipher. */
        *length = *length - icv_size;

        /* Move the decrypted data.  */
        if(data_offset != 0)
        {                             
            NX_SECURE_MEMMOVE(data, &data[data_offset], *length);
        }
    }
    else
#endif /* NX_SECURE_ENABLE_AEAD_CIPHER */
    {
        /* CBC mode has a specific structure for encrypted data, so handle that here:
               block-ciphered struct {
                   opaque IV[CipherSpec.block_length];
                   opaque content[TLSCompressed.length];
                   opaque MAC[CipherSpec.hash_size];
                   uint8 padding[GenericBlockCipher.padding_length];
                   uint8 padding_length;
               } GenericBlockCipher;
         */

        if (session_cipher_method -> nx_crypto_operation == NX_NULL)
        {
            return(NX_SECURE_TLS_SUCCESS);
        }

        /* Pointer to our encrypted data. */
        encrypted_data = data;

#if (NX_SECURE_TLS_TLS_1_0_ENABLED)
        if ((iv_size > *length) ||
            (iv_size > sizeof(save_iv)))
#else
        if (iv_size > *length)
#endif
        {

            /* Message length error. */
            return(NX_SECURE_TLS_INCORRECT_MESSAGE_LENGTH);
        }

#if (NX_SECURE_TLS_TLS_1_0_ENABLED)
        /* TLS 1.0 does not include the IV in the record, so use the one from the session.*/
        if (tls_session -> nx_secure_tls_protocol_version == NX_SECURE_TLS_VERSION_TLS_1_0)
        {
            if (session_cipher_method -> nx_crypto_algorithm == NX_CRYPTO_ENCRYPTION_AES_CBC)
            {
                /* New IV is the last encrypted block of the output. */
                NX_SECURE_MEMCPY(save_iv, &encrypted_data[*length - iv_size], iv_size);
            }
        }
        else /* TLS 1.1, 1.2 */
#endif
        {
            /* Copy IV from the beginning of the payload into our session buffer. */
            NX_SECURE_MEMCPY(iv, data, iv_size);

            /* Adjust payload length to account for IV that we saved off above. */
            *length -= iv_size;

            /* Adjust our encrypted data pointer to account for the IV. */
            encrypted_data = &data[iv_size];
        }

        /* Decrypt the message payload using the session crypto method, and move the decrypted data to the beginning of the buffer. */
        status = session_cipher_method -> nx_crypto_operation(NX_CRYPTO_DECRYPT,
                                                     handler,
                                                     (NX_CRYPTO_METHOD*)session_cipher_method,
                                                     key,
                                                     session_cipher_method -> nx_crypto_key_size_in_bits,
                                                     encrypted_data,
                                                     *length,
                                                     iv,
                                                     data,
                                                     *length,
                                                     crypto_method_metadata,
                                                     tls_session -> nx_secure_session_cipher_metadata_size,
                                                     NX_NULL, NX_NULL);

        if(status != NX_CRYPTO_SUCCESS)
        {
            return(status);
        }                                                     

        /* Get padding length from the final byte - CBC padding consists of a number of
           bytes each with a value equal to the padding length (e.g. 0x3 0x3 0x3 for 3 bytes of padding). */
        padding_length = data[*length - 1];

        /* If padding length is greater than our length,
          we have an error - don't check padding. */
        if (padding_length <= *length)
        {
            /* Check all padding values. */
            for(i = 0; i < padding_length; ++i)
            {
                if(data[*length - 1 - i] != padding_length)
                {
                    /* Padding byte is incorrect! */
                    status = NX_SECURE_TLS_PADDING_CHECK_FAILED;
                    break;
                }
            }
            /* If we get here, the padding checks out. */
            status = NX_SUCCESS;
        }
        else
        {
            /* Decryption or padding error! */
            status = NX_SECURE_TLS_PADDING_CHECK_FAILED;
        }

        if(status == NX_SUCCESS)
        {
            /* Adjust length to remove padding. */
            *length = (UINT)(*length - (UINT)(padding_length + 1));

        }

#if (NX_SECURE_TLS_TLS_1_0_ENABLED)
        /* Update our IV for CBC mode. */
        if (tls_session -> nx_secure_tls_protocol_version == NX_SECURE_TLS_VERSION_TLS_1_0)
        {
            if (session_cipher_method -> nx_crypto_algorithm == NX_CRYPTO_ENCRYPTION_AES_CBC)
            {
                NX_SECURE_MEMCPY(iv, save_iv, iv_size);
            }
        }
#endif

        /* Return error status if appropriate. */
        if(status != NX_SUCCESS)
        {
            return(status);
        }
    }

    return(NX_SUCCESS);
}

