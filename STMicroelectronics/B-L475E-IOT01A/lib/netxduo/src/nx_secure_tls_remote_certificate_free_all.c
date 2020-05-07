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


/* Include necessary system files.  */

#include "nx_secure_tls.h"

/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_secure_tls_remote_certificate_free_all          PORTABLE C      */
/*                                                           5.12         */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Express Logic, Inc.                                 */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function moves all remote certificate buffers back into the    */
/*    free certificate store, allowing them to be used for a new TLS      */
/*    session.                                                            */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    tls_session                           Pointer to TLS Session        */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _nx_secure_tls_remote_certificate_free                              */
/*                                          Free remote certificate       */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    Application Code                                                    */
/*    _nx_secure_tls_client_handshake       TLS client state machine      */
/*    _nx_secure_tls_process_clienthello    Process ClientHello           */
/*    _nx_secure_tls_session_renegotiate    Renegotiate TLS session       */
/*    _nx_secure_tls_session_reset          Clear TLS control block       */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  12-15-2017     Timothy Stapko           Initial Version 5.11          */
/*  08-15-2019     Timothy Stapko           Modified comment(s),          */
/*                                            add remote certificate      */
/*                                            memory optimization,        */
/*                                            resulting in version 5.12   */
/*                                                                        */
/**************************************************************************/
UINT _nx_secure_tls_remote_certificate_free_all(NX_SECURE_TLS_SESSION *tls_session)
{
UINT                              status = NX_SUCCESS;
NX_SECURE_X509_CERTIFICATE_STORE *store;
NX_SECURE_X509_CERT              *certificate;


    /* Reset the packet buffer if we allocated certificates from it. */
    tls_session->nx_secure_tls_packet_buffer_size = tls_session->nx_secure_tls_packet_buffer_original_size;

    /* Get the remote certificate store from our TLS session. */
    store = &tls_session -> nx_secure_tls_credentials.nx_secure_tls_certificate_store;

    certificate = store -> nx_secure_x509_remote_certificates;

    /* Loop through all remote certificates and remove each. */
    while (certificate != NX_NULL)
    {
        status = _nx_secure_tls_remote_certificate_free(tls_session, &certificate -> nx_secure_x509_distinguished_name);

        if (status != NX_SUCCESS)
        {
            return(status);
        }

        /* Get the new certificate list head after removal. */
        certificate = store -> nx_secure_x509_remote_certificates;
    }


    /* Return completion status.  */
    return(status);
}

