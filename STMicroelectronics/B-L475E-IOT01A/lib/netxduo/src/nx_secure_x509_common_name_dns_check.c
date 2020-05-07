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

/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_secure_x509_common_name_dns_check               PORTABLE C      */
/*                                                           5.12         */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Express Logic, Inc.                                 */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function checks a certificate's Common Name against a Top      */
/*    Level Domain name (TLD) provided by the caller for the purposes of  */
/*    DNS validation of a remote host. This utility function is intended  */
/*    to be called from within a certificate validation callback routine  */
/*    provided by the application. The TLD name should be the top part of */
/*    the URL used to access the remote host (the "."-separated string    */
/*    before the first slash).                                            */
/*                                                                        */
/*    NOTE 1: If the Common Name does not match the provided string, the  */
/*            "subject alt name" field is compared as well.               */
/*                                                                        */
/*    NOTE 2: It is important to understand the format of the common name */
/*            (and subject alt name) in expected certificates. For        */
/*            example, some certificates may use a raw IP address or a    */
/*            wild card. The DNS TLD string must be formatted such that   */
/*            it will match the expected values in received certificates. */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    certificate                           Pointer to certificate        */
/*    dns_tld                               Top-level domain name         */
/*    dns_tls_length                        Length of TLS in bytes        */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Validity of certificate       */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _nx_secure_x509_extension_find        Find extension in certificate */
/*    _nx_secure_x509_subject_alt_names_find                              */
/*                                          Find subject alt names        */
/*    _nx_secure_x509_wildcard_compare      Wildcard compare for names    */
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
/*                                            resulting in version 5.12   */
/*                                                                        */
/**************************************************************************/
UINT _nx_secure_x509_common_name_dns_check(NX_SECURE_X509_CERT *certificate, const UCHAR *dns_tld,
                                           UINT dns_tld_length)
{
INT                      compare_value;
UINT                     status;
const UCHAR             *common_name;
USHORT                   common_name_len;
NX_SECURE_X509_EXTENSION alt_name_extension;

    /* Get access to our certificate fields. */
    common_name = certificate -> nx_secure_x509_distinguished_name.nx_secure_x509_common_name;
    common_name_len = certificate -> nx_secure_x509_distinguished_name.nx_secure_x509_common_name_length;

    /* Compare the given string against the common name. */
    compare_value = _nx_secure_x509_wildcard_compare(dns_tld, dns_tld_length, common_name, common_name_len);

    if (compare_value == 0)
    {
        return(NX_SUCCESS);
    }

    /* Find the subject alt name extension in the certificate. */
    status = _nx_secure_x509_extension_find(certificate, &alt_name_extension, NX_SECURE_TLS_X509_TYPE_SUBJECT_ALT_NAME);

    /* See if extension present - it is OK if not present! */
    if (status == NX_SUCCESS)
    {
        /* Extract the subject alt name string from the parsed extension. */
        status = _nx_secure_x509_subject_alt_names_find(&alt_name_extension, dns_tld, dns_tld_length, NX_SECURE_X509_SUB_ALT_NAME_TAG_DNSNAME);

        if (status == NX_SUCCESS)
        {
            return(NX_SUCCESS);
        }
    }

    /* If we get here, none of the strings matched. */
    return(NX_SECURE_X509_CERTIFICATE_DNS_MISMATCH);
}

