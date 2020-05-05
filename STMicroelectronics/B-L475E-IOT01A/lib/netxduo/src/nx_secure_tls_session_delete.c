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
/*    _nx_secure_tls_session_delete                       PORTABLE C      */
/*                                                           5.12         */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Express Logic, Inc.                                 */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function deletes a TLS session object, returning any resources */
/*    to the system.                                                      */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    tls_session                           TLS control block             */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _nx_secure_tls_session_reset          Clear TLS control block       */
/*    tx_mutex_get                          Get protection mutex          */
/*    tx_mutex_put                          Put protection mutex          */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    Application Code                                                    */
/*    _nx_secure_dtls_session_delete        Delete the DTLS session       */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  06-09-2017     Timothy Stapko           Initial Version 5.10          */
/*  12-15-2017     Timothy Stapko           Modified comment(s),          */
/*                                            resulting in version 5.11   */
/*  08-15-2019     Timothy Stapko           Modified comment(s), fixed    */
/*                                            issue with linked list next */
/*                                            pointer being NULL, clear   */
/*                                            TLS ID to assure that       */
/*                                            session cannot be reused,   */
/*                                            resulting in version 5.12   */
/*                                                                        */
/**************************************************************************/
UINT  _nx_secure_tls_session_delete(NX_SECURE_TLS_SESSION *tls_session)
{
UINT status;

    /* Reset the TLS state so this socket can be reused. */
    status = _nx_secure_tls_session_reset(tls_session);

    /* Get the protection. */
    tx_mutex_get(&_nx_secure_tls_protection, TX_WAIT_FOREVER);

    /* Remove the TLS instance from the created list. */
    /* See if the TLS instance is the only one on the list. */
    if (tls_session == tls_session -> nx_secure_tls_created_next)
    {

        /* Only created TLS instance, just set the created list to NULL. */
        _nx_secure_tls_created_ptr = NX_NULL;
    }
    else
    {

        /* Otherwise, not the only created TLS, link-up the neighbors. */
        if(tls_session -> nx_secure_tls_created_next != NX_NULL)
        {
            (tls_session -> nx_secure_tls_created_next) -> nx_secure_tls_created_previous =
                    tls_session -> nx_secure_tls_created_previous;
        }

        (tls_session -> nx_secure_tls_created_previous) -> nx_secure_tls_created_next =
            tls_session -> nx_secure_tls_created_next;

        /* See if we have to update the created list head pointer. */
        if (_nx_secure_tls_created_ptr == tls_session)
        {

            /* Yes, move the head pointer to the next link. */
            _nx_secure_tls_created_ptr = tls_session -> nx_secure_tls_created_next;
        }
    }
    _nx_secure_tls_created_count--;

    /* Make sure the session is completely reset - set ID to zero for error checking. */
    tls_session -> nx_secure_tls_id = 0;

    /* Release the protection. */
    tx_mutex_put(&_nx_secure_tls_protection);

    return(status);
}

