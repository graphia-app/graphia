//=================================================================================================
/*!
//  \file blaze/math/simd/Setall.h
//  \brief Header file for the SIMD setall functionality
//
//  Copyright (C) 2012-2020 Klaus Iglberger - All Rights Reserved
//
//  This file is part of the Blaze library. You can redistribute it and/or modify it under
//  the terms of the New (Revised) BSD License. Redistribution and use in source and binary
//  forms, with or without modification, are permitted provided that the following conditions
//  are met:
//
//  1. Redistributions of source code must retain the above copyright notice, this list of
//     conditions and the following disclaimer.
//  2. Redistributions in binary form must reproduce the above copyright notice, this list
//     of conditions and the following disclaimer in the documentation and/or other materials
//     provided with the distribution.
//  3. Neither the names of the Blaze development group nor the names of its contributors
//     may be used to endorse or promote products derived from this software without specific
//     prior written permission.
//
//  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY
//  EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
//  OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT
//  SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
//  INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED
//  TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
//  BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
//  CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
//  ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
//  DAMAGE.
*/
//=================================================================================================

#ifndef _BLAZE_MATH_SIMD_SETALL_H_
#define _BLAZE_MATH_SIMD_SETALL_H_


//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <blaze/math/simd/BasicTypes.h>
#include <blaze/system/Inline.h>
#include <blaze/system/Vectorization.h>
#include <blaze/util/constraints/Integral.h>
#include <blaze/util/EnableIf.h>
#include <blaze/util/mpl/If.h>
#include <blaze/util/StaticAssert.h>
#include <blaze/util/Types.h>
#include <blaze/util/typetraits/HasSize.h>
#include <blaze/util/typetraits/IsIntegral.h>
#include <blaze/util/typetraits/IsSigned.h>


namespace blaze {

//=================================================================================================
//
//  8-BIT INTEGRAL SIMD TYPES
//
//=================================================================================================

//*************************************************************************************************
/*! \cond BLAZE_INTERNAL */
#if BLAZE_AVX512F_MODE
template< typename T >
BLAZE_ALWAYS_INLINE const If_t< IsSigned_v<T>, SIMDint8, SIMDuint8 >
   setall_epi8( T v0  = 0, T v1  = 0, T v2  = 0, T v3  = 0
              , T v4  = 0, T v5  = 0, T v6  = 0, T v7  = 0
              , T v8  = 0, T v9  = 0, T v10 = 0, T v11 = 0
              , T v12 = 0, T v13 = 0, T v14 = 0, T v15 = 0
              , T v16 = 0, T v17 = 0, T v18 = 0, T v19 = 0
              , T v20 = 0, T v21 = 0, T v22 = 0, T v23 = 0
              , T v24 = 0, T v25 = 0, T v26 = 0, T v27 = 0
              , T v28 = 0, T v29 = 0, T v30 = 0, T v31 = 0
              , T v32 = 0, T v33 = 0, T v34 = 0, T v35 = 0
              , T v36 = 0, T v37 = 0, T v38 = 0, T v39 = 0
              , T v40 = 0, T v41 = 0, T v42 = 0, T v43 = 0
              , T v44 = 0, T v45 = 0, T v46 = 0, T v47 = 0
              , T v48 = 0, T v49 = 0, T v50 = 0, T v51 = 0
              , T v52 = 0, T v53 = 0, T v54 = 0, T v55 = 0
              , T v56 = 0, T v57 = 0, T v58 = 0, T v59 = 0
              , T v60 = 0, T v61 = 0, T v62 = 0, T v63 = 0 ) noexcept
{
   return _mm512_set_epi8( v63, v62, v61, v60, v59, v58, v57, v56, v55, v54, v53, v52, v51, v50, v49, v48
                         , v47, v46, v45, v44, v43, v42, v41, v40, v39, v38, v37, v36, v35, v34, v33, v32
                         , v31, v30, v29, v28, v27, v26, v25, v24, v23, v22, v21, v20, v19, v18, v17, v16
                         , v15, v14, v13, v12, v11, v10, v9, v8, v7, v6, v5, v4, v3, v2, v1, v0 );
}
#elif BLAZE_AVX2_MODE
template< typename T >
BLAZE_ALWAYS_INLINE const If_t< IsSigned_v<T>, SIMDint8, SIMDuint8 >
   setall_epi8( T v0  = 0, T v1  = 0, T v2  = 0, T v3  = 0
              , T v4  = 0, T v5  = 0, T v6  = 0, T v7  = 0
              , T v8  = 0, T v9  = 0, T v10 = 0, T v11 = 0
              , T v12 = 0, T v13 = 0, T v14 = 0, T v15 = 0
              , T v16 = 0, T v17 = 0, T v18 = 0, T v19 = 0
              , T v20 = 0, T v21 = 0, T v22 = 0, T v23 = 0
              , T v24 = 0, T v25 = 0, T v26 = 0, T v27 = 0
              , T v28 = 0, T v29 = 0, T v30 = 0, T v31 = 0 ) noexcept
{
   return _mm256_set_epi8( v31, v30, v29, v28, v27, v26, v25, v24, v23, v22, v21, v20, v19, v18, v17, v16
                         , v15, v14, v13, v12, v11, v10, v9, v8, v7, v6, v5, v4, v3, v2, v1, v0 );
}
#elif BLAZE_SSE2_MODE
template< typename T >
BLAZE_ALWAYS_INLINE const If_t< IsSigned_v<T>, SIMDint8, SIMDuint8 >
   setall_epi8( T v0  = 0, T v1  = 0, T v2  = 0, T v3  = 0
              , T v4  = 0, T v5  = 0, T v6  = 0, T v7  = 0
              , T v8  = 0, T v9  = 0, T v10 = 0, T v11 = 0
              , T v12 = 0, T v13 = 0, T v14 = 0, T v15 = 0 ) noexcept
{
   return _mm_set_epi8( v15, v14, v13, v12, v11, v10, v9, v8, v7, v6, v5, v4, v3, v2, v1, v0 );
}
#else
template< typename T >
BLAZE_ALWAYS_INLINE const If_t< IsSigned_v<T>, SIMDint8, SIMDuint8 >
   setall_epi8( T v0 = 0 ) noexcept
{
   return v0;
}
#endif
/*! \endcond */
//*************************************************************************************************


//*************************************************************************************************
/*!\brief Sets all values in a SIMD vector to the given 1-byte integral values.
// \ingroup simd
//
// \param v0 The first given 1-byte integral value.
// \param vs The remaining 1-byte integral values.
// \return The SIMD vector of 1-byte integral values.
//
// This function sets the values of a SIMD vector to the given values. Depending on the available
// SIMD instruction set, the function accepts another maximum number of values:
//
// <table>
//    <tr><th>SIMD instruction set<th>Maximum number of values
//    <tr><td>SSE    <td>16
//    <tr><td>AVX    <td>32
//    <tr><td>AVX-512<td>64
// </table>
//
// In case less arguments are given than the SIMD vector contains values, the missing values are
// set to 0.
*/
template< typename T
        , typename... Ts >
BLAZE_ALWAYS_INLINE const EnableIf_t< IsIntegral_v<T> && HasSize_v<T,1UL>
                                    , If_t< IsSigned_v<T>, SIMDint8, SIMDuint8 > >
   setall( T v0, Ts... vs ) noexcept
{
   return setall_epi8( v0, vs... );
}
//*************************************************************************************************


//*************************************************************************************************
/*! \cond BLAZE_INTERNAL */
#if BLAZE_AVX512F_MODE
template< typename T >
BLAZE_ALWAYS_INLINE const If_t< IsSigned_v<T>, SIMDcint8, SIMDcuint8 >
   setall_epi8( const complex<T>& v0  = 0, const complex<T>& v1  = 0
              , const complex<T>& v2  = 0, const complex<T>& v3  = 0
              , const complex<T>& v4  = 0, const complex<T>& v5  = 0
              , const complex<T>& v6  = 0, const complex<T>& v7  = 0
              , const complex<T>& v8  = 0, const complex<T>& v9  = 0
              , const complex<T>& v10 = 0, const complex<T>& v11 = 0
              , const complex<T>& v12 = 0, const complex<T>& v13 = 0
              , const complex<T>& v14 = 0, const complex<T>& v15 = 0
              , const complex<T>& v16 = 0, const complex<T>& v17 = 0
              , const complex<T>& v18 = 0, const complex<T>& v19 = 0
              , const complex<T>& v20 = 0, const complex<T>& v21 = 0
              , const complex<T>& v22 = 0, const complex<T>& v23 = 0
              , const complex<T>& v24 = 0, const complex<T>& v25 = 0
              , const complex<T>& v26 = 0, const complex<T>& v27 = 0
              , const complex<T>& v28 = 0, const complex<T>& v29 = 0
              , const complex<T>& v30 = 0, const complex<T>& v31 = 0 ) noexcept
{
   return _mm512_set_epi8( v31.imag(), v31.real(), v30.imag(), v30.real()
                         , v29.imag(), v29.real(), v28.imag(), v28.real()
                         , v27.imag(), v27.real(), v26.imag(), v26.real()
                         , v25.imag(), v25.real(), v24.imag(), v24.real()
                         , v23.imag(), v23.real(), v22.imag(), v22.real()
                         , v21.imag(), v21.real(), v20.imag(), v20.real()
                         , v19.imag(), v19.real(), v18.imag(), v18.real()
                         , v17.imag(), v17.real(), v16.imag(), v16.real()
                         , v15.imag(), v15.real(), v14.imag(), v14.real()
                         , v13.imag(), v13.real(), v12.imag(), v12.real()
                         , v11.imag(), v11.real(), v10.imag(), v10.real()
                         , v9.imag(), v9.real(), v8.imag(), v8.real()
                         , v7.imag(), v7.real(), v6.imag(), v6.real()
                         , v5.imag(), v5.real(), v4.imag(), v4.real()
                         , v3.imag(), v3.real(), v2.imag(), v2.real()
                         , v1.imag(), v1.real(), v0.imag(), v0.real() );
}
#elif BLAZE_AVX2_MODE
template< typename T >
BLAZE_ALWAYS_INLINE const If_t< IsSigned_v<T>, SIMDcint8, SIMDcuint8 >
   setall_epi8( const complex<T>& v0 = 0, const complex<T>& v1 = 0
              , const complex<T>& v2 = 0, const complex<T>& v3 = 0
              , const complex<T>& v4 = 0, const complex<T>& v5 = 0
              , const complex<T>& v6 = 0, const complex<T>& v7 = 0
              , const complex<T>& v8 = 0, const complex<T>& v9 = 0
              , const complex<T>& v10 = 0, const complex<T>& v11 = 0
              , const complex<T>& v12 = 0, const complex<T>& v13 = 0
              , const complex<T>& v14 = 0, const complex<T>& v15 = 0 ) noexcept
{
   return _mm256_set_epi8( v15.imag(), v15.real(), v14.imag(), v14.real()
                         , v13.imag(), v13.real(), v12.imag(), v12.real()
                         , v11.imag(), v11.real(), v10.imag(), v10.real()
                         , v9.imag(), v9.real(), v8.imag(), v8.real()
                         , v7.imag(), v7.real(), v6.imag(), v6.real()
                         , v5.imag(), v5.real(), v4.imag(), v4.real()
                         , v3.imag(), v3.real(), v2.imag(), v2.real()
                         , v1.imag(), v1.real(), v0.imag(), v0.real() );
}
#elif BLAZE_SSE2_MODE
template< typename T >
BLAZE_ALWAYS_INLINE const If_t< IsSigned_v<T>, SIMDcint8, SIMDcuint8 >
   setall_epi8( const complex<T>& v0 = 0, const complex<T>& v1 = 0
              , const complex<T>& v2 = 0, const complex<T>& v3 = 0
              , const complex<T>& v4 = 0, const complex<T>& v5 = 0
              , const complex<T>& v6 = 0, const complex<T>& v7 = 0 ) noexcept
{
   return _mm_set_epi8( v7.imag(), v7.real(), v6.imag(), v6.real()
                      , v5.imag(), v5.real(), v4.imag(), v4.real()
                      , v3.imag(), v3.real(), v2.imag(), v2.real()
                      , v1.imag(), v1.real(), v0.imag(), v0.real() );
}
#else
template< typename T >
BLAZE_ALWAYS_INLINE const If_t< IsSigned_v<T>, SIMDcint8, SIMDcuint8 >
   setall_epi8( const complex<T>& v0 = 0 ) noexcept
{
   return v0;
}
#endif
/*! \endcond */
//*************************************************************************************************


//*************************************************************************************************
/*!\brief Sets all values in the vector to the given 1-byte integral complex values.
// \ingroup simd
//
// \param v0 The first given 1-byte integral complex value.
// \param vs The remaining 1-byte integral complex values.
// \return The SIMD vector of 1-byte integral complex values.
//
// This function sets the values of a SIMD vector to the given values. Depending on the available
// SIMD instruction set, the function accepts another maximum number of values:
//
// <table>
//    <tr><th>SIMD instruction set<th>Maximum number of values
//    <tr><td>SSE    <td>8
//    <tr><td>AVX    <td>16
//    <tr><td>AVX-512<td>32
// </table>
//
// In case less arguments are given than the SIMD vector contains values, the missing values are
// set to 0.
*/
template< typename T
        , typename... Ts >
BLAZE_ALWAYS_INLINE const EnableIf_t< IsIntegral_v<T> && HasSize_v<T,1UL>
                                    , If_t< IsSigned_v<T>, SIMDcint8, SIMDcuint8 > >
   setall( const complex<T>& v0, const complex<Ts>&... vs ) noexcept
{
   return setall_epi8( v0, vs... );

   BLAZE_STATIC_ASSERT( sizeof( complex<T> ) == 2UL*sizeof( T ) );
}
//*************************************************************************************************




//=================================================================================================
//
//  16-BIT INTEGRAL SIMD TYPES
//
//=================================================================================================

//*************************************************************************************************
/*! \cond BLAZE_INTERNAL */
#if BLAZE_AVX512F_MODE
template< typename T >
BLAZE_ALWAYS_INLINE const If_t< IsSigned_v<T>, SIMDint16, SIMDuint16 >
   setall_epi16( T v0  = 0, T v1  = 0, T v2  = 0, T v3  = 0
               , T v4  = 0, T v5  = 0, T v6  = 0, T v7  = 0
               , T v8  = 0, T v9  = 0, T v10 = 0, T v11 = 0
               , T v12 = 0, T v13 = 0, T v14 = 0, T v15 = 0
               , T v16 = 0, T v17 = 0, T v18 = 0, T v19 = 0
               , T v20 = 0, T v21 = 0, T v22 = 0, T v23 = 0
               , T v24 = 0, T v25 = 0, T v26 = 0, T v27 = 0
               , T v28 = 0, T v29 = 0, T v30 = 0, T v31 = 0 ) noexcept
{
   return _mm512_set_epi16( v31, v30, v29, v28, v27, v26, v25, v24, v23, v22, v21, v20, v19, v18, v17, v16
                          , v15, v14, v13, v12, v11, v10, v9, v8, v7, v6, v5, v4, v3, v2, v1, v0 );
}
#elif BLAZE_AVX2_MODE
template< typename T >
BLAZE_ALWAYS_INLINE const If_t< IsSigned_v<T>, SIMDint16, SIMDuint16 >
   setall_epi16( T v0  = 0, T v1  = 0, T v2  = 0, T v3  = 0
               , T v4  = 0, T v5  = 0, T v6  = 0, T v7  = 0
               , T v8  = 0, T v9  = 0, T v10 = 0, T v11 = 0
               , T v12 = 0, T v13 = 0, T v14 = 0, T v15 = 0 ) noexcept
{
   return _mm256_set_epi16( v15, v14, v13, v12, v11, v10, v9, v8, v7, v6, v5, v4, v3, v2, v1, v0 );
}
#elif BLAZE_SSE2_MODE
template< typename T >
BLAZE_ALWAYS_INLINE const If_t< IsSigned_v<T>, SIMDint16, SIMDuint16 >
   setall_epi16( T v0 = 0, T v1 = 0, T v2 = 0, T v3 = 0
               , T v4 = 0, T v5 = 0, T v6 = 0, T v7 = 0 ) noexcept
{
   return _mm_set_epi16( v7, v6, v5, v4, v3, v2, v1, v0 );
}
#else
template< typename T >
BLAZE_ALWAYS_INLINE const If_t< IsSigned_v<T>, SIMDint16, SIMDuint16 >
   setall_epi16( T v0 = 0 ) noexcept
{
   return v0;
}
#endif
/*! \endcond */
//*************************************************************************************************


//*************************************************************************************************
/*!\brief Sets all values in a SIMD vector to the given 2-byte integral values.
// \ingroup simd
//
// \param v0 The first given 2-byte integral value.
// \param vs The remaining 2-byte integral values.
// \return The SIMD vector of 2-byte integral values.
//
// This function sets the values of a SIMD vector to the given values. Depending on the available
// SIMD instruction set, the function accepts another maximum number of values:
//
// <table>
//    <tr><th>SIMD instruction set<th>Maximum number of values
//    <tr><td>SSE    <td>8
//    <tr><td>AVX    <td>16
//    <tr><td>AVX-512<td>32
// </table>
//
// In case less arguments are given than the SIMD vector contains values, the missing values are
// set to 0.
*/
template< typename T
        , typename... Ts >
BLAZE_ALWAYS_INLINE const EnableIf_t< IsIntegral_v<T> && HasSize_v<T,2UL>
                                    , If_t< IsSigned_v<T>, SIMDint16, SIMDuint16 > >
   setall( T v0, Ts... vs ) noexcept
{
   return setall_epi16( v0, vs... );
}
//*************************************************************************************************


//*************************************************************************************************
/*! \cond BLAZE_INTERNAL */
#if BLAZE_AVX512F_MODE
template< typename T >
BLAZE_ALWAYS_INLINE const If_t< IsSigned_v<T>, SIMDcint16, SIMDcuint16 >
   setall_epi16( const complex<T>& v0  = 0, const complex<T>& v1  = 0
               , const complex<T>& v2  = 0, const complex<T>& v3  = 0
               , const complex<T>& v4  = 0, const complex<T>& v5  = 0
               , const complex<T>& v6  = 0, const complex<T>& v7  = 0
               , const complex<T>& v8  = 0, const complex<T>& v9  = 0
               , const complex<T>& v10 = 0, const complex<T>& v11 = 0
               , const complex<T>& v12 = 0, const complex<T>& v13 = 0
               , const complex<T>& v14 = 0, const complex<T>& v15 = 0 ) noexcept
{
   return _mm512_set_epi16( v15.imag(), v15.real(), v14.imag(), v14.real()
                          , v13.imag(), v13.real(), v12.imag(), v12.real()
                          , v11.imag(), v11.real(), v10.imag(), v10.real()
                          , v9.imag(), v9.real(), v8.imag(), v8.real()
                          , v7.imag(), v7.real(), v6.imag(), v6.real()
                          , v5.imag(), v5.real(), v4.imag(), v4.real()
                          , v3.imag(), v3.real(), v2.imag(), v2.real()
                          , v1.imag(), v1.real(), v0.imag(), v0.real() );
}
#elif BLAZE_AVX2_MODE
template< typename T >
BLAZE_ALWAYS_INLINE const If_t< IsSigned_v<T>, SIMDcint16, SIMDcuint16 >
   setall_epi16( const complex<T>& v0 = 0, const complex<T>& v1 = 0
               , const complex<T>& v2 = 0, const complex<T>& v3 = 0
               , const complex<T>& v4 = 0, const complex<T>& v5 = 0
               , const complex<T>& v6 = 0, const complex<T>& v7 = 0 ) noexcept
{
   return _mm256_set_epi16( v7.imag(), v7.real(), v6.imag(), v6.real()
                          , v5.imag(), v5.real(), v4.imag(), v4.real()
                          , v3.imag(), v3.real(), v2.imag(), v2.real()
                          , v1.imag(), v1.real(), v0.imag(), v0.real() );
}
#elif BLAZE_SSE2_MODE
template< typename T >
BLAZE_ALWAYS_INLINE const If_t< IsSigned_v<T>, SIMDcint16, SIMDcuint16 >
   setall_epi16( const complex<T>& v0 = 0, const complex<T>& v1 = 0
               , const complex<T>& v2 = 0, const complex<T>& v3 = 0 ) noexcept
{
   return _mm_set_epi16( v3.imag(), v3.real(), v2.imag(), v2.real()
                       , v1.imag(), v1.real(), v0.imag(), v0.real() );
}
#else
template< typename T >
BLAZE_ALWAYS_INLINE const If_t< IsSigned_v<T>, SIMDcint16, SIMDcuint16 >
   setall_epi16( const complex<T>& v0 = 0 ) noexcept
{
   return v0;
}
#endif
/*! \endcond */
//*************************************************************************************************


//*************************************************************************************************
/*!\brief Sets all values in the vector to the given 2-byte integral complex values.
// \ingroup simd
//
// \param v0 The first given 2-byte integral complex value.
// \param vs The remaining 2-byte integral complex values.
// \return The SIMD vector of 2-byte integral complex values.
//
// This function sets the values of a SIMD vector to the given values. Depending on the available
// SIMD instruction set, the function accepts another maximum number of values:
//
// <table>
//    <tr><th>SIMD instruction set<th>Maximum number of values
//    <tr><td>SSE    <td>4
//    <tr><td>AVX    <td>8
//    <tr><td>AVX-512<td>16
// </table>
//
// In case less arguments are given than the SIMD vector contains values, the missing values are
// set to 0.
*/
template< typename T
        , typename... Ts >
BLAZE_ALWAYS_INLINE const EnableIf_t< IsIntegral_v<T> && HasSize_v<T,2UL>
                                    , If_t< IsSigned_v<T>, SIMDcint16, SIMDcuint16 > >
   setall( const complex<T>& v0, const complex<Ts>&... vs ) noexcept
{
   return setall_epi16( v0, vs... );

   BLAZE_STATIC_ASSERT( sizeof( complex<T> ) == 2UL*sizeof( T ) );
}
//*************************************************************************************************




//=================================================================================================
//
//  32-BIT INTEGRAL SIMD TYPES
//
//=================================================================================================

//*************************************************************************************************
/*! \cond BLAZE_INTERNAL */
#if BLAZE_AVX512F_MODE
template< typename T >
BLAZE_ALWAYS_INLINE const If_t< IsSigned_v<T>, SIMDint32, SIMDuint32 >
   setall_epi32( T v0  = 0, T v1  = 0, T v2  = 0, T v3  = 0
               , T v4  = 0, T v5  = 0, T v6  = 0, T v7  = 0
               , T v8  = 0, T v9  = 0, T v10 = 0, T v11 = 0
               , T v12 = 0, T v13 = 0, T v14 = 0, T v15 = 0 ) noexcept
{
   return _mm512_set_epi32( v15, v14, v13, v12, v11, v10, v9, v8, v7, v6, v5, v4, v3, v2, v1, v0 );
}
#elif BLAZE_AVX2_MODE
template< typename T >
BLAZE_ALWAYS_INLINE const If_t< IsSigned_v<T>, SIMDint32, SIMDuint32 >
   setall_epi32( T v0 = 0, T v1 = 0, T v2 = 0, T v3 = 0
               , T v4 = 0, T v5 = 0, T v6 = 0, T v7 = 0 ) noexcept
{
   return _mm256_set_epi32( v7, v6, v5, v4, v3, v2, v1, v0 );
}
#elif BLAZE_SSE2_MODE
template< typename T >
BLAZE_ALWAYS_INLINE const If_t< IsSigned_v<T>, SIMDint32, SIMDuint32 >
   setall_epi32( T v0 = 0, T v1 = 0, T v2 = 0, T v3 = 0 ) noexcept
{
   return _mm_set_epi32( v3, v2, v1, v0 );
}
#else
template< typename T >
BLAZE_ALWAYS_INLINE const If_t< IsSigned_v<T>, SIMDint32, SIMDuint32 >
   setall_epi32( T v0 = 0 ) noexcept
{
   return v0;
}
#endif
/*! \endcond */
//*************************************************************************************************


//*************************************************************************************************
/*!\brief Sets all values in a SIMD vector to the given 4-byte integral values.
// \ingroup simd
//
// \param v0 The first given 4-byte integral value.
// \param vs The remaining 4-byte integral values.
// \return The SIMD vector of 4-byte integral values.
//
// This function sets the values of a SIMD vector to the given values. Depending on the available
// SIMD instruction set, the function accepts another maximum number of values:
//
// <table>
//    <tr><th>SIMD instruction set<th>Maximum number of values
//    <tr><td>SSE    <td>4
//    <tr><td>AVX    <td>8
//    <tr><td>AVX-512<td>16
// </table>
//
// In case less arguments are given than the SIMD vector contains values, the missing values are
// set to 0.
*/
template< typename T
        , typename... Ts >
BLAZE_ALWAYS_INLINE const EnableIf_t< IsIntegral_v<T> && HasSize_v<T,4UL>
                                    , If_t< IsSigned_v<T>, SIMDint32, SIMDuint32 > >
   setall( T v0, Ts... vs ) noexcept
{
   return setall_epi32( v0, vs... );
}
//*************************************************************************************************


//*************************************************************************************************
/*! \cond BLAZE_INTERNAL */
#if BLAZE_AVX512F_MODE
template< typename T >
BLAZE_ALWAYS_INLINE const If_t< IsSigned_v<T>, SIMDcint32, SIMDcuint32 >
   setall_epi32( const complex<T>& v0 = 0, const complex<T>& v1 = 0
               , const complex<T>& v2 = 0, const complex<T>& v3 = 0
               , const complex<T>& v4 = 0, const complex<T>& v5 = 0
               , const complex<T>& v6 = 0, const complex<T>& v7 = 0 ) noexcept
{
   return _mm512_set_epi32( v7.imag(), v7.real(), v6.imag(), v6.real()
                          , v5.imag(), v5.real(), v4.imag(), v4.real()
                          , v3.imag(), v3.real(), v2.imag(), v2.real()
                          , v1.imag(), v1.real(), v0.imag(), v0.real() );
}
#elif BLAZE_AVX2_MODE
template< typename T >
BLAZE_ALWAYS_INLINE const If_t< IsSigned_v<T>, SIMDcint32, SIMDcuint32 >
   setall_epi32( const complex<T>& v0 = 0, const complex<T>& v1 = 0
               , const complex<T>& v2 = 0, const complex<T>& v3 = 0 ) noexcept
{
   return _mm256_set_epi32( v3.imag(), v3.real(), v2.imag(), v2.real()
                          , v1.imag(), v1.real(), v0.imag(), v0.real() );
}
#elif BLAZE_SSE2_MODE
template< typename T >
BLAZE_ALWAYS_INLINE const If_t< IsSigned_v<T>, SIMDcint32, SIMDcuint32 >
   setall_epi32( const complex<T>& v0 = 0, const complex<T>& v1 = 0 ) noexcept
{
   return _mm_set_epi32( v1.imag(), v1.real(), v0.imag(), v0.real() );
}
#else
template< typename T >
BLAZE_ALWAYS_INLINE const If_t< IsSigned_v<T>, SIMDcint32, SIMDcuint32 >
   setall_epi32( const complex<T>& v0 = 0 ) noexcept
{
   return v0;
}
#endif
/*! \endcond */
//*************************************************************************************************


//*************************************************************************************************
/*!\brief Sets all values in the vector to the given 4-byte integral complex values.
// \ingroup simd
//
// \param v0 The first given 4-byte integral complex value.
// \param vs The remaining 4-byte integral complex values.
// \return The SIMD vector of 4-byte integral complex values.
//
// This function sets the values of a SIMD vector to the given values. Depending on the available
// SIMD instruction set, the function accepts another maximum number of values:
//
// <table>
//    <tr><th>SIMD instruction set<th>Maximum number of values
//    <tr><td>SSE    <td>2
//    <tr><td>AVX    <td>4
//    <tr><td>AVX-512<td>8
// </table>
//
// In case less arguments are given than the SIMD vector contains values, the missing values are
// set to 0.
*/
template< typename T
        , typename... Ts >
BLAZE_ALWAYS_INLINE const EnableIf_t< IsIntegral_v<T> && HasSize_v<T,4UL>
                                    , If_t< IsSigned_v<T>, SIMDcint32, SIMDcuint32 > >
   setall( const complex<T>& v0, const complex<Ts>&... vs ) noexcept
{
   return setall_epi32( v0, vs... );

   BLAZE_STATIC_ASSERT( sizeof( complex<T> ) == 2UL*sizeof( T ) );
}
//*************************************************************************************************




//=================================================================================================
//
//  64-BIT INTEGRAL SIMD TYPES
//
//=================================================================================================

//*************************************************************************************************
/*! \cond BLAZE_INTERNAL */
#if BLAZE_AVX512F_MODE
template< typename T >
BLAZE_ALWAYS_INLINE const If_t< IsSigned_v<T>, SIMDint64, SIMDuint64 >
   setall_epi64( T v0 = 0, T v1 = 0, T v2 = 0, T v3 = 0
               , T v4 = 0, T v5 = 0, T v6 = 0, T v7 = 0 ) noexcept
{
   return _mm512_set_epi64( v7, v6, v5, v4, v3, v2, v1, v0 );
}
#elif BLAZE_AVX2_MODE
template< typename T >
BLAZE_ALWAYS_INLINE const If_t< IsSigned_v<T>, SIMDint64, SIMDuint64 >
   setall_epi64( T v0 = 0, T v1 = 0, T v2 = 0, T v3 = 0 ) noexcept
{
   return _mm256_set_epi64x( v3, v2, v1, v0 );
}
#elif BLAZE_SSE2_MODE
template< typename T >
BLAZE_ALWAYS_INLINE const If_t< IsSigned_v<T>, SIMDint64, SIMDuint64 >
   setall_epi64( T v0 = 0, T v1 = 0 ) noexcept
{
   return _mm_set_epi64x( v1, v0 );
}
#else
template< typename T >
BLAZE_ALWAYS_INLINE const If_t< IsSigned_v<T>, SIMDint64, SIMDuint64 >
   setall_epi64( T v0 = 0 ) noexcept
{
   return v0;
}
#endif
/*! \endcond */
//*************************************************************************************************


//*************************************************************************************************
/*!\brief Sets all values in a SIMD vector to the given 8-byte integral values.
// \ingroup simd
//
// \param v0 The first given 8-byte integral value.
// \param vs The remaining 8-byte integral values.
// \return The SIMD vector of 8-byte integral values.
//
// This function sets the values of a SIMD vector to the given values. Depending on the available
// SIMD instruction set, the function accepts another maximum number of values:
//
// <table>
//    <tr><th>SIMD instruction set<th>Maximum number of values
//    <tr><td>SSE    <td>2
//    <tr><td>AVX    <td>4
//    <tr><td>AVX-512<td>8
// </table>
//
// In case less arguments are given than the SIMD vector contains values, the missing values are
// set to 0.
*/
template< typename T
        , typename... Ts >
BLAZE_ALWAYS_INLINE const EnableIf_t< IsIntegral_v<T> && HasSize_v<T,8UL>
                                    , If_t< IsSigned_v<T>, SIMDint64, SIMDuint64 > >
   setall( T v0, Ts... vs ) noexcept
{
   return setall_epi64( v0, vs... );
}
//*************************************************************************************************


//*************************************************************************************************
/*! \cond BLAZE_INTERNAL */
#if BLAZE_AVX512F_MODE
template< typename T >
BLAZE_ALWAYS_INLINE const If_t< IsSigned_v<T>, SIMDcint64, SIMDcuint64 >
   setall_epi64( const complex<T>& v0 = 0, const complex<T>& v1 = 0
               , const complex<T>& v2 = 0, const complex<T>& v3 = 0 ) noexcept
{
   return _mm512_set_epi64( v3.imag(), v3.real(), v2.imag(), v2.real()
                          , v1.imag(), v1.real(), v0.imag(), v0.real() );
}
#elif BLAZE_AVX2_MODE
template< typename T >
BLAZE_ALWAYS_INLINE const If_t< IsSigned_v<T>, SIMDcint64, SIMDcuint64 >
   setall_epi64( const complex<T>& v0 = 0, const complex<T>& v1 = 0 ) noexcept
{
   return _mm256_set_epi64x( v1.imag(), v1.real(), v0.imag(), v0.real() );
}
#elif BLAZE_SSE2_MODE
template< typename T >
BLAZE_ALWAYS_INLINE const If_t< IsSigned_v<T>, SIMDcint64, SIMDcuint64 >
   setall_epi64( const complex<T>& v0 = 0 ) noexcept
{
   return _mm_set_epi64x( v0.imag(), v0.real() );
}
#else
template< typename T >
BLAZE_ALWAYS_INLINE const If_t< IsSigned_v<T>, SIMDcint64, SIMDcuint64 >
   setall_epi64( const complex<T>& v0 = 0 ) noexcept
{
   return v0;
}
#endif
/*! \endcond */
//*************************************************************************************************


//*************************************************************************************************
/*!\brief Sets all values in the vector to the given 8-byte integral complex values.
// \ingroup simd
//
// \param v0 The first given 8-byte integral complex value.
// \param vs The remaining 8-byte integral complex values.
// \return The SIMD vector of 8-byte integral complex values.
//
// This function sets the values of a SIMD vector to the given values. Depending on the available
// SIMD instruction set, the function accepts another maximum number of values:
//
// <table>
//    <tr><th>SIMD instruction set<th>Maximum number of values
//    <tr><td>SSE    <td>1
//    <tr><td>AVX    <td>2
//    <tr><td>AVX-512<td>4
// </table>
//
// In case less arguments are given than the SIMD vector contains values, the missing values are
// set to 0.
*/
template< typename T
        , typename... Ts >
BLAZE_ALWAYS_INLINE const EnableIf_t< IsIntegral_v<T> && HasSize_v<T,8UL>
                                    , If_t< IsSigned_v<T>, SIMDcint64, SIMDcuint64 > >
   setall( const complex<T>& v0, const complex<Ts>&... vs ) noexcept
{
   return setall_epi64( v0, vs... );

   BLAZE_STATIC_ASSERT( sizeof( complex<T> ) == 2UL*sizeof( T ) );
}
//*************************************************************************************************




//=================================================================================================
//
//  32-BIT FLOATING POINT SIMD TYPES
//
//=================================================================================================

//*************************************************************************************************
/*! \cond BLAZE_INTERNAL */
#if BLAZE_AVX512F_MODE
BLAZE_ALWAYS_INLINE const SIMDfloat
   setall_ps( float v0  = 0.0F, float v1  = 0.0F, float v2  = 0.0F, float v3  = 0.0F
            , float v4  = 0.0F, float v5  = 0.0F, float v6  = 0.0F, float v7  = 0.0F
            , float v8  = 0.0F, float v9  = 0.0F, float v10 = 0.0F, float v11 = 0.0F
            , float v12 = 0.0F, float v13 = 0.0F, float v14 = 0.0F, float v15 = 0.0F ) noexcept
{
   return _mm512_set_ps( v15, v14, v13, v12, v11, v10, v9, v8, v7, v6, v5, v4, v3, v2, v1, v0 );
}
#elif BLAZE_AVX_MODE
BLAZE_ALWAYS_INLINE const SIMDfloat
   setall_ps( float v0 = 0.0F, float v1 = 0.0F, float v2 = 0.0F, float v3 = 0.0F
            , float v4 = 0.0F, float v5 = 0.0F, float v6 = 0.0F, float v7 = 0.0F ) noexcept
{
   return _mm256_set_ps( v7, v6, v5, v4, v3, v2, v1, v0 );
}
#elif BLAZE_SSE_MODE
BLAZE_ALWAYS_INLINE const SIMDfloat
   setall_ps( float v0 = 0.0F, float v1 = 0.0F, float v2 = 0.0F, float v3 = 0.0F ) noexcept
{
   return _mm_set_ps( v3, v2, v1, v0 );
}
#else
BLAZE_ALWAYS_INLINE const SIMDfloat
   setall_ps( float v1 = 0.0F, float v0 = 0.0F ) noexcept
{
   return v0;
}
#endif
/*! \endcond */
//*************************************************************************************************


//*************************************************************************************************
/*!\brief Sets all values in a SIMD vector to the given \c float values.
// \ingroup simd
//
// \param v0 The first given \c float value.
// \param vs The remaining \c float values.
// \return The SIMD vector of \c float values.
//
// This function sets the values of a SIMD vector to the given values. Depending on the available
// SIMD instruction set, the function accepts another maximum number of values:
//
// <table>
//    <tr><th>SIMD instruction set<th>Maximum number of values
//    <tr><td>SSE    <td>4
//    <tr><td>AVX    <td>8
//    <tr><td>AVX-512<td>16
// </table>
//
// In case less arguments are given than the SIMD vector contains values, the missing values are
// set to 0.
*/
template< typename... Ts >
SIMDfloat setall( float v0, Ts... vs ) noexcept
{
   return setall_ps( v0, vs... );
}
//*************************************************************************************************


//*************************************************************************************************
/*! \cond BLAZE_INTERNAL */
#if BLAZE_AVX512F_MODE
BLAZE_ALWAYS_INLINE const SIMDcfloat
   setall_ps( const complex<float>& v0 = 0.0F, const complex<float>& v1 = 0.0F
            , const complex<float>& v2 = 0.0F, const complex<float>& v3 = 0.0F
            , const complex<float>& v4 = 0.0F, const complex<float>& v5 = 0.0F
            , const complex<float>& v6 = 0.0F, const complex<float>& v7 = 0.0F ) noexcept
{
   return _mm512_set_ps( v7.imag(), v7.real(), v6.imag(), v6.real()
                       , v5.imag(), v5.real(), v4.imag(), v4.real()
                       , v3.imag(), v3.real(), v2.imag(), v2.real()
                       , v1.imag(), v1.real(), v0.imag(), v0.real() );
}
#elif BLAZE_AVX_MODE
BLAZE_ALWAYS_INLINE const SIMDcfloat
   setall_ps( const complex<float>& v0 = 0.0F, const complex<float>& v1 = 0.0F
            , const complex<float>& v2 = 0.0F, const complex<float>& v3 = 0.0F ) noexcept
{
   return _mm256_set_ps( v3.imag(), v3.real(), v2.imag(), v2.real()
                       , v1.imag(), v1.real(), v0.imag(), v0.real() );
}
#elif BLAZE_SSE_MODE
BLAZE_ALWAYS_INLINE const SIMDcfloat
   setall_ps( const complex<float>& v0 = 0.0F, const complex<float>& v1 = 0.0F ) noexcept
{
   return _mm_set_ps( v1.imag(), v1.real(), v0.imag(), v0.real() );
}
#else
BLAZE_ALWAYS_INLINE const SIMDcfloat
   setall_ps( const complex<float>& v0 = 0.0F ) noexcept
{
   return v0;
}
#endif
/*! \endcond */
//*************************************************************************************************


//*************************************************************************************************
/*!\brief Sets all values in the vector to the given \c complex<float> values.
// \ingroup simd
//
// \param v0 The first given \c complex<float> value.
// \param vs The remaining \c complex<float> values.
// \return The set vector of \c complex<float> values.
//
// This function sets the values of a SIMD vector to the given values. Depending on the available
// SIMD instruction set, the function accepts another maximum number of values:
//
// <table>
//    <tr><th>SIMD instruction set<th>Maximum number of values
//    <tr><td>SSE    <td>2
//    <tr><td>AVX    <td>4
//    <tr><td>AVX-512<td>8
// </table>
//
// In case less arguments are given than the SIMD vector contains values, the missing values are
// set to 0.
*/
template< typename... Ts >
SIMDcfloat setall( const complex<float>& v0, Ts... vs ) noexcept
{
   return setall_ps( v0, vs... );

   BLAZE_STATIC_ASSERT( sizeof( complex<float> ) == 2UL*sizeof( float ) );
}
//*************************************************************************************************




//=================================================================================================
//
//  64-BIT FLOATING POINT SIMD TYPES
//
//=================================================================================================

//*************************************************************************************************
/*! \cond BLAZE_INTERNAL */
#if BLAZE_AVX512F_MODE
BLAZE_ALWAYS_INLINE const SIMDdouble
   setall_pd( double v0 = 0.0, double v1 = 0.0, double v2 = 0.0, double v3 = 0.0
            , double v4 = 0.0, double v5 = 0.0, double v6 = 0.0, double v7 = 0.0 ) noexcept
{
   return _mm512_set_pd( v7, v6, v5, v4, v3, v2, v1, v0 );
}
#elif BLAZE_AVX_MODE
BLAZE_ALWAYS_INLINE const SIMDdouble
   setall_pd( double v0 = 0.0, double v1 = 0.0, double v2 = 0.0, double v3 = 0.0 ) noexcept
{
   return _mm256_set_pd( v3, v2, v1, v0 );
}
#elif BLAZE_SSE2_MODE
BLAZE_ALWAYS_INLINE const SIMDdouble
   setall_pd( double v0 = 0.0, double v1 = 0.0 ) noexcept
{
   return _mm_set_pd( v1, v0 );
}
#else
BLAZE_ALWAYS_INLINE const SIMDdouble
   setall_pd( double v0 = 0.0 ) noexcept
{
   return v0;
}
#endif
/*! \endcond */
//*************************************************************************************************


//*************************************************************************************************
/*!\brief Sets all values in a SIMD vector to the given \c double values.
// \ingroup simd
//
// \param v0 The first given \c double value.
// \param vs The remaining \c double values.
// \return The SIMD vector of \c double values.
//
// This function sets the values of a SIMD vector to the given values. Depending on the available
// SIMD instruction set, the function accepts another maximum number of values:
//
// <table>
//    <tr><th>SIMD instruction set<th>Maximum number of values
//    <tr><td>SSE    <td>2
//    <tr><td>AVX    <td>4
//    <tr><td>AVX-512<td>8
// </table>
//
// In case less arguments are given than the SIMD vector contains values, the missing values are
// set to 0.
*/
template< typename... Ts >
SIMDdouble setall( double v0, Ts... vs ) noexcept
{
   return setall_pd( v0, vs... );
}
//*************************************************************************************************


//*************************************************************************************************
/*! \cond BLAZE_INTERNAL */
#if BLAZE_AVX512F_MODE
BLAZE_ALWAYS_INLINE const SIMDcdouble
   setall_pd( const complex<double>& v0 = 0.0, const complex<double>& v1 = 0.0
            , const complex<double>& v2 = 0.0, const complex<double>& v3 = 0.0 ) noexcept
{
   return _mm512_set_pd( v3.imag(), v3.real(), v2.imag(), v2.real()
                       , v1.imag(), v1.real(), v0.imag(), v0.real() );
}
#elif BLAZE_AVX_MODE
BLAZE_ALWAYS_INLINE const SIMDcdouble
   setall_pd( const complex<double>& v0 = 0.0, const complex<double>& v1 = 0.0 ) noexcept
{
   return _mm256_set_pd( v1.imag(), v1.real(), v0.imag(), v0.real() );
}
#elif BLAZE_SSE2_MODE
BLAZE_ALWAYS_INLINE const SIMDcdouble
   setall_pd( const complex<double>& v0 = 0.0 ) noexcept
{
   return _mm_set_pd( v0.imag(), v0.real() );
}
#else
BLAZE_ALWAYS_INLINE const SIMDcdouble
   setall_pd( const complex<double>& v0 = 0.0 ) noexcept
{
   return v0;
}
#endif
/*! \endcond */
//*************************************************************************************************


//*************************************************************************************************
/*!\brief Sets all values in the vector to the given \c complex<double> values.
// \ingroup simd
//
// \param v0 The first given \c complex<double> value.
// \param vs The remaining \c complex<double> values.
// \return The set vector of \c complex<double> values.
//
// This function sets the values of a SIMD vector to the given values. Depending on the available
// SIMD instruction set, the function accepts another maximum number of values:
//
// <table>
//    <tr><th>SIMD instruction set<th>Maximum number of values
//    <tr><td>SSE    <td>1
//    <tr><td>AVX    <td>2
//    <tr><td>AVX-512<td>4
// </table>
//
// In case less arguments are given than the SIMD vector contains values, the missing values are
// set to 0.
*/
template< typename... Ts >
SIMDcdouble setall( const complex<double>& v0, Ts... vs ) noexcept
{
   return setall_pd( v0, vs... );

   BLAZE_STATIC_ASSERT( sizeof( complex<double> ) == 2UL*sizeof( double ) );
}
//*************************************************************************************************

} // namespace blaze

#endif
