/**************************************************************************/
/*                                                                        */
/*       Copyright (c) Microsoft Corporation. All rights reserved.        */
/*                                                                        */
/*       This software is licensed under the Microsoft Software License   */
/*       Terms for Microsoft Azure RTOS. Full text of the license can be  */
/*       found in the LICENSE file at https://aka.ms/AzureRTOS_EULA       */
/*       and in the root directory of this software.                      */
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
/*    _nxe_secure_x509_crl_revocation_check               PORTABLE C      */
/*                                                           6.0          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function checks for errors in the X.509 CRL revocation check.  */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    crl_data                              Pointer to DER-encoded CRL    */
/*    crl_length                            Length of CRL data in buffer  */
/*    store                                 Certificate store to be used  */
/*    certificate                           The certificate being checked */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _nx_secure_x509_crl_revocation_check                                */
/*                                          Actual X509 CRL revocation    */
/*                                            check call                  */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  05-19-2020     Timothy Stapko           Initial Version 6.0           */
/*                                                                        */
/**************************************************************************/
UINT _nxe_secure_x509_crl_revocation_check(const UCHAR *crl_data, UINT crl_length,
                                           NX_SECURE_X509_CERTIFICATE_STORE *store,
                                           NX_SECURE_X509_CERT *certificate)
{
UINT status;

    /* Check for pointer errors. */
    if ((certificate == NX_NULL) || (crl_data == NX_NULL) || (crl_length == 0) || (store == NX_NULL))
    {
        return(NX_PTR_ERROR);
    }

    /* Check for appropriate caller.  */
    NX_THREADS_ONLY_CALLER_CHECKING

    /* Make the actual call. */
    status = _nx_secure_x509_crl_revocation_check(crl_data, crl_length, store, certificate);

    return(status);
}

