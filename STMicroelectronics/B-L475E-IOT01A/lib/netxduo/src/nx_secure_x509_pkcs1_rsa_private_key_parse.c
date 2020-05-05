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

static VOID _nx_secure_asn1_parse_unsigned_integer(const UCHAR *data, ULONG length,
                                                   const UCHAR **target, USHORT *target_length);

/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_secure_x509_pkcs1_rsa_private_key_parse         PORTABLE C      */
/*                                                           5.12         */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Express Logic, Inc.                                 */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function parses a DER-encoded PKCS#1-formatted private RSA key */
/*    for use with X509 certificates.                                     */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    buffer                                Pointer data to be parsed     */
/*    length                                Length of data in buffer      */
/*    bytes_processed                       Return bytes processed        */
/*    rsa_key                               Return RSA key structure      */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _nx_secure_asn1_parse_unsigned_integer                              */
/*                                          Parse unsigned interger       */
/*    _nx_secure_x509_asn1_tlv_block_parse  Parse ASN.1 block             */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    _nx_secure_x509_certificate_initialize                              */
/*                                          Initialize certificate        */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  06-09-2017     Timothy Stapko           Initial Version 5.10          */
/*  12-15-2017     Timothy Stapko           Modified comment(s),          */
/*                                            supported context-specific  */
/*                                            ASN.1 tags, added logic to  */
/*                                            support vendor-defined      */
/*                                            private key type,           */
/*                                            resulting in version 5.11   */
/*  08-15-2019     Timothy Stapko           Modified comment(s),          */
/*                                            added buffer size checking, */
/*                                            resulting in version 5.12   */
/*                                                                        */
/**************************************************************************/
UINT _nx_secure_x509_pkcs1_rsa_private_key_parse(const UCHAR *buffer, UINT length,
                                                 UINT *bytes_processed,
                                                 NX_SECURE_RSA_PRIVATE_KEY *rsa_key)
{
USHORT       tlv_type;
USHORT       tlv_type_class;
ULONG        tlv_length;
const UCHAR *tlv_data;
ULONG        header_length;
ULONG        seq_length;
UINT         status;
USHORT       version;


    /* Parse an ASN.1 DER-encoded PKCS#1 formatted RSA private key file. */

    /* From RFC 3447, PKCS #1.                                                 */
    /* RSAPrivateKey ::= SEQUENCE {                                            */
    /*     version           Version,                                          */
    /*     modulus           INTEGER,  -- n                                    */
    /*     publicExponent    INTEGER,  -- e                                    */
    /*     privateExponent   INTEGER,  -- d                                    */
    /*     prime1            INTEGER,  -- p                                    */
    /*     prime2            INTEGER,  -- q                                    */
    /*     exponent1         INTEGER,  -- d mod (p-1)                          */
    /*     exponent2         INTEGER,  -- d mod (q-1)                          */
    /*     coefficient       INTEGER,  -- (inverse of q) mod p                 */
    /*     otherPrimeInfos   OtherPrimeInfos OPTIONAL                          */
    /* }                                                                       */
    /*                                                                         */
    /* Version ::= INTEGER { two-prime(0), multi(1) }                          */
    /*               (CONSTRAINED BY                                           */
    /*               {-- version must be multi if otherPrimeInfos present --}) */
    /*                                                                         */
    /*                                                                         */
    /* OtherPrimeInfos ::= SEQUENCE SIZE(1..MAX) OF OtherPrimeInfo             */
    /*                                                                         */
    /* OtherPrimeInfo ::= SEQUENCE {                                           */
    /*     prime             INTEGER,  -- ri                                   */
    /*     exponent          INTEGER,  -- di                                   */
    /*     coefficient       INTEGER   -- ti                                   */
    /* }                                                                       */

    /*  Parse a TLV block and get information to continue parsing. */
    status = _nx_secure_x509_asn1_tlv_block_parse(buffer, (ULONG *)&length, &tlv_type, &tlv_type_class, &tlv_length, &tlv_data, &header_length);

    /*  Make sure we parsed the block alright. */
    if (status != 0)
    {
        return(status);
    }

    if (tlv_type != NX_SECURE_ASN_TAG_SEQUENCE || tlv_type_class != NX_SECURE_ASN_TAG_CLASS_UNIVERSAL)
    {
        return(NX_SECURE_X509_INVALID_CERTIFICATE_SEQUENCE);
    }

    *bytes_processed = header_length;
    seq_length = tlv_length;

    /* First item in the RSA key sequence is a version field. */
    status = _nx_secure_x509_asn1_tlv_block_parse(tlv_data, &seq_length, &tlv_type, &tlv_type_class, &tlv_length, &tlv_data, &header_length);

    /*  Make sure we parsed the block alright. */
    if (status != 0)
    {
        return(status);
    }

    if (tlv_type != NX_SECURE_ASN_TAG_INTEGER || tlv_type_class != NX_SECURE_ASN_TAG_CLASS_UNIVERSAL)
    {
        return(NX_SECURE_PKCS1_INVALID_PRIVATE_KEY);
    }

    /* Update byte count. */
    *bytes_processed += (header_length + tlv_length);

    /* Version has 2 values: two-prime(0) and multi(1). It is multi only if the "OtherPrimeInfo" field is present. */
    version = tlv_data[0];

    if (version != 0x00 && version != 0x01)
    {
        return(NX_SECURE_PKCS1_INVALID_PRIVATE_KEY);
    }

    /* Advance our working pointer past the last field. */
    tlv_data = &tlv_data[tlv_length];


    /* Parse our next field, the modulus. */
    status = _nx_secure_x509_asn1_tlv_block_parse(tlv_data, &seq_length, &tlv_type, &tlv_type_class, &tlv_length, &tlv_data, &header_length);

    /*  Make sure we parsed the block alright. */
    if (status != 0)
    {
        return(status);
    }

    if (tlv_type != NX_SECURE_ASN_TAG_INTEGER || tlv_type_class != NX_SECURE_ASN_TAG_CLASS_UNIVERSAL)
    {
        return(NX_SECURE_PKCS1_INVALID_PRIVATE_KEY);
    }

    /* Update byte count. */
    *bytes_processed += (header_length + tlv_length);

    /* The modulus is an integer, so no padding bytes are needed (as with a BITSTRING). */
    if (rsa_key != NULL)
    {
        _nx_secure_asn1_parse_unsigned_integer(tlv_data, tlv_length, &rsa_key -> nx_secure_rsa_public_modulus,
                                               &rsa_key -> nx_secure_rsa_public_modulus_length);
    }

    /* Advance our working pointer past the last field. */
    tlv_data = &tlv_data[tlv_length];

    /* Parse our next field, the public exponent. */
    status = _nx_secure_x509_asn1_tlv_block_parse(tlv_data, &seq_length, &tlv_type, &tlv_type_class, &tlv_length, &tlv_data, &header_length);

    /*  Make sure we parsed the block alright. */
    if (status != 0)
    {
        return(status);
    }

    if (tlv_type != NX_SECURE_ASN_TAG_INTEGER || tlv_type_class != NX_SECURE_ASN_TAG_CLASS_UNIVERSAL)
    {
        return(NX_SECURE_PKCS1_INVALID_PRIVATE_KEY);
    }

    /* Update byte count. */
    *bytes_processed += (header_length + tlv_length);

    /* The exponent is an integer, so no padding bytes are needed (as with a BITSTRING). */
    if (rsa_key != NULL)
    {
        _nx_secure_asn1_parse_unsigned_integer(tlv_data, tlv_length, &rsa_key -> nx_secure_rsa_public_exponent,
                                               &rsa_key -> nx_secure_rsa_public_exponent_length);
    }

    /* Advance our working pointer past the last field. */
    tlv_data = &tlv_data[tlv_length];

    /* Parse our next field, the private exponent. */
    status = _nx_secure_x509_asn1_tlv_block_parse(tlv_data, &seq_length, &tlv_type, &tlv_type_class, &tlv_length, &tlv_data, &header_length);

    /*  Make sure we parsed the block alright. */
    if (status != 0)
    {
        return(status);
    }

    if (tlv_type != NX_SECURE_ASN_TAG_INTEGER || tlv_type_class != NX_SECURE_ASN_TAG_CLASS_UNIVERSAL)
    {
        return(NX_SECURE_PKCS1_INVALID_PRIVATE_KEY);
    }

    /* Update byte count. */
    *bytes_processed += (header_length + tlv_length);

    /* The value is an integer, so no padding bytes are needed (as with a BITSTRING). */
    if (rsa_key != NULL)
    {
        _nx_secure_asn1_parse_unsigned_integer(tlv_data, tlv_length, &rsa_key -> nx_secure_rsa_private_exponent,
                                               &rsa_key -> nx_secure_rsa_private_exponent_length);
    }

    /* Advance our working pointer past the last field. */
    tlv_data = &tlv_data[tlv_length];

    /* Parse our next field, the private prime P. */
    status = _nx_secure_x509_asn1_tlv_block_parse(tlv_data, &seq_length, &tlv_type, &tlv_type_class, &tlv_length, &tlv_data, &header_length);

    /*  Make sure we parsed the block alright. */
    if (status != 0)
    {
        return(status);
    }

    if (tlv_type != NX_SECURE_ASN_TAG_INTEGER || tlv_type_class != NX_SECURE_ASN_TAG_CLASS_UNIVERSAL)
    {
        return(NX_SECURE_PKCS1_INVALID_PRIVATE_KEY);
    }

    /* Update byte count. */
    *bytes_processed += (header_length + tlv_length);

    /* The modulus is an integer, so no padding bytes are needed (as with a BITSTRING). */
    if (rsa_key != NULL)
    {
        _nx_secure_asn1_parse_unsigned_integer(tlv_data, tlv_length, &rsa_key -> nx_secure_rsa_private_prime_p,
                                               &rsa_key -> nx_secure_rsa_private_prime_p_length);
    }

    /* Advance our working pointer past the last field. */
    tlv_data = &tlv_data[tlv_length];

    /* Parse our next field, the private prime Q. */
    status = _nx_secure_x509_asn1_tlv_block_parse(tlv_data, &seq_length, &tlv_type, &tlv_type_class, &tlv_length, &tlv_data, &header_length);

    /*  Make sure we parsed the block alright. */
    if (status != 0)
    {
        return(status);
    }

    if (tlv_type != NX_SECURE_ASN_TAG_INTEGER || tlv_type_class != NX_SECURE_ASN_TAG_CLASS_UNIVERSAL)
    {
        return(NX_SECURE_PKCS1_INVALID_PRIVATE_KEY);
    }

    /* Update byte count. */
    *bytes_processed += (header_length + tlv_length);

    /* The value is an integer, so no padding bytes are needed (as with a BITSTRING). */
    if (rsa_key != NULL)
    {
        _nx_secure_asn1_parse_unsigned_integer(tlv_data, tlv_length, &rsa_key -> nx_secure_rsa_private_prime_q,
                                               &rsa_key -> nx_secure_rsa_private_prime_q_length);
    }

    return(NX_SECURE_X509_SUCCESS);
}

/* This is a helper function to parse DER-encoded signed integers found in the RSA key file.
   It converts the signed integer into an unsigned character block for use in RSA calculations.
   DER-encoded integers are in 2's-complement format and thus can be negative. All
   RSA keys are unsigned integers so we need to compensate for the extra padding.
   If the first byte is 0x00, check the top bit of the next byte. If that bit
   is set, then discard the 0x00 byte. */
/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_secure_asn1_parse_unsigned_integer              PORTABLE C      */
/*                                                           5.12         */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Express Logic, Inc.                                 */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function parses an ASN.1 DER-encoded unsigned integer.         */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    data                                  Pointer data to be parsed     */
/*    length                                Length of data in buffer      */
/*    target                                Return target processed       */
/*    target_length                         Return length of target       */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    None                                                                */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    None                                                                */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    _nx_secure_x509_pkcs1_rsa_private_key_parse                         */
/*                                          Parse RSA private key field   */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  06-09-2017     Timothy Stapko           Initial Version 5.10          */
/*  12-15-2017     Timothy Stapko           Modified comment(s),          */
/*                                            resulting in version 5.11   */
/*  08-15-2019     Timothy Stapko           Modified comment(s),          */
/*                                            resulting in version 5.12   */
/*                                                                        */
/**************************************************************************/
static VOID _nx_secure_asn1_parse_unsigned_integer(const UCHAR *data, ULONG length,
                                                   const UCHAR **target, USHORT *target_length)
{
UINT offset;

    /* If there is only one byte, don't do anything. */
    if (length <= 1)
    {
        return;
    }

    /* Default is not to skip the first byte. */
    offset = 0;

    if (data[0] == 0x00 && ((data[1] & 0x80) != 0))
    {
        /* Discard the first byte when copying to the target. */
        offset = 1;
    }

    /* Finally set the data pointer and length based on our offset. */
    *target = &data[offset];
    *target_length = (USHORT)(length - offset);
}

