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
/** NetX Crypto Component                                                 */
/**                                                                       */
/**   Elliptic Curve                                                      */
/**                                                                       */
/**************************************************************************/
/**************************************************************************/


/**************************************************************************/
/*                                                                        */
/*  APPLICATION INTERFACE DEFINITION                       RELEASE        */
/*                                                                        */
/*    nx_crypto_ec.h                                      PORTABLE C      */
/*                                                           6.0          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This file defines the symbols, structures and operations for        */
/*    Elliptic Curve Crypto.                                              */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  05-19-2020     Timothy Stapko           Initial Version 6.0           */
/*                                                                        */
/**************************************************************************/

#ifndef NX_CRYPTO_EC_H
#define NX_CRYPTO_EC_H

/* Determine if a C++ compiler is being used.  If so, ensure that standard
   C is used to process the API information.  */
#ifdef __cplusplus

/* Yes, C++ compiler is present.  Use standard C.  */
extern   "C" {

#endif

#include "nx_crypto.h"
#include "nx_crypto_huge_number.h"

#define NX_CRYPTO_EC_POINT_AFFINE     0
#define NX_CRYPTO_EC_POINT_PROJECTIVE 1

#define NX_CRYPTO_EC_FP               0
#define NX_CRYPTO_EC_F2M              1

/* Define Elliptic Curve point. */
typedef struct
{
    UINT                  nx_crypto_ec_point_type;
    NX_CRYPTO_HUGE_NUMBER nx_crypto_ec_point_x;
    NX_CRYPTO_HUGE_NUMBER nx_crypto_ec_point_y;
    NX_CRYPTO_HUGE_NUMBER nx_crypto_ec_point_z;
} NX_CRYPTO_EC_POINT;

typedef struct
{
    USHORT *nx_crypto_ec_polynomial_data;
    UINT    nx_crypto_ec_polynomial_size;
} NX_CRYPTO_EC_POLYNOMIAL;

/* Define fixed points */
typedef struct
{

    /* Window width */
    UINT nx_crypto_ec_fixed_points_window_width;

    /* Bits of curve (m). */
    UINT nx_crypto_ec_fixed_points_bits;

    /* d = (m + w - 1) / w */
    UINT nx_crypto_ec_fixed_points_d;

    /* e = (d + 1) / 2 */
    UINT nx_crypto_ec_fixed_points_e;

    /* [a(w-1),...a(0)]G */
    /* 0G and 1G are not stored. */
    /* The count of fixed points are 2 ^ w - 2. */
    NX_CRYPTO_EC_POINT *nx_crypto_ec_fixed_points_array;

    /* 2^e[a(w-1),...a(0)]G */
    /* 0G is not stored. */
    /* The count of fixed points are 2 ^ w - 1. */
    NX_CRYPTO_EC_POINT *nx_crypto_ec_fixed_points_array_2e;
} NX_CRYPTO_EC_FIXED_POINTS;

/* Define Elliptic Curve. */
typedef struct NX_CRYPTO_EC_STRUCT
{
    CHAR *nx_crypto_ec_name;
    UINT  nx_crypto_ec_id;
    UINT  nx_crypto_ec_window_width;
    UINT  nx_crypto_ec_bits;
    union
    {
        NX_CRYPTO_HUGE_NUMBER   fp;
        NX_CRYPTO_EC_POLYNOMIAL f2m;
    }                          nx_crypto_ec_field;
    NX_CRYPTO_HUGE_NUMBER      nx_crypto_ec_a;
    NX_CRYPTO_HUGE_NUMBER      nx_crypto_ec_b;
    NX_CRYPTO_EC_POINT         nx_crypto_ec_g;
    NX_CRYPTO_HUGE_NUMBER      nx_crypto_ec_n;
    NX_CRYPTO_HUGE_NUMBER      nx_crypto_ec_h;
    NX_CRYPTO_EC_FIXED_POINTS *nx_crypto_ec_fixed_points;
    VOID (*nx_crypto_ec_add)(struct NX_CRYPTO_EC_STRUCT *curve,
                             NX_CRYPTO_EC_POINT *left,
                             NX_CRYPTO_EC_POINT *right,
                             HN_UBASE *scratch);
    VOID (*nx_crypto_ec_subtract)(struct NX_CRYPTO_EC_STRUCT *curve,
                                  NX_CRYPTO_EC_POINT *left,
                                  NX_CRYPTO_EC_POINT *right,
                                  HN_UBASE *scratch);
    VOID (*nx_crypto_ec_multiple)(struct NX_CRYPTO_EC_STRUCT *curve,
                                  NX_CRYPTO_EC_POINT *g,
                                  NX_CRYPTO_HUGE_NUMBER *d,
                                  NX_CRYPTO_EC_POINT *r,
                                  HN_UBASE *scratch);
    VOID (*nx_crypto_ec_reduce)(struct NX_CRYPTO_EC_STRUCT *curve,
                                NX_CRYPTO_HUGE_NUMBER *value,
                                HN_UBASE *scratch);
} NX_CRYPTO_EC;

#define NX_CRYPTO_EC_POINT_INITIALIZE(p, type, buff, size)                              \
    NX_CRYPTO_HUGE_NUMBER_INITIALIZE(&((p) -> nx_crypto_ec_point_x), buff, size);       \
    NX_CRYPTO_HUGE_NUMBER_INITIALIZE(&((p) -> nx_crypto_ec_point_y), buff, size);       \
    if ((type) == NX_CRYPTO_EC_POINT_PROJECTIVE) {                                      \
        NX_CRYPTO_HUGE_NUMBER_INITIALIZE(&((p) -> nx_crypto_ec_point_z), buff, size); } \
    (p) -> nx_crypto_ec_point_type = (type);

#define NX_CRYPTO_EC_POINT_SETUP(p, x, x_size, y, y_size, z, z_size)         \
    _nx_crypto_huge_number_setup(&((p) -> nx_crypto_ec_point_x), x, x_size); \
    _nx_crypto_huge_number_setup(&((p) -> nx_crypto_ec_point_y), y, y_size); \
    if ((p) -> nx_crypto_ec_point_type == NX_CRYPTO_EC_POINT_PROJECTIVE) {   \
        _nx_crypto_huge_number_setup(&((p) -> nx_crypto_ec_point_z), z, z_size); }

#define NX_CRYPTO_EC_POINT_EXTRACT(p, x, x_size, x_out_size, y, y_size, y_out_size, z, z_size, z_out_size) \
    _nx_crypto_huge_number_extract(&((p) -> nx_crypto_ec_point_x), x, x_size, x_out_size);                 \
    _nx_crypto_huge_number_extract(&((p) -> nx_crypto_ec_point_y), y, y_size, y_out_size);                 \
    if ((p) -> nx_crypto_ec_point_type == NX_CRYPTO_EC_POINT_PROJECTIVE) {                                 \
        _nx_crypto_huge_number_extract(&((p) -> nx_crypto_ec_point_z), z, z_size, z_out_size); }

#define NX_CRYPTO_EC_MULTIPLE_DIGIT_REDUCE(curve, value, digit, result, scratch) \
    _nx_crypto_huge_number_multiply_digit(value, digit, result);                 \
    curve -> nx_crypto_ec_reduce(curve, result, scratch);

#define NX_CRYPTO_EC_MULTIPLE_REDUCE(curve, left, right, result, scratch) \
    _nx_crypto_huge_number_multiply(left, right, result);                 \
    curve -> nx_crypto_ec_reduce(curve, result, scratch);

#define NX_CRYPTO_EC_SQUARE_REDUCE(curve, value, result, scratch) \
    _nx_crypto_huge_number_square(value, result);                 \
    curve -> nx_crypto_ec_reduce(curve, result, scratch);

#define NX_CRYPTO_EC_SHIFT_LEFT_REDUCE(curve, value, shift, scratch) \
    _nx_crypto_huge_number_shift_left(value, shift);                 \
    curve -> nx_crypto_ec_reduce(curve, value, scratch);

#define NX_CRYPTO_EC_SECP192R1_DATA_SETUP(s, b, c0, c1, c2, c3, c4, c5) \
    b[0] = c0; b[1] = c1; b[2] = c2;                                    \
    b[3] = c3; b[4] = c4; b[5] = c5;                                    \
    _nx_crypto_huge_number_setup(s, (UCHAR *)b, 24);

#define NX_CRYPTO_EC_SECP224R1_DATA_SETUP(s, b, c0, c1, c2, c3, c4, c5, c6) \
    b[0] = c0; b[1] = c1; b[2] = c2;  b[3] = c3;                            \
    b[4] = c4; b[5] = c5; b[6] = c6;                                        \
    _nx_crypto_huge_number_setup(s, (UCHAR *)b, 28);

#define NX_CRYPTO_EC_SECP256R1_DATA_SETUP(s, b, c0, c1, c2, c3, c4, c5, c6, c7) \
    b[0] = c0; b[1] = c1; b[2] = c2;  b[3] = c3;                                \
    b[4] = c4; b[5] = c5; b[6] = c6;  b[7] = c7;                                \
    _nx_crypto_huge_number_setup(s, (UCHAR *)b, 32);

#define NX_CRYPTO_EC_SECP384R1_DATA_SETUP(s, b, c0, c1, c2, c3, c4, c5, c6, c7, c8, c9, c10, c11) \
    b[0] = c0; b[1] = c1; b[2] = c2;  b[3] = c3;                                                  \
    b[4] = c4; b[5] = c5; b[6] = c6;  b[7] = c7;                                                  \
    b[8] = c8; b[9] = c9; b[10] = c10;  b[11] = c11;                                              \
    _nx_crypto_huge_number_setup(s, (UCHAR *)b, 48);

extern NX_CRYPTO_CONST NX_CRYPTO_EC _nx_crypto_ec_secp192r1;
extern NX_CRYPTO_CONST NX_CRYPTO_EC _nx_crypto_ec_secp224r1;
extern NX_CRYPTO_CONST NX_CRYPTO_EC _nx_crypto_ec_secp256r1;
extern NX_CRYPTO_CONST NX_CRYPTO_EC _nx_crypto_ec_secp384r1;
extern NX_CRYPTO_CONST NX_CRYPTO_EC _nx_crypto_ec_secp521r1;

#define NX_CRYPTO_EC_GET_SECP192R1(curve) curve = (NX_CRYPTO_EC *)&_nx_crypto_ec_secp192r1
#define NX_CRYPTO_EC_GET_SECP224R1(curve) curve = (NX_CRYPTO_EC *)&_nx_crypto_ec_secp224r1
#define NX_CRYPTO_EC_GET_SECP256R1(curve) curve = (NX_CRYPTO_EC *)&_nx_crypto_ec_secp256r1
#define NX_CRYPTO_EC_GET_SECP384R1(curve) curve = (NX_CRYPTO_EC *)&_nx_crypto_ec_secp384r1
#define NX_CRYPTO_EC_GET_SECP521R1(curve) curve = (NX_CRYPTO_EC *)&_nx_crypto_ec_secp521r1

UINT _nx_crypto_ec_point_is_infinite(NX_CRYPTO_EC_POINT *point);
VOID _nx_crypto_ec_point_set_infinite(NX_CRYPTO_EC_POINT *point);
UINT _nx_crypto_ec_point_setup(NX_CRYPTO_EC_POINT *point, UCHAR *byte_stream, UINT byte_stream_size);
VOID _nx_crypto_ec_point_extract_uncompressed(NX_CRYPTO_EC *curve, NX_CRYPTO_EC_POINT *point, UCHAR *byte_stream,
                                              UINT byte_stream_size, UINT *huge_number_size);

VOID _nx_crypto_ec_point_fp_affine_to_projective(NX_CRYPTO_EC_POINT *point);
VOID _nx_crypto_ec_point_fp_projective_to_affine(NX_CRYPTO_EC *curve,
                                                 NX_CRYPTO_EC_POINT *point,
                                                 HN_UBASE *scratch);

VOID _nx_crypto_ec_secp192r1_reduce(NX_CRYPTO_EC *curve,
                                    NX_CRYPTO_HUGE_NUMBER *value,
                                    HN_UBASE *scratch);
VOID _nx_crypto_ec_secp224r1_reduce(NX_CRYPTO_EC *curve,
                                    NX_CRYPTO_HUGE_NUMBER *value,
                                    HN_UBASE *scratch);
VOID _nx_crypto_ec_secp256r1_reduce(NX_CRYPTO_EC *curve,
                                    NX_CRYPTO_HUGE_NUMBER *value,
                                    HN_UBASE *scratch);
VOID _nx_crypto_ec_secp384r1_reduce(NX_CRYPTO_EC *curve,
                                    NX_CRYPTO_HUGE_NUMBER *value,
                                    HN_UBASE *scratch);
VOID _nx_crypto_ec_secp521r1_reduce(NX_CRYPTO_EC *curve,
                                    NX_CRYPTO_HUGE_NUMBER *value,
                                    HN_UBASE *scratch);
VOID _nx_crypto_ec_fp_reduce(NX_CRYPTO_EC *curve,
                             NX_CRYPTO_HUGE_NUMBER *value,
                             HN_UBASE *scratch);
VOID _nx_crypto_ec_fp_projective_add(NX_CRYPTO_EC *curve,
                                     NX_CRYPTO_EC_POINT *projective_point,
                                     NX_CRYPTO_EC_POINT *affine_point,
                                     HN_UBASE *scratch);
VOID _nx_crypto_ec_fp_projective_double(NX_CRYPTO_EC *curve,
                                        NX_CRYPTO_EC_POINT *projective_point,
                                        HN_UBASE *scratch);
VOID _nx_crypto_ec_fp_affine_add(NX_CRYPTO_EC *curve,
                                 NX_CRYPTO_EC_POINT *left,
                                 NX_CRYPTO_EC_POINT *right,
                                 HN_UBASE *scratch);
VOID _nx_crypto_ec_fp_affine_subtract(NX_CRYPTO_EC *curve,
                                      NX_CRYPTO_EC_POINT *left,
                                      NX_CRYPTO_EC_POINT *right,
                                      HN_UBASE *scratch);
VOID _nx_crypto_ec_fp_projective_multiple(NX_CRYPTO_EC *curve,
                                          NX_CRYPTO_EC_POINT *g,
                                          NX_CRYPTO_HUGE_NUMBER *d,
                                          NX_CRYPTO_EC_POINT *r,
                                          HN_UBASE *scratch);
VOID _nx_crypto_ec_fp_fixed_multiple(NX_CRYPTO_EC *curve,
                                     NX_CRYPTO_HUGE_NUMBER *d,
                                     NX_CRYPTO_EC_POINT *r,
                                     HN_UBASE *scratch);

VOID _nx_crypto_ec_naf_compute(NX_CRYPTO_HUGE_NUMBER *d, HN_UBASE *naf_data, UINT *naf_size);
VOID _nx_crypto_ec_add_digit_reduce(NX_CRYPTO_EC *curve,
                                    NX_CRYPTO_HUGE_NUMBER *value,
                                    HN_UBASE digit,
                                    HN_UBASE *scratch);
VOID _nx_crypto_ec_subtract_digit_reduce(NX_CRYPTO_EC *curve,
                                         NX_CRYPTO_HUGE_NUMBER *value,
                                         HN_UBASE digit,
                                         HN_UBASE *scratch);
VOID _nx_crypto_ec_add_reduce(NX_CRYPTO_EC *curve,
                              NX_CRYPTO_HUGE_NUMBER *left,
                              NX_CRYPTO_HUGE_NUMBER *right,
                              HN_UBASE *scratch);
VOID _nx_crypto_ec_subtract_reduce(NX_CRYPTO_EC *curve,
                                   NX_CRYPTO_HUGE_NUMBER *left,
                                   NX_CRYPTO_HUGE_NUMBER *right,
                                   HN_UBASE *scratch);
VOID _nx_crypto_ec_precomputation(NX_CRYPTO_EC *curve,
                                  UINT window_width,
                                  UINT bits,
                                  HN_UBASE **scratch_pptr);
VOID _nx_crypto_ec_fixed_output(NX_CRYPTO_EC *curve,
                                INT (*output)(const CHAR *format, ...),
                                const CHAR *tab,
                                const CHAR *line_ending);
UINT _nx_crypto_ec_get_named_curve(NX_CRYPTO_EC **curve, UINT curve_id);
UINT _nx_crypto_ec_key_pair_generation_extra(NX_CRYPTO_EC *curve,
                                             NX_CRYPTO_EC_POINT *g,
                                             NX_CRYPTO_HUGE_NUMBER *private_key,
                                             NX_CRYPTO_EC_POINT *public_key,
                                             HN_UBASE *scratch);
NX_CRYPTO_KEEP UINT _nx_crypto_ec_key_pair_stream_generate(NX_CRYPTO_EC *curve,
                                                           UCHAR *output,
                                                           ULONG output_length_in_byte,
                                                           ULONG *actual_output_length,
                                                           HN_UBASE *scratch);

UINT _nx_crypto_method_ec_secp192r1_operation(UINT op,
                                              VOID *handle,
                                              struct NX_CRYPTO_METHOD_STRUCT *method,
                                              UCHAR *key, NX_CRYPTO_KEY_SIZE key_size_in_bits,
                                              UCHAR *input, ULONG input_length_in_byte,
                                              UCHAR *iv_ptr,
                                              UCHAR *output, ULONG output_length_in_byte,
                                              VOID *crypto_metadata, ULONG crypto_metadata_size,
                                              VOID *packet_ptr,
                                              VOID (*nx_crypto_hw_process_callback)(VOID *, UINT));
UINT _nx_crypto_method_ec_secp224r1_operation(UINT op,
                                              VOID *handle,
                                              struct NX_CRYPTO_METHOD_STRUCT *method,
                                              UCHAR *key, NX_CRYPTO_KEY_SIZE key_size_in_bits,
                                              UCHAR *input, ULONG input_length_in_byte,
                                              UCHAR *iv_ptr,
                                              UCHAR *output, ULONG output_length_in_byte,
                                              VOID *crypto_metadata, ULONG crypto_metadata_size,
                                              VOID *packet_ptr,
                                              VOID (*nx_crypto_hw_process_callback)(VOID *, UINT));
UINT _nx_crypto_method_ec_secp256r1_operation(UINT op,
                                              VOID *handle,
                                              struct NX_CRYPTO_METHOD_STRUCT *method,
                                              UCHAR *key, NX_CRYPTO_KEY_SIZE key_size_in_bits,
                                              UCHAR *input, ULONG input_length_in_byte,
                                              UCHAR *iv_ptr,
                                              UCHAR *output, ULONG output_length_in_byte,
                                              VOID *crypto_metadata, ULONG crypto_metadata_size,
                                              VOID *packet_ptr,
                                              VOID (*nx_crypto_hw_process_callback)(VOID *, UINT));
UINT _nx_crypto_method_ec_secp384r1_operation(UINT op,
                                              VOID *handle,
                                              struct NX_CRYPTO_METHOD_STRUCT *method,
                                              UCHAR *key, NX_CRYPTO_KEY_SIZE key_size_in_bits,
                                              UCHAR *input, ULONG input_length_in_byte,
                                              UCHAR *iv_ptr,
                                              UCHAR *output, ULONG output_length_in_byte,
                                              VOID *crypto_metadata, ULONG crypto_metadata_size,
                                              VOID *packet_ptr,
                                              VOID (*nx_crypto_hw_process_callback)(VOID *, UINT));
UINT _nx_crypto_method_ec_secp521r1_operation(UINT op,
                                              VOID *handle,
                                              struct NX_CRYPTO_METHOD_STRUCT *method,
                                              UCHAR *key, NX_CRYPTO_KEY_SIZE key_size_in_bits,
                                              UCHAR *input, ULONG input_length_in_byte,
                                              UCHAR *iv_ptr,
                                              UCHAR *output, ULONG output_length_in_byte,
                                              VOID *crypto_metadata, ULONG crypto_metadata_size,
                                              VOID *packet_ptr,
                                              VOID (*nx_crypto_hw_process_callback)(VOID *, UINT));
#ifdef __cplusplus
}
#endif

#endif /* NX_CRYPTO_EC_H */

