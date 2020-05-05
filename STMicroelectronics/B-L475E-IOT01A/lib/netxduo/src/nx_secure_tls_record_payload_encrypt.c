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

static UINT _nx_secure_tls_record_data_encrypt(NX_SECURE_TLS_SESSION *tls_session, UCHAR *data, UINT *length,
                                               ULONG sequence_num[NX_SECURE_TLS_SEQUENCE_NUMBER_SIZE],
                                               UCHAR record_type);

static UCHAR _nx_secure_tls_record_block_buffer[NX_SECURE_TLS_MAX_CIPHER_BLOCK_SIZE];

/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_secure_tls_record_payload_encrypt               PORTABLE C      */
/*                                                           5.12         */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Express Logic, Inc.                                 */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function encrypts the payload of an outgoing TLS record using  */
/*    the session keys generated and ciphersuite determined during the    */
/*    TLS handshake.                                                      */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    tls_session                           TLS control block             */
/*    send_packet                           Pointer to packet data        */
/*    sequence_num                          Current TLS/DTLS message num  */
/*    record_type                           TLS record type               */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    [nx_crypto_operation]                 Decryption ciphers            */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    _nx_secure_dtls_send_record           Send DTLS encrypted record    */
/*    _nx_secure_tls_send_record            Send TLS encrypted record     */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  06-09-2017     Timothy Stapko           Initial Version 5.10          */
/*  12-15-2017     Timothy Stapko           Modified comment(s),          */
/*                                            supported more ciphersuites,*/
/*                                            supported ECJPAKE, supported*/
/*                                            data in packet chain,       */
/*                                            resulting in version 5.11   */
/*  08-15-2019     Timothy Stapko           Modified comment(s), added    */
/*                                            logic to clear encryption   */
/*                                            key and other secret data,  */
/*                                            added flexibility of using  */
/*                                            macros instead of direct C  */
/*                                            library function calls,     */
/*                                            added CBC mode checking for */
/*                                            chained packet, supported   */
/*                                            AES-GCM mode, fixed packet  */
/*                                            buffer overflow while       */
/*                                            padding for AES-CBC,        */
/*                                            removed cipher suite lookup,*/
/*                                            resulting in version 5.12   */
/*                                                                        */
/**************************************************************************/
UINT _nx_secure_tls_record_payload_encrypt(NX_SECURE_TLS_SESSION *tls_session, NX_PACKET *send_packet,
                                           ULONG sequence_num[NX_SECURE_TLS_SEQUENCE_NUMBER_SIZE],
                                           UCHAR record_type)
{
UINT                                  status;
NX_CRYPTO_METHOD                     *session_cipher_method;
UINT                                  block_size;
UCHAR                                *iv;
UINT                                  encrypted_length;
NX_PACKET                            *current_packet;
ULONG                                 current_length;
ULONG                                 rounded_length;
ULONG                                 remainder_length;
UINT                                  data_offset;
UCHAR                                 padding_length;


    if (tls_session -> nx_secure_tls_session_ciphersuite == NX_NULL)
    {

        /* Likely internal error since at this point ciphersuite negotiation was theoretically completed. */
        return(NX_SECURE_TLS_UNKNOWN_CIPHERSUITE);
    }

    /* Select the encryption algorithm based on the ciphersuite. Then, using the session keys and the chosen
       cipher, encrypt the data in place. */
    session_cipher_method = tls_session -> nx_secure_tls_session_ciphersuite -> nx_secure_tls_session_cipher;

    block_size = session_cipher_method -> nx_crypto_block_size_in_bytes;

    /* Make sure our block size is small enough to fit into our buffer. */
    NX_ASSERT(block_size <= NX_SECURE_TLS_MAX_CIPHER_BLOCK_SIZE);

    /* Select IV based on the current mode. */
    if (tls_session -> nx_secure_tls_socket_type == NX_SECURE_TLS_SESSION_TYPE_SERVER)
    {
        /* The socket is a TLS server, so use the server cipher to encrypt. */
        iv = tls_session -> nx_secure_tls_key_material.nx_secure_tls_server_iv;
    }
    else
    {
        /* The socket is a TLS client, so use the client cipher to encrypt. */
        iv = tls_session -> nx_secure_tls_key_material.nx_secure_tls_client_iv;
    }


    /* Iterate through the packet chain using a temporary pointer. */
    current_packet = send_packet;

    /* Offset into current packet data. */
    data_offset = 0;

    /* See if we need to add any data to the beginning of the payload such as an IV (e.g. for AES-CBC mode). */
    /* !!! NOTE: This relies on nx_secure_tls_packet_allocate reserving block_size bytes between NX_PACKET.nx_packet_prepend_ptr
                 and nx_packet_append_ptr !!! */
    switch (session_cipher_method -> nx_crypto_algorithm)
    {
    case NX_CRYPTO_ENCRYPTION_AES_CBC:
        /* TLS 1.0 does not use an explicit IV in CBC-mode ciphers, so don't include it
           in the record. */
        if (tls_session -> nx_secure_tls_protocol_version != NX_SECURE_TLS_VERSION_TLS_1_0)
        {

            if (block_size > ((ULONG)(current_packet -> nx_packet_data_end) - (ULONG)(current_packet -> nx_packet_prepend_ptr)))
            {

                /* Packet buffer too small. */
                return(NX_SECURE_TLS_PACKET_BUFFER_TOO_SMALL);
            }

            /* IV size is equal to the AES block size. Copy our IV into our data buffer
               at the head of the payload. */
            NX_SECURE_MEMCPY(current_packet -> nx_packet_prepend_ptr, iv, block_size);
            data_offset += block_size;
        }

        /* Padding - final output must be an integral multiple of the block size (16 bytes for AES
         * modes used in TLS). If the data is not a multiple, the padding consists of bytes each
         * with the value of the length of the padding (e.g. for 3 bytes, the padding would be 0x03,
         * 0x03, 0x03).
         */
        if (block_size > 0)
        {
            padding_length = (UCHAR)(block_size - (send_packet -> nx_packet_length % block_size));
            NX_SECURE_MEMSET(_nx_secure_tls_record_block_buffer, padding_length - 1, padding_length);
            status = nx_packet_data_append(send_packet, _nx_secure_tls_record_block_buffer,
                                       padding_length, tls_session -> nx_secure_tls_packet_pool,
                                       NX_WAIT_FOREVER);

            if (status)
            {
                return(status);
            }
        }
        break;
    default:
        /* Default, do nothing - IVs only for ciphers that need them. */
        break;
    }

    if ((current_packet -> nx_packet_next != NX_NULL) &&
        (session_cipher_method -> nx_crypto_algorithm != NX_CRYPTO_ENCRYPTION_AES_CBC))
    {

        /* Encryption of chained packet data is supported for CBC mode only. */
        return(NX_SECURE_TLS_UNSUPPORTED_CIPHER);
    }

    /* Loop through all packets in the chain. */
    do
    {
        /* Get our current packet length. Use the data_offset from any previous iterations. */
        current_length = (ULONG)(current_packet -> nx_packet_append_ptr - current_packet -> nx_packet_prepend_ptr) - data_offset;

        /* See if there are more packets in the chain. */
        if (current_packet -> nx_packet_next == NX_NULL)
        {
            /* Encrypt any remaining data in the current packet since it is our last. */
            encrypted_length = current_length;

            /* Offset should be such that the remaining data in the packet is remainder_length bytes. Add the block-boundary
               length to the previous offset to get this new offset. Also, add padding as this is the last bit of data to
               be encrypted (all previous packets should have been encrypted without padding).  */
            status = _nx_secure_tls_record_data_encrypt(tls_session, &current_packet -> nx_packet_prepend_ptr[data_offset], &encrypted_length, sequence_num, record_type);

            if (status != NX_SUCCESS)
            {
                return(status);
            }

            /* If encrypted length is longer than the expected length, some padding was added. Adjust packet accordingly. */
            current_packet -> nx_packet_append_ptr = current_packet -> nx_packet_append_ptr + (encrypted_length - current_length);
            send_packet -> nx_packet_length = send_packet -> nx_packet_length + (encrypted_length - current_length);
        }
        else
        {
            /* Figure out how much we can encrypt. Get an evenly-divisible
               block of data and the remainder. */
            if (block_size == 0)
            {
                remainder_length = 0;
            }
            else
            {
                remainder_length = (ULONG)((current_length % block_size));
            }
            rounded_length = current_length - remainder_length;

            /* Encrypt remaining data in the current packet from our previous offset to evenly-divisible block boundary. */
            encrypted_length = rounded_length;
            status = _nx_secure_tls_record_data_encrypt(tls_session, &current_packet -> nx_packet_prepend_ptr[data_offset], &encrypted_length, sequence_num, record_type);

            if (status != NX_SUCCESS)
            {
                return(status);
            }

            /* There should be no padding added, so the encrypted length should be the same as the plaintext length. */
            NX_ASSERT(encrypted_length == rounded_length);

            /* Copy data into temporary buffer for encryption.
               Pointers:                          packet2->prepend_ptr[data_offset]---v
               Lengths:   |  current length |  remainder |  block size - remainder    |  packet 2 remainder   |
               Data:      |**Packet 1 data***************|**Packet 2 data*************************************|
               Temporary:                   |   record block buffer  [block size]     |
             */
            encrypted_length = block_size;

            /* Offset for remainder bytes is rounded_length + data_offset. */
            NX_SECURE_MEMCPY(&_nx_secure_tls_record_block_buffer[0], &current_packet -> nx_packet_prepend_ptr[rounded_length + data_offset], remainder_length);
            NX_SECURE_MEMCPY(&_nx_secure_tls_record_block_buffer[remainder_length], current_packet -> nx_packet_next -> nx_packet_prepend_ptr, block_size - remainder_length);

            /* Encrypt the remainder block. */
            status = _nx_secure_tls_record_data_encrypt(tls_session, _nx_secure_tls_record_block_buffer, &encrypted_length, sequence_num, record_type);

            if (status != NX_SUCCESS)
            {

#ifdef NX_SECURE_KEY_CLEAR
                NX_SECURE_MEMSET(_nx_secure_tls_record_block_buffer, 0, block_size);
#endif /* NX_SECURE_KEY_CLEAR  */

                return(status);
            }

            /* Copy data from temporary buffer back into packets. */
            NX_SECURE_MEMCPY(&current_packet -> nx_packet_prepend_ptr[rounded_length + data_offset], &_nx_secure_tls_record_block_buffer[0], remainder_length);
            NX_SECURE_MEMCPY(current_packet -> nx_packet_next -> nx_packet_prepend_ptr, &_nx_secure_tls_record_block_buffer[remainder_length], block_size - remainder_length);

#ifdef NX_SECURE_KEY_CLEAR
            NX_SECURE_MEMSET(_nx_secure_tls_record_block_buffer, 0, block_size);
#endif /* NX_SECURE_KEY_CLEAR  */

            /* Finally, our new offset for the next round is the number of bytes we already
               encrypted (along with the remainder bytes) in the next packet. */
            data_offset = block_size - remainder_length;
        }

        /* Move to the next packet. */
        current_packet = current_packet -> nx_packet_next;
    } while (current_packet != NX_NULL);

    return(NX_SUCCESS);
}


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_secure_tls_record_data_encrypt                  PORTABLE C      */
/*                                                           5.12         */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Express Logic, Inc.                                 */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function is a static helper function used to encrypt a block   */
/*    of data separate from an NX_PACKET structure. This allows the       */
/*    caller (_nx_secure_tls_record_payload_encrypt) to handle data that  */
/*    spans multiple packets.                                             */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    tls_session                           TLS control block             */
/*    data                                  Pointer to packet data        */
/*    length                                Data length/return length     */
/*    sequence_num                          TLS message number            */
/*    record_type                           TLS record type               */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    [nx_crypto_operation]                 Decryption ciphers            */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    _nx_secure_tls_record_payload_encrypt Encrypt payload               */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  12-15-2017     Timothy Stapko           Initial Version 5.11          */
/*  08-15-2019     Timothy Stapko           Modified comment(s)           */
/*                                            optimized the logic, added  */
/*                                            support for AEAD cipher,    */
/*                                            fixed packet buffer overflow*/
/*                                            while padding for AES-CBC,  */
/*                                            removed cipher suite lookup,*/
/*                                            resulting in version 5.12   */
/*                                                                        */
/**************************************************************************/
static UINT _nx_secure_tls_record_data_encrypt(NX_SECURE_TLS_SESSION *tls_session, UCHAR *data,
                                               UINT *length,
                                               ULONG sequence_num[NX_SECURE_TLS_SEQUENCE_NUMBER_SIZE],
                                               UCHAR record_type)
{
UINT                                  status = NX_SUCCESS;
VOID                                 *crypto_method_metadata;
VOID                                 *handler = NX_NULL;
UCHAR                                *iv;
NX_CRYPTO_METHOD                     *session_cipher_method;
UINT                                  block_size;
UCHAR                                *key = NX_NULL;
#ifdef NX_SECURE_ENABLE_AEAD_CIPHER
UCHAR                                 additional_data[13];
UCHAR                                 nonce[13];
#else
    NX_PARAMETER_NOT_USED(sequence_num);
    NX_PARAMETER_NOT_USED(record_type);
#endif /* NX_SECURE_ENABLE_AEAD_CIPHER */

    /* Select the encryption algorithm based on the ciphersuite. Then, using the session keys and the chosen
       cipher, encrypt the data in place. */
    session_cipher_method = tls_session -> nx_secure_tls_session_ciphersuite -> nx_secure_tls_session_cipher;

    block_size = session_cipher_method -> nx_crypto_block_size_in_bytes;

    /* Select our cipher structure and IV based on the current mode. */
    if (tls_session -> nx_secure_tls_socket_type == NX_SECURE_TLS_SESSION_TYPE_SERVER)
    {
        /* The socket is a TLS server, so use the server cipher to encrypt. */
        crypto_method_metadata = tls_session -> nx_secure_session_cipher_metadata_area_server;
        handler = tls_session -> nx_secure_session_cipher_handler_server;
        iv = tls_session -> nx_secure_tls_key_material.nx_secure_tls_server_iv;
        if (session_cipher_method -> nx_crypto_init == NX_NULL)
        {
            key = tls_session -> nx_secure_tls_key_material.nx_secure_tls_server_write_key;
        }
    }
    else
    {
        /* The socket is a TLS client, so use the client cipher to encrypt. */
        crypto_method_metadata = tls_session -> nx_secure_session_cipher_metadata_area_client;
        handler = tls_session -> nx_secure_session_cipher_handler_client;
        iv = tls_session -> nx_secure_tls_key_material.nx_secure_tls_client_iv;
        if (session_cipher_method -> nx_crypto_init == NX_NULL)
        {
            key = tls_session -> nx_secure_tls_key_material.nx_secure_tls_client_write_key;
        }
    }

#ifdef NX_SECURE_ENABLE_AEAD_CIPHER
    if ((session_cipher_method -> nx_crypto_algorithm == NX_CRYPTO_ENCRYPTION_AES_CCM_8) ||
        (session_cipher_method -> nx_crypto_algorithm == NX_CRYPTO_ENCRYPTION_AES_CCM_12) ||
        (session_cipher_method -> nx_crypto_algorithm == NX_CRYPTO_ENCRYPTION_AES_CCM_16) ||
        (session_cipher_method -> nx_crypto_algorithm == NX_CRYPTO_ENCRYPTION_AES_GCM_16) ||
        NX_SECURE_AEAD_CIPHER_CHECK(session_cipher_method -> nx_crypto_algorithm))
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

        /* The length of CCMClientNonce or CCMServerNonce is 12 bytes.  */
        nonce[0] = 12;

        /* Copy client_write_IV or server_write_IV.  */
        NX_SECURE_MEMCPY(&nonce[1], iv, 4);

        /* Correct the endianness of our sequence number before hashing. */
        nonce[5] = (UCHAR)(sequence_num[1] >> 24);
        nonce[6] = (UCHAR)(sequence_num[1] >> 16);
        nonce[7] = (UCHAR)(sequence_num[1] >> 8);
        nonce[8] = (UCHAR)(sequence_num[1]);
        nonce[9] = (UCHAR)(sequence_num[0] >> 24);
        nonce[10] = (UCHAR)(sequence_num[0] >> 16);
        nonce[11] = (UCHAR)(sequence_num[0] >> 8);
        nonce[12] = (UCHAR)(sequence_num[0]);

        /*  additional_data = seq_num + TLSCompressed.type +
                        TLSCompressed.version + TLSCompressed.length;
         */
        NX_SECURE_MEMCPY(additional_data, &nonce[5], 8);
        additional_data[8]  = record_type;
        additional_data[9]  = (UCHAR)(tls_session -> nx_secure_tls_protocol_version >> 8);
        additional_data[10] = (UCHAR)(tls_session -> nx_secure_tls_protocol_version);
        additional_data[11] = (UCHAR)(*length >> 8);
        additional_data[12] = (UCHAR)(*length);

        /* Encrypt the message payload in place using the session crypto method. Note that the IV is not encrypted.*/
        if (session_cipher_method -> nx_crypto_operation != NX_NULL)
        {

            /* Set additional data pointer and length.  */
            status = session_cipher_method -> nx_crypto_operation(NX_CRYPTO_SET_ADDITIONAL_DATA,
                                                                  handler,
                                                                  session_cipher_method,
                                                                  NX_NULL,
                                                                  NX_NULL,
                                                                  additional_data,
                                                                  13,
                                                                  NX_NULL,
                                                                  NX_NULL,
                                                                  NX_NULL,
                                                                  crypto_method_metadata,
                                                                  tls_session -> nx_secure_session_cipher_metadata_size,
                                                                  NX_NULL, NX_NULL);

            if (status == NX_SUCCESS)
            {

                /* Encrypt the payload.  */
                status = session_cipher_method -> nx_crypto_operation(NX_CRYPTO_ENCRYPT,
                                                                      handler,
                                                                      session_cipher_method,
                                                                      key,
                                                                      session_cipher_method -> nx_crypto_key_size_in_bits,
                                                                      data,
                                                                      *length,
                                                                      nonce,
                                                                      data,
                                                                      *length + (session_cipher_method -> nx_crypto_ICV_size_in_bits >> 8),
                                                                      crypto_method_metadata,
                                                                      tls_session -> nx_secure_session_cipher_metadata_size,
                                                                      NX_NULL, NX_NULL);
            }

        }

#ifdef NX_SECURE_KEY_CLEAR
        NX_SECURE_MEMSET(additional_data, 0, sizeof(additional_data));
        NX_SECURE_MEMSET(nonce, 0, 5);
#endif /* NX_SECURE_KEY_CLEAR  */

        if (status != NX_SECURE_TLS_SUCCESS)
        {
            return(status);
        }

        *length = *length + (session_cipher_method -> nx_crypto_ICV_size_in_bits >> 3) + 8;

        /* Move the encrypted data.  */
        NX_SECURE_MEMMOVE(&data[8], data, *length - 8);

        /* Copy nonce_explicit to the beginning of the output.  */
        NX_SECURE_MEMCPY(data, &nonce[5], 8);
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
            return(NX_SUCCESS);
        }

        /* Encrypt the message payload in place using the session crypto method. Note that the IV is not encrypted.*/
        status = session_cipher_method -> nx_crypto_operation(NX_CRYPTO_ENCRYPT,
                                                              handler,
                                                              session_cipher_method,
                                                              key,
                                                              session_cipher_method -> nx_crypto_key_size_in_bits,
                                                              data,
                                                              *length,
                                                              iv,
                                                              data,
                                                              *length,
                                                              crypto_method_metadata,
                                                              tls_session -> nx_secure_session_cipher_metadata_size,
                                                              NX_NULL, NX_NULL);

        /* CBC-mode ciphers need to have their IV's updated after encryption. */
        if (session_cipher_method -> nx_crypto_algorithm == NX_CRYPTO_ENCRYPTION_AES_CBC)
        {
            /* New IV is the last encrypted block of the output. */
            NX_SECURE_MEMCPY(iv, &data[*length - block_size], block_size);
        }
    }

    return(status);
}

