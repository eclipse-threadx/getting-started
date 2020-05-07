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
/*    _nx_secure_x509_wildcard_compare                    PORTABLE C      */
/*                                                           5.12         */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Express Logic, Inc.                                 */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function compares a name (string) against a name string using  */
/*    wildcards as found in the Common Name and subjectAltName fields of  */
/*    an X.509 certificate. This is primarily used when checking a DNS    */
/*    name against an X.509 certificate provided by a remote host.        */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    dns_name                              Name to check                 */
/*    dns_name_len                          Length of name                */
/*    wildcard_name                         String with name or wildcard  */
/*    wildcard_len                          Length of wildcard            */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    compare value                         0 if equal, else non-zero     */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    None                                                                */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    _nx_secure_x509_common_name_dns_check Check Common Name by DNS      */
/*    _nx_secure_x509_subject_alt_names_find                              */
/*                                          Find subject alt names        */
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
INT _nx_secure_x509_wildcard_compare(const UCHAR *dns_name, UINT dns_name_len,
                                     const UCHAR *wildcard_name, UINT wildcard_len)
{
INT dns_offset;
INT wildcard_offset;

    dns_offset = (INT)dns_name_len - 1;
    wildcard_offset = (INT)wildcard_len - 1;

    /* Walk backwards through each name. */
    while (dns_offset >= 0 && wildcard_offset >= 0)
    {
        /* Check each character. */
        if (dns_name[dns_offset] != wildcard_name[wildcard_offset])
        {
            /* Characters do not match, check for wildcard. */
            if (wildcard_name[wildcard_offset] == '*')
            {
                if (wildcard_offset != 0 || wildcard_name[1] != '.')
                {
                    /* Only match wildcard character when it is
                       the only character of the left-most label. */
                    return(1);
                }

                while (dns_offset >= 0)
                {
                    if (dns_name[dns_offset] == '.')
                    {
                        /* Wildcard does not match full stops.  */
                        return(1);
                    }
                    dns_offset--;
                }
                /* Wildcard match, they are OK. */
                return(0);
            }

            /* No match and no wildcard. */
            return(1);
        }

        /* Adjust offsets. */
        dns_offset--;
        wildcard_offset--;
    }

    if (dns_offset != -1 || wildcard_offset != -1)
    {
        /* Length mismatch. */
        return(1);
    }
    /* Both names are exactly the same. */
    return(0);
}

