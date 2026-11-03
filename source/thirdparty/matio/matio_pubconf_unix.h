/*
 * Copyright (c) 2015-2026, The matio contributors
 * Copyright (c) 2010-2014, Christopher C. Hulbert
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#ifndef MATIO_PUBCONF_H
#define MATIO_PUBCONF_H 1

/* Matio major version number */
#define MATIO_MAJOR_VERSION 1

/* Matio minor version number */
#define MATIO_MINOR_VERSION 6

/* Matio release level number */
#define MATIO_RELEASE_LEVEL 0

/* Matio version number */
#define MATIO_VERSION 160

/* Matio version string */
#define MATIO_VERSION_STR "1.6.0"

/* Default file format */
#define MAT_FT_DEFAULT MAT_FT_MAT5

/* Define to 1 if you have the <stdint.h> header file. */
#define MATIO_HAVE_STDINT_H 1

/* Define to 1 if you have the <inttypes.h> header file. */
#define MATIO_HAVE_INTTYPES_H 1

#if !MATIO_HAVE_STDINT_H && defined(_MSC_VER)
#include "stdint_msvc.h"
#endif

/* int16 type */
#define _mat_int16_t int16_t

/* int32 type */
#define _mat_int32_t int32_t

/* int64 type */
#define _mat_int64_t int64_t

/* int8 type */
#define _mat_int8_t int8_t

/* uint16 type */
#define _mat_uint16_t uint16_t

/* uint32 type */
#define _mat_uint32_t uint32_t

/* uint64 type */
#define _mat_uint64_t uint64_t

/* uint8 type */
#define _mat_uint8_t uint8_t

#if MATIO_HAVE_INTTYPES_H
#   include <inttypes.h>
#endif

#if MATIO_HAVE_STDINT_H
#   include <stdint.h>
#endif

#ifdef _mat_int64_t
    typedef _mat_int64_t mat_int64_t;
#endif
#ifdef _mat_uint64_t
    typedef _mat_uint64_t mat_uint64_t;
#endif
#ifdef _mat_int32_t
    typedef _mat_int32_t mat_int32_t;
#endif
#ifdef _mat_uint32_t
    typedef _mat_uint32_t mat_uint32_t;
#endif
#ifdef _mat_int16_t
    typedef _mat_int16_t mat_int16_t;
#endif
#ifdef _mat_uint16_t
    typedef _mat_uint16_t mat_uint16_t;
#endif
#ifdef _mat_int8_t
    typedef _mat_int8_t mat_int8_t;
#endif
#ifdef _mat_uint8_t
    typedef _mat_uint8_t mat_uint8_t;
#endif

/*
  The following macros handle noreturn attributes according to the
  C11/C++11 standard with fallback to GNU, Clang or MSVC extensions if using
  an older compiler.
*/
#if defined(__STDC_VERSION__) && __STDC_VERSION__ >= 201112L
#define MATIO_NORETURN _Noreturn
#define MATIO_NORETURNATTR
#elif __cplusplus >= 201103L
#if (defined(__GNUC__) && __GNUC__ >= 5) || \
    (defined(__GNUC__) && defined(__GNUC_MINOR__) && __GNUC__ == 4 && __GNUC_MINOR__ >= 8)
#define MATIO_NORETURN [[noreturn]]
#define MATIO_NORETURNATTR
#elif (defined(__GNUC__) && __GNUC__ >= 3) || \
      (defined(__GNUC__) && defined(__GNUC_MINOR__) && __GNUC__ == 2 && __GNUC_MINOR__ >= 8)
#define MATIO_NORETURN
#define MATIO_NORETURNATTR __attribute__((noreturn))
#elif defined(__GNUC__)
#define MATIO_NORETURN
#define MATIO_NORETURNATTR
#else
#define MATIO_NORETURN [[noreturn]]
#define MATIO_NORETURNATTR
#endif
#elif defined(__clang__)
#if __has_attribute(noreturn)
#define MATIO_NORETURN
#define MATIO_NORETURNATTR __attribute__((noreturn))
#else
#define MATIO_NORETURN
#define MATIO_NORETURNATTR
#endif
#elif (defined(__GNUC__) && __GNUC__ >= 3) || \
      (defined(__GNUC__) && defined(__GNUC_MINOR__) && __GNUC__ == 2 && __GNUC_MINOR__ >= 8) || \
      (defined(__SUNPRO_C) && __SUNPRO_C >= 0x5110)
#define MATIO_NORETURN
#define MATIO_NORETURNATTR __attribute__((noreturn))
#elif (defined(_MSC_VER) && _MSC_VER >= 1200) || \
       defined(__BORLANDC__)
#define MATIO_NORETURN __declspec(noreturn)
#define MATIO_NORETURNATTR
#else
#define MATIO_NORETURN
#define MATIO_NORETURNATTR
#endif

/*
  The following macros handle format attributes for type-checks against a
  format string.
*/

#if defined(__GNUC__) && __GNUC__ >= 3
#define MATIO_FORMATATTR_PRINTF1 __attribute__((format(printf, 1, 2)))
#define MATIO_FORMATATTR_PRINTF2 __attribute__((format(printf, 2, 3)))
#define MATIO_FORMATATTR_VPRINTF __attribute__((format(printf, 1, 0)))
#elif defined(__clang__)
#if __has_attribute(format)
#define MATIO_FORMATATTR_PRINTF1 __attribute__((format(printf, 1, 2)))
#define MATIO_FORMATATTR_PRINTF2 __attribute__((format(printf, 2, 3)))
#define MATIO_FORMATATTR_VPRINTF __attribute__((format(printf, 1, 0)))
#else
#define MATIO_FORMATATTR_PRINTF1
#define MATIO_FORMATATTR_PRINTF2
#define MATIO_FORMATATTR_VPRINTF
#endif
#else
#define MATIO_FORMATATTR_PRINTF1
#define MATIO_FORMATATTR_PRINTF2
#define MATIO_FORMATATTR_VPRINTF
#endif

/*
  The following macro handles the deprecated attribute.
*/

#if defined(__GNUC__) || defined(__clang__)
#define MATIO_DEPRECATED __attribute__((deprecated))
#elif defined(_MSC_VER)
#define MATIO_DEPRECATED __declspec(deprecated)
#else
#define MATIO_DEPRECATED
#endif

#endif /* MATIO_PUBCONF_H */
