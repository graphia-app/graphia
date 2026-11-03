//=================================================================================================
/*!
//  \file blaze/math/expressions/DVecFixExpr.h
//  \brief Header file for the dense vector fix expression
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

#ifndef _BLAZE_MATH_EXPRESSIONS_DVECFIXEXPR_H_
#define _BLAZE_MATH_EXPRESSIONS_DVECFIXEXPR_H_


//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <array>
#include <blaze/math/constraints/DenseVector.h>
#include <blaze/math/constraints/Expression.h>
#include <blaze/math/constraints/TransposeFlag.h>
#include <blaze/math/Exception.h>
#include <blaze/math/expressions/DenseVector.h>
#include <blaze/math/InitializerList.h>
#include <blaze/system/MacroDisable.h>
#include <blaze/util/Types.h>


namespace blaze {

//=================================================================================================
//
//  CLASS DVECFIXEXPR
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Expression object for fixing the size of a dense vector.
// \ingroup dense_vector_expression
//
// The DVecFixExpr class represents the compile time expression for fixing the size of dense
// vectors.
*/
template< typename VT  // Type of the dense vector
        , bool TF >    // Transpose flag
class DVecFixExpr
{
 public:
   //**Constructor*********************************************************************************
   /*!\brief Constructor for the DVecTransposer class.
   //
   // \param dv The dense vector operand.
   */
   explicit inline DVecFixExpr( VT& dv ) noexcept
      : dv_( dv )  // The dense vector operand
   {}
   //**********************************************************************************************

   //**********************************************************************************************
   /*!\brief List assignment to all vector elements.
   //
   // \param list The initializer list.
   // \exception std::invalid_argument Invalid assignment to fixed-size vector.
   // \return Reference to the assigned fixed-size vector.
   //
   // This assignment operator offers the option to directly (copy) assign to all elements of the
   // vector by means of an initializer list. In case the size of the given initializer doesn't
   // match the size of this vector, a \a std::invalid_argument exception is thrown.
   */
   template< typename Type >  // Type of the initializer list elements
   DVecFixExpr& operator=( initializer_list<Type> list )
   {
      if( dv_.size() != list.size() ) {
         BLAZE_THROW_INVALID_ARGUMENT( "Invalid assignment to fixed-size vector" );
      }

      dv_ = list;

      return *this;
   }
   //**********************************************************************************************

   //**********************************************************************************************
   /*!\brief Array assignment to all vector elements.
   //
   // \param array Static array for the assignment.
   // \exception std::invalid_argument Invalid assignment to fixed-size vector.
   // \return Reference to the assigned fixed-size vector.
   //
   // This assignment operator offers the option to directly (copy) assign to all elements of the
   // vector by means of a static array. In case the size of the given array doesn't match the
   // size of this vector, a \a std::invalid_argument exception is thrown.
   */
   template< typename Other  // Data type of the static array
           , size_t Dim >    // Dimension of the static array
   DVecFixExpr& operator=( const Other (&array)[Dim] )
   {
      if( dv_.size() != Dim ) {
         BLAZE_THROW_INVALID_ARGUMENT( "Invalid assignment to fixed-size vector" );
      }

      dv_ = array;

      return *this;
   }
   //**********************************************************************************************

   //**********************************************************************************************
   /*!\brief Array assignment to all vector elements.
   //
   // \param array The given std::array for the assignment.
   // \exception std::invalid_argument Invalid assignment to fixed-size vector.
   // \return Reference to the assigned fixed-size vector.
   //
   // This assignment operator offers the option to directly (copy) assign to all elements of the
   // vector by means of a std::array. In case the size of the given array doesn't match the size
   // of this vector, a \a std::invalid_argument exception is thrown.
   */
   template< typename Other  // Data type of the std::array
           , size_t Dim >    // Dimension of the std::array
   DVecFixExpr& operator=( const std::array<Other,Dim>& array )
   {
      if( dv_.size() != Dim ) {
         BLAZE_THROW_INVALID_ARGUMENT( "Invalid assignment to fixed-size vector" );
      }

      dv_ = array;

      return *this;
   }
   //**********************************************************************************************

   //**********************************************************************************************
   /*!\brief Assignment operator for different vectors.
   //
   // \param rhs Vector to be copied.
   // \exception std::invalid_argument Invalid assignment to fixed-size vector.
   // \return Reference to the assigned fixed-size vector.
   //
   // This assignment operator offers the option to directly (copy) assign to all elements of the
   // vector by means of a vector. In case the size of the given vector doesn't match the size
   // of this vector, a \a std::invalid_argument exception is thrown.
   */
   template< typename VT2 >  // Type of the right-hand side vector
   DVecFixExpr& operator=( const Vector<VT2,TF>& rhs )
   {
      if( dv_.size() != (*rhs).size() ) {
         BLAZE_THROW_INVALID_ARGUMENT( "Invalid assignment to fixed-size vector" );
      }

      dv_ = *rhs;

      return *this;
   }
   //**********************************************************************************************

 private:
   //**Member variables****************************************************************************
   VT& dv_;  //!< The dense vector operand.
   //**********************************************************************************************

   //**Compile time checks*************************************************************************
   /*! \cond BLAZE_INTERNAL */
   BLAZE_CONSTRAINT_MUST_BE_DENSE_VECTOR_TYPE( VT );
   BLAZE_CONSTRAINT_MUST_BE_VECTOR_WITH_TRANSPOSE_FLAG( VT, TF );
   BLAZE_CONSTRAINT_MUST_NOT_BE_EXPRESSION_TYPE( VT );
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
/*!\brief Fixing the size of the given dense vector.
// \ingroup dense_vector
//
// \param dv The dense vector to be size-fixed.
// \return The size-fixed dense vector.
//
// This function returns an expression representing the size-fixed given dense vector:

   \code
   blaze::DynamicVector<double> a;
   blaze::DynamicVector<double> b;
   // ... Resizing and initialization
   fix( b ) = a;
   \endcode
*/
template< typename VT  // Type of the dense vector
        , bool TF >    // Transpose flag
decltype(auto) fix( DenseVector<VT,TF>& dv ) noexcept
{
   return DVecFixExpr<VT,TF>( *dv );
}
//*************************************************************************************************

} // namespace blaze

#endif
