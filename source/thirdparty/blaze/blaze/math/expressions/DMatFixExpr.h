//=================================================================================================
/*!
//  \file blaze/math/expressions/DMatFixExpr.h
//  \brief Header file for the dense matrix fix expression
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

#ifndef _BLAZE_MATH_EXPRESSIONS_DMATFIXEXPR_H_
#define _BLAZE_MATH_EXPRESSIONS_DMATFIXEXPR_H_


//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <array>
#include <blaze/math/constraints/DenseMatrix.h>
#include <blaze/math/constraints/Expression.h>
#include <blaze/math/constraints/StorageOrder.h>
#include <blaze/math/Exception.h>
#include <blaze/math/expressions/DenseMatrix.h>
#include <blaze/math/InitializerList.h>
#include <blaze/system/MacroDisable.h>
#include <blaze/util/Types.h>


namespace blaze {

//=================================================================================================
//
//  CLASS DMATFIXEXPR
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Expression object for fixing the size of a dense matrix.
// \ingroup dense_matrix_expression
//
// The DMatFixExpr class represents the compile time expression for fixing the size of dense
// matrices.
*/
template< typename MT  // Type of the dense matrix
        , bool SO >    // Storage order
class DMatFixExpr
{
 public:
   //**Constructor*********************************************************************************
   /*!\brief Constructor for the DMatTransposer class.
   //
   // \param dm The dense matrix operand.
   */
   explicit inline DMatFixExpr( MT& dm ) noexcept
      : dm_( dm )  // The dense matrix operand
   {}
   //**********************************************************************************************

   //**********************************************************************************************
   /*!\brief List assignment to all matrix elements.
   //
   // \param list The initializer list.
   // \exception std::invalid_argument Invalid assignment to fixed-size matrix.
   // \return Reference to the assigned fixed-size matrix.
   //
   // This assignment operator offers the option to directly (copy) assign to all elements of the
   // matrix by means of an initializer list. In case the size of the given initializer doesn't
   // match the size of this matrix, a \a std::invalid_argument exception is thrown.
   */
   template< typename Type >  // Type of the initializer list elements
   DMatFixExpr& operator=( initializer_list< initializer_list<Type> > list )
   {
      if( dm_.rows() != list.size() || dm_.columns() != determineColumns( list ) ) {
         BLAZE_THROW_INVALID_ARGUMENT( "Invalid assignment to fixed-size matrix" );
      }

      dm_ = list;

      return *this;
   }
   //**********************************************************************************************

   //**********************************************************************************************
   /*!\brief Array assignment to all matrix elements.
   //
   // \param array Static array for the assignment.
   // \exception std::invalid_argument Invalid assignment to fixed-size matrix.
   // \return Reference to the assigned fixed-size matrix.
   //
   // This assignment operator offers the option to directly (copy) assign to all elements of the
   // matrix by means of a static array. In case the size of the given array doesn't match the
   // size of this matrix, a \a std::invalid_argument exception is thrown.
   */
   template< typename Other  // Data type of the static array
           , size_t Rows     // Number of rows of the static array
           , size_t Cols >   // Number of columns of the static array
   DMatFixExpr& operator=( const Other (&array)[Rows][Cols] )
   {
      if( dm_.rows() != Rows || dm_.columns() != Cols ) {
         BLAZE_THROW_INVALID_ARGUMENT( "Invalid assignment to fixed-size matrix" );
      }

      dm_ = array;

      return *this;
   }
   //**********************************************************************************************

   //**********************************************************************************************
   /*!\brief Array assignment to all matrix elements.
   //
   // \param array The given std::array for the assignment.
   // \exception std::invalid_argument Invalid assignment to fixed-size matrix.
   // \return Reference to the assigned fixed-size matrix.
   //
   // This assignment operator offers the option to directly (copy) assign to all elements of the
   // matrix by means of a std::array. In case the size of the given array doesn't match the size
   // of this matrix, a \a std::invalid_argument exception is thrown.
   */
   template< typename Other  // Data type of the std::array
           , size_t Rows     // Number of rows of the std::array
           , size_t Cols >   // Number of columns of the std::array
   DMatFixExpr& operator=( const std::array<std::array<Other,Cols>,Rows>& array )
   {
      if( dm_.rows() != Rows || dm_.columns() != Cols ) {
         BLAZE_THROW_INVALID_ARGUMENT( "Invalid assignment to fixed-size matrix" );
      }

      dm_ = array;

      return *this;
   }
   //**********************************************************************************************

   //**********************************************************************************************
   /*!\brief Assignment operator for different matrices.
   //
   // \param rhs Matrix to be copied.
   // \exception std::invalid_argument Invalid assignment to fixed-size matrix.
   // \return Reference to the assigned fixed-size matrix.
   //
   // This assignment operator offers the option to directly (copy) assign to all elements of the
   // matrix by means of a matrix. In case the size of the given matrix doesn't match the size
   // of this matrix, a \a std::invalid_argument exception is thrown.
   */
   template< typename MT2  // Type of the right-hand side matrix
           , bool SO2 >    // Storage order of the right-hand side matrix
   DMatFixExpr& operator=( const Matrix<MT2,SO2>& rhs )
   {
      if( dm_.rows() != (*rhs).rows() || dm_.columns() != (*rhs).columns() ) {
         BLAZE_THROW_INVALID_ARGUMENT( "Invalid assignment to fixed-size matrix" );
      }

      dm_ = *rhs;

      return *this;
   }
   //**********************************************************************************************

 private:
   //**Member variables****************************************************************************
   MT& dm_;  //!< The dense matrix operand.
   //**********************************************************************************************

   //**Compile time checks*************************************************************************
   /*! \cond BLAZE_INTERNAL */
   BLAZE_CONSTRAINT_MUST_BE_DENSE_MATRIX_TYPE( MT );
   BLAZE_CONSTRAINT_MUST_BE_MATRIX_WITH_STORAGE_ORDER( MT, SO );
   BLAZE_CONSTRAINT_MUST_NOT_BE_EXPRESSION_TYPE( MT );
   /*! \endcond */
   //**********************************************************************************************
};
//*************************************************************************************************




//=================================================================================================
//
//  GLOBAL OPERATORS
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Fixing the size of the given dense matrix.
// \ingroup dense_matrix
//
// \param dm The dense matrix to be size-fixed.
// \return The size-fixed dense matrix.
//
// This function returns an expression representing the size-fixed given dense matrix:

   \code
   blaze::DynamicMatrix<double> A;
   blaze::DynamicMatrix<double> B;
   // ... Resizing and initialization
   fix( B ) = A;
   \endcode
*/
template< typename MT  // Type of the dense matrix
        , bool SO >    // Storage order
decltype(auto) fix( DenseMatrix<MT,SO>& dm ) noexcept
{
   return DMatFixExpr<MT,SO>( *dm );
}
//*************************************************************************************************

} // namespace blaze

#endif
