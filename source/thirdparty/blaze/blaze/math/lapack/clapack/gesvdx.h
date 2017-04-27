//=================================================================================================
/*!
//  \file blaze/math/lapack/clapack/gesvdx.h
//  \brief Header file for the CLAPACK gesvdx wrapper functions
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

#ifndef _BLAZE_MATH_LAPACK_CLAPACK_GESVDX_H_
#define _BLAZE_MATH_LAPACK_CLAPACK_GESVDX_H_


//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <blaze/util/Complex.h>
#include <blaze/util/StaticAssert.h>


//=================================================================================================
//
//  LAPACK FORWARD DECLARATIONS
//
//=================================================================================================

//*************************************************************************************************
/*! \cond BLAZE_INTERNAL */
extern "C" {

void sgesvdx_( char* jobu, char* jobv, char* range, int* m, int* n, float*  A, int* lda, float*  vl, float*  vu, int* il, int* iu, int* ns, float*  s, float*  U, int* ldu, float*  V, int* ldv, float*  work, int* lwork, int* iwork, int* info );
void dgesvdx_( char* jobu, char* jobv, char* range, int* m, int* n, double* A, int* lda, double* vl, double* vu, int* il, int* iu, int* ns, double* s, double* U, int* ldu, double* V, int* ldv, double* work, int* lwork, int* iwork, int* info );
void cgesvdx_( char* jobu, char* jobv, char* range, int* m, int* n, float*  A, int* lda, float*  vl, float*  vu, int* il, int* iu, int* ns, float*  s, float*  U, int* ldu, float*  V, int* ldv, float*  work, int* lwork, float*  rwork, int* iwork, int* info );
void zgesvdx_( char* jobu, char* jobv, char* range, int* m, int* n, double* A, int* lda, double* vl, double* vu, int* il, int* iu, int* ns, double* s, double* U, int* ldu, double* V, int* ldv, double* work, int* lwork, double* rwork, int* iwork, int* info );

}
/*! \endcond */
//*************************************************************************************************




namespace blaze {

//=================================================================================================
//
//  LAPACK SVD FUNCTIONS (GESVDX)
//
//=================================================================================================

//*************************************************************************************************
/*!\name LAPACK SVD functions (gesvdx) */
//@{
inline void gesvdx( char jobu, char jobv, char range, int m, int n, float* A, int lda,
                    float vl, float vu, int il, int iu, int* ns,
                    float* s, float* U, int ldu, float* V, int ldv,
                    float* work, int lwork, int* iwork, int* info );

inline void gesvdx( char jobu, char jobv, char range, int m, int n, double* A, int lda,
                    double vl, double vu, int il, int iu, int* ns,
                    double* s, double* U, int ldu, double* V, int ldv,
                    double* work, int lwork, int* iwork, int* info );

inline void gesvdx( char jobu, char jobv, char range, int m, int n, complex<float>* A, int lda,
                    float vl, float vu, int il, int iu, int* ns,
                    float* s, complex<float>* U, int ldu, complex<float>* V, int ldv,
                    complex<float>* work, int lwork, float* rwork, int* iwork, int* info );

inline void gesvdx( char jobu, char jobv, char range, int m, int n, complex<double>* A, int lda,
                    double vl, double vu, int il, int iu, int* ns,
                    double* s, complex<double>* U, int ldu, complex<double>* V, int ldv,
                    complex<double>* work, int lwork, double* rwork, int* iwork, int* info );
//@}
//*************************************************************************************************


//*************************************************************************************************
/*!\brief LAPACK kernel for the singular value decomposition (SVD) of the given dense general
//        single precision column-major matrix.
// \ingroup lapack_singular_value
//
// \param jobu Specifies the computation of the left singular vectors (\c 'V' or \c 'N').
// \param jobv Specifies the computation of the right singular vectors (\c 'V' or \c 'N').
// \param range Specifies the range of singular values to find (\c 'A', \c 'V', or \c 'I').
// \param m The number of rows of the given matrix \f$[0..\infty)\f$.
// \param n The number of columns of the given matrix \f$[0..\infty)\f$.
// \param A Pointer to the first element of the single precision column-major matrix.
// \param lda The total number of elements between two columns of the matrix A \f$[0..\infty)\f$.
// \param vl The lower bound of the interval to be searched for singular values (\a vl < \a vu).
// \param vu The upper bound of the interval to be searched for singular values (\a vl < \a vu).
// \param il The index of the smallest singular value to be returned (0 <= \a il <= \a iu).
// \param iu The index of the largest singular value to be returned (0 <= \a il <= \a iu).
// \param ns The total number of singular values found (0 <= \a ns <= min(\a m,\a n)).
// \param s Pointer to the first element of the vector for the singular values.
// \param U Pointer to the first element of the column-major matrix for the left singular vectors.
// \param ldu The total number of elements between two columns of the matrix U \f$[0..\infty)\f$.
// \param V Pointer to the first element of the column-major matrix for the right singular vectors.
// \param ldv The total number of elements between two columns of the matrix V \f$[0..\infty)\f$.
// \param work Auxiliary array; size >= max( 1, \a lwork ).
// \param lwork The dimension of the array \a work; see online reference for details.
// \param iwork Auxiliary array; size >= 12*min(\a m,\a n).
// \param info Return code of the function call.
// \return void
//
// This function computes a specified number of singular values of a general \a m-by-\a n single
// precision complex column-major matrix based on the LAPACK sgesvdx() function. Optionally, it
// computes a specified number of left and right singular vectors. The complete decomposition
// has the form

                           \f[ A = U \cdot S \cdot V, \f]

// where \c S is a \a m-by-\a n matrix, which is zero except for its min(\a m,\a n) diagonal
// elements, which are stored in \a s, \a U is an \a m-by-\a m orthogonal matrix, and \a V
// is a \a n-by-\a n orthogonal matrix. The diagonal elements of \c S are the singular values
// of \a A; they are real and non-negative, and are returned in descending order. The first
// min(\a m,\a n) columns of \a U and rows of \a V are the left and right singular vectors
// of \a A.
//
// The parameter \a jobu specifies the computation of the left singular vectors:
//
//   - \c 'V': Either the first min(\a m, \a n) columns of \a U (the left singular vectors) or
//             the number specified by \a range are computed.
//   - \c 'N': No columns of \a U (no left singular vectors) are computed.
//
// The parameter \a jobv specifies the computation of the right singular vectors:
//
//   - \c 'V': Either the first min(\a m, \a n) rows of \a V (the right singular vectors) or
//             the number specified by \a range are computed.
//   - \c 'N': No rows of \a V (no right singular vectors) are computed.
//
// The parameter \a range specifies the amount of singular values/vectors to be found:
//
//   - \c 'A': All singular values will be found.
//   - \c 'V': All singular values in the half-open interval \f$(vl..vu]\f$ will be found.
//   - \c 'I': The \a il-th through \a iu-th singular values will be found.
//
// The \a info argument provides feedback on the success of the function call:
//
//   - = 0: The decomposition finished successfully.
//   - < 0: If info = -i, the i-th argument had an illegal value.
//   - > 0: If info = i, the decomposition has been completed, but i superdiagonals did not converge.
//
// For more information on the sgesvdx() function, see the LAPACK online documentation browser:
//
//        http://www.netlib.org/lapack/explore-html/
//
// \note This function can only be used if a fitting LAPACK library, which supports this function,
// is available and linked to the executable. Otherwise a call to this function will result in a
// linker error.
*/
inline void gesvdx( char jobu, char jobv, char range, int m, int n, float* A, int lda,
                    float vl, float vu, int il, int iu, int* ns,
                    float* s, float* U, int ldu, float* V, int ldv,
                    float* work, int lwork, int* iwork, int* info )
{
   ++il;
   ++iu;

   sgesvdx_( &jobu, &jobv, &range, &m, &n, A, &lda, &vl, &vu, &il, &iu, ns,
             s, U, &ldu, V, &ldv, work, &lwork, iwork, info );
}
//*************************************************************************************************


//*************************************************************************************************
/*!\brief LAPACK kernel for the singular value decomposition (SVD) of the given dense general
//        double precision column-major matrix.
// \ingroup lapack_singular_value
//
// \param jobu Specifies the computation of the left singular vectors (\c 'V' or \c 'N').
// \param jobv Specifies the computation of the right singular vectors (\c 'V' or \c 'N').
// \param range Specifies the range of singular values to find (\c 'A', \c 'V', or \c 'I').
// \param m The number of rows of the given matrix \f$[0..\infty)\f$.
// \param n The number of columns of the given matrix \f$[0..\infty)\f$.
// \param A Pointer to the first element of the double precision column-major matrix.
// \param lda The total number of elements between two columns of the matrix A \f$[0..\infty)\f$.
// \param vl The lower bound of the interval to be searched for singular values (\a vl < \a vu).
// \param vu The upper bound of the interval to be searched for singular values (\a vl < \a vu).
// \param il The index of the smallest singular value to be returned (0 <= \a il <= \a iu).
// \param iu The index of the largest singular value to be returned (0 <= \a il <= \a iu).
// \param ns The total number of singular values found (0 <= \a ns <= min(\a m,\a n)).
// \param s Pointer to the first element of the vector for the singular values.
// \param U Pointer to the first element of the column-major matrix for the left singular vectors.
// \param ldu The total number of elements between two columns of the matrix U \f$[0..\infty)\f$.
// \param V Pointer to the first element of the column-major matrix for the right singular vectors.
// \param ldv The total number of elements between two columns of the matrix V \f$[0..\infty)\f$.
// \param work Auxiliary array; size >= max( 1, \a lwork ).
// \param lwork The dimension of the array \a work; see online reference for details.
// \param iwork Auxiliary array; size >= 12*min(\a m,\a n).
// \param info Return code of the function call.
// \return void
//
// This function computes a specified number of singular values of a general \a m-by-\a n single
// precision complex column-major matrix based on the LAPACK dgesvdx() function. Optionally, it
// computes a specified number of left and right singular vectors. The complete decomposition
// has the form

                           \f[ A = U \cdot S \cdot V, \f]

// where \c S is a \a m-by-\a n matrix, which is zero except for its min(\a m,\a n) diagonal
// elements, which are stored in \a s, \a U is an \a m-by-\a m orthogonal matrix, and \a V
// is a \a n-by-\a n orthogonal matrix. The diagonal elements of \c S are the singular values
// of \a A; they are real and non-negative, and are returned in descending order. The first
// min(\a m,\a n) columns of \a U and rows of \a V are the left and right singular vectors
// of \a A.
//
// The parameter \a jobu specifies the computation of the left singular vectors:
//
//   - \c 'V': Either the first min(\a m, \a n) columns of \a U (the left singular vectors) or
//             the number specified by \a range are computed.
//   - \c 'N': No columns of \a U (no left singular vectors) are computed.
//
// The parameter \a jobv specifies the computation of the right singular vectors:
//
//   - \c 'V': Either the first min(\a m, \a n) rows of \a V (the right singular vectors) or
//             the number specified by \a range are computed.
//   - \c 'N': No rows of \a V (no right singular vectors) are computed.
//
// The parameter \a range specifies the amount of singular values/vectors to be found:
//
//   - \c 'A': All singular values will be found.
//   - \c 'V': All singular values in the half-open interval \f$(vl..vu]\f$ will be found.
//   - \c 'I': The \a il-th through \a iu-th singular values will be found.
//
// The \a info argument provides feedback on the success of the function call:
//
//   - = 0: The decomposition finished successfully.
//   - < 0: If info = -i, the i-th argument had an illegal value.
//   - > 0: If info = i, the decomposition has been completed, but i superdiagonals did not converge.
//
// For more information on the dgesvdx() function, see the LAPACK online documentation browser:
//
//        http://www.netlib.org/lapack/explore-html/
//
// \note This function can only be used if a fitting LAPACK library, which supports this function,
// is available and linked to the executable. Otherwise a call to this function will result in a
// linker error.
*/
inline void gesvdx( char jobu, char jobv, char range, int m, int n, double* A, int lda,
                    double vl, double vu, int il, int iu, int* ns,
                    double* s, double* U, int ldu, double* V, int ldv,
                    double* work, int lwork, int* iwork, int* info )
{
   ++il;
   ++iu;

   dgesvdx_( &jobu, &jobv, &range, &m, &n, A, &lda, &vl, &vu, &il, &iu, ns,
             s, U, &ldu, V, &ldv, work, &lwork, iwork, info );
}
//*************************************************************************************************


//*************************************************************************************************
/*!\brief LAPACK kernel for the singular value decomposition (SVD) of the given dense general
//        single precision complex column-major matrix.
// \ingroup lapack_singular_value
//
// \param jobu Specifies the computation of the left singular vectors (\c 'V' or \c 'N').
// \param jobv Specifies the computation of the right singular vectors (\c 'V' or \c 'N').
// \param range Specifies the range of singular values to find (\c 'A', \c 'V', or \c 'I').
// \param m The number of rows of the given matrix \f$[0..\infty)\f$.
// \param n The number of columns of the given matrix \f$[0..\infty)\f$.
// \param A Pointer to the first element of the single precision column-major matrix.
// \param lda The total number of elements between two columns of the matrix A \f$[0..\infty)\f$.
// \param vl The lower bound of the interval to be searched for singular values (\a vl < \a vu).
// \param vu The upper bound of the interval to be searched for singular values (\a vl < \a vu).
// \param il The index of the smallest singular value to be returned (0 <= \a il <= \a iu).
// \param iu The index of the largest singular value to be returned (0 <= \a il <= \a iu).
// \param ns The total number of singular values found (0 <= \a ns <= min(\a m,\a n)).
// \param s Pointer to the first element of the vector for the singular values.
// \param U Pointer to the first element of the column-major matrix for the left singular vectors.
// \param ldu The total number of elements between two columns of the matrix U \f$[0..\infty)\f$.
// \param V Pointer to the first element of the column-major matrix for the right singular vectors.
// \param ldv The total number of elements between two columns of the matrix V \f$[0..\infty)\f$.
// \param work Auxiliary array; size >= max( 1, \a lwork ).
// \param lwork The dimension of the array \a work; see online reference for details.
// \param rwork Auxiliary array; size >= min(\a m,\a n) * ( min(\a m,\a n)^2 + 15*min(\a m,\a n) ).
// \param iwork Auxiliary array; size >= 12*min(\a m,\a n).
// \param info Return code of the function call.
// \return void
//
// This function computes a specified number of singular values of a general \a m-by-\a n single
// precision complex column-major matrix based on the LAPACK cgesvdx() function. Optionally, it
// computes a specified number of left and right singular vectors. The complete decomposition
// has the form

                           \f[ A = U \cdot S \cdot V, \f]

// where \c S is a \a m-by-\a n matrix, which is zero except for its min(\a m,\a n) diagonal
// elements, which are stored in \a s, \a U is an \a m-by-\a m orthogonal matrix, and \a V
// is a \a n-by-\a n orthogonal matrix. The diagonal elements of \c S are the singular values
// of \a A; they are real and non-negative, and are returned in descending order. The first
// min(\a m,\a n) columns of \a U and rows of \a V are the left and right singular vectors
// of \a A.
//
// The parameter \a jobu specifies the computation of the left singular vectors:
//
//   - \c 'V': Either the first min(\a m, \a n) columns of \a U (the left singular vectors) or
//             the number specified by \a range are computed.
//   - \c 'N': No columns of \a U (no left singular vectors) are computed.
//
// The parameter \a jobv specifies the computation of the right singular vectors:
//
//   - \c 'V': Either the first min(\a m, \a n) rows of \a V (the right singular vectors) or
//             the number specified by \a range are computed.
//   - \c 'N': No rows of \a V (no right singular vectors) are computed.
//
// The parameter \a range specifies the amount of singular values/vectors to be found:
//
//   - \c 'A': All singular values will be found.
//   - \c 'V': All singular values in the half-open interval \f$(vl..vu]\f$ will be found.
//   - \c 'I': The \a il-th through \a iu-th singular values will be found.
//
// The \a info argument provides feedback on the success of the function call:
//
//   - = 0: The decomposition finished successfully.
//   - < 0: If info = -i, the i-th argument had an illegal value.
//   - > 0: If info = i, the decomposition has been completed, but i superdiagonals did not converge.
//
// For more information on the cgesvdx() function, see the LAPACK online documentation browser:
//
//        http://www.netlib.org/lapack/explore-html/
//
// \note This function can only be used if a fitting LAPACK library, which supports this function,
// is available and linked to the executable. Otherwise a call to this function will result in a
// linker error.
*/
inline void gesvdx( char jobu, char jobv, char range, int m, int n, complex<float>* A, int lda,
                    float vl, float vu, int il, int iu, int* ns,
                    float* s, complex<float>* U, int ldu, complex<float>* V, int ldv,
                    complex<float>* work, int lwork, float* rwork, int* iwork, int* info )
{
   BLAZE_STATIC_ASSERT( sizeof( complex<float> ) == 2UL*sizeof( float ) );

   ++il;
   ++iu;

   cgesvdx_( &jobu, &jobv, &range, &m, &n, reinterpret_cast<float*>( A ), &lda,
             &vl, &vu, &il, &iu, ns, s,
             reinterpret_cast<float*>( U ), &ldu, reinterpret_cast<float*>( V ), &ldv,
             reinterpret_cast<float*>( work ), &lwork, rwork, iwork, info );
}
//*************************************************************************************************


//*************************************************************************************************
/*!\brief LAPACK kernel for the singular value decomposition (SVD) of the given dense general
//        double precision complex column-major matrix.
// \ingroup lapack_singular_value
//
// \param jobu Specifies the computation of the left singular vectors (\c 'V' or \c 'N').
// \param jobv Specifies the computation of the right singular vectors (\c 'V' or \c 'N').
// \param range Specifies the range of singular values to find (\c 'A', \c 'V', or \c 'I').
// \param m The number of rows of the given matrix \f$[0..\infty)\f$.
// \param n The number of columns of the given matrix \f$[0..\infty)\f$.
// \param A Pointer to the first element of the double precision column-major matrix.
// \param lda The total number of elements between two columns of the matrix A \f$[0..\infty)\f$.
// \param vl The lower bound of the interval to be searched for singular values (\a vl < \a vu).
// \param vu The upper bound of the interval to be searched for singular values (\a vl < \a vu).
// \param il The index of the smallest singular value to be returned (0 <= \a il <= \a iu).
// \param iu The index of the largest singular value to be returned (0 <= \a il <= \a iu).
// \param ns The total number of singular values found (0 <= \a ns <= min(\a m,\a n)).
// \param s Pointer to the first element of the vector for the singular values.
// \param U Pointer to the first element of the column-major matrix for the left singular vectors.
// \param ldu The total number of elements between two columns of the matrix U \f$[0..\infty)\f$.
// \param V Pointer to the first element of the column-major matrix for the right singular vectors.
// \param ldv The total number of elements between two columns of the matrix V \f$[0..\infty)\f$.
// \param work Auxiliary array; size >= max( 1, \a lwork ).
// \param lwork The dimension of the array \a work; see online reference for details.
// \param rwork Auxiliary array; size >= min(\a m,\a n) * ( min(\a m,\a n)^2 + 15*min(\a m,\a n) ).
// \param iwork Auxiliary array; size >= 12*min(\a m,\a n).
// \param info Return code of the function call.
// \return void
//
// This function computes a specified number of singular values of a general \a m-by-\a n single
// precision complex column-major matrix based on the LAPACK zgesvdx() function. Optionally, it
// computes a specified number of left and right singular vectors. The complete decomposition
// has the form

                           \f[ A = U \cdot S \cdot V, \f]

// where \c S is a \a m-by-\a n matrix, which is zero except for its min(\a m,\a n) diagonal
// elements, which are stored in \a s, \a U is an \a m-by-\a m orthogonal matrix, and \a V
// is a \a n-by-\a n orthogonal matrix. The diagonal elements of \c S are the singular values
// of \a A; they are real and non-negative, and are returned in descending order. The first
// min(\a m,\a n) columns of \a U and rows of \a V are the left and right singular vectors
// of \a A.
//
// The parameter \a jobu specifies the computation of the left singular vectors:
//
//   - \c 'V': Either the first min(\a m, \a n) columns of \a U (the left singular vectors) or
//             the number specified by \a range are computed.
//   - \c 'N': No columns of \a U (no left singular vectors) are computed.
//
// The parameter \a jobv specifies the computation of the right singular vectors:
//
//   - \c 'V': Either the first min(\a m, \a n) rows of \a V (the right singular vectors) or
//             the number specified by \a range are computed.
//   - \c 'N': No rows of \a V (no right singular vectors) are computed.
//
// The parameter \a range specifies the amount of singular values/vectors to be found:
//
//   - \c 'A': All singular values will be found.
//   - \c 'V': All singular values in the half-open interval \f$(vl..vu]\f$ will be found.
//   - \c 'I': The \a il-th through \a iu-th singular values will be found.
//
// The \a info argument provides feedback on the success of the function call:
//
//   - = 0: The decomposition finished successfully.
//   - < 0: If info = -i, the i-th argument had an illegal value.
//   - > 0: If info = i, the decomposition has been completed, but i superdiagonals did not converge.
//
// For more information on the zgesvdx() function, see the LAPACK online documentation browser:
//
//        http://www.netlib.org/lapack/explore-html/
//
// \note This function can only be used if a fitting LAPACK library, which supports this function,
// is available and linked to the executable. Otherwise a call to this function will result in a
// linker error.
*/
inline void gesvdx( char jobu, char jobv, char range, int m, int n, complex<double>* A, int lda,
                    double vl, double vu, int il, int iu, int* ns,
                    double* s, complex<double>* U, int ldu, complex<double>* V, int ldv,
                    complex<double>* work, int lwork, double* rwork, int* iwork, int* info )
{
   BLAZE_STATIC_ASSERT( sizeof( complex<double> ) == 2UL*sizeof( double ) );

   ++il;
   ++iu;

   zgesvdx_( &jobu, &jobv, &range, &m, &n, reinterpret_cast<double*>( A ), &lda,
             &vl, &vu, &il, &iu, ns, s,
             reinterpret_cast<double*>( U ), &ldu, reinterpret_cast<double*>( V ), &ldv,
             reinterpret_cast<double*>( work ), &lwork, rwork, iwork, info );
}
//*************************************************************************************************

} // namespace blaze

#endif
