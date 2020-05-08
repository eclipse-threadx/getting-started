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

static UCHAR _generated_hash[NX_SECURE_TLS_MAX_HASH_SIZE];

/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_secure_tls_verify_mac                           PORTABLE C      */
/*                                                           6.0          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function verifies the Message Authentication Code (MAC) that   */
/*    is included in encrypted TLS records. It hashes the incoming        */
/*    message data and then compares it to the MAC in the received        */
/*    record. If there is a mismatch, then the record has been corrupted  */
/*    in transit and represents a possible security breach.               */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    tls_session                           TLS control block             */
/*    header_data                           TLS record header data        */
/*    header_length                         Length of header data         */
/*    data                                  TLS record payload data       */
/*    length                                Length of payload data        */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _nx_secure_tls_hash_record            Generate payload data hash    */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    _nx_secure_tls_process_record         Process TLS record data       */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  05-19-2020     Timothy Stapko           Initial Version 6.0           */
/*                                                                        */
/**************************************************************************/
UINT _nx_secure_tls_verify_mac(NX_SECURE_TLS_SESSION *tls_session, UCHAR *header_data,
                               USHORT header_length, UCHAR *data, UINT *length)
{
UCHAR                                *mac_secret;
USHORT                                hash_size;
INT                                   compare_result;
USHORT                                data_length;
UCHAR                                *received_hash;
UINT                                  hash_length;
UCHAR                                 header[6];

    if (tls_session -> nx_secure_tls_session_ciphersuite == NX_NULL)
    {

        /* Likely internal error since at this point ciphersuite negotiation was theoretically completed. */
        return(NX_SECURE_TLS_UNKNOWN_CIPHERSUITE);
    }

    /* Get the hash size and MAC secret for our current session. */
    hash_size = tls_session -> nx_secure_tls_session_ciphersuite -> nx_secure_tls_hash_size;

    /* Select our proper MAC secret for hashing. */
    if (tls_session -> nx_secure_tls_socket_type == NX_SECURE_TLS_SESSION_TYPE_SERVER)
    {
        /* If we are a server, we need to use the client's MAC secret. */
        mac_secret = tls_session -> nx_secure_tls_key_material.nx_secure_tls_client_write_mac_secret;
    }
    else
    {
        /* We are a client, so use the server's MAC secret. */
        mac_secret = tls_session -> nx_secure_tls_key_material.nx_secure_tls_server_write_mac_secret;
    }

    /* Check for 0-length records. With nothing to hash, don't continue to hash generation. */
    if (hash_size >= *length)
    {

        if (header_data[0] == (UCHAR)(NX_SECURE_TLS_APPLICATION_DATA) &&
            *length == hash_size)
        {
            /* BEAST attack mitigation. In TLS 1.0 and SSLv3, the implicit IV enables the BEAST
               attack. Some implementations counter the attack by sending an empty record which
               has the effect of resetting the IVs. We normally don't allow empty records since there
               is no data to hash, but in this case we want to allow it. */
            *length = 0;

            /* Increment the sequence number. */
            if ((tls_session -> nx_secure_tls_remote_sequence_number[0] + 1) == 0)
            {
                /* Check for overflow of the 32-bit unsigned number. */
                tls_session -> nx_secure_tls_remote_sequence_number[1]++;
            }
            tls_session -> nx_secure_tls_remote_sequence_number[0]++;

            return(NX_SUCCESS);
        }

        /* The record data was smaller than the selected hash... Error. */
        return(NX_SECURE_TLS_INCORRECT_MESSAGE_LENGTH);
    }

    /* Adjust our length so we only hash the record data, not the hash as well. */
    data_length = (USHORT)(*length - hash_size);

    /* Copy the header data into our local buffer so we can change it if we need to. */
    if (header_length > sizeof(header))
    {
        return(NX_SECURE_TLS_HASH_MAC_VERIFY_FAILURE);
    }
    NX_SECURE_MEMCPY(header, header_data, header_length);

    /* Adjust the length in the header to match the length of the data before the hash was added. */
    header[3] = (UCHAR)((data_length >> 8) & 0x00FF);
    header[4] = (UCHAR)(data_length & 0x00FF);

    /* Generate the hash on the plaintext data. */
    _nx_secure_tls_hash_record(tls_session, tls_session -> nx_secure_tls_remote_sequence_number, header, header_length,
                               data, (USHORT)(data_length), _generated_hash, &hash_length, mac_secret);

    /* Increment the sequence number. */
    if ((tls_session -> nx_secure_tls_remote_sequence_number[0] + 1) == 0)
    {
        /* Check for overflow of the 32-bit unsigned number. */
        tls_session -> nx_secure_tls_remote_sequence_number[1]++;
    }
    tls_session -> nx_secure_tls_remote_sequence_number[0]++;

    /* Now, compare the hash we generated to the one we received. */
    received_hash = &data[data_length];
    compare_result = NX_SECURE_MEMCMP(received_hash, _generated_hash, hash_size);

#ifdef NX_SECURE_KEY_CLEAR
    NX_SECURE_MEMSET(_generated_hash, 0, sizeof(_generated_hash));
#endif /* NX_SECURE_KEY_CLEAR  */

    /* Before we return, adjust our data size so the caller will only see data, not the hash. */
    *length = data_length;

    /* If the hashes match, we are all good. Otherwise we have a problem. */
    if (compare_result == 0)
    {
        return(NX_SECURE_TLS_SUCCESS);
    }
    else
    {
        return(NX_SECURE_TLS_HASH_MAC_VERIFY_FAILURE);
    }
}

