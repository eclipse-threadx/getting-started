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
/*    _nx_secure_x509_key_usage_extension_parse           PORTABLE C      */
/*                                                           5.12         */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Express Logic, Inc.                                 */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function parses through an X.509 certificate keyUsage          */
/*    extension and returns the Authentication Key Usage bitfield for use */
/*    by the application.                                                 */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    certificate                           Pointer to X.509 certificate  */
/*    bitfield                              keyUsage bitfield return      */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _nx_secure_x509_asn1_tlv_block_parse  Parse ASN.1 block             */
/*    _nx_secure_x509_extension_find        Find extension in certificate */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    Application Code                                                    */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  12-15-2017     Timothy Stapko           Initial Version 5.11          */
/*  08-15-2019     Timothy Stapko           Modified comment(s),          */
/*                                            added buffer size checking, */
/*                                            resulting in version 5.12   */
/*                                                                        */
/**************************************************************************/
UINT _nx_secure_x509_key_usage_extension_parse(NX_SECURE_X509_CERT *certificate, USHORT *bitfield)
{
USHORT                   tlv_type;
USHORT                   tlv_type_class;
ULONG                    tlv_length;
const UCHAR             *tlv_data;
const UCHAR             *current_buffer;
ULONG                    length;
ULONG                    header_length;
UINT                     status;
NX_SECURE_X509_EXTENSION key_usage_extension;

    /* Find and parse the keyUsage extension. */
    /* keyUsage ASN.1 format:

       id-ce-keyUsage OBJECT IDENTIFIER ::=  { id-ce 15 }

       KeyUsage ::= BIT STRING {
           digitalSignature        (0),
           nonRepudiation          (1), -- recent editions of X.509 have
                                -- renamed this bit to contentCommitment
           keyEncipherment         (2),
           dataEncipherment        (3),
           keyAgreement            (4),
           keyCertSign             (5),
           cRLSign                 (6),
           encipherOnly            (7),
           decipherOnly            (8) }
     */

    /* Find the KeyUsage extension in the certificate. */
    status = _nx_secure_x509_extension_find(certificate, &key_usage_extension, NX_SECURE_TLS_X509_TYPE_KEY_USAGE);

    /* See if extension present - it is OK if not present! */
    if (status != NX_SUCCESS)
    {
        return(status);
    }

    /* The length of our extensions is the length of the sequence. */
    current_buffer = key_usage_extension.nx_secure_x509_extension_data;
    length = key_usage_extension.nx_secure_x509_extension_data_length;

    /*  Parse the bit string. */
    status = _nx_secure_x509_asn1_tlv_block_parse(current_buffer, &length, &tlv_type, &tlv_type_class, &tlv_length, &tlv_data, &header_length);

    /*  Make sure we parsed the block alright. */
    if (status != 0)
    {
        return(status);
    }

    /* If the next item up is not a sequence, then it isn't an extensions block. */
    if (!(tlv_type_class == NX_SECURE_ASN_TAG_CLASS_UNIVERSAL && tlv_type == NX_SECURE_ASN_TAG_BIT_STRING))
    {
        /* We were expecting a bitfield but got something else. */
        return(NX_SECURE_X509_INVALID_EXTENSION_SEQUENCE);
    }

    /* Check the bit string length. */
    if (tlv_length > sizeof(USHORT) || tlv_length < 2)
    {
        return(NX_SECURE_X509_INVALID_EXTENSION_SEQUENCE);
    }

    /* DER-encoding of a BIT STRING with flag values uses the top octet of the 2 byte string
       to encode the number of 0 bits at the end of the lower octet. Thus, we need to extract
       the top byte and right-shift the bottom byte to get the actual bitfield value. */
    *bitfield = (USHORT)(tlv_data[1] >> tlv_data[0]);


    return(NX_SUCCESS);
}

