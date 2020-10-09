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
/*    _nx_secure_tls_remote_certificate_buffer_allocate   PORTABLE C      */
/*                                                           6.0          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function allocates buffer space to hold incoming certificates  */
/*    sent by the remote host. The provided buffer must have enough space */
/*    allocated for the maximum size of a certificate that may be provided*/
/*    by a remote host times the expected size of the provided certificate*/
/*    chain. The size needed can be calculated using the following        */
/*    formula:                                                            */
/*                                                                        */
/*    size = (<# of certs>) * (sizeof(NX_SECURE_X509_CERT) +              */
/*                            <expected max cert size (~2KB)>)            */
/*                                                                        */
/*    The space will be divided equally amongst the number of certificates*/
/*    that can be carved from the provided buffer.                        */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    tls_session                           Pointer to TLS Session        */
/*    certs_number                          Number of client certs        */
/*    certificate_buffer                    Buffer allocated for certs    */
/*    buffer_size                           Buffer size in bytes          */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _nx_secure_tls_remote_certificate_allocate                          */
/*                                          Allocate space for certs      */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    Application Code                                                    */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  05-19-2020     Timothy Stapko           Initial Version 6.0           */
/*                                                                        */
/**************************************************************************/
UINT  _nx_secure_tls_remote_certificate_buffer_allocate(NX_SECURE_TLS_SESSION *tls_session, UINT certs_number, VOID *certificate_buffer, ULONG buffer_size)
{
UINT status;
UINT metadata_size;
UINT cert_buffer_size;
UCHAR *buffer_ptr;
NX_SECURE_X509_CERT *cert_ptr;
UINT count;

    /* Calculate the size of the X509 control blocks needed. */
    metadata_size = sizeof(NX_SECURE_X509_CERT) * certs_number;

    /* Check that buffer is large enough. */
    if(buffer_size < metadata_size || certs_number == 0 || buffer_size == 0)
    {
        return(NX_INVALID_PARAMETERS);
    }

    /* Calculate the per-certificate size allocated from the buffer. */
    cert_buffer_size = (buffer_size - metadata_size) / certs_number;

    /* Check that the certificate buffer size makes sense. */
    if(cert_buffer_size < NX_SECURE_TLS_MINIMUM_CERTIFICATE_SIZE)
    {
        return(NX_INVALID_PARAMETERS);
    }

    /* Get a working pointer to our certificate buffer. */
    buffer_ptr = (UCHAR*)(certificate_buffer);

    for(count = 0; count < certs_number; count++)
    {
        /* Allocate space for the cert control block. */
        cert_ptr = (NX_SECURE_X509_CERT*)(buffer_ptr);

        /* Advance working pointer past control block. */
        buffer_ptr += sizeof(NX_SECURE_X509_CERT);

        /* Now allocate space for remote certificates. */
        status = _nx_secure_tls_remote_certificate_allocate(tls_session, cert_ptr, buffer_ptr, cert_buffer_size);

        if(status != NX_SUCCESS)
        {
            return(status);
        }

        /* Advance working pointer past certificate buffer. */
        buffer_ptr += cert_buffer_size;
    }
    /* Return completion status.  */
    return(NX_SUCCESS);
}

