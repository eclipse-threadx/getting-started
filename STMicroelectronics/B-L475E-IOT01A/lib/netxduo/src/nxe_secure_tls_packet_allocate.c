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

/* Bring in externs for caller checking code.  */

NX_SECURE_CALLER_CHECKING_EXTERNS

/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nxe_secure_tls_packet_allocate                     PORTABLE C      */
/*                                                           5.12         */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Express Logic, Inc.                                 */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function checks for errors in TLS packet allocate call.        */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    tls_session                           TLS control block             */
/*    pool_ptr                              Pool to allocate packet from  */
/*    packet_ptr                            Pointer to place allocated    */
/*                                            packet pointer              */
/*    wait_option                           Suspension option             */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _nx_secure_tls_packet_allocate        Actual packet allocate call.  */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    Application                                                         */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  06-09-2017     Timothy Stapko           Initial Version 5.10          */
/*  12-15-2017     Timothy Stapko           Modified comment(s),          */
/*                                            optimized the logic,        */
/*                                            resulting in version 5.11   */
/*  08-15-2019     Timothy Stapko           Modified comment(s), check TLS*/
/*                                            session initialization,     */
/*                                            added caller checking,      */
/*                                            resulting in version 5.12   */
/*                                                                        */
/**************************************************************************/
UINT _nxe_secure_tls_packet_allocate(NX_SECURE_TLS_SESSION *tls_session, NX_PACKET_POOL *pool_ptr,
                                     NX_PACKET **packet_ptr, ULONG wait_option)
{
UINT status;

    /* Verify tls_session is valid. */

    /* Make sure the session is initialized. */
    if(tls_session -> nx_secure_tls_id != NX_SECURE_TLS_ID)
    {
        return(NX_SECURE_TLS_SESSION_UNINITIALIZED);
    }

    /* Verify the TCP socket valid and is connected. */
    if (tls_session -> nx_secure_tls_tcp_socket == NX_NULL)
    {
        return(NX_SECURE_TLS_SESSION_UNINITIALIZED);
    }

    if ((tls_session -> nx_secure_tls_tcp_socket -> nx_tcp_socket_connect_ip.nxd_ip_version !=  NX_IP_VERSION_V4) &&
        (tls_session -> nx_secure_tls_tcp_socket -> nx_tcp_socket_connect_ip.nxd_ip_version !=  NX_IP_VERSION_V6))
    {
        return(NX_SECURE_TLS_SESSION_UNINITIALIZED);
    }

    if (tls_session -> nx_secure_tls_tcp_socket -> nx_tcp_socket_state != NX_TCP_ESTABLISHED)
    {
        return(NX_SECURE_TLS_SESSION_UNINITIALIZED);
    }

    /* Check for appropriate caller.  */
    NX_THREADS_ONLY_CALLER_CHECKING

    status =  _nx_secure_tls_packet_allocate(tls_session, pool_ptr, packet_ptr, wait_option);

    return(status);
}

