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


/**************************************************************************/
/*                                                                        */
/*  PORT SPECIFIC C INFORMATION                            RELEASE        */
/*                                                                        */
/*    nx_secure_user.h                                    PORTABLE C      */
/*                                                           5.12         */
/*                                                                        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Express Logic, Inc.                                 */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This file contains user defines for configuring NetX Secure in      */
/*    specific ways. This file will have an effect only if the            */
/*    application and NetX Secure library are built with                  */
/*    NX_SECURE_INCLUDE_USER_DEFINE_FILE defined.                         */
/*    Note that all the defines in this file may also be made on the      */
/*    command line when building NetX library and application objects.    */
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

#ifndef SRC_NX_SECURE_USER_H
#define SRC_NX_SECURE_USER_H


/* Define various build options for the NetX Secure port.  The application should either make changes
   here by commenting or un-commenting the conditional compilation defined OR supply the defines
   though the compiler's equivalent of the -D option.  */


/* Override various options with default values already assigned in nx_secure_tls.h */


/* NX_SECURE_TLS_ENABLE_SSL_3_0 defines whether or not to enable SSL 3.0 protocol support.
   BY default SSL 3.0 is not supported. */
/*
   #define NX_SECURE_TLS_ENABLE_SSL_3_0
 */

/* NX_SECURE_TLS_ENABLE_TLS_1_0 defines whether or not to enable TLS 1.0 protocol support.
   BY default TLS 1.0 is not supported. */
/*
   #define NX_SECURE_TLS_ENABLE_TLS_1_0
 */

/* NX_SECURE_TLS_DISABLE_TLS_1_1 defines whether or not to disable TLS 1.1 protocol support.
   BY default TLS 1.1 is supported. */
/*
   #define NX_SECURE_TLS_DISABLE_TLS_1_1
 */

/* NX_SECURE_ENABLE_PSK_CIPHERSUITES enables Pre-Shared Key.  By default
   this feature is not enabled. */
/*
   #define NX_SECURE_ENABLE_PSK_CIPHERSUITES
 */



#endif

