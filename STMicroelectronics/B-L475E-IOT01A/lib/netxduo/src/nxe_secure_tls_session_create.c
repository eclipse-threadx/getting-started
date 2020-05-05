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

/* Bring in externs for caller checking code.  */

NX_SECURE_CALLER_CHECKING_EXTERNS

/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nxe_secure_tls_session_create                      PORTABLE C      */
/*                                                           5.12         */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Express Logic, Inc.                                 */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function checks for errors in the TLS session create call.     */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    session_ptr                           TLS session control block     */
/*    crypto_table                          crypto method table           */
/*    metadata_buffer                       Encryption metadata area      */
/*    metadata_size                         Encryption metadata size      */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _nx_secure_tls_session_create         Actual TLS session create     */
/*                                            call                        */
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
/*                                            optimized the logic,        */
/*                                            resulting in version 5.11   */
/*  08-15-2019     Timothy Stapko           Modified comment(s),          */
/*                                            checked duplicate creation, */
/*                                            added caller checking,      */
/*                                            resulting in version 5.12   */
/*                                                                        */
/**************************************************************************/
UINT  _nxe_secure_tls_session_create(NX_SECURE_TLS_SESSION *session_ptr,
                                     const NX_SECURE_TLS_CRYPTO *cipher_table,
                                     VOID *metadata_area,
                                     ULONG metadata_size)
{
UINT status;
NX_SECURE_TLS_SESSION *created_tls_session;
ULONG created_count;

    if ((session_ptr == NX_NULL) || (cipher_table == NX_NULL) || (metadata_area == NX_NULL))
    {
        return(NX_PTR_ERROR);
    }

    /* Loop to check for the IP instance already created.  */
    created_tls_session = _nx_secure_tls_created_ptr;
    created_count = _nx_secure_tls_created_count;
    while (created_count--)
    {

        /* Is the new ip already created?  */
        if (session_ptr == created_tls_session)
        {

            /* Duplicate tls session created, return an error!  */
            return(NX_PTR_ERROR);
        }

        /* Move to next entry.  */
        created_tls_session = created_tls_session -> nx_secure_tls_created_next;
    }

    /* Check for appropriate caller.  */
    NX_THREADS_ONLY_CALLER_CHECKING

    status = _nx_secure_tls_session_create(session_ptr, cipher_table, metadata_area, metadata_size);

    /* Return completion status.  */
    return(status);
}

