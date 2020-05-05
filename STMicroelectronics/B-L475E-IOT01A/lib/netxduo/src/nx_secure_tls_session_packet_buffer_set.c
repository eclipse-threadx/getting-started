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
/*    _nx_secure_tls_session_packet_buffer_set            PORTABLE C      */
/*                                                           5.12         */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Express Logic, Inc.                                 */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function sets the buffer TLS uses to reassemble incoming       */
/*    messages which may span multiple TCP packets.                       */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    session_ptr                           TLS control block             */
/*    buffer_ptr                            Pointer to buffer             */
/*    buffer_size                           Buffer area size              */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    None                                                                */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    Application Code                                                    */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  06-09-2017     Timothy Stapko           Initial Version 5.10          */
/*  12-15-2017     Timothy Stapko           Modified comment(s),          */
/*                                            resulting in version 5.11   */
/*  08-15-2019     Timothy Stapko           Modified comment(s), add four */
/*                                            byte alignment and adjust,  */
/*                                            add remote certificate      */
/*                                            memory optimization, fix    */
/*                                            static analysis report,     */
/*                                            resulting in version 5.12   */
/*                                                                        */
/**************************************************************************/
UINT  _nx_secure_tls_session_packet_buffer_set(NX_SECURE_TLS_SESSION *session_ptr,
                                               UCHAR *buffer_ptr, ULONG buffer_size)
{

    /* For machines that don't auto-align, check and adjust for four byte alignment. */
    if (((ULONG)buffer_ptr) & 0x3)
    {
        buffer_ptr = (UCHAR *)(((((ULONG)buffer_ptr) & 0xFFFFFFFC) + 4) & 0xFFFFFFFF);
        buffer_size -= (((ULONG)buffer_ptr) & 0x3);
    }

    /* Check size of buffer for alignment after above adjustment. */
    if (buffer_size & 0x3)
    {
        buffer_size = buffer_size & 0xFFFFFFFC;
    }


    /* Set the buffer and its size. */
    session_ptr -> nx_secure_tls_packet_buffer = buffer_ptr;
    session_ptr -> nx_secure_tls_packet_buffer_size = buffer_size;
    
    /* Save off the original size of the buffer. */
    session_ptr -> nx_secure_tls_packet_buffer_original_size = buffer_size;

    return(NX_SUCCESS);
}

