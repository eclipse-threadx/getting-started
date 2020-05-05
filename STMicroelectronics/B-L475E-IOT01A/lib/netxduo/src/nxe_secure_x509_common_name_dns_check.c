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
/**    X509 Digital Certificates                                          */
/**                                                                       */
/**************************************************************************/
/**************************************************************************/

#define NX_SECURE_SOURCE_CODE

#include "nx_secure_tls.h"
#include "nx_secure_x509.h"

/* Bring in externs for caller checking code.  */

NX_SECURE_CALLER_CHECKING_EXTERNS

/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nxe_secure_x509_common_name_dns_check              PORTABLE C      */
/*                                                           5.12         */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Express Logic, Inc.                                 */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function checks for errors in the X.509 DNS name check.        */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    certificate                           Pointer to certificate        */
/*    dns_tld                               Top-level domain name         */
/*    dns_tld_length                        Length of TLD in bytes        */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Validity of certificate       */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _nx_secure_x509_common_name_dns_check                               */
/*                                          Actual X509 DNS name check    */
/*                                            call                        */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    Application code                                                    */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  12-15-2017     Timothy Stapko           Initial Version 5.11          */
/*  08-15-2019     Timothy Stapko           Modified comment(s),          */
/*                                            added caller checking,      */
/*                                            resulting in version 5.12   */
/*                                                                        */
/**************************************************************************/
UINT _nxe_secure_x509_common_name_dns_check(NX_SECURE_X509_CERT *certificate, const UCHAR *dns_tld,
                                            UINT dns_tld_length)
{
UINT status;

    /* Check for pointer errors. */
    if ((certificate == NX_NULL) || (dns_tld == NX_NULL) || (dns_tld_length == 0))
    {
        return(NX_PTR_ERROR);
    }

    /* Check for appropriate caller.  */
    NX_THREADS_ONLY_CALLER_CHECKING

    /* Make the actual call. */
    status = _nx_secure_x509_common_name_dns_check(certificate, dns_tld, dns_tld_length);

    return(status);
}

