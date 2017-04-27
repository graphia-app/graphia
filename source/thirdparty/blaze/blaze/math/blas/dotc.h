//=================================================================================================
/*!
//  \file blaze/math/blas/dotc.h
//  \brief Header file for BLAS conjugate dot product (dotc)
//
//  Copyright (C) 2013 Klaus Iglberger - All Rights Reserved
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

#ifndef _BLAZE_MATH_BLAS_DOTC_H_
#define _BLAZE_MATH_BLAS_DOTC_H_


//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <blaze/math/Aliases.h>
#include <blaze/math/constraints/BLASCompatible.h>
#include <blaze/math/constraints/Computation.h>
#include <blaze/math/constraints/ConstDataAccess.h>
#include <blaze/math/expressions/DenseVector.h>
#include <blaze/system/BLAS.h>
#include <blaze/system/Inline.h>
#include <blaze/util/Assert.h>
#include <blaze/util/Complex.h>
#include <blaze/util/NumericCast.h>
#include <blaze/util/StaticAssert.h>


namespace blaze {

//=================================================================================================
//
//  BLAS WRAPPER FUNCTIONS (DOTC)
//
//=================================================================================================

//*************************************************************************************************
/*!\name BLAS wrapper functions (dotc) */
//@{
#if BLAZE_BLAS_MODE

BLAZE_ALWAYS_INLINE float dotc( int n, const float* x, int incX, const float* y, int incY );

BLAZE_ALWAYS_INLINE double dotc( int n, const double* x, int incX, const double* y, int incY );

BLAZE_ALWAYS_INLINE complex<float> dotc( int n, const complex<float>* x, int incX,
                                         const complex<float>* y, int incY );

BLAZE_ALWAYS_INLINE complex<double> dotc( int n, const complex<double>* x, int incX,
                                          const complex<double>* y, int incY );

template< typename VT1, bool TF1, typename VT2, bool TF2 >
BLAZE_ALWAYS_INLINE ElementType_<VT1> dotc( const DenseVector<VT1,TF1>& x, const DenseVector<VT2,TF2>& y );

#endif
//@}
//*************************************************************************************************


//*************************************************************************************************
#if BLAZE_BLAS_MODE
/*!\brief BLAS kernel for a dense vector complex conjugate dot product for single precision operands
//        (\f$ s=\vec{x}*\vec{y} \f$).
// \ingroup blas
//
// \param n The size of the two dense vectors \a x and \a y \f$[0..\infty)\f$.
// \param x Pointer to the first element of vector \a x.
// \param incX The stride within vector \a x.
// \param y Pointer to the first element of vector \a y.
// \param incY The stride within vector \a y.
// \return void
//
// This function performs the dot product of the complex conjugate of a single precision dense
// vector with another single precision dense vector based on the BLAS cblas_sdot() function.
*/
BLAZE_ALWAYS_INLINE float dotc( int n, const float* x, int incX, const float* y, int incY )
{
   return cblas_sdot( n, x, incX, y, incY );
}
#endif
//*************************************************************************************************


//*************************************************************************************************
#if BLAZE_BLAS_MODE
/*!\brief BLAS kernel for a dense vector complex conjugate dot product for double precision operands
//        (\f$ s=\vec{x}*\vec{y} \f$).
// \ingroup blas
//
// \param n The size of the two dense vectors \a x and \a y \f$[0..\infty)\f$.
// \param x Pointer to the first element of vector \a x.
// \param incX The stride within vector \a x.
// \param y Pointer to the first element of vector \a y.
// \param incY The stride within vector \a y.
// \return void
//
// This function performs the dot product of the complex conjugate of a double precision dense
// vector with another double precision dense vector based on the BLAS cblas_ddot() function.
*/
BLAZE_ALWAYS_INLINE double dotc( int n, const double* x, int incX, const double* y, int incY )
{
   return cblas_ddot( n, x, incX, y, incY );
}
#endif
//*************************************************************************************************


//*************************************************************************************************
#if BLAZE_BLAS_MODE
/*!\brief BLAS kernel for a dense vector conjugate dot product for single precision complex
//        operands (\f$ s=\vec{x}*\vec{y} \f$).
// \ingroup blas
//
// \param n The size of the two dense vectors \a x and \a y \f$[0..\infty)\f$.
// \param x Pointer to the first element of vector \a x.
// \param incX The stride within vector \a x.
// \param y Pointer to the first element of vector \a y.
// \param incY The stride within vector \a y.
// \return void
//
// This function performs the dot product of the complex conjugate of a single precision
// complex dense vector with another single precision complex dense vector based on the BLAS
// cblas_cdotc_sub() function.
*/
BLAZE_ALWAYS_INLINE complex<float> dotc( int n, const complex<float>* x, int incX,
                                         const complex<float>* y, int incY )
{
   BLAZE_STATIC_ASSERT( sizeof( complex<float> ) == 2UL*sizeof( float ) );

   complex<float> tmp;

#ifdef OPENBLAS_VERSION
   cblas_cdotc_sub( n, reinterpret_cast<const float*>( x ), incX,
                    reinterpret_cast<const float*>( y ), incY,
                    reinterpret_cast<openblas_complex_float*>( &tmp ) );
#else
   cblas_cdotc_sub( n, reinterpret_cast<const float*>( x ), incX,
                    reinterpret_cast<const float*>( y ), incY, &tmp );
#endif

   return tmp;
}
#endif
//*************************************************************************************************


//*************************************************************************************************
#if BLAZE_BLAS_MODE
/*!\brief BLAS kernel for a dense vector complex conjugate dot product for double precision
//        complex operands (\f$ s=\vec{x}*\vec{y} \f$).
// \ingroup blas
//
// \param n The size of the two dense vectors \a x and \a y \f$[0..\infty)\f$.
// \param x Pointer to the first element of vector \a x.
// \param incX The stride within vector \a x.
// \param y Pointer to the first element of vector \a y.
// \param incY The stride within vector \a y.
// \return void
//
// This function performs the dot product of the complex conjugate of a double precision
// complex dense vector with another double precision complex dense vector based on the BLAS
// cblas_zdotc_sub() function.
*/
BLAZE_ALWAYS_INLINE complex<double> dotc( int n, const complex<double>* x, int incX,
                                          const complex<double>* y, int incY )
{
   BLAZE_STATIC_ASSERT( sizeof( complex<double> ) == 2UL*sizeof( double ) );

   complex<double> tmp;

#ifdef OPENBLAS_VERSION
   cblas_zdotc_sub( n, reinterpret_cast<const double*>( x ), incX,
                    reinterpret_cast<const double*>( y ), incY,
                    reinterpret_cast<openblas_complex_double*>( &tmp ) );
#else
   cblas_zdotc_sub( n, reinterpret_cast<const double*>( x ), incX,
                    reinterpret_cast<const double*>( y ), incY, &tmp );
#endif

   return tmp;
}
#endif
//*************************************************************************************************


//*************************************************************************************************
#if BLAZE_BLAS_MODE
/*!\brief BLAS kernel for a dense vector complex conjugate dot product (\f$ s=\vec{x}*\vec{y} \f$).
// \ingroup blas
//
// \param x The left-hand side dense vector operand.
// \param y The right-hand side dense vector operand.
// \return void
//
// This function performs the dot product of the complex conjugate of a dense vector with another
// dense vector based on the BLAS dotc() functions. Note that the function only works for vectors
// with \c float, \c double, \c complex<float>, or \c complex<double> element type. The attempt
// to call the function with vectors of any other element type results in a compile time error.
*/
template< typename VT1, bool TF1, typename VT2, bool TF2 >
ElementType_<VT1> dotc( const DenseVector<VT1,TF1>& x, const DenseVector<VT2,TF2>& y )
{
   BLAZE_CONSTRAINT_MUST_NOT_BE_COMPUTATION_TYPE( VT1 );
   BLAZE_CONSTRAINT_MUST_NOT_BE_COMPUTATION_TYPE( VT2 );

   BLAZE_CONSTRAINT_MUST_HAVE_CONST_DATA_ACCESS( VT1 );
   BLAZE_CONSTRAINT_MUST_HAVE_CONST_DATA_ACCESS( VT2 );

   BLAZE_CONSTRAINT_MUST_BE_BLAS_COMPATIBLE_TYPE( ElementType_<VT1> );
   BLAZE_CONSTRAINT_MUST_BE_BLAS_COMPATIBLE_TYPE( ElementType_<VT2> );

   const int n( numeric_cast<int>( (~x).size() ) );

   return dotc( n, (~x).data(), 1, (~y).data(), 1 );
}
#endif
//*************************************************************************************************

} // namespace blaze

#endif
