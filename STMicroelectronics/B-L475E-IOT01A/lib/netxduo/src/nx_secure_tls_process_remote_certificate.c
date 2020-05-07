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
/*    _nx_secure_tls_process_remote_certificate           PORTABLE C      */
/*                                                           5.12         */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Express Logic, Inc.                                 */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function processes an incoming TLS server certificate message, */
/*    extracting the RSA or DH public key and verifying the validity of   */
/*    the certificate. It parses the X509 certificate and fills in the    */
/*    relevant information if the caller has allocated space for it using */
/*    nx_secure_remote_certificate_allocate.                              */
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
/*    _nx_secure_tls_remote_certificate_verify                            */
/*                                          Verify the server certificate */
/*    _nx_secure_x509_certificate_list_add  Add incoming cert to store    */
/*    _nx_secure_x509_certificate_parse     Extract public key data       */
/*    _nx_secure_x509_free_certificate_get  Get free cert for storage     */
/*    tx_mutex_get                          Get protection mutex          */
/*    tx_mutex_put                          Put protection mutex          */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    _nx_secure_dtls_client_handshake      DTLS client state machine     */
/*    _nx_secure_tls_client_handshake       TLS client state machine      */
/*    _nx_secure_tls_server_handshake       TLS server state machine      */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  06-09-2017     Timothy Stapko           Initial Version 5.10          */
/*  12-15-2017     Timothy Stapko           Modified comment(s),          */
/*                                            optimized the logic,        */
/*                                            resulting in version 5.11   */
/*  08-15-2019     Timothy Stapko           Modified comment(s),          */
/*                                            free remote certificates    */
/*                                            after validation complete,  */
/*                                            added flexibility of using  */
/*                                            macros instead of direct C  */
/*                                            library function calls,     */
/*                                            fixed the usage of crypto   */
/*                                            metadata for hash method,   */
/*                                            fix issues with proper      */
/*                                            CertificateVerify handling, */
/*                                            allow duplicate certificates*/
/*                                            for servers that send them, */
/*                                            add remote certificate      */
/*                                            memory optimization,        */
/*                                            static analysis bug fixes,  */
/*                                            resulting in version 5.12   */
/*                                                                        */
/**************************************************************************/
UINT _nx_secure_tls_process_remote_certificate(NX_SECURE_TLS_SESSION *tls_session,
                                               UCHAR *packet_buffer, UINT message_length)
{
UINT                 length;
UINT                 total_length;
UINT                 cert_length = 0;
UINT                 status;
NX_SECURE_X509_CERT *certificate;
UCHAR               *endpoint_raw_ptr;
UINT                 endpoint_length;
UINT                 bytes_processed;
UCHAR               *cert_buffer;
ULONG               cert_buf_size;


    /* Structure:
     * |      3       |                     <Total Length>                      |
     * | Total length |       3        |   <Cert[0] Length> | 3 + <Cert[x] Len> |
     * |              | Cert[0] Length |   Certificate[0]   |    More certs...  |
     */


    /* At this point, the packet buffer is filled with a TLS record. We can use the remainder of
        the buffer to hold certificate structures for parsing. The remote certificates will
        remain in the packet buffer and only the X.509 parsing structure (NX_SECURE_X509_CERT) 
        will be allocated. See _nx_secure_tls_process_remote_certificate for more info. */
    /* Typical layout of packet buffer and certificate buffer. Cert buffer allocates top-down, X.509 parsing structures
        are allocated and used only until the certificate chain verification step. After that, the remote certs are cleared
        and then the endpoint certificate is copied into the cert buffer (only the endpoint) for later use. The packet buffer
        following this function (on success) should have the following layout (assuming no user-allocated certs):
        |                      Packet buffer                     |    Certificate buffer    | 
        |<-----------data------------------>|-->  free space  <--| Endpoint Cert 1 | X.509  | 
    */
    /* Certificate buffer is at the end of the record in the record assembly buffer. */
    cert_buffer = &tls_session->nx_secure_tls_packet_buffer[message_length];

    /* The size of the buffer is the remaining space in the record assembly buffer. */
    cert_buf_size = tls_session -> nx_secure_tls_packet_buffer_size - message_length;

    /* Use our length as an index into the buffer. */
    length = 0;


    /* Extract the certificate(s) from the incoming data, starting with. */
    total_length = (UINT)((packet_buffer[0] << 16) + (packet_buffer[1] << 8) + packet_buffer[2]);
    length = length + 3;

    /* Make sure what we extracted makes sense. */
    if (total_length > message_length)
    {
        return(NX_SECURE_TLS_INCORRECT_MESSAGE_LENGTH);
    }

    /* See if remote host sent an empty certificate message. */
    if (total_length == 0)
    {
        /*  No certificate received! */
        return(NX_SECURE_TLS_EMPTY_REMOTE_CERTIFICATE_RECEIVED);
    }

    /* Keep subtracting from the total length until no more certificates left. */
    while (total_length > 0)
    {
        /* Extract the next certificate's length. */
        cert_length = (UINT)((packet_buffer[length] << 16) + (packet_buffer[length + 1] << 8) + packet_buffer[length + 2]);
        length = length + 3;

        /* Make sure the individual cert length makes sense. */
        if ((cert_length + 3) > total_length)
        {
            return(NX_SECURE_TLS_INCORRECT_MESSAGE_LENGTH);
        }

        /* Advance the variable of total_length. */
        total_length -= (3 + cert_length);

        /* Get a reference to the remote endpoint certificate that was allocated earlier. */
        status = _nx_secure_x509_free_certificate_get(&tls_session -> nx_secure_tls_credentials.nx_secure_tls_certificate_store,
                                                      &certificate);

        /* If there are no free certificates, attempt to allocate from the packet reassembly buffer 
           (the certificate buffer is carved from the packet buffer in nx_secure_tls_process_record). */
        if (status != NX_SUCCESS)
        {
            /* No remote certificates added. Instead try extracting space from packet buffer. */
            if(cert_buf_size < sizeof(NX_SECURE_X509_CERT))
            {
                /* Not enough space to allocate the X.509 structure. */
                return(NX_SECURE_TLS_INSUFFICIENT_CERT_SPACE);                
            }

            /* Get space for the parsing structure. */
            cert_buf_size -= sizeof(NX_SECURE_X509_CERT);
            certificate = (NX_SECURE_X509_CERT*)(&cert_buffer[cert_buf_size]);

            /* Point structure to certificate being parsed. Note that the certificate 
               structure points directly into the packet buffer where the certificate
               is located - this certificate structure must NOT be used outside this function. */
            certificate -> nx_secure_x509_certificate_raw_data_length = cert_length;
            certificate -> nx_secure_x509_certificate_raw_data = &packet_buffer[length];
            certificate -> nx_secure_x509_certificate_raw_buffer_size = cert_length;
            certificate -> nx_secure_x509_user_allocated_cert = NX_FALSE;
        }
        else
        {
            /* Make sure we have enough space to save our certificate. */
            if (certificate -> nx_secure_x509_certificate_raw_buffer_size < cert_length)
            {
                return(NX_SECURE_TLS_INSUFFICIENT_CERT_SPACE);
            }

            /* Copy the certificate from the packet buffer into our allocated certificate space. */
            certificate -> nx_secure_x509_certificate_raw_data_length = cert_length;
            NX_SECURE_MEMCPY(certificate -> nx_secure_x509_certificate_raw_data, &packet_buffer[length], cert_length);
        }
        length += cert_length;
        
        /* Release the protection. */
        tx_mutex_put(&_nx_secure_tls_protection);

        /* Parse the DER-encoded X509 certificate to extract the public key data. */
        status = _nx_secure_x509_certificate_parse(certificate -> nx_secure_x509_certificate_raw_data, cert_length, &bytes_processed, certificate);

        /* Get the protection. */
        tx_mutex_get(&_nx_secure_tls_protection, TX_WAIT_FOREVER);

        /* Make sure we parsed a valid certificate. */
        if (status != NX_SUCCESS)
        {
            return(status);
        }

        /* Assign the TLS Session metadata areas to the certificate for later use. */
        certificate -> nx_secure_x509_public_cipher_metadata_area = tls_session -> nx_secure_public_cipher_metadata_area;
        certificate -> nx_secure_x509_public_cipher_metadata_size = tls_session -> nx_secure_public_cipher_metadata_size;

        certificate -> nx_secure_x509_hash_metadata_area = tls_session -> nx_secure_hash_mac_metadata_area;
        certificate -> nx_secure_x509_hash_metadata_size = tls_session -> nx_secure_hash_mac_metadata_size;

        /* Add the certificate to the remote store. */
        /* Parse and initialize the remote certificate for use in subsequent operations. Allow duplicates in case
           the server is mis-configured and sends the same certificate twice. */
        status = _nx_secure_x509_certificate_list_add(&tls_session -> nx_secure_tls_credentials.nx_secure_tls_certificate_store.nx_secure_x509_remote_certificates,
                                                      certificate, NX_TRUE);

        /* Make sure we parsed a valid certificate. */
        if (status != NX_SUCCESS)
        {
            return(status);
        }

        /* Make sure the certificate has it's cipher table initialized. */
        certificate -> nx_secure_x509_cipher_table = tls_session -> nx_secure_tls_crypto_table -> nx_secure_tls_x509_cipher_table;
        certificate -> nx_secure_x509_cipher_table_size = tls_session -> nx_secure_tls_crypto_table -> nx_secure_tls_x509_cipher_table_size;
    }

    /* =============================== CERTIFICATE CHAIN VERIFICATION ======================================== */
    /* Verify the certificates we received are valid against the trusted store. */
    status = _nx_secure_tls_remote_certificate_verify(tls_session);

    if(status != NX_SUCCESS)
    {
        return(status);
    }    

    /* ======================== Save off the endpoint certificate for later use. =============================*/
    /* Get the endpoint from the certificates we just parsed. */
    status = _nx_secure_x509_remote_endpoint_certificate_get(&tls_session -> nx_secure_tls_credentials.nx_secure_tls_certificate_store,
                                                            &certificate);
    if(status != NX_SUCCESS)                                                             
    {
        return(status);
    }    

    /* If the endpoint certificate was NOT allocated by the user application, we need to
        make a copy and save it for later use. */
    if(!certificate -> nx_secure_x509_user_allocated_cert)
    { 
        /* Free all certificates that we added to the packet buffer. Do this before the 
           call to nx_secure_x509_free_certificate_get so that if there are user-allocated
           certificates the endpoint is put into one of them. */
        status = _nx_secure_tls_remote_certificate_free_all(tls_session);

        if(status != NX_SUCCESS)
        {
            return(status);
        }

        /* Save the raw pointer to endpoint so we can clear remote certificate state. */
        endpoint_raw_ptr = certificate->nx_secure_x509_certificate_raw_data;
        endpoint_length = certificate->nx_secure_x509_certificate_raw_data_length;

        /* Now allocate a new certificate for the endpoint. */
        status = _nx_secure_x509_free_certificate_get(&tls_session -> nx_secure_tls_credentials.nx_secure_tls_certificate_store,
                                                    &certificate);

        /* If there are no free certificates, attempt to allocate from the packet reassembly buffer 
            (the certificate buffer is carved from the packet buffer in nx_secure_tls_process_record). */
        if (status != NX_SUCCESS)
        {
            /* No remote certificates added. Instead try extracting space from packet buffer. */
            cert_buffer = &tls_session->nx_secure_tls_packet_buffer[message_length];
            cert_buf_size = tls_session->nx_secure_tls_packet_buffer_size - message_length;

            if(cert_buf_size < sizeof(NX_SECURE_X509_CERT))
            {
                /* Not enough space to allocate the X.509 structure. */
                return(NX_SECURE_TLS_INSUFFICIENT_CERT_SPACE);                
            }

            /* Get space for the parsing structure. */
            cert_buf_size -= sizeof(NX_SECURE_X509_CERT);
            certificate = (NX_SECURE_X509_CERT*)(&cert_buffer[cert_buf_size]);

            if(cert_buf_size < endpoint_length)
            {
                /* Not enough space to allocate the raw certificate data. */
                return(NX_SECURE_TLS_INSUFFICIENT_CERT_SPACE);                
            }

            /* Allocate space for the endpoint certificate. */
            cert_buf_size -= endpoint_length;

            /* Point structure to certificate being parsed. */
            certificate -> nx_secure_x509_certificate_raw_data = &cert_buffer[cert_buf_size];
            certificate -> nx_secure_x509_certificate_raw_buffer_size = endpoint_length;

            /* Update total remaining size. */
            tls_session->nx_secure_tls_packet_buffer_size -= cert_buf_size;
        }

        /* Copy the certificate data to the end of the certificate buffer or use an allocated certificate. */
        certificate -> nx_secure_x509_certificate_raw_data_length = endpoint_length;
        NX_SECURE_MEMCPY(certificate->nx_secure_x509_certificate_raw_data, endpoint_raw_ptr, endpoint_length);
        
        /* Release the protection. */
        tx_mutex_put(&_nx_secure_tls_protection);

        /* Re-parse the certificate using the original data. */
        status = _nx_secure_x509_certificate_parse(certificate -> nx_secure_x509_certificate_raw_data, cert_length, &bytes_processed, certificate);

        /* Get the protection. */
        tx_mutex_get(&_nx_secure_tls_protection, TX_WAIT_FOREVER);

        /* Make sure we parsed a valid certificate. */
        if (status != NX_SUCCESS)
        {
            return(status);
        }
    
        /* Re-add the remote endpoint certificate for later use. */
        status = _nx_secure_x509_certificate_list_add(&tls_session -> nx_secure_tls_credentials.nx_secure_tls_certificate_store.nx_secure_x509_remote_certificates,
                                                        certificate, NX_TRUE);

        if (status != NX_SUCCESS)
        {
            return(status);
        }

        /* Make sure the certificate has it's cipher table initialized. */
        certificate -> nx_secure_x509_cipher_table = tls_session -> nx_secure_tls_crypto_table -> nx_secure_tls_x509_cipher_table;
        certificate -> nx_secure_x509_cipher_table_size = tls_session -> nx_secure_tls_crypto_table -> nx_secure_tls_x509_cipher_table_size;    
    }
#ifdef NX_SECURE_TLS_CLIENT_DISABLED
    /* If TLS Client is disabled and we have processed a ServerCertificate message, something is wrong... */
    tls_session -> nx_secure_tls_server_state = NX_SECURE_TLS_SERVER_STATE_ERROR;

    return(NX_SECURE_TLS_INVALID_STATE);
#else
    /* Set our state to indicate we successfully parsed the Certificate message. */
    tls_session -> nx_secure_tls_client_state = NX_SECURE_TLS_CLIENT_STATE_SERVER_CERTIFICATE;

    return(status);
#endif
}

