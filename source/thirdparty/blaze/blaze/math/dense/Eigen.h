//=================================================================================================
/*!
//  \file blaze/math/dense/Eigen.h
//  \brief Header file for the dense matrix eigenvalue functions
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

#ifndef _BLAZE_MATH_DENSE_EIGEN_H_
#define _BLAZE_MATH_DENSE_EIGEN_H_


//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <blaze/math/Aliases.h>
#include <blaze/math/constraints/Adaptor.h>
#include <blaze/math/constraints/BLASCompatible.h>
#include <blaze/math/constraints/Computation.h>
#include <blaze/math/constraints/MutableDataAccess.h>
#include <blaze/math/expressions/DenseMatrix.h>
#include <blaze/math/expressions/DenseVector.h>
#include <blaze/math/lapack/geev.h>
#include <blaze/math/lapack/heevd.h>
#include <blaze/math/lapack/syevd.h>
#include <blaze/math/typetraits/IsHermitian.h>
#include <blaze/math/typetraits/IsRowMajorMatrix.h>
#include <blaze/math/typetraits/IsSymmetric.h>
#include <blaze/math/typetraits/RemoveAdaptor.h>
#include <blaze/util/DisableIf.h>
#include <blaze/util/EnableIf.h>
#include <blaze/util/mpl/And.h>
#include <blaze/util/mpl/Or.h>
#include <blaze/util/typetraits/IsComplex.h>
#include <blaze/util/typetraits/IsFloatingPoint.h>


namespace blaze {

//=================================================================================================
//
//  EIGENVALUE FUNCTIONS
//
//=================================================================================================

//*************************************************************************************************
/*!\name Eigenvalue functions */
//@{
template< typename MT, bool SO, typename VT, bool TF >
inline void eigen( const DenseMatrix<MT,SO>& A, DenseVector<VT,TF>& w );

template< typename MT1, bool SO1, typename VT, bool TF, typename MT2, bool SO2 >
inline void eigen( const DenseMatrix<MT1,SO1>& A, DenseVector<VT,TF>& w, DenseMatrix<MT2,SO2>& V );
//@}
//*************************************************************************************************


//*************************************************************************************************
/*! \cond BLAZE_INTERNAL */
/*!\brief Backend for the eigenvalue computation of the given dense symmetric matrix.
// \ingroup dense_matrix
//
// \param A The given symmetric matrix.
// \param w The resulting vector of eigenvalues.
// \return void
// \exception std::invalid_argument Invalid non-square matrix provided.
// \exception std::invalid_argument Vector cannot be resized.
// \exception std::runtime_error Eigenvalue computation failed.
//
// This function is the backend implementation for computing the eigenvalues of the given
// dense symmetric matrix.\n
// This function must \b NOT be called explicitly! It is used internally for the dispatch to
// the correct LAPACK function. Calling this function explicitly might result in erroneous
// results and/or in compilation errors. Instead of using this function use the according
// eigen() function.
*/
template< typename MT  // Type of the matrix A
        , bool SO      // Storage order of the matrix A
        , typename VT  // Type of the vector w
        , bool TF >    // Transpose flag of the vector w
inline EnableIf_< And< IsSymmetric<MT>, IsFloatingPoint< ElementType_<MT> > > >
   eigen_backend( const DenseMatrix<MT,SO>& A, DenseVector<VT,TF>& w )
{
   using Tmp = ResultType_< RemoveAdaptor_<MT> >;

   BLAZE_CONSTRAINT_MUST_NOT_BE_ADAPTOR_TYPE( Tmp );
   BLAZE_CONSTRAINT_MUST_NOT_BE_COMPUTATION_TYPE( Tmp );
   BLAZE_CONSTRAINT_MUST_HAVE_MUTABLE_DATA_ACCESS( Tmp );
   BLAZE_CONSTRAINT_MUST_BE_BLAS_COMPATIBLE_TYPE( ElementType_<Tmp> );

   Tmp tmp( A );

   syevd( tmp, ~w, 'N', 'L' );
}
/*! \endcond */
//*************************************************************************************************


//*************************************************************************************************
/*! \cond BLAZE_INTERNAL */
/*!\brief Backend for the eigenvalue computation of the given dense Hermitian matrix.
// \ingroup dense_matrix
//
// \param A The given Hermitian matrix.
// \param w The resulting vector of eigenvalues.
// \return void
// \exception std::invalid_argument Invalid non-square matrix provided.
// \exception std::invalid_argument Vector cannot be resized.
// \exception std::runtime_error Eigenvalue computation failed.
//
// This function is the backend implementation for computing the eigenvalues of the given
// dense Hermitian matrix.\n
// This function must \b NOT be called explicitly! It is used internally for the dispatch to
// the correct LAPACK function. Calling this function explicitly might result in erroneous
// results and/or in compilation errors. Instead of using this function use the according
// eigen() function.
*/
template< typename MT  // Type of the matrix A
        , bool SO      // Storage order of the matrix A
        , typename VT  // Type of the vector w
        , bool TF >    // Transpose flag of the vector w
inline EnableIf_< And< IsHermitian<MT>, IsComplex< ElementType_<MT> > > >
   eigen_backend( const DenseMatrix<MT,SO>& A, DenseVector<VT,TF>& w )
{
   using Tmp = ResultType_< RemoveAdaptor_<MT> >;

   BLAZE_CONSTRAINT_MUST_NOT_BE_ADAPTOR_TYPE( Tmp );
   BLAZE_CONSTRAINT_MUST_NOT_BE_COMPUTATION_TYPE( Tmp );
   BLAZE_CONSTRAINT_MUST_HAVE_MUTABLE_DATA_ACCESS( Tmp );
   BLAZE_CONSTRAINT_MUST_BE_BLAS_COMPATIBLE_TYPE( ElementType_<Tmp> );

   Tmp tmp( A );

   heevd( tmp, ~w, 'N', 'L' );
}
/*! \endcond */
//*************************************************************************************************


//*************************************************************************************************
/*! \cond BLAZE_INTERNAL */
/*!\brief Backend for the eigenvalue computation of the given dense general matrix.
// \ingroup dense_matrix
//
// \param A The given general matrix.
// \param w The resulting vector of eigenvalues.
// \return void
// \exception std::invalid_argument Invalid non-square matrix provided.
// \exception std::invalid_argument Vector cannot be resized.
// \exception std::runtime_error Eigenvalue computation failed.
//
// This function is the backend implementation for computing the eigenvalues of the given
// dense general matrix.\n
// This function must \b NOT be called explicitly! It is used internally for the dispatch to
// the correct LAPACK function. Calling this function explicitly might result in erroneous
// results and/or in compilation errors. Instead of using this function use the according
// eigen() function.
*/
template< typename MT  // Type of the matrix A
        , bool SO      // Storage order of the matrix A
        , typename VT  // Type of the vector w
        , bool TF >    // Transpose flag of the vector w
inline DisableIf_< Or< And< IsSymmetric<MT>, IsFloatingPoint< ElementType_<MT> > >
                     , And< IsHermitian<MT>, IsComplex< ElementType_<MT> > > > >
   eigen_backend( const DenseMatrix<MT,SO>& A, DenseVector<VT,TF>& w )
{
   using Tmp = ResultType_< RemoveAdaptor_<MT> >;

   BLAZE_CONSTRAINT_MUST_NOT_BE_ADAPTOR_TYPE( Tmp );
   BLAZE_CONSTRAINT_MUST_NOT_BE_COMPUTATION_TYPE( Tmp );
   BLAZE_CONSTRAINT_MUST_HAVE_MUTABLE_DATA_ACCESS( Tmp );
   BLAZE_CONSTRAINT_MUST_BE_BLAS_COMPATIBLE_TYPE( ElementType_<Tmp> );

   Tmp tmp( A );

   geev( tmp, ~w );
}
/*! \endcond */
//*************************************************************************************************


//*************************************************************************************************
/*!\brief Eigenvalue computation of the given dense matrix.
// \ingroup dense_matrix
//
// \param A The given general matrix.
// \param w The resulting vector of eigenvalues.
// \return void
// \exception std::invalid_argument Invalid non-square matrix provided.
// \exception std::invalid_argument Vector cannot be resized.
// \exception std::runtime_error Eigenvalue computation failed.
//
// This function computes the eigenvalues of the given \a n-by-\a n matrix. The eigenvalues are
// returned in the given vector \a w, which is resized to the correct size (if possible and
// necessary).
//
// Please note that in case the given matrix is either a compile time symmetric matrix with
// floating point elements or an Hermitian matrix with complex elements, the resulting eigenvalues
// will be of floating point type and therefore the elements of the given eigenvalue vector are
// expected to be of floating point type. In all other cases they are expected to be of complex
// type. Also please note that for complex eigenvalues no order of eigenvalues can be assumed,
// except that complex conjugate pairs of eigenvalues appear consecutively with the eigenvalue
// having the positive imaginary part first.
//
// The function fails if ...
//
//  - ... the given matrix \a A is not a square matrix;
//  - ... the given vector \a w is a fixed size vector and the size doesn't match;
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

   DynamicMatrix<double,rowMajor> A( 5UL, 5UL );  // The general matrix A
   // ... Initialization

   DynamicVector<complex<double>,columnVector> w( 5UL );  // The vector for the complex eigenvalues

   eigen( A, w );
   \endcode

   \code
   using blaze::SymmetricMatrix;
   using blaze::DynamicMatrix;
   using blaze::DynamicVector;
   using blaze::rowMajor;
   using blaze::columnVector;

   SymmetricMatrix< DynamicMatrix<double,rowMajor> > A( 5UL, 5UL );  // The symmetric matrix A
   // ... Initialization

   DynamicVector<double,columnVector> w( 5UL );  // The vector for the real eigenvalues

   eigen( A, w );
   \endcode

   \code
   using blaze::HermitianMatrix;
   using blaze::DynamicMatrix;
   using blaze::DynamicVector;
   using blaze::rowMajor;
   using blaze::columnVector;

   DynamicMatrix<complex<double>,rowMajor> A( 5UL, 5UL );  // The Hermitian matrix A
   // ... Initialization

   DynamicVector<double,columnVector> w( 5UL );  // The vector for the real eigenvalues

   eigen( A, w );
   \endcode

// \note This function only works for matrices with \c float, \c double, \c complex<float>, or
// \c complex<double> element type. The attempt to call the function with matrices of any other
// element type results in a compile time error!
//
// \note This function can only be used if a fitting LAPACK library is available and linked to
// the executable. Otherwise a call to this function will result in a linker error.
//
// \note Further options for computing eigenvalues and eigenvectors are available via the geev(),
// syev(), syevd(), syevx(), heev(), heevd(), and heevx() functions.
*/
template< typename MT  // Type of the matrix A
        , bool SO      // Storage order of the matrix A
        , typename VT  // Type of the vector w
        , bool TF >    // Transpose flag of the vector w
inline void eigen( const DenseMatrix<MT,SO>& A, DenseVector<VT,TF>& w )
{
   BLAZE_CONSTRAINT_MUST_NOT_BE_COMPUTATION_TYPE( MT );
   BLAZE_CONSTRAINT_MUST_BE_BLAS_COMPATIBLE_TYPE( ElementType_<MT> );

   BLAZE_CONSTRAINT_MUST_NOT_BE_COMPUTATION_TYPE( VT );
   BLAZE_CONSTRAINT_MUST_HAVE_MUTABLE_DATA_ACCESS( VT );
   BLAZE_CONSTRAINT_MUST_BE_BLAS_COMPATIBLE_TYPE( ElementType_<VT> );

   eigen_backend( ~A, ~w );
}
//*************************************************************************************************


//*************************************************************************************************
/*! \cond BLAZE_INTERNAL */
/*!\brief Backend for the eigenvalue computation of the given dense symmetric matrix.
// \ingroup dense_matrix
//
// \param A The given symmetric matrix.
// \param w The resulting vector of eigenvalues.
// \param V The resulting matrix of eigenvectors.
// \return void
// \exception std::invalid_argument Invalid non-square matrix provided.
// \exception std::invalid_argument Vector cannot be resized.
// \exception std::invalid_argument Matrix cannot be resized.
// \exception std::runtime_error Eigenvalue computation failed.
//
// This function is the backend implementation for computing the eigenvalues of the given
// dense symmetric matrix.\n
// This function must \b NOT be called explicitly! It is used internally for the dispatch to
// the correct LAPACK function. Calling this function explicitly might result in erroneous
// results and/or in compilation errors. Instead of using this function use the according
// eigen() function.
*/
template< typename MT1  // Type of the matrix A
        , bool SO1      // Storage order of the matrix A
        , typename VT   // Type of the vector w
        , bool TF       // Transpose flag of the vector w
        , typename MT2  // Type of the matrix V
        , bool SO2 >    // Storage order of the matrix V
inline EnableIf_< And< IsSymmetric<MT1>, IsFloatingPoint< ElementType_<MT1> > > >
   eigen_backend( const DenseMatrix<MT1,SO1>& A, DenseVector<VT,TF>& w, DenseMatrix<MT2,SO2>& V )
{
   using Tmp = ResultType_< RemoveAdaptor_<MT1> >;

   BLAZE_CONSTRAINT_MUST_NOT_BE_ADAPTOR_TYPE( Tmp );
   BLAZE_CONSTRAINT_MUST_NOT_BE_COMPUTATION_TYPE( Tmp );
   BLAZE_CONSTRAINT_MUST_HAVE_MUTABLE_DATA_ACCESS( Tmp );
   BLAZE_CONSTRAINT_MUST_BE_BLAS_COMPATIBLE_TYPE( ElementType_<Tmp> );

   Tmp tmp( A );

   syevd( tmp, ~w, 'V', 'L' );

   (~V) = tmp;
}
/*! \endcond */
//*************************************************************************************************


//*************************************************************************************************
/*! \cond BLAZE_INTERNAL */
/*!\brief Backend for the eigenvalue computation of the given dense Hermitian matrix.
// \ingroup dense_matrix
//
// \param A The given Hermitian matrix.
// \param w The resulting vector of eigenvalues.
// \param V The resulting matrix of eigenvectors.
// \return void
// \exception std::invalid_argument Invalid non-square matrix provided.
// \exception std::invalid_argument Vector cannot be resized.
// \exception std::invalid_argument Matrix cannot be resized.
// \exception std::runtime_error Eigenvalue computation failed.
//
// This function is the backend implementation for computing the eigenvalues of the given
// dense Hermitian matrix.\n
// This function must \b NOT be called explicitly! It is used internally for the dispatch to
// the correct LAPACK function. Calling this function explicitly might result in erroneous
// results and/or in compilation errors. Instead of using this function use the according
// eigen() function.
*/
template< typename MT1  // Type of the matrix A
        , bool SO1      // Storage order of the matrix A
        , typename VT   // Type of the vector w
        , bool TF       // Transpose flag of the vector w
        , typename MT2  // Type of the matrix V
        , bool SO2 >    // Storage order of the matrix V
inline EnableIf_< And< IsHermitian<MT1>, IsComplex< ElementType_<MT1> > > >
   eigen_backend( const DenseMatrix<MT1,SO1>& A, DenseVector<VT,TF>& w, DenseMatrix<MT2,SO2>& V )
{
   using Tmp = ResultType_< RemoveAdaptor_<MT1> >;

   BLAZE_CONSTRAINT_MUST_NOT_BE_ADAPTOR_TYPE( Tmp );
   BLAZE_CONSTRAINT_MUST_NOT_BE_COMPUTATION_TYPE( Tmp );
   BLAZE_CONSTRAINT_MUST_HAVE_MUTABLE_DATA_ACCESS( Tmp );
   BLAZE_CONSTRAINT_MUST_BE_BLAS_COMPATIBLE_TYPE( ElementType_<Tmp> );

   Tmp tmp( A );

   heevd( tmp, ~w, 'V', 'L' );

   (~V) = tmp;
}
/*! \endcond */
//*************************************************************************************************


//*************************************************************************************************
/*! \cond BLAZE_INTERNAL */
/*!\brief Backend for the eigenvalue computation of the given dense general matrix.
// \ingroup dense_matrix
//
// \param A The given general matrix.
// \param w The resulting vector of eigenvalues.
// \param V The resulting matrix of eigenvectors.
// \return void
// \exception std::invalid_argument Invalid non-square matrix provided.
// \exception std::invalid_argument Vector cannot be resized.
// \exception std::invalid_argument Matrix cannot be resized.
// \exception std::runtime_error Eigenvalue computation failed.
//
// This function is the backend implementation for computing the eigenvalues of the given
// dense general matrix.\n
// This function must \b NOT be called explicitly! It is used internally for the dispatch to
// the correct LAPACK function. Calling this function explicitly might result in erroneous
// results and/or in compilation errors. Instead of using this function use the according
// eigen() function.
*/
template< typename MT1  // Type of the matrix A
        , bool SO1      // Storage order of the matrix A
        , typename VT   // Type of the vector w
        , bool TF       // Transpose flag of the vector w
        , typename MT2  // Type of the matrix V
        , bool SO2 >    // Storage order of the matrix V
inline DisableIf_< Or< And< IsSymmetric<MT1>, IsFloatingPoint< ElementType_<MT1> > >
                     , And< IsHermitian<MT1>, IsComplex< ElementType_<MT1> > > > >
   eigen_backend( const DenseMatrix<MT1,SO1>& A, DenseVector<VT,TF>& w, DenseMatrix<MT2,SO2>& V )
{
   using Tmp = ResultType_< RemoveAdaptor_<MT1> >;

   BLAZE_CONSTRAINT_MUST_NOT_BE_ADAPTOR_TYPE( Tmp );
   BLAZE_CONSTRAINT_MUST_NOT_BE_COMPUTATION_TYPE( Tmp );
   BLAZE_CONSTRAINT_MUST_HAVE_MUTABLE_DATA_ACCESS( Tmp );
   BLAZE_CONSTRAINT_MUST_BE_BLAS_COMPATIBLE_TYPE( ElementType_<Tmp> );

   Tmp tmp( A );

   if( IsRowMajorMatrix<MT1>::value )
      geev( tmp, ~V, ~w );
   else
      geev( tmp, ~w, ~V );
}
/*! \endcond */
//*************************************************************************************************


//*************************************************************************************************
/*!\brief Eigenvalue computation of the given dense matrix.
// \ingroup dense_matrix
//
// \param A The given general matrix.
// \param w The resulting vector of eigenvalues.
// \param V The resulting matrix of eigenvectors.
// \return void
// \exception std::invalid_argument Invalid non-square matrix provided.
// \exception std::invalid_argument Vector cannot be resized.
// \exception std::invalid_argument Matrix cannot be resized.
// \exception std::runtime_error Eigenvalue computation failed.
//
// This function computes the eigenvalues and eigenvectors of the given \a n-by-\a n matrix.
// The eigenvalues are returned in the given vector \a w and the eigenvectors are returned in the
// given matrix \a V, which are both resized to the correct dimensions (if possible and necessary).
//
// Please note that in case the given matrix is either a compile time symmetric matrix with
// floating point elements or an Hermitian matrix with complex elements, the resulting eigenvalues
// will be of floating point type and therefore the elements of the given eigenvalue vector are
// expected to be of floating point type. In all other cases they are expected to be of complex
// type. Also please note that for complex eigenvalues no order of eigenvalues can be assumed,
// except that complex conjugate pairs of eigenvalues appear consecutively with the eigenvalue
// having the positive imaginary part first.
//
// In case \a A is a row-major matrix, the left eigenvectors are returned in the rows of \a V,
// in case \a A is a column-major matrix, the right eigenvectors are returned in the columns of
// \a V. In case the given matrix is a compile time symmetric matrix with floating point elements,
// the resulting eigenvectors will be of floating point type and therefore the elements of the
// given eigenvector matrix are expected to be of floating point type. In all other cases they
// are expected to be of complex type.
//
// The function fails if ...
//
//  - ... the given matrix \a A is not a square matrix;
//  - ... the given vector \a w is a fixed size vector and the size doesn't match;
//  - ... the given matrix \a V is a fixed size matrix and the dimensions don't match;
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

   DynamicMatrix<double,rowMajor> A( 5UL, 5UL );  // The general matrix A
   // ... Initialization

   DynamicVector<complex<double>,columnVector> w( 5UL );   // The vector for the complex eigenvalues
   DynamicMatrix<complex<double>,rowMajor> V( 5UL, 5UL );  // The matrix for the left eigenvectors

   eigen( A, w, V );
   \endcode

   \code
   using blaze::SymmetricMatrix;
   using blaze::DynamicMatrix;
   using blaze::DynamicVector;
   using blaze::rowMajor;
   using blaze::columnVector;

   SymmetricMatrix< DynamicMatrix<double,rowMajor> > A( 5UL, 5UL );  // The symmetric matrix A
   // ... Initialization

   DynamicVector<double,columnVector> w( 5UL );       // The vector for the real eigenvalues
   DynamicMatrix<double,rowMajor>     V( 5UL, 5UL );  // The matrix for the left eigenvectors

   eigen( A, w, V );
   \endcode

   \code
   using blaze::HermitianMatrix;
   using blaze::DynamicMatrix;
   using blaze::DynamicVector;
   using blaze::rowMajor;
   using blaze::columnVector;

   HermitianMatrix< DynamicMatrix<complex<double>,rowMajor> > A( 5UL, 5UL );  // The Hermitian matrix A
   // ... Initialization

   DynamicVector<double,columnVector>      w( 5UL );       // The vector for the real eigenvalues
   DynamicMatrix<complex<double>,rowMajor> V( 5UL, 5UL );  // The matrix for the left eigenvectors

   eigen( A, w, V );
   \endcode

// \note This function only works for matrices with \c float, \c double, \c complex<float>, or
// \c complex<double> element type. The attempt to call the function with matrices of any other
// element type results in a compile time error!
//
// \note This function can only be used if a fitting LAPACK library is available and linked to
// the executable. Otherwise a call to this function will result in a linker error.
//
// \note Further options for computing eigenvalues and eigenvectors are available via the geev(),
// syev(), syevd(), syevx(), heev(), heevd(), and heevx() functions.
*/
template< typename MT1  // Type of the matrix A
        , bool SO1      // Storage order of the matrix A
        , typename VT   // Type of the vector w
        , bool TF       // Transpose flag of the vector w
        , typename MT2  // Type of the matrix V
        , bool SO2 >    // Storage order of the matrix V
inline void eigen( const DenseMatrix<MT1,SO1>& A, DenseVector<VT,TF>& w, DenseMatrix<MT2,SO2>& V )
{
   BLAZE_CONSTRAINT_MUST_NOT_BE_COMPUTATION_TYPE( MT1 );
   BLAZE_CONSTRAINT_MUST_BE_BLAS_COMPATIBLE_TYPE( ElementType_<MT1> );

   BLAZE_CONSTRAINT_MUST_NOT_BE_COMPUTATION_TYPE( VT );
   BLAZE_CONSTRAINT_MUST_HAVE_MUTABLE_DATA_ACCESS( VT );
   BLAZE_CONSTRAINT_MUST_BE_BLAS_COMPATIBLE_TYPE( ElementType_<VT> );

   BLAZE_CONSTRAINT_MUST_NOT_BE_ADAPTOR_TYPE( MT2 );
   BLAZE_CONSTRAINT_MUST_NOT_BE_COMPUTATION_TYPE( MT2 );
   BLAZE_CONSTRAINT_MUST_HAVE_MUTABLE_DATA_ACCESS( MT2 );
   BLAZE_CONSTRAINT_MUST_BE_BLAS_COMPATIBLE_TYPE( ElementType_<MT2> );

   eigen_backend( ~A, ~w, ~V );
}
//*************************************************************************************************

} // namespace blaze

#endif
