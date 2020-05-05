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
/*    nx_secure_port.h                                    PORTABLE C      */
/*                                                           5.12         */
/*                                                                        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Express Logic, Inc.                                 */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This file contains data type definitions for the specific platform. */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  08-15-2019     Timothy Stapko           Initial Version 5.12          */
/*                                                                        */
/**************************************************************************/

#ifndef NX_SECURE_PORT_H
#define NX_SECURE_PORT_H


/* Determine if the optional NetX Seucre user define file should be used.  */

/*
   #define NX_SECURE_INCLUDE_USER_DEFINE_FILE
 */
#ifdef NX_SECURE_INCLUDE_USER_DEFINE_FILE


/* Yes, include the user defines in nx_user.h. The defines in this file may
   alternately be defined on the command line.  */

#include "nx_secure_user.h"
#endif

#define NX_SECURE_ENABLE_AEAD_CIPHER
#define NX_SECURE_AEAD_CIPHER_CHECK(a) ((a) == NX_CRYPTO_ENCRYPTION_AES_GCM_16)

/* Define the version ID of NetX Secure.  This may be utilized by the application.  */

#ifdef NX_SECURE_SYSTEM_INIT
CHAR                            _nx_secure_version_id[] =
                                    "Copyright (c) 1996-2019 Express Logic Inc. * NetX Secure Version 5.12.5.0 *";
#else
extern  CHAR                    _nx_secure_version_id[];
#endif

#endif

