//=================================================================================================
/*!
//  \file blaze/math/lapack/heevx.h
//  \brief Header file for the LAPACK Hermitian matrix eigenvalue functions (heevx)
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

#ifndef _BLAZE_MATH_LAPACK_HEEVX_H_
#define _BLAZE_MATH_LAPACK_HEEVX_H_


//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <memory>
#include <blaze/math/Aliases.h>
#include <blaze/math/constraints/Adaptor.h>
#include <blaze/math/constraints/BLASCompatible.h>
#include <blaze/math/constraints/Computation.h>
#include <blaze/math/constraints/MutableDataAccess.h>
#include <blaze/math/Exception.h>
#include <blaze/math/expressions/DenseMatrix.h>
#include <blaze/math/expressions/DenseVector.h>
#include <blaze/math/lapack/clapack/heevx.h>
#include <blaze/math/typetraits/IsResizable.h>
#include <blaze/math/typetraits/IsRowMajorMatrix.h>
#include <blaze/util/Assert.h>
#include <blaze/util/constraints/Builtin.h>
#include <blaze/util/constraints/Complex.h>
#include <blaze/util/NumericCast.h>
#include <blaze/util/Types.h>
#include <blaze/util/typetraits/IsFloatingPoint.h>


namespace blaze {

//=================================================================================================
//
//  LAPACK HERMITIAN MATRIX EIGENVALUE FUNCTIONS (HEEVX)
//
//=================================================================================================

//*************************************************************************************************
/*!\name LAPACK Hermitian matrix eigenvalue functions (heevx) */
//@{
template< typename MT, bool SO, typename VT, bool TF >
inline size_t heevx( DenseMatrix<MT,SO>& A, DenseVector<VT,TF>& w, char uplo );

template< typename MT, bool SO, typename VT, bool TF, typename ST >
inline size_t heevx( DenseMatrix<MT,SO>& A, DenseVector<VT,TF>& w, char uplo, ST low, ST upp );

template< typename MT1, bool SO1, typename VT, bool TF, typename MT2, bool SO2 >
inline size_t heevx( DenseMatrix<MT1,SO1>& A, DenseVector<VT,TF>& w,
                     DenseMatrix<MT2,SO2>& Z, char uplo );

template< typename MT1, bool SO1, typename VT, bool TF, typename MT2, bool SO2, typename ST >
inline size_t heevx( DenseMatrix<MT1,SO1>& A, DenseVector<VT,TF>& w,
                     DenseMatrix<MT2,SO2>& Z, char uplo, ST low, ST upp );
//@}
//*************************************************************************************************


//*************************************************************************************************
/*! \cond BLAZE_INTERNAL */
/*!\brief Backend of the LAPACK heevx kernel for Hermitian matrices.
// \ingroup lapack_eigenvalue
//
// \param A The given Hermitian matrix.
// \param w The resulting vector of eigenvalues.
// \param uplo \c 'L' to use the lower part of the matrix, \c 'U' to use the upper part.
// \param range Specifies the range of eigenvalues to find (\c 'A', \c 'V', or \c 'I').
// \param vl The lower bound of the interval to be searched for eigenvalues (\a vl < \a vu).
// \param vu The upper bound of the interval to be searched for eigenvalues (\a vl < \a vu).
// \param il The index of the smallest eigenvalue to be returned (0 <= \a il <= \a iu).
// \param iu The index of the largest eigenvalue to be returned (0 <= \a il <= \a iu).
// \return The total number of eigenvalues found.
// \exception std::runtime_error Eigenvalue computation failed.
//
// This function is the backend implementation for computing the eigenvalues of the given
// dense Hermitian matrix.\n
// This function must \b NOT be called explicitly! It is used internally for the dispatch to
// the correct LAPACK function. Calling this function explicitly might result in erroneous
// results and/or in compilation errors. Instead of using this function use the according
// heevx() function.
*/
template< typename MT    // Type of the matrix A
        , bool SO        // Storage order of the matrix A
        , typename VT    // Type of the vector w
        , bool TF        // Transpose flag of the vector w
        , typename ST >  // Type of the scalar boundary values
inline size_t heevx_backend( DenseMatrix<MT,SO>& A, DenseVector<VT,TF>& w,
                             char uplo, char range, ST vl, ST vu, int il, int iu )
{
   BLAZE_INTERNAL_ASSERT( isSquare( ~A ), "Invalid non-square matrix detected" );
   BLAZE_INTERNAL_ASSERT( range != 'A' || (~w).size() == (~A).rows(), "Invalid vector dimension detected" );
   BLAZE_INTERNAL_ASSERT( range != 'V' || (~w).size() == (~A).rows(), "Invalid vector dimension detected" );
   BLAZE_INTERNAL_ASSERT( range != 'I' || (~w).size() == size_t( iu-il+1 ), "Invalid vector dimension detected" );

   using CT = ElementType_<MT>;
   using BT = UnderlyingElement_<CT>;

   BLAZE_CONSTRAINT_MUST_BE_COMPLEX_TYPE( CT );
   BLAZE_CONSTRAINT_MUST_BE_BUILTIN_TYPE( BT );

   int n   ( numeric_cast<int>( (~A).rows() ) );
   int lda ( numeric_cast<int>( (~A).spacing() ) );
   int m   ( 0 );
   int info( 0 );

   int lwork( 12*n + 2 );
   const std::unique_ptr<CT[]>  work ( new CT[lwork] );
   const std::unique_ptr<BT[]>  rwork( new BT[7*n] );
   const std::unique_ptr<int[]> iwork( new int[5*n] );
   const std::unique_ptr<int[]> ifail( new int[n] );

   heevx( 'N', range, uplo, n, (~A).data(), lda, vl, vu, il, iu, BT(0), &m, (~w).data(),
          nullptr, 1, work.get(), lwork, rwork.get(), iwork.get(), ifail.get(), &info );

   const size_t num( numeric_cast<size_t>( m ) );

   BLAZE_INTERNAL_ASSERT( info >= 0, "Invalid argument for eigenvalue computation" );
   BLAZE_INTERNAL_ASSERT( num <= (~w).size(), "Invalid number of eigenvalues detected" );

   if( info > 0 ) {
      BLAZE_THROW_LAPACK_ERROR( "Eigenvalue computation failed" );
   }

   return num;
}
/*! \endcond */
//*************************************************************************************************


//*************************************************************************************************
/*!\brief LAPACK kernel for the eigenvalue computation of the given dense Hermitian matrix.
// \ingroup lapack_eigenvalue
//
// \param A The given Hermitian matrix.
// \param w The resulting vector of eigenvalues.
// \param uplo \c 'L' to use the lower part of the matrix, \c 'U' to use the upper part.
// \return The total number of eigenvalues found.
// \exception std::invalid_argument Invalid non-square matrix provided.
// \exception std::invalid_argument Vector cannot be resized.
// \exception std::invalid_argument Invalid uplo argument provided.
// \exception std::runtime_error Eigenvalue computation failed.
//
// This function computes all eigenvalues of a Hermitian \a n-by-\a n matrix based on the
// LAPACK heevx() functions. The real eigenvalues are returned in ascending order in the given
// vector \a w. \a w is resized to the correct size (if possible and necessary).
//
// Note that this function can only be used for general, non-adapted matrices with \c float,
// \c double, \c complex<float>, or \c complex<double> element type. The attempt to call the
// function with any adapted matrix or matrices of any other element type results in a compile
// time error!
//
// The function fails if ...
//
//  - ... the given matrix \a A is not a square matrix;
//  - ... the given vector \a w is a fixed size vector and the size doesn't match;
//  - ... the given \a uplo argument is neither \c 'L' nor \c 'U';
//  - ... the eigenvalue computation fails.
//
// In all failure cases an exception is thrown.
//
// Examples:

   \code
   using blaze::DynamicMatrix;
   using blaze::DynamicVector;
   using blaze::rowMajor;
   using blaze::columnVector;

   DynamicMatrix<complex<double>,rowMajor> A( 5UL, 5UL );  // The Hermitian matrix A
   // ... Initialization

   DynamicVector<double,columnVector> w( 5UL );  // The vector for the real eigenvalues

   heevx( A, w, 'L' );
   \endcode

// For more information on the heevx() functions (i.e. cheevx() and zheevx()) see the LAPACK
// online documentation browser:
//
//        http://www.netlib.org/lapack/explore-html/
//
// \note This function can only be used if a fitting LAPACK library, which supports this function,
// is available and linked to the executable. Otherwise a call to this function will result in a
// linker error.
*/
template< typename MT  // Type of the matrix A
        , bool SO      // Storage order of the matrix A
        , typename VT  // Type of the vector w
        , bool TF >    // Transpose flag of the vector w
inline size_t heevx( DenseMatrix<MT,SO>& A, DenseVector<VT,TF>& w, char uplo )
{
   BLAZE_CONSTRAINT_MUST_NOT_BE_ADAPTOR_TYPE( MT );
   BLAZE_CONSTRAINT_MUST_NOT_BE_COMPUTATION_TYPE( MT );
   BLAZE_CONSTRAINT_MUST_HAVE_MUTABLE_DATA_ACCESS( MT );
   BLAZE_CONSTRAINT_MUST_BE_BLAS_COMPATIBLE_TYPE( ElementType_<MT> );
   BLAZE_CONSTRAINT_MUST_BE_COMPLEX_TYPE( ElementType_<MT> );

   BLAZE_CONSTRAINT_MUST_NOT_BE_COMPUTATION_TYPE( VT );
   BLAZE_CONSTRAINT_MUST_HAVE_MUTABLE_DATA_ACCESS( VT );
   BLAZE_CONSTRAINT_MUST_BE_BLAS_COMPATIBLE_TYPE( ElementType_<VT> );
   BLAZE_CONSTRAINT_MUST_BE_BUILTIN_TYPE( ElementType_<VT> );

   using CT = ElementType_<MT>;
   using BT = UnderlyingElement_<CT>;

   const size_t N( (~A).rows() );

   if( !isSquare( ~A ) ) {
      BLAZE_THROW_INVALID_ARGUMENT( "Invalid non-square matrix provided" );
   }

   if( uplo != 'L' && uplo != 'U' ) {
      BLAZE_THROW_INVALID_ARGUMENT( "Invalid uplo argument provided" );
   }

   resize( ~w, N, false );

   if( N == 0UL ) {
      return 0;
   }

   return heevx_backend( ~A, ~w, uplo, 'A', BT(), BT(), 0, 0 );
}
//*************************************************************************************************


//*************************************************************************************************
/*!\brief LAPACK kernel for computing the eigenvalues of the given dense Hermitian matrix.
// \ingroup lapack_eigenvalue
//
// \param A The given Hermitian matrix.
// \param w The resulting vector of eigenvalues.
// \param uplo \c 'L' to use the lower part of the matrix, \c 'U' to use the upper part.
// \param low The lower bound of the interval to be searched for eigenvalues.
// \param upp The upper bound of the interval to be searched for eigenvalues.
// \return The total number of eigenvalues found.
// \exception std::invalid_argument Invalid non-square matrix provided.
// \exception std::invalid_argument Vector cannot be resized.
// \exception std::invalid_argument Invalid uplo argument provided.
// \exception std::invalid_argument Invalid value range provided.
// \exception std::invalid_argument Invalid index range provided.
// \exception std::runtime_error Eigenvalue computation failed.
//
// This function computes a specific number of eigenvalues of a Hermitian \a n-by-\a n matrix
// based on the LAPACK heevx() functions. The number of eigenvalues to be computed is specified
// by the lower bound \a low and the upper bound \a upp, which either form an integral or a
// floating point range.
//
// In case \a low and \a upp are of integral type, the function computes all eigenvalues in the
// index range \f$[low..upp]\f$. The \a num resulting real eigenvalues are stored in ascending
// order in the given vector \a w, which is either resized (if possible) or expected to be a
// \a num-dimensional vector.
//
// In case \a low and \a upp are of floating point type, the function computes all eigenvalues
// in the half-open interval \f$(low..upp]\f$. The resulting real eigenvalues are stored in
// ascending order in the given vector \a w, which is either resized (if possible) or expected
// to be an \a n-dimensional vector.
//
// Note that this function can only be used for general, non-adapted matrices with \c float,
// \c double, \c complex<float>, or \c complex<double> element type. The attempt to call the
// function with any adapted matrix or matrices of any other element type results in a compile
// time error!
//
// The function fails if ...
//
//  - ... the given matrix \a A is not a square matrix;
//  - ... the given vector \a w is a fixed size vector and the size doesn't match;
//  - ... the given scalar values don't form a proper range;
//  - ... the given \a uplo argument is neither \c 'L' nor \c 'U';
//  - ... the eigenvalue computation fails.
//
// In all failure cases an exception is thrown.
//
// Examples:

   \code
   using blaze::DynamicMatrix;
   using blaze::DynamicVector;
   using blaze::rowMajor;
   using blaze::columnVector;

   DynamicMatrix<complex<double>,rowMajor> A( 5UL, 5UL );  // The Hermitian matrix A
   // ... Initialization

   DynamicVector<double,columnVector> w( 5UL );  // The vector for the real eigenvalues

   heevx( A, w, 'L', 1.0, 2.0 );  // Computes all eigenvalues in the interval (1..2]
   \endcode

   \code
   using blaze::DynamicMatrix;
   using blaze::DynamicVector;
   using blaze::rowMajor;
   using blaze::columnVector;

   DynamicMatrix<complex<double>,rowMajor> A( 5UL, 5UL );  // The Hermitian matrix A
   // ... Initialization

   DynamicVector<double,columnVector> w( 3UL );  // The vector for the real eigenvalues

   heevx( A, w, 'L', 0, 2 );  // Computes the first three eigenvalues
   \endcode

// For more information on the heevx() functions (i.e. cheevx() and zheevx()) see the LAPACK
// online documentation browser:
//
//        http://www.netlib.org/lapack/explore-html/
//
// \note This function can only be used if a fitting LAPACK library, which supports this function,
// is available and linked to the executable. Otherwise a call to this function will result in a
// linker error.
*/
template< typename MT    // Type of the matrix A
        , bool SO        // Storage order of the matrix A
        , typename VT    // Type of the vector w
        , bool TF        // Transpose flag of the vector w
        , typename ST >  // Type of the scalar boundary values
inline size_t heevx( DenseMatrix<MT,SO>& A, DenseVector<VT,TF>& w, char uplo, ST low, ST upp )
{
   BLAZE_CONSTRAINT_MUST_NOT_BE_ADAPTOR_TYPE( MT );
   BLAZE_CONSTRAINT_MUST_NOT_BE_COMPUTATION_TYPE( MT );
   BLAZE_CONSTRAINT_MUST_HAVE_MUTABLE_DATA_ACCESS( MT );
   BLAZE_CONSTRAINT_MUST_BE_BLAS_COMPATIBLE_TYPE( ElementType_<MT> );
   BLAZE_CONSTRAINT_MUST_BE_COMPLEX_TYPE( ElementType_<MT> );

   BLAZE_CONSTRAINT_MUST_NOT_BE_COMPUTATION_TYPE( VT );
   BLAZE_CONSTRAINT_MUST_HAVE_MUTABLE_DATA_ACCESS( VT );
   BLAZE_CONSTRAINT_MUST_BE_BLAS_COMPATIBLE_TYPE( ElementType_<VT> );
   BLAZE_CONSTRAINT_MUST_BE_BUILTIN_TYPE( ElementType_<VT> );

   if( !isSquare( ~A ) ) {
      BLAZE_THROW_INVALID_ARGUMENT( "Invalid non-square matrix provided" );
   }

   if( IsFloatingPoint<ST>::value && low >= upp ) {
      BLAZE_THROW_INVALID_ARGUMENT( "Invalid value range provided" );
   }

   if( !IsFloatingPoint<ST>::value && low > upp ) {
      BLAZE_THROW_INVALID_ARGUMENT( "Invalid index range provided" );
   }

   const size_t N( (~A).rows() );
   const size_t num( IsFloatingPoint<ST>::value ? N : size_t( upp - low ) + 1UL );

   if( !IsFloatingPoint<ST>::value && num > N ) {
      BLAZE_THROW_INVALID_ARGUMENT( "Invalid index range provided" );
   }

   if( uplo != 'L' && uplo != 'U' ) {
      BLAZE_THROW_INVALID_ARGUMENT( "Invalid uplo argument provided" );
   }

   resize( ~w, num, false );

   if( N == 0UL ) {
      return 0;
   }

   const char range( IsFloatingPoint<ST>::value ? 'V' : 'I' );
   const ST   vl   ( IsFloatingPoint<ST>::value ? low : ST() );
   const ST   vu   ( IsFloatingPoint<ST>::value ? upp : ST() );
   const int  il   ( IsFloatingPoint<ST>::value ? 0 : numeric_cast<int>( low ) );
   const int  iu   ( IsFloatingPoint<ST>::value ? 0 : numeric_cast<int>( upp ) );

   return heevx_backend( ~A, ~w, uplo, range, vl, vu, il, iu );
}
//*************************************************************************************************


//*************************************************************************************************
/*! \cond BLAZE_INTERNAL */
/*!\brief Backend of the LAPACK heevx kernel for Hermitian matrices.
// \ingroup lapack_eigenvalue
//
// \param A The given Hermitian matrix.
// \param w The resulting vector of eigenvalues.
// \param Z The resulting matrix of eigenvectors.
// \param uplo \c 'L' to use the lower part of the matrix, \c 'U' to use the upper part.
// \param range Specifies the range of eigenvalues to find (\c 'A', \c 'V', or \c 'I').
// \param vl The lower bound of the interval to be searched for eigenvalues (\a vl < \a vu).
// \param vu The upper bound of the interval to be searched for eigenvalues (\a vl < \a vu).
// \param il The index of the smallest eigenvalue to be returned (0 <= \a il <= \a iu).
// \param iu The index of the largest eigenvalue to be returned (0 <= \a il <= \a iu).
// \return The total number of eigenvalues found.
// \exception std::runtime_error Eigenvalue computation failed.
//
// This function is the backend implementation for computing the eigenvalues of the given
// dense Hermitian matrix.\n
// This function must \b NOT be called explicitly! It is used internally for the dispatch to
// the correct LAPACK function. Calling this function explicitly might result in erroneous
// results and/or in compilation errors. Instead of using this function use the according
// heevx() function.
*/
template< typename MT1   // Type of the matrix A
        , bool SO1       // Storage order of the matrix A
        , typename VT    // Type of the vector w
        , bool TF        // Transpose flag of the vector w
        , typename MT2   // Type of the matrix Z
        , bool SO2       // Storage order of the matrix Z
        , typename ST >  // Type of the scalar boundary values
inline size_t heevx_backend( DenseMatrix<MT1,SO1>& A, DenseVector<VT,TF>& w,
                             DenseMatrix<MT2,SO2>& Z, char uplo, char range,
                             ST vl, ST vu, int il, int iu )
{
   BLAZE_INTERNAL_ASSERT( isSquare( ~A ), "Invalid non-square matrix detected" );
   BLAZE_INTERNAL_ASSERT( range != 'A' || (~w).size() == (~A).rows(), "Invalid vector dimension detected" );
   BLAZE_INTERNAL_ASSERT( range != 'V' || (~w).size() == (~A).rows(), "Invalid vector dimension detected" );
   BLAZE_INTERNAL_ASSERT( range != 'I' || (~w).size() == size_t( iu-il+1 ), "Invalid vector dimension detected" );
   BLAZE_INTERNAL_ASSERT( SO2  || (~Z).rows()    == (~w).size(), "Invalid matrix dimension detected" );
   BLAZE_INTERNAL_ASSERT( SO2  || (~Z).columns() == (~A).rows(), "Invalid matrix dimension detected" );
   BLAZE_INTERNAL_ASSERT( !SO2 || (~Z).rows()    == (~A).rows(), "Invalid matrix dimension detected" );
   BLAZE_INTERNAL_ASSERT( !SO2 || (~Z).columns() == (~w).size(), "Invalid matrix dimension detected" );

   using CT = ElementType_<MT1>;
   using BT = UnderlyingElement_<CT>;

   BLAZE_CONSTRAINT_MUST_BE_COMPLEX_TYPE( CT );
   BLAZE_CONSTRAINT_MUST_BE_BUILTIN_TYPE( BT );

   int n   ( numeric_cast<int>( (~A).rows() ) );
   int lda ( numeric_cast<int>( (~A).spacing() ) );
   int m   ( 0 );
   int ldz ( numeric_cast<int>( (~Z).spacing() ) );
   int info( 0 );

   int lwork( 12*n + 2 );
   const std::unique_ptr<CT[]>  work ( new CT[lwork] );
   const std::unique_ptr<BT[]>  rwork( new BT[7*n] );
   const std::unique_ptr<int[]> iwork( new int[5*n] );
   const std::unique_ptr<int[]> ifail( new int[n] );

   heevx( 'N', range, uplo, n, (~A).data(), lda, vl, vu, il, iu, BT(0), &m, (~w).data(),
          (~Z).data(), ldz, work.get(), lwork, rwork.get(), iwork.get(), ifail.get(), &info );

   const size_t num( numeric_cast<size_t>( m ) );

   BLAZE_INTERNAL_ASSERT( info >= 0, "Invalid argument for eigenvalue computation" );
   BLAZE_INTERNAL_ASSERT( num <= (~w).size(), "Invalid number of eigenvalues detected" );

   if( info > 0 ) {
      BLAZE_THROW_LAPACK_ERROR( "Eigenvalue computation failed" );
   }

   return num;
}
/*! \endcond */
//*************************************************************************************************


//*************************************************************************************************
/*!\brief LAPACK kernel for computing the eigenvalues of the given dense Hermitian matrix.
// \ingroup lapack_eigenvalue
//
// \param A The given Hermitian matrix.
// \param w The resulting vector of eigenvalues.
// \param Z The resulting matrix of eigenvectors.
// \param uplo \c 'L' to use the lower part of the matrix, \c 'U' to use the upper part.
// \return The total number of eigenvalues found.
// \exception std::invalid_argument Invalid non-square matrix provided.
// \exception std::invalid_argument Vector cannot be resized.
// \exception std::invalid_argument Matrix cannot be resized.
// \exception std::invalid_argument Invalid uplo argument provided.
// \exception std::runtime_error Eigenvalue computation failed.
//
// This function computes all eigenvalues of a Hermitian \a n-by-\a n matrix based on the LAPACK
// heevx() functions. Additionally, it computes all eigenvectors of \a A. The real eigenvalues
// are returned in ascending order in the given vector \a w. The eigenvectors are returned in
// the rows of \a Z in case \a Z is a row-major matrix and in the columns of \a Z in case \a Z
// is a column-major matrix. Both \a w and \a Z are resized to the correct dimensions (if
// possible and necessary).
//
// Note that this function can only be used for general, non-adapted matrices with \c float,
// \c double, \c complex<float>, or \c complex<double> element type. The attempt to call the
// function with any adapted matrix or matrices of any other element type results in a compile
// time error!
//
// The function fails if ...
//
//  - ... the given matrix \a A is not a square matrix;
//  - ... the given vector \a w is a fixed size vector and the size doesn't match;
//  - ... the given matrix \a Z is a fixed size matrix and the dimensions don't match;
//  - ... the given \a uplo argument is neither \c 'L' nor \c 'U';
//  - ... the eigenvalue computation fails.
//
// In all failure cases an exception is thrown.
//
// Examples:

   \code
   using blaze::DynamicMatrix;
   using blaze::DynamicVector;
   using blaze::rowMajor;
   using blaze::columnVector;

   DynamicMatrix<complex<double>,rowMajor> A( 5UL, 5UL );  // The Hermitian matrix A
   // ... Initialization

   DynamicVector<double,columnVector>      w( 5UL );       // The vector for the real eigenvalues
   DynamicMatrix<complex<double>,rowMajor> Z( 5UL, 5UL );  // The matrix for the complex eigenvectors

   heevx( A, w, 'L' );
   \endcode

// For more information on the heevx() functions (i.e. cheevx() and zheevx()) see the LAPACK
// online documentation browser:
//
//        http://www.netlib.org/lapack/explore-html/
//
// \note This function can only be used if a fitting LAPACK library, which supports this function,
// is available and linked to the executable. Otherwise a call to this function will result in a
// linker error.
*/
template< typename MT1  // Type of the matrix A
        , bool SO1      // Storage order of the matrix A
        , typename VT   // Type of the vector w
        , bool TF       // Transpose flag of the vector w
        , typename MT2  // Type of the matrix Z
        , bool SO2 >    // Storage order of the matrix Z
inline size_t heevx( DenseMatrix<MT1,SO1>& A, DenseVector<VT,TF>& w,
                     DenseMatrix<MT2,SO2>& Z, char uplo )
{
   BLAZE_CONSTRAINT_MUST_NOT_BE_ADAPTOR_TYPE( MT1 );
   BLAZE_CONSTRAINT_MUST_NOT_BE_COMPUTATION_TYPE( MT1 );
   BLAZE_CONSTRAINT_MUST_HAVE_MUTABLE_DATA_ACCESS( MT1 );
   BLAZE_CONSTRAINT_MUST_BE_BLAS_COMPATIBLE_TYPE( ElementType_<MT1> );
   BLAZE_CONSTRAINT_MUST_BE_COMPLEX_TYPE( ElementType_<MT1> );

   BLAZE_CONSTRAINT_MUST_NOT_BE_COMPUTATION_TYPE( VT );
   BLAZE_CONSTRAINT_MUST_HAVE_MUTABLE_DATA_ACCESS( VT );
   BLAZE_CONSTRAINT_MUST_BE_BLAS_COMPATIBLE_TYPE( ElementType_<VT> );
   BLAZE_CONSTRAINT_MUST_BE_BUILTIN_TYPE( ElementType_<VT> );

   BLAZE_CONSTRAINT_MUST_NOT_BE_ADAPTOR_TYPE( MT2 );
   BLAZE_CONSTRAINT_MUST_NOT_BE_COMPUTATION_TYPE( MT2 );
   BLAZE_CONSTRAINT_MUST_HAVE_MUTABLE_DATA_ACCESS( MT2 );
   BLAZE_CONSTRAINT_MUST_BE_BLAS_COMPATIBLE_TYPE( ElementType_<MT2> );
   BLAZE_CONSTRAINT_MUST_BE_COMPLEX_TYPE( ElementType_<MT2> );

   using CT = ElementType_<MT1>;
   using BT = UnderlyingElement_<CT>;

   const size_t N( (~A).rows() );

   if( !isSquare( ~A ) ) {
      BLAZE_THROW_INVALID_ARGUMENT( "Invalid non-square matrix provided" );
   }

   if( uplo != 'L' && uplo != 'U' ) {
      BLAZE_THROW_INVALID_ARGUMENT( "Invalid uplo argument provided" );
   }

   resize( ~w, N, false );
   resize( ~Z, N, N, false );

   if( N == 0UL ) {
      return 0;
   }

   return heevx_backend( ~A, ~w, ~Z, uplo, 'A', BT(), BT(), 0, 0 );
}
//*************************************************************************************************


//*************************************************************************************************
/*!\brief LAPACK kernel for computing the eigenvalues of the given dense Hermitian matrix.
// \ingroup lapack_eigenvalue
//
// \param A The given Hermitian matrix.
// \param w The resulting vector of eigenvalues.
// \param Z The resulting matrix of eigenvectors.
// \param uplo \c 'L' to use the lower part of the matrix, \c 'U' to use the upper part.
// \param low The lower bound of the interval to be searched for eigenvalues.
// \param upp The upper bound of the interval to be searched for eigenvalues.
// \return The total number of eigenvalues found.
// \exception std::invalid_argument Invalid non-square matrix provided.
// \exception std::invalid_argument Vector cannot be resized.
// \exception std::invalid_argument Matrix cannot be resized.
// \exception std::invalid_argument Invalid uplo argument provided.
// \exception std::invalid_argument Invalid value range provided.
// \exception std::invalid_argument Invalid index range provided.
// \exception std::runtime_error Eigenvalue computation failed.
//
// This function computes a specific number of eigenvalues of a Hermitian \a n-by-\a n matrix
// based on the LAPACK heevx() functions. Additionally, it computes a specific number of
// eigenvectors of \a A. The number of eigenvalues to be computed is specified by the lower
// bound \a low and the upper bound \a upp, which either form an integral or a floating point
// range.
//
// In case \a low and \a upp are of integral type, the function computes all eigenvalues in the
// index range \f$[low..upp]\f$. The \a num resulting real eigenvalues are stored in ascending
// order in the given vector \a w, which is either resized (if possible) or expected to be a
// \a num-dimensional vector. The eigenvectors are returned in the rows of \a Z in case \a Z is
// row-major matrix and in the columns of \a Z in case \a Z is a column-major matrix. \a Z is
// resized (if possible) or expected to be a \a num-by-\a n row-major matrix or a \a n-by-\a num
// column-major matrix.
//
// In case \a low and \a upp are of floating point type, the function computes all eigenvalues
// in the half-open interval \f$(low..upp]\f$. The resulting real eigenvalues are stored in
// ascending order in the given vector \a w, which is either resized (if possible) or expected
// to be an \a n-dimensional vector. The eigenvectors are returned in the rows of \a Z in case
// \a Z is a row-major matrix and in the columns of \a Z in case \a Z is a column-major matrix.
// \a Z is resized (if possible) or expected to be a \a n-by-\a n matrix.
//
// Note that this function can only be used for general, non-adapted matrices with \c float,
// \c double, \c complex<float>, or \c complex<double> element type. The attempt to call the
// function with any adapted matrix or matrices of any other element type results in a compile
// time error!
//
// The function fails if ...
//
//  - ... the given matrix \a A is not a square matrix;
//  - ... the given vector \a w is a fixed size vector and the size doesn't match;
//  - ... the given matrix \a Z is a fixed size matrix and the dimensions don't match;
//  - ... the given \a uplo argument is neither \c 'L' nor \c 'U';
//  - ... the eigenvalue computation fails.
//
// In all failure cases an exception is thrown.
//
// Examples:

   \code
   using blaze::DynamicMatrix;
   using blaze::DynamicVector;
   using blaze::rowMajor;
   using blaze::columnVector;

   DynamicMatrix<complex<double>,rowMajor> A( 5UL, 5UL );  // The Hermitian matrix A
   // ... Initialization

   DynamicVector<double,columnVector>      w( 5UL );       // The vector for the real eigenvalues
   DynamicMatrix<complex<double>,rowMajor> Z( 5UL, 5UL );  // The matrix for the complex eigenvectors

   heevx( A, w, Z, 'L', 1.0, 2.0 );  // Computes all eigenvalues in the interval (1..2]
   \endcode

   \code
   using blaze::DynamicMatrix;
   using blaze::DynamicVector;
   using blaze::rowMajor;
   using blaze::columnVector;

   DynamicMatrix<complex<double>,rowMajor> A( 5UL, 5UL );  // The Hermitian matrix A
   // ... Initialization

   DynamicVector<double,columnVector>      w( 3UL );       // The vector for the real eigenvalues
   DynamicMatrix<complex<double>,rowMajor> Z( 3UL, 5UL );  // The matrix for the complex eigenvectors

   heevx( A, w, Z, 'L', 0, 2 );  // Computes the first three eigenvalues
   \endcode

// For more information on the heevx() functions (i.e. cheevx() and zheevx()) see the LAPACK
// online documentation browser:
//
//        http://www.netlib.org/lapack/explore-html/
//
// \note This function can only be used if a fitting LAPACK library, which supports this function,
// is available and linked to the executable. Otherwise a call to this function will result in a
// linker error.
*/
template< typename MT1   // Type of the matrix A
        , bool SO1       // Storage order of the matrix A
        , typename VT    // Type of the vector w
        , bool TF        // Transpose flag of the vector w
        , typename MT2   // Type of the matrix Z
        , bool SO2       // Storage order of the matrix Z
        , typename ST >  // Type of the scalar boundary values
inline size_t heevx( DenseMatrix<MT1,SO1>& A, DenseVector<VT,TF>& w,
                     DenseMatrix<MT2,SO2>& Z, char uplo, ST low, ST upp )
{
   BLAZE_CONSTRAINT_MUST_NOT_BE_ADAPTOR_TYPE( MT1 );
   BLAZE_CONSTRAINT_MUST_NOT_BE_COMPUTATION_TYPE( MT1 );
   BLAZE_CONSTRAINT_MUST_HAVE_MUTABLE_DATA_ACCESS( MT1 );
   BLAZE_CONSTRAINT_MUST_BE_BLAS_COMPATIBLE_TYPE( ElementType_<MT1> );
   BLAZE_CONSTRAINT_MUST_BE_COMPLEX_TYPE( ElementType_<MT1> );

   BLAZE_CONSTRAINT_MUST_NOT_BE_COMPUTATION_TYPE( VT );
   BLAZE_CONSTRAINT_MUST_HAVE_MUTABLE_DATA_ACCESS( VT );
   BLAZE_CONSTRAINT_MUST_BE_BLAS_COMPATIBLE_TYPE( ElementType_<VT> );
   BLAZE_CONSTRAINT_MUST_BE_BUILTIN_TYPE( ElementType_<VT> );

   BLAZE_CONSTRAINT_MUST_NOT_BE_ADAPTOR_TYPE( MT2 );
   BLAZE_CONSTRAINT_MUST_NOT_BE_COMPUTATION_TYPE( MT2 );
   BLAZE_CONSTRAINT_MUST_HAVE_MUTABLE_DATA_ACCESS( MT2 );
   BLAZE_CONSTRAINT_MUST_BE_BLAS_COMPATIBLE_TYPE( ElementType_<MT2> );
   BLAZE_CONSTRAINT_MUST_BE_COMPLEX_TYPE( ElementType_<MT2> );

   if( !isSquare( ~A ) ) {
      BLAZE_THROW_INVALID_ARGUMENT( "Invalid non-square matrix provided" );
   }

   if( IsFloatingPoint<ST>::value && low >= upp ) {
      BLAZE_THROW_INVALID_ARGUMENT( "Invalid value range provided" );
   }

   if( !IsFloatingPoint<ST>::value && low > upp ) {
      BLAZE_THROW_INVALID_ARGUMENT( "Invalid index range provided" );
   }

   const size_t N( (~A).rows() );
   const size_t num( IsFloatingPoint<ST>::value ? N : size_t( upp - low ) + 1UL );

   if( !IsFloatingPoint<ST>::value && num > N ) {
      BLAZE_THROW_INVALID_ARGUMENT( "Invalid index range provided" );
   }

   if( uplo != 'L' && uplo != 'U' ) {
      BLAZE_THROW_INVALID_ARGUMENT( "Invalid uplo argument provided" );
   }

   resize( ~w, num, false );
   resize( ~Z, ( IsRowMajorMatrix<MT2>::value ? num : N ),
           ( IsRowMajorMatrix<MT2>::value ? N : num ), false );

   if( N == 0UL ) {
      return 0;
   }

   const char range( IsFloatingPoint<ST>::value ? 'V' : 'I' );
   const ST   vl   ( IsFloatingPoint<ST>::value ? low : ST() );
   const ST   vu   ( IsFloatingPoint<ST>::value ? upp : ST() );
   const int  il   ( IsFloatingPoint<ST>::value ? 0 : numeric_cast<int>( low ) );
   const int  iu   ( IsFloatingPoint<ST>::value ? 0 : numeric_cast<int>( upp ) );

   return heevx_backend( ~A, ~w, ~Z, uplo, range, vl, vu, il, iu );
}
//*************************************************************************************************

} // namespace blaze

#endif
