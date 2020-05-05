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
/*    _nx_secure_tls_remote_certificate_verify            PORTABLE C      */
/*                                                           5.12         */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Express Logic, Inc.                                 */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function verifies the authenticity of a certificate provided   */
/*    by the remote host by checking its digital signature against the    */
/*    trusted store, checking the certificate's validity period, and      */
/*    optionally checking the Common Name against the Top-Level Domain    */
/*    (TLD) name used to access the remote host.                          */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    tls_session                           TLS session                   */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Certificate validity status   */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _nx_secure_x509_certificate_chain_verify                            */
/*                                          Verify cert against stores    */
/*    _nx_secure_x509_expiration_check      Verify expiration of cert     */
/*    _nx_secure_x509_remote_endpoint_certificate_get                     */
/*                                          Get remote host certificate   */
/*    [nx_secure_tls_session_certificate_callback]                        */
/*                                          Session certificate callback  */
/*    [nx_secure_tls_session_time_function] Session time callback         */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    _nx_secure_tls_process_remote_certificate                           */
/*                                          Process server certificate    */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  06-09-2017     Timothy Stapko           Initial Version 5.10          */
/*  12-15-2017     Timothy Stapko           Modified comment(s),          */
/*                                            fixed compiler warnings,    */
/*                                            added expiration date check,*/
/*                                            resulting in version 5.11   */
/*  08-15-2019     Timothy Stapko           Modified comment(s), and      */
/*                                            improved internal logic,    */
/*                                            fixed the usage of crypto   */
/*                                            metadata for hash method,   */
/*                                            fix issues with proper      */
/*                                            CertificateVerify handling, */
/*                                            resulting in version 5.12   */
/*                                                                        */
/**************************************************************************/
UINT _nx_secure_tls_remote_certificate_verify(NX_SECURE_TLS_SESSION *tls_session)
{
UINT                              status;
NX_SECURE_X509_CERT              *remote_certificate;
NX_SECURE_X509_CERTIFICATE_STORE *store;
ULONG                             current_time;


    /* We need to find the remote certificate that represents the endpoint - the leaf in the PKI. */

    /* Process, following X509 basic certificate authentication (RFC 5280):
     *    1. Last certificate in chain is the end entity - start with it.
     *    2. Build chain from issuer to issuer - linked list of issuers. Find in stores: [ Remote, Trusted ]
     *    3. Walk list from end certificate back to a root CA in the trusted store, verifying each signature.
     *       Additionally, any policy enforcement should be done at each step.
     */
    store = &tls_session -> nx_secure_tls_credentials.nx_secure_tls_certificate_store;

    /* Extract the remote certificate processed earlier. */
    status = _nx_secure_x509_remote_endpoint_certificate_get(store, &remote_certificate);

    if (status)
    {
        /* No certificate found, error! */
        return(NX_SECURE_TLS_NO_CERT_SPACE_ALLOCATED);
    }

    /* Assign the TLS Session metadata areas to the certificate for later use. */
    remote_certificate -> nx_secure_x509_public_cipher_metadata_area = tls_session -> nx_secure_public_cipher_metadata_area;
    remote_certificate -> nx_secure_x509_public_cipher_metadata_size = tls_session -> nx_secure_public_cipher_metadata_size;

    remote_certificate -> nx_secure_x509_hash_metadata_area = tls_session -> nx_secure_hash_mac_metadata_area;
    remote_certificate -> nx_secure_x509_hash_metadata_size = tls_session -> nx_secure_hash_mac_metadata_size;

    /* See if we have a timestamp function to get the current time. */
    current_time = 0;
    if (tls_session -> nx_secure_tls_session_time_function != NX_NULL)
    {
        /* Get the current time from our callback. */
        current_time = tls_session -> nx_secure_tls_session_time_function();

        /* Check the remote certificate against the current time. */
        status = _nx_secure_x509_expiration_check(remote_certificate, current_time);

        if (status != NX_SUCCESS)
        {
            return(status);
        }
    }

    /* Now verify our remote certificate chain. If the certificate can be linked to an issuer in the trusted store
       through an issuer chain, this function will return NX_SUCCESS. */
    status = _nx_secure_x509_certificate_chain_verify(store, remote_certificate);

    if (status != NX_SUCCESS)
    {
        return(status);
    }

    /* Now, see if the application has defined a callback to check additional certificate information. */
    if (tls_session -> nx_secure_tls_session_certificate_callback != NX_NULL)
    {
        /* Call the user-defined callback to allow the application to perform additional validation. */
        status = tls_session -> nx_secure_tls_session_certificate_callback(tls_session, remote_certificate);
    }

    /* If remote certificate verification was a success, we have received credentials
       from the remote host and may now pass Finished message processing once received.
       If this is a TLS Server, defer setting the remote credentials flag until after
       we have received and processed the CertificateVerify message. */
    if (tls_session -> nx_secure_tls_socket_type == NX_SECURE_TLS_SESSION_TYPE_CLIENT && status == NX_SUCCESS)
    {
        tls_session -> nx_secure_tls_received_remote_credentials = NX_TRUE;
    }

    return(status);
}

