/* src/matioConfig.h.  Generated from matioConfig.h.in by configure.  */
/*
 * Copyright (c) 2015-2022, The matio contributors
 * Copyright (c) 2012-2014, Christopher C. Hulbert
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

/* Debug enabled */
/* #undef DEBUG */

/* Extended sparse matrix data types */
#define EXTENDED_SPARSE /**/

/* Define to dummy `main' function (if any) required to link to the Fortran
   libraries. */
/* #undef FC_DUMMY_MAIN */

/* Define if F77 and FC dummy `main' functions are identical. */
/* #undef FC_DUMMY_MAIN_EQ_F77 */

/* Define to a macro mangling the given C identifier (in lower and upper
   case), which must not contain underscores, for linking with Fortran. */
/* #undef FC_FUNC */

/* As FC_FUNC, but for C identifiers containing underscores. */
/* #undef FC_FUNC_ */

/* Define to 1 if you have the `fseeko' function. */
#define HAVE_FSEEKO 1

/* Define to 1 if you have the `ftello' function. */
#define HAVE_FTELLO 1

/* Define to 1 if you have the `fseeko64' function. */
/* #undef HAVE_FSEEKO64 */

/* Define to 1 if you have the `ftello64' function. */
/* #undef HAVE_FTELLO64 */

/* Define to 1 if you have the `_fseeki64' function. */
/* #undef HAVE__FSEEKI64 */

/* Define to 1 if you have the `_ftelli64' function. */
/* #undef HAVE__FTELLI64 */

/* Define if 64-bit file address support in 32-bit OS. */
#define _FILE_OFFSET_BITS 64

/* Define if 64-bit file address support in 32-bit OS. */
#define _LARGEFILE64_SOURCE 1

/* Define to 1 if you have the `asprintf' function. */
#define HAVE_ASPRINTF 1

/* Define to 1 if you have the <dlfcn.h> header file. */
#define HAVE_DLFCN_H 1

/* Have HDF5 */
/* #undef HAVE_HDF5 */

/* Define to 1 if the system has the type `intmax_t'. */
#define HAVE_INTMAX_T 1

/* Define to 1 if you have the <inttypes.h> header file. */
#define HAVE_INTTYPES_H 1

/* Define to 1 if you have the <intsafe.h> header file. */
/* #undef HAVE_INTSAFE_H */

/* Define to 1 if you have the `m' library (-lm). */
#define HAVE_LIBM 1

/* Define to 1 if you have the `localeconv' function. */
#define HAVE_LOCALECONV 1

/* Define to 1 if you have the <locale.h> header file. */
#define HAVE_LOCALE_H 1

/* Define to 1 if the system has the type `long double'. */
#define HAVE_LONG_DOUBLE 1

/* Define to 1 if the system has the type `long long int'. */
#define HAVE_LONG_LONG_INT 1

/* Have MAT int16 */
#define HAVE_MAT_INT16_T /**/

/* Have MAT int32 */
#define HAVE_MAT_INT32_T /**/

/* Have MAT int64 */
#define HAVE_MAT_INT64_T /**/

/* Have MAT int8 */
#define HAVE_MAT_INT8_T /**/

/* Have MAT uint16 */
#define HAVE_MAT_UINT16_T /**/

/* Have MAT uint32 */
#define HAVE_MAT_UINT32_T /**/

/* Have MAT uint64 */
#define HAVE_MAT_UINT64_T /**/

/* Have MAT uint8 */
#define HAVE_MAT_UINT8_T /**/

/* Define to 1 if you have the <memory.h> header file. */
#define HAVE_MEMORY_H 1

/* Define to 1 if the system has the type `ptrdiff_t'. */
#define HAVE_PTRDIFF_T 1

/* Define to 1 if you have a C99 compliant `snprintf' function. */
#define HAVE_SNPRINTF 1

/* Define to 1 if you have the <stdarg.h> header file. */
#define HAVE_STDARG_H 1

/* Define to 1 if you have the <stddef.h> header file. */
#define HAVE_STDDEF_H 1

/* Define to 1 if you have the <stdint.h> header file. */
#define HAVE_STDINT_H 1

/* Define to 1 if you have the <stdlib.h> header file. */
#define HAVE_STDLIB_H 1

/* Define to 1 if you have the <strings.h> header file. */
#define HAVE_STRINGS_H 1

/* Define to 1 if you have the <string.h> header file. */
#define HAVE_STRING_H 1

/* Define to 1 if `decimal_point' is member of `struct lconv'. */
#define HAVE_STRUCT_LCONV_DECIMAL_POINT 1

/* Define to 1 if `thousands_sep' is member of `struct lconv'. */
#define HAVE_STRUCT_LCONV_THOUSANDS_SEP 1

/* Define to 1 if you have the <sys/stat.h> header file. */
#define HAVE_SYS_STAT_H 1

/* Define to 1 if you have the <sys/types.h> header file. */
#define HAVE_SYS_TYPES_H 1

/* Define to 1 if the system has the type `uintmax_t'. */
#define HAVE_UINTMAX_T 1

/* Define to 1 if the system has the type `uintptr_t'. */
#define HAVE_UINTPTR_T 1

/* Define to 1 if you have the <unistd.h> header file. */
#define HAVE_UNISTD_H 1

/* Define to 1 if the system has the type `unsigned long long int'. */
#define HAVE_UNSIGNED_LONG_LONG_INT 1

/* Define to 1 if you have the <varargs.h> header file. */
/* #undef HAVE_VARARGS_H */

/* Define to 1 if you have the `vasprintf' function. */
#define HAVE_VASPRINTF 1

/* Define to 1 if you have the `va_copy' function or macro. */
#define HAVE_VA_COPY 1

/* Define to 1 if you have a C99 compliant `vsnprintf' function. */
#define HAVE_VSNPRINTF 1

/* Have zlib */
#define HAVE_ZLIB 1

/* Define to 1 if you have the `__va_copy' function or macro. */
/* #undef HAVE___VA_COPY */

/* OS is Linux */
/* #undef LINUX */

/* Define to the sub-directory in which libtool stores uninstalled libraries.
   */
#define LT_OBJDIR ".libs/"

/* MAT v7.3 file support */
/* #undef MAT73 */

/* Platform */
#define MATIO_PLATFORM "x86_64-apple-darwin21.6.0"

/* Debug disabled */
#define NODEBUG /**/

/* Fixed types in safe-math.h disabled */
#define PSNIP_SAFE_NO_FIXED 1

/* Name of package */
#define PACKAGE "matio"

/* Define to the address where bug reports for this package should be sent. */
#define PACKAGE_BUGREPORT "t-beu@users.sourceforge.net"

/* Define to the full name of this package. */
#define PACKAGE_NAME "MATIO"

/* Define to the full name and version of this package. */
#define PACKAGE_STRING "MATIO 1.5.23"

/* Define to the one symbol short name of this package. */
#define PACKAGE_TARNAME "matio"

/* Define to the home page for this package. */
#define PACKAGE_URL "https://sourceforge.net/projects/matio"

/* Define to the version of this package. */
#define PACKAGE_VERSION "1.5.23"

/* The size of `char', as computed by sizeof. */
#define SIZEOF_CHAR 1

/* The size of `double', as computed by sizeof. */
#define SIZEOF_DOUBLE 8

/* The size of `float', as computed by sizeof. */
#define SIZEOF_FLOAT 4

/* The size of `int', as computed by sizeof. */
#define SIZEOF_INT 4

/* The size of `long', as computed by sizeof. */
#define SIZEOF_LONG 8

/* The size of `long long', as computed by sizeof. */
#define SIZEOF_LONG_LONG 8

/* The size of `short', as computed by sizeof. */
#define SIZEOF_SHORT 2

/* The size of `size_t', as computed by sizeof. */
#define SIZEOF_SIZE_T 8

/* The size of `void *', as computed by sizeof. */
#define SIZEOF_VOID_P 8

/* Define to 1 if you have the ANSI C header files. */
#define STDC_HEADERS 1

/* OS is Solaris */
/* #undef SUN */

/* Version number of package */
#define VERSION "1.5.23"

/* Z prefix */
/* #undef Z_PREFIX */
