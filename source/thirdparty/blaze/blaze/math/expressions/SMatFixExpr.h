//=================================================================================================
/*!
//  \file blaze/math/expressions/SMatFixExpr.h
//  \brief Header file for the sparse matrix fix expression
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

#ifndef _BLAZE_MATH_EXPRESSIONS_SMATFIXEXPR_H_
#define _BLAZE_MATH_EXPRESSIONS_SMATFIXEXPR_H_


//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <blaze/math/constraints/SparseMatrix.h>
#include <blaze/math/constraints/Expression.h>
#include <blaze/math/constraints/StorageOrder.h>
#include <blaze/math/Exception.h>
#include <blaze/math/expressions/SparseMatrix.h>
#include <blaze/math/InitializerList.h>
#include <blaze/system/MacroDisable.h>


namespace blaze {

//=================================================================================================
//
//  CLASS SMATFIXEXPR
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Expression object for fixing the size of a sparse matrix.
// \ingroup sparse_matrix_expression
//
// The SMatFixExpr class represents the compile time expression for fixing the size of
// sparse matrices.
*/
template< typename MT  // Type of the sparse matrix
        , bool SO >    // Storage order
class SMatFixExpr
{
 public:
   //**Constructor*********************************************************************************
   /*!\brief Constructor for the SMatTransposer class.
   //
   // \param sm The sparse matrix operand.
   */
   explicit inline SMatFixExpr( MT& sm ) noexcept
      : sm_( sm )  // The sparse matrix operand
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
   SMatFixExpr& operator=( initializer_list< initializer_list<Type> > list )
   {
      if( sm_.rows() != list.size() || sm_.columns() != determineColumns( list ) ) {
         BLAZE_THROW_INVALID_ARGUMENT( "Invalid assignment to fixed-size matrix" );
      }

      sm_ = list;

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
   SMatFixExpr& operator=( const Matrix<MT2,SO2>& rhs )
   {
      if( sm_.rows() != (*rhs).rows() || sm_.columns() != (*rhs).columns() ) {
         BLAZE_THROW_INVALID_ARGUMENT( "Invalid assignment to fixed-size matrix" );
      }

      sm_ = *rhs;

      return *this;
   }
   //**********************************************************************************************

 private:
   //**Member variables****************************************************************************
   MT& sm_;  //!< The sparse matrix operand.
   //**********************************************************************************************

   //**Compile time checks*************************************************************************
   /*! \cond BLAZE_INTERNAL */
   BLAZE_CONSTRAINT_MUST_BE_SPARSE_MATRIX_TYPE( MT );
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
/*!\brief Fixing the size of the given sparse matrix.
// \ingroup sparse_matrix
//
// \param sm The sparse matrix to be size-fixed.
// \return The size-fixed sparse matrix.
//
// This function returns an expression representing the size-fixed given sparse matrix:

   \code
   blaze::CompressedMatrix<double> A;
   blaze::CompressedMatrix<double> B;
   // ... Resizing and initialization
   fix( B ) = A;
   \endcode
*/
template< typename MT  // Type of the sparse matrix
        , bool SO >    // Storage order
decltype(auto) fix( SparseMatrix<MT,SO>& sm ) noexcept
{
   return SMatFixExpr<MT,SO>( *sm );
}
//*************************************************************************************************

} // namespace blaze

#endif
