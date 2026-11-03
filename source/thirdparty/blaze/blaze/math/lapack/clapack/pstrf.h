//=================================================================================================
/*!
//  \file blaze/math/lapack/clapack/pstrf.h
//  \brief Header file for the CLAPACK pstrf wrapper functions
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

#ifndef _BLAZE_MATH_LAPACK_CLAPACK_PSTRF_H_
#define _BLAZE_MATH_LAPACK_CLAPACK_PSTRF_H_


//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <blaze/math/blas/Types.h>
#include <blaze/util/Complex.h>
#include <blaze/util/StaticAssert.h>
#include <blaze/util/Types.h>


//=================================================================================================
//
//  LAPACK FORWARD DECLARATIONS
//
//=================================================================================================

//*************************************************************************************************
/*! \cond BLAZE_INTERNAL */
#if !defined(INTEL_MKL_VERSION) && !defined(BLAS_H)
extern "C" {

void spstrf_( char* uplo, blaze::blas_int_t* n, float* A, blaze::blas_int_t* lda,
              blaze::blas_int_t* piv, blaze::blas_int_t* rank, float* tol, float* work,
              blaze::blas_int_t* info, blaze::fortran_charlen_t nuplo );
void dpstrf_( char* uplo, blaze::blas_int_t* n, double* A, blaze::blas_int_t* lda,
              blaze::blas_int_t* piv, blaze::blas_int_t* rank, double* tol, double* work,
              blaze::blas_int_t* info, blaze::fortran_charlen_t nuplo );
void cpstrf_( char* uplo, blaze::blas_int_t* n, float* A, blaze::blas_int_t* lda,
              blaze::blas_int_t* piv, blaze::blas_int_t* rank, float* tol, float* work,
              blaze::blas_int_t* info, blaze::fortran_charlen_t nuplo );
void zpstrf_( char* uplo, blaze::blas_int_t* n, double* A, blaze::blas_int_t* lda,
              blaze::blas_int_t* piv, blaze::blas_int_t* rank, double* tol, double* work,
              blaze::blas_int_t* info, blaze::fortran_charlen_t nuplo );

}
#endif
/*! \endcond */
//*************************************************************************************************




namespace blaze {

//=================================================================================================
//
//  LAPACK LLH PIVOTING (CHOLESKY) DECOMPOSITION FUNCTIONS (PSTRF)
//
//=================================================================================================

//*************************************************************************************************
/*!\name LAPACK LLH (Cholesky) decomposition functions (pstrf) */
//@{
void pstrf( char uplo, blas_int_t n, float* A, blas_int_t lda,
            blaze::blas_int_t* piv, blaze::blas_int_t* rank, float tol, float* work,
            blas_int_t* info );

void pstrf( char uplo, blas_int_t n, double* A, blas_int_t lda,
            blaze::blas_int_t* piv, blaze::blas_int_t* rank, double tol, double* work,
            blas_int_t* info );

void pstrf( char uplo, blas_int_t n, complex<float>* A, blas_int_t lda,
            blaze::blas_int_t* piv, blaze::blas_int_t* rank, float tol, complex<float>* work,
            blas_int_t* info );

void pstrf( char uplo, blas_int_t n, complex<double>* A, blas_int_t lda,
            blaze::blas_int_t* piv, blaze::blas_int_t* rank, double tol, complex<double>* work,
            blas_int_t* info );
//@}
//*************************************************************************************************


//*************************************************************************************************
/*!\brief LAPACK kernel for the pivoting Cholesky decomposition of the given dense semi-positive
//        definite single precision column-major matrix.
// \ingroup lapack_decomposition
//
// \param uplo \c 'L' to use the lower part of the matrix, \c 'U' to use the upper part.
// \param n The number of rows/columns of the matrix \f$[0..\infty)\f$.
// \param A Pointer to the first element of the single precision column-major matrix.
// \param lda The total number of elements between two columns of the matrix \f$[0..\infty)\f$.
// \param piv The pivoting indices applied to \c A; size >= n.
// \param rank The rank of A given by the number of steps the algorithm completed.
// \param tol The user-defined tolerance.
// \param work Auxiliary array; size >= \f$ 2*n \f$.
// \param info Return code of the function call.
// \return void
//
// This function performs the dense matrix Cholesky decomposition of a symmetric semi-positive
// definite single precision column-major matrix based on the LAPACK spstrf() function. The
// decomposition has the form

                      \f[ A = P^T U^{H} U P \texttt{ (if uplo = 'U'), or }
                          A = P L L^{H} P^T \texttt{ (if uplo = 'L'), } \f]

// where \c U is an upper triangular matrix and \c L is a lower triangular matrix. The resulting
// decomposition is stored within \a A: In case \a uplo is set to \c 'L' the result is stored in
// the lower part of the matrix and the upper part remains untouched, in case \a uplo is set
// to \c 'U' the result is stored in the upper part and the lower part remains untouched. The
// pivoting matrix is stored as vector in the array \c piv such that the nonzero entries are
// \f$ P(piv(K),K) = 1 \f$.
//
// The \a info argument provides feedback on the success of the function call:
//
//   - = 0: The decomposition finished successfully.
//   - < 0: If info = -i, the i-th argument had an illegal value.
//   - > 0: If info = i, the leading minor of order i is not positive definite.
//
// For more information on the spstrf() function, see the LAPACK online documentation browser:
//
//        http://www.netlib.org/lapack/explore-html/
//
// \note This function can only be used if a fitting LAPACK library, which supports this function,
// is available and linked to the executable. Otherwise a call to this function will result in a
// linker error.
*/
inline void pstrf( char uplo, blas_int_t n, float* A, blas_int_t lda,
                   blaze::blas_int_t* piv, blaze::blas_int_t* rank, float tol,
                   float* work, blas_int_t* info )
{
#if defined(INTEL_MKL_VERSION)
   BLAZE_STATIC_ASSERT( sizeof( MKL_INT ) == sizeof( blas_int_t ) );
#endif

   spstrf_( &uplo, &n, A, &lda, piv, rank, &tol, work, info
#if !defined(INTEL_MKL_VERSION) && !defined(BLAS_H)
          , blaze::fortran_charlen_t(1)
#endif
          );
}
//*************************************************************************************************


//*************************************************************************************************
/*!\brief LAPACK kernel for the pivoting Cholesky decomposition of the given dense semi-positive
//        definite double precision column-major matrix.
// \ingroup lapack_decomposition
//
// \param uplo \c 'L' to use the lower part of the matrix, \c 'U' to use the upper part.
// \param n The number of rows/columns of the matrix \f$[0..\infty)\f$.
// \param A Pointer to the first element of the double precision column-major matrix.
// \param lda The total number of elements between two columns of the matrix \f$[0..\infty)\f$.
// \param piv The pivoting indices applied to \c A; size >= n.
// \param rank The rank of A given by the number of steps the algorithm completed.
// \param tol The user-defined tolerance.
// \param work Auxiliary array; size >= \f$ 2*n \f$.
// \param info Return code of the function call.
// \return void
//
// This function performs the dense matrix Cholesky decomposition of a symmetric semi-positive
// definite double precision column-major matrix based on the LAPACK dpstrf() function. The
// decomposition has the form

                      \f[ A = P^T U^{H} U P \texttt{ (if uplo = 'U'), or }
                          A = P L L^{H} P^T \texttt{ (if uplo = 'L'), } \f]

// where \c U is an upper triangular matrix and \c L is a lower triangular matrix. The resulting
// decomposition is stored within \a A: In case \a uplo is set to \c 'L' the result is stored in
// the lower part of the matrix and the upper part remains untouched, in case \a uplo is set
// to \c 'U' the result is stored in the upper part and the lower part remains untouched. The
// pivoting matrix is stored as vector in the array \c piv such that the nonzero entries are
// \f$ P(piv(K),K) = 1 \f$.
//
// The \a info argument provides feedback on the success of the function call:
//
//   - = 0: The decomposition finished successfully.
//   - < 0: If info = -i, the i-th argument had an illegal value.
//   - > 0: If info = i, the leading minor of order i is not positive definite.
//
// For more information on the dpstrf() function, see the LAPACK online documentation browser:
//
//        http://www.netlib.org/lapack/explore-html/
//
// \note This function can only be used if a fitting LAPACK library, which supports this function,
// is available and linked to the executable. Otherwise a call to this function will result in a
// linker error.
*/
inline void pstrf( char uplo, blas_int_t n, double* A, blas_int_t lda,
                   blaze::blas_int_t* piv, blaze::blas_int_t* rank, double tol,
                   double* work, blas_int_t* info )
{
#if defined(INTEL_MKL_VERSION)
   BLAZE_STATIC_ASSERT( sizeof( MKL_INT ) == sizeof( blas_int_t ) );
#endif

   dpstrf_( &uplo, &n, A, &lda, piv, rank, &tol, work, info
#if !defined(INTEL_MKL_VERSION) && !defined(BLAS_H)
          , blaze::fortran_charlen_t(1)
#endif
          );
}
//*************************************************************************************************


//*************************************************************************************************
/*!\brief LAPACK kernel for the pivoting Cholesky decomposition of the given dense semi-positive
//        definite single precision complex column-major matrix.
// \ingroup lapack_decomposition
//
// \param uplo \c 'L' to use the lower part of the matrix, \c 'U' to use the upper part.
// \param n The number of rows/columns of the matrix \f$[0..\infty)\f$.
// \param A Pointer to the first element of the single precision complex column-major matrix.
// \param lda The total number of elements between two columns of the matrix \f$[0..\infty)\f$.
// \param piv The pivoting indices applied to \c A; size >= n.
// \param rank The rank of A given by the number of steps the algorithm completed.
// \param tol The user-defined tolerance.
// \param work Auxiliary array; size >= \f$ 2*n \f$.
// \param info Return code of the function call.
// \return void
//
// This function performs the dense matrix Cholesky decomposition of a symmetric semi-positive
// definite single precision complex column-major matrix based on the LAPACK cpstrf() function.
// The decomposition has the form

                      \f[ A = P^T U^{H} U P \texttt{ (if uplo = 'U'), or }
                          A = P L L^{H} P^T \texttt{ (if uplo = 'L'), } \f]

// where \c U is an upper triangular matrix and \c L is a lower triangular matrix. The resulting
// decomposition is stored within \a A: In case \a uplo is set to \c 'L' the result is stored in
// the lower part of the matrix and the upper part remains untouched, in case \a uplo is set
// to \c 'U' the result is stored in the upper part and the lower part remains untouched. The
// pivoting matrix is stored as vector in the array \c piv such that the nonzero entries are
// \f$ P(piv(K),K) = 1 \f$.
//
// The \a info argument provides feedback on the success of the function call:
//
//   - = 0: The decomposition finished successfully.
//   - < 0: If info = -i, the i-th argument had an illegal value.
//   - > 0: If info = i, the leading minor of order i is not positive definite.
//
// For more information on the cpstrf() function, see the LAPACK online documentation browser:
//
//        http://www.netlib.org/lapack/explore-html/
//
// \note This function can only be used if a fitting LAPACK library, which supports this function,
// is available and linked to the executable. Otherwise a call to this function will result in a
// linker error.
*/
inline void pstrf( char uplo, blas_int_t n, complex<float>* A, blas_int_t lda,
                   blaze::blas_int_t* piv, blaze::blas_int_t* rank, float tol,
                   complex<float>* work, blas_int_t* info )
{
#if defined(INTEL_MKL_VERSION)
   BLAZE_STATIC_ASSERT( sizeof( MKL_INT ) == sizeof( blas_int_t ) );
#endif

#if defined(INTEL_MKL_VERSION)
   BLAZE_STATIC_ASSERT( sizeof( MKL_INT ) == sizeof( blas_int_t ) );
   BLAZE_STATIC_ASSERT( sizeof( MKL_Complex8 ) == sizeof( complex<float> ) );
   using ET = MKL_Complex8;
#else
   using ET = float;
#endif

   cpstrf_( &uplo, &n, reinterpret_cast<ET*>(A), &lda, piv, rank,
            &tol, reinterpret_cast<ET*>(work), info
#if !defined(INTEL_MKL_VERSION) && !defined(BLAS_H)
          , blaze::fortran_charlen_t(1)
#endif
          );
}
//*************************************************************************************************


//*************************************************************************************************
/*!\brief LAPACK kernel for the pivoting Cholesky decomposition of the given dense semi-positive
//        definite double precision complex column-major matrix.
// \ingroup lapack_decomposition
//
// \param uplo \c 'L' to use the lower part of the matrix, \c 'U' to use the upper part.
// \param n The number of rows/columns of the matrix \f$[0..\infty)\f$.
// \param A Pointer to the first element of the double precision complex column-major matrix.
// \param lda The total number of elements between two columns of the matrix \f$[0..\infty)\f$.
// \param piv The pivoting indices applied to \c A; size >= n.
// \param rank The rank of A given by the number of steps the algorithm completed.
// \param tol The user-defined tolerance.
// \param work Auxiliary array; size >= \f$ 2*n \f$.
// \param info Return code of the function call.
// \return void
//
// This function performs the dense matrix Cholesky decomposition of a symmetric semi-positive
// definite double precision complex column-major matrix based on the LAPACK zpstrf() function.
// The decomposition has the form

                      \f[ A = P^T U^{H} U P \texttt{ (if uplo = 'U'), or }
                          A = P L L^{H} P^T \texttt{ (if uplo = 'L'), } \f]

// where \c U is an upper triangular matrix and \c L is a lower triangular matrix. The resulting
// decomposition is stored within \a A: In case \a uplo is set to \c 'L' the result is stored in
// the lower part of the matrix and the upper part remains untouched, in case \a uplo is set
// to \c 'U' the result is stored in the upper part and the lower part remains untouched. The
// pivoting matrix is stored as vector in the array \c piv such that the nonzero entries are
// \f$ P(piv(K),K) = 1 \f$.
//
// The \a info argument provides feedback on the success of the function call:
//
//   - = 0: The decomposition finished successfully.
//   - < 0: If info = -i, the i-th argument had an illegal value.
//   - > 0: If info = i, the leading minor of order i is not positive definite.
//
// For more information on the zpstrf() function, see the LAPACK online documentation browser:
//
//        http://www.netlib.org/lapack/explore-html/
//
// \note This function can only be used if a fitting LAPACK library, which supports this function,
// is available and linked to the executable. Otherwise a call to this function will result in a
// linker error.
*/
inline void pstrf( char uplo, blas_int_t n, complex<double>* A, blas_int_t lda,
                   blaze::blas_int_t* piv, blaze::blas_int_t* rank, double tol,
                   complex<double>* work, blas_int_t* info )
{
#if defined(INTEL_MKL_VERSION)
   BLAZE_STATIC_ASSERT( sizeof( MKL_INT ) == sizeof( blas_int_t ) );
#endif

#if defined(INTEL_MKL_VERSION)
   BLAZE_STATIC_ASSERT( sizeof( MKL_INT ) == sizeof( blas_int_t ) );
   BLAZE_STATIC_ASSERT( sizeof( MKL_Complex16 ) == sizeof( complex<double> ) );
   using ET = MKL_Complex16;
#else
   using ET = double;
#endif

   zpstrf_( &uplo, &n, reinterpret_cast<ET*>(A), &lda, piv, rank,
            &tol, reinterpret_cast<ET*>(work), info
#if !defined(INTEL_MKL_VERSION) && !defined(BLAS_H)
          , blaze::fortran_charlen_t(1)
#endif
          );
}
//*************************************************************************************************

} // namespace blaze

#endif
