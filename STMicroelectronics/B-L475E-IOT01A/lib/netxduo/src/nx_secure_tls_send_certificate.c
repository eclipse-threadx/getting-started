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
/*    _nx_secure_tls_send_certificate                     PORTABLE C      */
/*                                                           5.12         */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Express Logic, Inc.                                 */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function populates an NX_PACKET with the TLS Certificate       */
/*    message, which contains the identity certificate for this device.   */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    tls_session                           TLS control block             */
/*    send_packet                           Packet to be filled           */
/*    wait_option                           Controls timeout actions      */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _nx_secure_x509_distinguished_name_compare                          */
/*                                          Compare distinguished name    */
/*    _nx_secure_x509_local_device_certificate_get                        */
/*                                          Get local certificate to send */
/*    nx_packet_data_append                 Append data to packet         */
/*    tx_mutex_get                          Get protection mutex          */
/*    tx_mutex_put                          Put protection mutex          */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    _nx_secure_dtls_client_handshake      DTLS client state machine     */
/*    _nx_secure_dtls_server_handshake      DTLS server state machine     */
/*    _nx_secure_tls_client_handshake       TLS client state machine      */
/*    _nx_secure_tls_server_handshake       TLS server state machine      */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  06-09-2017     Timothy Stapko           Initial Version 5.10          */
/*  12-15-2017     Timothy Stapko           Modified comment(s),          */
/*                                            fixed compiler warnings,    */
/*                                            supported empty certificate */
/*                                            messages, released mutex    */
/*                                            before block operation,     */
/*                                            optimized the logic, added  */
/*                                            compare bit fields,         */
/*                                            resulting in version 5.11   */
/*  08-15-2019     Timothy Stapko           Modified comment(s), and      */
/*                                            improved internal logic,    */
/*                                            added wait_option, improved */
/*                                            packet length verification, */
/*                                            resulting in version 5.12   */
/*                                                                        */
/**************************************************************************/
UINT _nx_secure_tls_send_certificate(NX_SECURE_TLS_SESSION *tls_session, NX_PACKET *send_packet,
                                     ULONG wait_option)
{
UINT                 length;
UINT                 total_length;
UCHAR                length_buffer[3];
UINT                 status = NX_SECURE_TLS_SUCCESS;
NX_SECURE_X509_CERT *cert;
INT                  compare_result = 0;
UCHAR               *record_start;


    /* See if the local certificate has been overridden. If so, use that instead. */
    if (tls_session -> nx_secure_tls_credentials.nx_secure_tls_active_certificate != NX_NULL)
    {
        cert = tls_session -> nx_secure_tls_credentials.nx_secure_tls_active_certificate;
    }
    else
    {
        /* Get reference to local device certificate. NX_NULL is passed for name to get default entry. */
        status = _nx_secure_x509_local_device_certificate_get(&tls_session -> nx_secure_tls_credentials.nx_secure_tls_certificate_store,
                                                              NX_NULL, &cert);
    }

#ifndef NX_SECURE_TLS_CLIENT_DISABLED
    /* See if this is a TLS client sending a certificate in response to a certificate request from
       the remote server. */
    if (tls_session -> nx_secure_tls_socket_type == NX_SECURE_TLS_SESSION_TYPE_CLIENT &&
        (status == NX_SECURE_TLS_CERTIFICATE_NOT_FOUND))
    {
        /* If this is a TLS client, as per the RFC we can have no certificate assigned in which
           case our response to the server that has requested our certificate will contain
           an empty certificate field. */
        cert = NX_NULL;

        /* Clear the requested flag so no further certificate-specific messages are sent. */
        tls_session -> nx_secure_tls_client_certificate_requested = NX_FALSE;
    }
    else
#endif
    {
        if (status)
        {
            /* No certificate found, error! */
            return(status);
        }
    }

    if (((ULONG)(send_packet -> nx_packet_data_end) - (ULONG)(send_packet -> nx_packet_append_ptr)) < 3u)
    {

        /* Packet buffer is too small to hold random and ID. */
        return(NX_SECURE_TLS_PACKET_BUFFER_TOO_SMALL);
    }

    /* Save a reference to the start of our record. */
    record_start = send_packet -> nx_packet_append_ptr;

    /* Pointer to where we are going to place the next certificate. */
    /* The first 3 bytes hold the total length field.  Therefore
       certificate data starts from offset 3.*/
    send_packet -> nx_packet_append_ptr = send_packet -> nx_packet_append_ptr + 3;
    send_packet -> nx_packet_length = send_packet -> nx_packet_length + (USHORT)(3);

    /* Total length is the length of all certificates and is the first 3 bytes of the message. */
    total_length = 0;

    while (status == NX_SUCCESS)
    {
        /* Place certificate data into the packet buffer with the appropriate length.
           NOTE: We need to use the RAW data of the certificate when sending, not the parsed certificate! */
        length = cert -> nx_secure_x509_certificate_raw_data_length;

        /* Total length is increased by the length of the certificate plus the 3 bytes for
           the certificate length parameter. */
        total_length += (length + 3);

        /* Put the length of this certificate into the buffer. */
        length_buffer[0] = (UCHAR)((length & 0xFF0000) >> 16);
        length_buffer[1] = (UCHAR)((length & 0xFF00) >> 8);
        length_buffer[2] = (UCHAR)(length & 0xFF);

        /* Release the protection before suspending on nx_packet_data_append. */
        tx_mutex_put(&_nx_secure_tls_protection);

        /* Put the length into the buffer. */
        status = nx_packet_data_append(send_packet, length_buffer, 3,
                                       tls_session -> nx_secure_tls_packet_pool, wait_option);

        /* Get the protection after nx_packet_data_append. */
        tx_mutex_get(&_nx_secure_tls_protection, TX_WAIT_FOREVER);

        if (status != NX_SUCCESS)
        {
            return(status);
        }

        /* Release the protection before suspending on nx_packet_data_append. */
        tx_mutex_put(&_nx_secure_tls_protection);

        /* Put the certificate data into the buffer. */
        status = nx_packet_data_append(send_packet, cert -> nx_secure_x509_certificate_raw_data, length,
                                       tls_session -> nx_secure_tls_packet_pool, wait_option);

        /* Get the protection after nx_packet_data_append. */
        tx_mutex_get(&_nx_secure_tls_protection, TX_WAIT_FOREVER);

        if (status != NX_SUCCESS)
        {
            return(status);
        }

        /* Get certificate issuer - if it exists in the store, send it, otherwise we are done. */
        status = _nx_secure_x509_local_device_certificate_get(&tls_session -> nx_secure_tls_credentials.nx_secure_tls_certificate_store,
                                                              &cert -> nx_secure_x509_issuer, &cert);

        /* If certificate was not found, don't try to dereference. */
        if (status == NX_SUCCESS)
        {
            /* Prevent infinite loop if certificate is self-signed. */
            compare_result = _nx_secure_x509_distinguished_name_compare(&cert -> nx_secure_x509_distinguished_name,
                                                                        &cert -> nx_secure_x509_issuer, NX_SECURE_X509_NAME_ALL_FIELDS);
            if (compare_result == 0)
            {
                break;
            }
        }

        /* If an issuer certificate was not found, that is OK - this should actually be the common case
         * as any certificate that is not self-signed should not have its root CA certificate on the
         * same device.
         */
    }

    /* Put the total length of all certificates into the buffer. */
    record_start[0] = (UCHAR)((total_length & 0xFF0000) >> 16);
    record_start[1] = (UCHAR)((total_length & 0xFF00) >> 8);
    record_start[2] = (UCHAR)(total_length & 0xFF);

    return(NX_SUCCESS);
}
