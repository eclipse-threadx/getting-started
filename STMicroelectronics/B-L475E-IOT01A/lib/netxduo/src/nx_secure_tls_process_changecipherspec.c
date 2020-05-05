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
/*    _nx_secure_tls_process_changecipherspec             PORTABLE C      */
/*                                                           5.12         */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Express Logic, Inc.                                 */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function processes an incoming ChangeCipherSpec message and    */
/*    sets the TLS state machine state accordingly.                       */
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
/*    _nx_secure_tls_session_keys_set       Set session keys              */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    _nx_secure_dtls_process_record        Process DTLS record           */
/*    _nx_secure_tls_process_record         Process TLS record            */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  06-09-2017     Timothy Stapko           Initial Version 5.10          */
/*  12-15-2017     Timothy Stapko           Modified comment(s),          */
/*                                            optimize the logic,         */
/*                                            resulting in version 5.11   */
/*  08-15-2019     Timothy Stapko           Modified comment(s), and      */
/*                                            added flexibility of using  */
/*                                            macros instead of direct C  */
/*                                            library function calls,     */
/*                                            resulting in version 5.12   */
/*                                                                        */
/**************************************************************************/
UINT _nx_secure_tls_process_changecipherspec(NX_SECURE_TLS_SESSION *tls_session,
                                             UCHAR *packet_buffer, UINT message_length)
{
UINT status;

    /* Verify that we received a proper ChangeCipherSpec message. */
    if (message_length != 1)
    {
        return(NX_SECURE_TLS_INCORRECT_MESSAGE_LENGTH);
    }

    /* The contents of a ChangeCipherSpec payload should always be a single byte with value 1. */
    if (packet_buffer[0] != 0x1)
    {
        return(NX_SECURE_TLS_BAD_CIPHERSPEC);
    }

    /* The remote session is now active - all incoming records from this point will be hashed and encrypted. */
    tls_session -> nx_secure_tls_remote_session_active = 1;

    /* Reset the sequence number now that we are starting a new session. */
    NX_SECURE_MEMSET(tls_session -> nx_secure_tls_remote_sequence_number, 0, sizeof(tls_session -> nx_secure_tls_remote_sequence_number));

    /* Set our remote session keys since we have received a CCS from the remote host. */
    status = _nx_secure_tls_session_keys_set(tls_session, NX_SECURE_TLS_KEY_SET_REMOTE);

    return(status);
}

