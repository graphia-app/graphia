/* src/matio_pubconf.h.  Generated from matio_pubconf.h.in by configure.  */
/*
 * Copyright (c) 2015-2022, The matio contributors
 * Copyright (c) 2010-2014, Christopher C. Hulbert
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this
 *    list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef MATIO_PUBCONF_H
#define MATIO_PUBCONF_H 1

/* Matio major version number */
#define MATIO_MAJOR_VERSION 1

/* Matio minor version number */
#define MATIO_MINOR_VERSION 5

/* Matio release level number */
#define MATIO_RELEASE_LEVEL 23

/* Matio version number */
#define MATIO_VERSION 1523

/* Matio version string */
#define MATIO_VERSION_STR "1.5.23"

/* Default file format */
#define MAT_FT_DEFAULT MAT_FT_MAT5

/* Define to 1 if you have the <stdint.h> header file. */
#define MATIO_HAVE_STDINT_H 1

/* Define to 1 if you have the <inttypes.h> header file. */
#define MATIO_HAVE_INTTYPES_H 1

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
#include <inttypes.h>
#endif

#if MATIO_HAVE_STDINT_H
#include <stdint.h>
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
#elif (defined(__GNUC__) && __GNUC__ >= 3) ||                                                 \
    (defined(__GNUC__) && defined(__GNUC_MINOR__) && __GNUC__ == 2 && __GNUC_MINOR__ >= 8) || \
    (defined(__SUNPRO_C) && __SUNPRO_C >= 0x5110)
#define MATIO_NORETURN
#define MATIO_NORETURNATTR __attribute__((noreturn))
#elif (defined(_MSC_VER) && _MSC_VER >= 1200) || defined(__BORLANDC__)
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

#endif /* MATIO_PUBCONF_H */
