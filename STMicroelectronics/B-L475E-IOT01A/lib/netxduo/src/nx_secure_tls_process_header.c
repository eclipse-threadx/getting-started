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
/*    _nx_secure_tls_process_header                       PORTABLE C      */
/*                                                           5.12         */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Express Logic, Inc.                                 */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function processes an NX_PACKET data structure, extracting     */
/*    and parsing a TLS header recevied from a remote host.               */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    tls_session                           Pointer to TLS control block  */
/*    packet_ptr                            Pointer to incoming packet    */
/*    record_offset                         Offset of current record      */
/*    message_type                          Return message type value     */
/*    length                                Return message length value   */
/*    header_data                           Pointer to header to parse    */
/*    header_length                         Length of header data (bytes) */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    nx_packet_data_extract_offset         Extract data from NX_PACKET   */
/*    _nx_secure_tls_check_protocol_version Check incoming TLS version    */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    _nx_secure_tls_process_record         Process TLS record            */
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
/*                                            resulting in version 5.12   */
/*                                                                        */
/**************************************************************************/
UINT _nx_secure_tls_process_header(NX_SECURE_TLS_SESSION *tls_session, NX_PACKET *packet_ptr,
                                   ULONG record_offset, USHORT *message_type, UINT *length,
                                   UCHAR *header_data, USHORT *header_length)
{
ULONG  bytes_copied;
UINT   status;
USHORT protocol_version;
ULONG  remaining_bytes = 5;


    *header_length = 5;

    while (remaining_bytes)
    {

        /* Check the packet. */
        if (packet_ptr == NX_NULL)
        {

            /* There was an error in extracting the header from the supplied packet. */
            return(NX_SECURE_TLS_INVALID_PACKET);
        }

        /* Process the TLS record header, which will set the state. */
        status = nx_packet_data_extract_offset(packet_ptr, record_offset, &header_data[5 - remaining_bytes],
                                               remaining_bytes, &bytes_copied);

        /* Make sure we actually got a header. */
        if (status != NX_SUCCESS)
        {

            /* There was an error in extracting the header from the supplied packet. */
            return(NX_SECURE_TLS_INVALID_PACKET);
        }

        record_offset = 0;
        remaining_bytes -= bytes_copied;

        packet_ptr = packet_ptr -> nx_packet_queue_next;
    }

    /* Extract message type from packet/record. */
    *message_type = header_data[0];

    /* Extract the protocol version. */
    protocol_version = (USHORT)(((USHORT)header_data[1] << 8) | header_data[2]);

    /* Get the length of the TLS data. */
    *length = (UINT)(((UINT)header_data[3] << 8) + header_data[4]);

    /* Check the protocol version, except when we haven't established a version yet */
    if (tls_session -> nx_secure_tls_protocol_version != 0)
    {
        /* Check the record's protocol version against the current session. */
        status = _nx_secure_tls_check_protocol_version(tls_session, protocol_version, NX_SECURE_TLS);
        return(status);
    }

    return(NX_SUCCESS);
}

