//=================================================================================================
/*!
//  \file blaze/math/expressions/TDMatTSMatMultExpr.h
//  \brief Header file for the transpose dense matrix/transpose sparse matrix multiplication expression
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

#ifndef _BLAZE_MATH_EXPRESSIONS_TDMATTSMATMULTEXPR_H_
#define _BLAZE_MATH_EXPRESSIONS_TDMATTSMATMULTEXPR_H_


//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <blaze/math/Aliases.h>
#include <blaze/math/constraints/ColumnMajorMatrix.h>
#include <blaze/math/constraints/DenseMatrix.h>
#include <blaze/math/constraints/MatMatMultExpr.h>
#include <blaze/math/constraints/RowMajorMatrix.h>
#include <blaze/math/constraints/SparseMatrix.h>
#include <blaze/math/constraints/StorageOrder.h>
#include <blaze/math/Exception.h>
#include <blaze/math/expressions/Computation.h>
#include <blaze/math/expressions/DenseMatrix.h>
#include <blaze/math/expressions/Forward.h>
#include <blaze/math/expressions/MatMatMultExpr.h>
#include <blaze/math/Functions.h>
#include <blaze/math/functors/DeclDiag.h>
#include <blaze/math/functors/DeclHerm.h>
#include <blaze/math/functors/DeclLow.h>
#include <blaze/math/functors/DeclSym.h>
#include <blaze/math/functors/DeclUpp.h>
#include <blaze/math/functors/Noop.h>
#include <blaze/math/shims/Conjugate.h>
#include <blaze/math/shims/IsDefault.h>
#include <blaze/math/shims/Reset.h>
#include <blaze/math/shims/Serial.h>
#include <blaze/math/SIMD.h>
#include <blaze/math/traits/ColumnExprTrait.h>
#include <blaze/math/traits/MultExprTrait.h>
#include <blaze/math/traits/MultTrait.h>
#include <blaze/math/traits/RowExprTrait.h>
#include <blaze/math/traits/SubmatrixExprTrait.h>
#include <blaze/math/traits/TDMatDeclDiagExprTrait.h>
#include <blaze/math/traits/TDMatDeclHermExprTrait.h>
#include <blaze/math/traits/TDMatDeclLowExprTrait.h>
#include <blaze/math/traits/TDMatDeclSymExprTrait.h>
#include <blaze/math/traits/TDMatDeclUppExprTrait.h>
#include <blaze/math/traits/TDMatDVecMultExprTrait.h>
#include <blaze/math/traits/TDMatSVecMultExprTrait.h>
#include <blaze/math/traits/TSMatDVecMultExprTrait.h>
#include <blaze/math/traits/TSMatSVecMultExprTrait.h>
#include <blaze/math/traits/TDVecTDMatMultExprTrait.h>
#include <blaze/math/traits/TDVecTSMatMultExprTrait.h>
#include <blaze/math/traits/TSVecTDMatMultExprTrait.h>
#include <blaze/math/typetraits/Columns.h>
#include <blaze/math/typetraits/HasSIMDAdd.h>
#include <blaze/math/typetraits/HasSIMDMult.h>
#include <blaze/math/typetraits/IsAligned.h>
#include <blaze/math/typetraits/IsColumnMajorMatrix.h>
#include <blaze/math/typetraits/IsColumnVector.h>
#include <blaze/math/typetraits/IsComputation.h>
#include <blaze/math/typetraits/IsDenseMatrix.h>
#include <blaze/math/typetraits/IsDenseVector.h>
#include <blaze/math/typetraits/IsDiagonal.h>
#include <blaze/math/typetraits/IsExpression.h>
#include <blaze/math/typetraits/IsLower.h>
#include <blaze/math/typetraits/IsResizable.h>
#include <blaze/math/typetraits/IsRowVector.h>
#include <blaze/math/typetraits/IsSIMDCombinable.h>
#include <blaze/math/typetraits/IsSparseMatrix.h>
#include <blaze/math/typetraits/IsSparseVector.h>
#include <blaze/math/typetraits/IsStrictlyLower.h>
#include <blaze/math/typetraits/IsStrictlyUpper.h>
#include <blaze/math/typetraits/IsTriangular.h>
#include <blaze/math/typetraits/IsUniLower.h>
#include <blaze/math/typetraits/IsUniUpper.h>
#include <blaze/math/typetraits/IsUpper.h>
#include <blaze/math/typetraits/RequiresEvaluation.h>
#include <blaze/math/typetraits/Rows.h>
#include <blaze/system/Optimizations.h>
#include <blaze/system/Thresholds.h>
#include <blaze/util/Assert.h>
#include <blaze/util/constraints/Reference.h>
#include <blaze/util/EnableIf.h>
#include <blaze/util/FunctionTrace.h>
#include <blaze/util/IntegralConstant.h>
#include <blaze/util/InvalidType.h>
#include <blaze/util/mpl/And.h>
#include <blaze/util/mpl/Bool.h>
#include <blaze/util/mpl/If.h>
#include <blaze/util/mpl/Or.h>
#include <blaze/util/Types.h>
#include <blaze/util/typetraits/IsBuiltin.h>
#include <blaze/util/typetraits/RemoveReference.h>


namespace blaze {

//=================================================================================================
//
//  CLASS TDMATTSMATMULTEXPR
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Expression object for transpose dense matrix-transpose sparse matrix multiplications.
// \ingroup dense_matrix_expression
//
// The TDMatTSMatMultExpr class represents the compile time expression for multiplications between
// a column-major dense matrix and a column-major sparse matrix.
*/
template< typename MT1  // Type of the left-hand side dense matrix
        , typename MT2  // Type of the right-hand side sparse matrix
        , bool SF       // Symmetry flag
        , bool HF       // Hermitian flag
        , bool LF       // Lower flag
        , bool UF >     // Upper flag
class TDMatTSMatMultExpr : public DenseMatrix< TDMatTSMatMultExpr<MT1,MT2,SF,HF,LF,UF>, true >
                         , private MatMatMultExpr
                         , private Computation
{
 private:
   //**Type definitions****************************************************************************
   typedef ResultType_<MT1>     RT1;  //!< Result type of the left-hand side dense matrix expression.
   typedef ResultType_<MT2>     RT2;  //!< Result type of the right-hand side sparse matrix expression.
   typedef ElementType_<RT1>    ET1;  //!< Element type of the left-hand side dense matrix expression.
   typedef ElementType_<RT2>    ET2;  //!< Element type of the right-hand side sparse matrix expression.
   typedef CompositeType_<MT1>  CT1;  //!< Composite type of the left-hand side dense matrix expression.
   typedef CompositeType_<MT2>  CT2;  //!< Composite type of the right-hand side sparse matrix expression.
   //**********************************************************************************************

   //**********************************************************************************************
   //! Compilation switch for the composite type of the left-hand side dense matrix expression.
   enum : bool { evaluateLeft = IsComputation<MT1>::value || RequiresEvaluation<MT1>::value };
   //**********************************************************************************************

   //**********************************************************************************************
   //! Compilation switch for the composite type of the right-hand side sparse matrix expression.
   enum : bool { evaluateRight = IsComputation<MT2>::value || RequiresEvaluation<MT2>::value };
   //**********************************************************************************************

   //**********************************************************************************************
   //! Compilation switches for the kernel generation.
   enum : bool {
      SYM  = ( SF && !( HF || LF || UF )    ),  //!< Flag for symmetric matrices.
      HERM = ( HF && !( LF || UF )          ),  //!< Flag for Hermitian matrices.
      LOW  = ( LF || ( ( SF || HF ) && UF ) ),  //!< Flag for lower matrices.
      UPP  = ( UF || ( ( SF || HF ) && LF ) )   //!< Flag for upper matrices.
   };
   //**********************************************************************************************

   //**********************************************************************************************
   /*! \cond BLAZE_INTERNAL */
   //! Helper structure for the explicit application of the SFINAE principle.
   /*! The IsEvaluationRequired struct is a helper struct for the selection of the parallel
       evaluation strategy. In case either of the two matrix operands requires an intermediate
       evaluation, the nested \value will be set to 1, otherwise it will be 0. */
   template< typename T1, typename T2, typename T3 >
   struct IsEvaluationRequired {
      enum : bool { value = ( evaluateLeft || evaluateRight ) };
   };
   /*! \endcond */
   //**********************************************************************************************

   //**********************************************************************************************
   /*! \cond BLAZE_INTERNAL */
   //! Helper structure for the explicit application of the SFINAE principle.
   /*! In case all three involved data types are suited for a vectorized computation of the
       matrix multiplication, the nested \value will be set to 1, otherwise it will be 0. */
   template< typename T1, typename T2, typename T3 >
   struct UseVectorizedKernel {
      enum : bool { value = useOptimizedKernels &&
                            !IsDiagonal<T2>::value &&
                            T1::simdEnabled && T2::simdEnabled &&
                            IsColumnMajorMatrix<T1>::value &&
                            IsSIMDCombinable< ElementType_<T1>
                                            , ElementType_<T2>
                                            , ElementType_<T3> >::value &&
                            HasSIMDAdd< ElementType_<T2>, ElementType_<T3> >::value &&
                            HasSIMDMult< ElementType_<T2>, ElementType_<T3> >::value };
   };
   /*! \endcond */
   //**********************************************************************************************

   //**********************************************************************************************
   /*! \cond BLAZE_INTERNAL */
   //! Helper structure for the explicit application of the SFINAE principle.
   /*! In case a vectorized computation of the matrix multiplication is not possible, but a
       loop-unrolled computation is feasible, the nested \value will be set to 1, otherwise
       it will be 0. */
   template< typename T1, typename T2, typename T3 >
   struct UseOptimizedKernel {
      enum : bool { value = useOptimizedKernels &&
                            !UseVectorizedKernel<T1,T2,T3>::value &&
                            !IsDiagonal<T2>::value &&
                            !IsResizable< ElementType_<T1> >::value &&
                            !IsResizable<ET2>::value };
   };
   /*! \endcond */
   //**********************************************************************************************

   //**********************************************************************************************
   /*! \cond BLAZE_INTERNAL */
   //! Helper structure for the explicit application of the SFINAE principle.
   /*! In case neither a vectorized nor optimized computation is possible, the nested \value will
       be set to 1, otherwise it will be 0. */
   template< typename T1, typename T2, typename T3 >
   struct UseDefaultKernel {
      enum : bool { value = !UseVectorizedKernel<T1,T2,T3>::value &&
                            !UseOptimizedKernel<T1,T2,T3>::value };
   };
   /*! \endcond */
   //**********************************************************************************************

   //**********************************************************************************************
   /*! \cond BLAZE_INTERNAL */
   //! Type of the functor for forwarding an expression to another assign kernel.
   /*! In case a temporary matrix needs to be created, this functor is used to forward the
       resulting expression to another assign kernel. */
   typedef IfTrue_< HERM
                  , DeclHerm
                  , IfTrue_< SYM
                           , DeclSym
                           , IfTrue_< LOW
                                    , IfTrue_< UPP
                                             , DeclDiag
                                             , DeclLow >
                                    , IfTrue_< UPP
                                             , DeclUpp
                                             , Noop > > > >  ForwardFunctor;
   /*! \endcond */
   //**********************************************************************************************

 public:
   //**Type definitions****************************************************************************
   //! Type of this TDMatTSMatMultExpr instance.
   typedef TDMatTSMatMultExpr<MT1,MT2,SF,HF,LF,UF>  This;

   typedef MultTrait_<RT1,RT2>         ResultType;     //!< Result type for expression template evaluations.
   typedef OppositeType_<ResultType>   OppositeType;   //!< Result type with opposite storage order for expression template evaluations.
   typedef TransposeType_<ResultType>  TransposeType;  //!< Transpose type for expression template evaluations.
   typedef ElementType_<ResultType>    ElementType;    //!< Resulting element type.
   typedef SIMDTrait_<ElementType>     SIMDType;       //!< Resulting SIMD element type.
   typedef const ElementType           ReturnType;     //!< Return type for expression template evaluations.
   typedef const ResultType            CompositeType;  //!< Data type for composite expression templates.

   //! Composite type of the left-hand side dense matrix expression.
   typedef If_< IsExpression<MT1>, const MT1, const MT1& >  LeftOperand;

   //! Composite type of the right-hand side sparse matrix expression.
   typedef If_< IsExpression<MT2>, const MT2, const MT2& >  RightOperand;

   //! Type for the assignment of the left-hand side dense matrix operand.
   typedef IfTrue_< evaluateLeft, const RT1, CT1 >  LT;

   //! Type for the assignment of the right-hand side sparse matrix operand.
   typedef IfTrue_< evaluateRight, const RT2, CT2 >  RT;
   //**********************************************************************************************

   //**Compilation flags***************************************************************************
   //! Compilation switch for the expression template evaluation strategy.
   enum : bool { simdEnabled = !IsDiagonal<MT1>::value &&
                               MT1::simdEnabled &&
                               HasSIMDAdd<ET1,ET2>::value &&
                               HasSIMDMult<ET1,ET2>::value };

   //! Compilation switch for the expression template assignment strategy.
   enum : bool { smpAssignable = !evaluateLeft  && MT1::smpAssignable &&
                                 !evaluateRight && MT2::smpAssignable };
   //**********************************************************************************************

   //**SIMD properties*****************************************************************************
   //! The number of elements packed within a single SIMD element.
   enum : size_t { SIMDSIZE = SIMDTrait<ElementType>::size };
   //**********************************************************************************************

   //**Constructor*********************************************************************************
   /*!\brief Constructor for the TDMatTSMatMultExpr class.
   //
   // \param lhs The left-hand side dense matrix operand of the multiplication expression.
   // \param rhs The right-hand side sparse matrix operand of the multiplication expression.
   */
   explicit inline TDMatTSMatMultExpr( const MT1& lhs, const MT2& rhs ) noexcept
      : lhs_( lhs )  // Left-hand side dense matrix of the multiplication expression
      , rhs_( rhs )  // Right-hand side sparse matrix of the multiplication expression
   {
      BLAZE_INTERNAL_ASSERT( lhs.columns() == rhs.rows(), "Invalid matrix sizes" );
   }
   //**********************************************************************************************

   //**Access operator*****************************************************************************
   /*!\brief 2D-access to the matrix elements.
   //
   // \param i Access index for the row. The index has to be in the range \f$[0..M-1]\f$.
   // \param j Access index for the column. The index has to be in the range \f$[0..N-1]\f$.
   // \return The resulting value.
   */
   inline ReturnType operator()( size_t i, size_t j ) const {
      BLAZE_INTERNAL_ASSERT( i < lhs_.rows()   , "Invalid row access index"    );
      BLAZE_INTERNAL_ASSERT( j < rhs_.columns(), "Invalid column access index" );

      if( IsDiagonal<MT1>::value ) {
         return lhs_(i,i) * rhs_(i,j);
      }
      else if( IsDiagonal<MT2>::value ) {
         return lhs_(i,j) * rhs_(j,j);
      }
      else if( IsTriangular<MT1>::value || IsTriangular<MT2>::value ) {
         const size_t begin( ( IsUpper<MT1>::value )
                             ?( ( IsLower<MT2>::value )
                                ?( max( ( IsStrictlyUpper<MT1>::value ? i+1UL : i )
                                      , ( IsStrictlyLower<MT2>::value ? j+1UL : j ) ) )
                                :( IsStrictlyUpper<MT1>::value ? i+1UL : i ) )
                             :( ( IsLower<MT2>::value )
                                ?( IsStrictlyLower<MT2>::value ? j+1UL : j )
                                :( 0UL ) ) );
         const size_t end( ( IsLower<MT1>::value )
                           ?( ( IsUpper<MT2>::value )
                              ?( min( ( IsStrictlyLower<MT1>::value ? i : i+1UL )
                                    , ( IsStrictlyUpper<MT2>::value ? j : j+1UL ) ) )
                              :( IsStrictlyLower<MT1>::value ? i : i+1UL ) )
                           :( ( IsUpper<MT2>::value )
                              ?( IsStrictlyUpper<MT2>::value ? j : j+1UL )
                              :( lhs_.columns() ) ) );

         if( begin >= end ) return ElementType();

         const size_t n( end - begin );

         return subvector( row( lhs_, i ), begin, n ) * subvector( column( rhs_, j ), begin, n );
      }
      else {
         return row( lhs_, i ) * column( rhs_, j );
      }
   }
   //**********************************************************************************************

   //**At function*********************************************************************************
   /*!\brief Checked access to the matrix elements.
   //
   // \param i Access index for the row. The index has to be in the range \f$[0..M-1]\f$.
   // \param j Access index for the column. The index has to be in the range \f$[0..N-1]\f$.
   // \return The resulting value.
   // \exception std::out_of_range Invalid matrix access index.
   */
   inline ReturnType at( size_t i, size_t j ) const {
      if( i >= lhs_.rows() ) {
         BLAZE_THROW_OUT_OF_RANGE( "Invalid row access index" );
      }
      if( j >= rhs_.columns() ) {
         BLAZE_THROW_OUT_OF_RANGE( "Invalid column access index" );
      }
      return (*this)(i,j);
   }
   //**********************************************************************************************

   //**Rows function*******************************************************************************
   /*!\brief Returns the current number of rows of the matrix.
   //
   // \return The number of rows of the matrix.
   */
   inline size_t rows() const noexcept {
      return lhs_.rows();
   }
   //**********************************************************************************************

   //**Columns function****************************************************************************
   /*!\brief Returns the current number of columns of the matrix.
   //
   // \return The number of columns of the matrix.
   */
   inline size_t columns() const noexcept {
      return rhs_.columns();
   }
   //**********************************************************************************************

   //**Left operand access*************************************************************************
   /*!\brief Returns the left-hand side transpose dense matrix operand.
   //
   // \return The left-hand side transpose dense matrix operand.
   */
   inline LeftOperand leftOperand() const noexcept {
      return lhs_;
   }
   //**********************************************************************************************

   //**Right operand access************************************************************************
   /*!\brief Returns the right-hand side transpose sparse matrix operand.
   //
   // \return The right-hand side transpose sparse matrix operand.
   */
   inline RightOperand rightOperand() const noexcept {
      return rhs_;
   }
   //**********************************************************************************************

   //**********************************************************************************************
   /*!\brief Returns whether the expression can alias with the given address \a alias.
   //
   // \param alias The alias to be checked.
   // \return \a true in case the expression can alias, \a false otherwise.
   */
   template< typename T >
   inline bool canAlias( const T* alias ) const noexcept {
      return ( lhs_.isAliased( alias ) || rhs_.isAliased( alias ) );
   }
   //**********************************************************************************************

   //**********************************************************************************************
   /*!\brief Returns whether the expression is aliased with the given address \a alias.
   //
   // \param alias The alias to be checked.
   // \return \a true in case an alias effect is detected, \a false otherwise.
   */
   template< typename T >
   inline bool isAliased( const T* alias ) const noexcept {
      return ( lhs_.isAliased( alias ) || rhs_.isAliased( alias ) );
   }
   //**********************************************************************************************

   //**********************************************************************************************
   /*!\brief Returns whether the operands of the expression are properly aligned in memory.
   //
   // \return \a true in case the operands are aligned, \a false if not.
   */
   inline bool isAligned() const noexcept {
      return lhs_.isAligned();
   }
   //**********************************************************************************************

   //**********************************************************************************************
   /*!\brief Returns whether the expression can be used in SMP assignments.
   //
   // \return \a true in case the expression can be used in SMP assignments, \a false if not.
   */
   inline bool canSMPAssign() const noexcept {
      return ( rows() * columns() >= SMP_TDMATTSMATMULT_THRESHOLD ) && !IsDiagonal<MT1>::value;
   }
   //**********************************************************************************************

 private:
   //**Member variables****************************************************************************
   LeftOperand  lhs_;  //!< Left-hand side dense matrix of the multiplication expression.
   RightOperand rhs_;  //!< Right-hand side sparse matrix of the multiplication expression.
   //**********************************************************************************************

   //**Assignment to dense matrices****************************************************************
   /*! \cond BLAZE_INTERNAL */
   /*!\brief Assignment of a transpose dense matrix-transpose sparse matrix multiplication to a
   //        dense matrix (\f$ A=B*C \f$).
   // \ingroup dense_matrix
   //
   // \param lhs The target left-hand side dense matrix.
   // \param rhs The right-hand side multiplication expression to be assigned.
   // \return void
   //
   // This function implements the performance optimized assignment of a transpose dense matrix-
   // transpose sparse matrix multiplication expression to a dense matrix.
   */
   template< typename MT  // Type of the target dense matrix
           , bool SO >    // Storage order of the target dense matrix
   friend inline void assign( DenseMatrix<MT,SO>& lhs, const TDMatTSMatMultExpr& rhs )
   {
      BLAZE_FUNCTION_TRACE;

      BLAZE_INTERNAL_ASSERT( (~lhs).rows()    == rhs.rows()   , "Invalid number of rows"    );
      BLAZE_INTERNAL_ASSERT( (~lhs).columns() == rhs.columns(), "Invalid number of columns" );

      LT A( serial( rhs.lhs_ ) );  // Evaluation of the left-hand side dense matrix operand
      RT B( serial( rhs.rhs_ ) );  // Evaluation of the right-hand side sparse matrix operand

      BLAZE_INTERNAL_ASSERT( A.rows()    == rhs.lhs_.rows()   , "Invalid number of rows"    );
      BLAZE_INTERNAL_ASSERT( A.columns() == rhs.lhs_.columns(), "Invalid number of columns" );
      BLAZE_INTERNAL_ASSERT( B.rows()    == rhs.rhs_.rows()   , "Invalid number of rows"    );
      BLAZE_INTERNAL_ASSERT( B.columns() == rhs.rhs_.columns(), "Invalid number of columns" );
      BLAZE_INTERNAL_ASSERT( A.rows()    == (~lhs).rows()     , "Invalid number of rows"    );
      BLAZE_INTERNAL_ASSERT( B.columns() == (~lhs).columns()  , "Invalid number of columns" );

      TDMatTSMatMultExpr::selectAssignKernel( ~lhs, A, B );
   }
   /*! \endcond */
   //**********************************************************************************************

   //**Default assignment to dense matrices********************************************************
   /*! \cond BLAZE_INTERNAL */
   /*!\brief Default assignment of a transpose dense matrix-transpose sparse matrix multiplication
   //        to dense matrices (\f$ A=B*C \f$).
   // \ingroup dense_matrix
   //
   // \param C The target left-hand side dense matrix.
   // \param A The left-hand side dense matrix operand.
   // \param B The right-hand side sparse matrix operand.
   // \return void
   //
   // This function implements the default assignment kernel for the transpose dense matrix-
   // transpose sparse matrix multiplication.
   */
   template< typename MT3    // Type of the left-hand side target matrix
           , typename MT4    // Type of the left-hand side matrix operand
           , typename MT5 >  // Type of the right-hand side matrix operand
   static inline EnableIf_< UseDefaultKernel<MT3,MT4,MT5> >
      selectAssignKernel( MT3& C, const MT4& A, const MT5& B )
   {
      typedef ConstIterator_<MT5>  ConstIterator;

      const size_t block( Or< IsColumnMajorMatrix<MT3>, IsDiagonal<MT4> >::value ? A.rows() : 64UL );

      reset( C );

      for( size_t ii=0UL; ii<A.rows(); ii+=block )
      {
         const size_t itmp( min( ii+block, A.rows() ) );

         for( size_t j=0UL; j<B.columns(); ++j )
         {
            ConstIterator element( B.begin(j) );
            const ConstIterator end( B.end(j) );

            for( ; element!=end; ++element )
            {
               const size_t j1( element->index() );

               if( IsDiagonal<MT4>::value )
               {
                  C(j1,j) = A(j1,j1) * element->value();
               }
               else
               {
                  const size_t ibegin( ( IsLower<MT4>::value )
                                       ?( ( LOW )
                                          ?( max( j, ii, ( IsStrictlyLower<MT4>::value ? j1+1UL : j1 ) ) )
                                          :( max( ii, ( IsStrictlyLower<MT4>::value ? j1+1UL : j1 ) ) ) )
                                       :( LOW ? max(j,ii) : ii ) );
                  const size_t iend( ( IsUpper<MT4>::value )
                                     ?( ( SYM || HERM || UPP )
                                        ?( min( j+1UL, itmp, ( IsStrictlyUpper<MT4>::value ? j1 : j1+1UL ) ) )
                                        :( min( itmp, ( IsStrictlyUpper<MT4>::value ? j1 : j1+1UL ) ) ) )
                                     :( SYM || HERM || UPP ? min(j+1UL,itmp) : itmp ) );

                  if( ( SYM || HERM || LOW || UPP || IsTriangular<MT4>::value ) && ( ibegin >= iend ) )
                     continue;

                  BLAZE_INTERNAL_ASSERT( ibegin <= iend, "Invalid loop indices detected" );

                  for( size_t i=ibegin; i<iend; ++i ) {
                     if( isDefault( C(i,j) ) )
                        C(i,j) = A(i,j1) * element->value();
                     else
                        C(i,j) += A(i,j1) * element->value();
                  }
               }
            }
         }
      }

      if( SYM || HERM ) {
         for( size_t j=0UL; j<B.columns(); ++j ) {
            for( size_t i=j+1UL; i<A.rows(); ++i ) {
               C(i,j) = HERM ? conj( C(j,i) ) : C(j,i);
            }
         }
      }
   }
   /*! \endcond */
   //**********************************************************************************************

   //**Optimized assignment to dense matrices******************************************************
   /*! \cond BLAZE_INTERNAL */
   /*!\brief Optimized assignment of a transpose dense matrix-transpose sparse matrix multiplication
   //        to dense matrices (\f$ A=B*C \f$).
   // \ingroup dense_matrix
   //
   // \param C The target left-hand side dense matrix.
   // \param A The left-hand side dense matrix operand.
   // \param B The right-hand side sparse matrix operand.
   // \return void
   //
   // This function implements the optimized assignment kernel for the transpose dense matrix-
   // transpose sparse matrix multiplication.
   */
   template< typename MT3    // Type of the left-hand side target matrix
           , typename MT4    // Type of the left-hand side matrix operand
           , typename MT5 >  // Type of the right-hand side matrix operand
   static inline EnableIf_< UseOptimizedKernel<MT3,MT4,MT5> >
      selectAssignKernel( MT3& C, const MT4& A, const MT5& B )
   {
      typedef ConstIterator_<MT5>  ConstIterator;

      const size_t block( IsColumnMajorMatrix<MT3>::value ? A.rows() : 64UL );

      reset( C );

      for( size_t ii=0UL; ii<A.rows(); ii+=block )
      {
         const size_t itmp( min( ii+block, A.rows() ) );

         for( size_t j=0UL; j<B.columns(); ++j )
         {
            const ConstIterator end( B.end(j) );
            ConstIterator element( B.begin(j) );

            const size_t nonzeros( B.nonZeros(j) );
            const size_t kpos( nonzeros & size_t(-4) );
            BLAZE_INTERNAL_ASSERT( ( nonzeros - ( nonzeros % 4UL ) ) == kpos, "Invalid end calculation" );

            for( size_t k=0UL; k<kpos; k+=4UL )
            {
               const size_t j1( element->index() );
               const ET2    v1( element->value() );
               ++element;
               const size_t j2( element->index() );
               const ET2    v2( element->value() );
               ++element;
               const size_t j3( element->index() );
               const ET2    v3( element->value() );
               ++element;
               const size_t j4( element->index() );
               const ET2    v4( element->value() );
               ++element;

               BLAZE_INTERNAL_ASSERT( j1 < j2 && j2 < j3 && j3 < j4, "Invalid sparse matrix index detected" );

               const size_t ibegin( ( IsLower<MT4>::value )
                                    ?( ( LOW )
                                       ?( max( j, ii, ( IsStrictlyLower<MT4>::value ? j1+1UL : j1 ) ) )
                                       :( max( ii, ( IsStrictlyLower<MT4>::value ? j1+1UL : j1 ) ) ) )
                                    :( LOW ? max(j,ii) : ii ) );
               const size_t iend( ( IsUpper<MT4>::value )
                                  ?( ( SYM || HERM || UPP )
                                     ?( min( j+1UL, itmp, ( IsStrictlyUpper<MT4>::value ? j4 : j4+1UL ) ) )
                                     :( min( itmp, ( IsStrictlyUpper<MT4>::value ? j4 : j4+1UL ) ) ) )
                                  :( SYM || HERM || UPP ? min(j+1UL,itmp) : itmp ) );

               if( ( SYM || HERM || LOW || UPP || IsTriangular<MT4>::value ) && ( ibegin >= iend ) )
                  continue;

               BLAZE_INTERNAL_ASSERT( ibegin <= iend, "Invalid loop indices detected" );

               const size_t inum( iend - ibegin );
               const size_t ipos( ibegin + ( inum & size_t(-4) ) );
               BLAZE_INTERNAL_ASSERT( ( ibegin + inum - ( inum % 4UL ) ) == ipos, "Invalid end calculation" );

               for( size_t i=ibegin; i<ipos; i+=4UL ) {
                  C(i    ,j) += A(i    ,j1) * v1 + A(i    ,j2) * v2 + A(i    ,j3) * v3 + A(i    ,j4) * v4;
                  C(i+1UL,j) += A(i+1UL,j1) * v1 + A(i+1UL,j2) * v2 + A(i+1UL,j3) * v3 + A(i+1UL,j4) * v4;
                  C(i+2UL,j) += A(i+2UL,j1) * v1 + A(i+2UL,j2) * v2 + A(i+2UL,j3) * v3 + A(i+2UL,j4) * v4;
                  C(i+3UL,j) += A(i+3UL,j1) * v1 + A(i+3UL,j2) * v2 + A(i+3UL,j3) * v3 + A(i+3UL,j4) * v4;
               }
               for( size_t i=ipos; i<iend; ++i ) {
                  C(i,j) += A(i,j1) * v1 + A(i,j2) * v2 + A(i,j3) * v3 + A(i,j4) * v4;
               }
            }

            for( ; element!=end; ++element )
            {
               const size_t j1( element->index() );
               const ET2    v1( element->value() );

               const size_t ibegin( ( IsLower<MT4>::value )
                                    ?( ( LOW )
                                       ?( max( j, ii, ( IsStrictlyLower<MT4>::value ? j1+1UL : j1 ) ) )
                                       :( max( ii, ( IsStrictlyLower<MT4>::value ? j1+1UL : j1 ) ) ) )
                                    :( LOW ? max(j,ii) : ii ) );
               const size_t iend( ( IsUpper<MT4>::value )
                                  ?( ( SYM || HERM || UPP )
                                     ?( min( j+1UL, itmp, ( IsStrictlyUpper<MT4>::value ? j1 : j1+1UL ) ) )
                                     :( min( itmp, ( IsStrictlyUpper<MT4>::value ? j1 : j1+1UL ) ) ) )
                                  :( SYM || HERM || UPP ? min(j+1UL,itmp) : itmp ) );

               if( ( SYM || HERM || LOW || UPP || IsTriangular<MT4>::value ) && ( ibegin >= iend ) )
                  continue;

               BLAZE_INTERNAL_ASSERT( ibegin <= iend, "Invalid loop indices detected" );

               const size_t inum( iend - ibegin );
               const size_t ipos( ibegin + ( inum & size_t(-4) ) );
               BLAZE_INTERNAL_ASSERT( ( ibegin + inum - ( inum % 4UL ) ) == ipos, "Invalid end calculation" );

               for( size_t i=ibegin; i<ipos; i+=4UL ) {
                  C(i    ,j) += A(i    ,j1) * v1;
                  C(i+1UL,j) += A(i+1UL,j1) * v1;
                  C(i+2UL,j) += A(i+2UL,j1) * v1;
                  C(i+3UL,j) += A(i+3UL,j1) * v1;
               }
               for( size_t i=ipos; i<iend; ++i ) {
                  C(i,j) += A(i,j1) * v1;
               }
            }
         }
      }

      if( SYM || HERM ) {
         for( size_t j=0UL; j<B.columns(); ++j ) {
            for( size_t i=j+1UL; i<A.rows(); ++i ) {
               C(i,j) = HERM ? conj( C(j,i) ) : C(j,i);
            }
         }
      }
   }
   /*! \endcond */
   //**********************************************************************************************

   //**Vectorized assignment to column-major dense matrices****************************************
   /*! \cond BLAZE_INTERNAL */
   /*!\brief Vectorized assignment of a transpose dense matrix-transpose sparse matrix
   //        multiplication to column-major dense matrices (\f$ A=B*C \f$).
   // \ingroup dense_matrix
   //
   // \param C The target left-hand side dense matrix.
   // \param A The left-hand side dense matrix operand.
   // \param B The right-hand side sparse matrix operand.
   // \return void
   //
   // This function implements the vectorized column-major assignment kernel for the transpose dense
   // matrix-transpose sparse matrix multiplication.
   */
   template< typename MT3    // Type of the left-hand side target matrix
           , typename MT4    // Type of the left-hand side matrix operand
           , typename MT5 >  // Type of the right-hand side matrix operand
   static inline EnableIf_< UseVectorizedKernel<MT3,MT4,MT5> >
      selectAssignKernel( MT3& C, const MT4& A, const MT5& B )
   {
      typedef ConstIterator_<MT5>  ConstIterator;

      constexpr bool remainder( !IsPadded<MT3>::value || !IsPadded<MT4>::value );

      reset( C );

      for( size_t j=0UL; j<B.columns(); ++j )
      {
         const ConstIterator end( B.end(j) );
         ConstIterator element( B.begin(j) );

         const size_t nonzeros( B.nonZeros(j) );
         const size_t kpos( nonzeros & size_t(-4) );
         BLAZE_INTERNAL_ASSERT( ( nonzeros - ( nonzeros % 4UL ) ) == kpos, "Invalid end calculation" );

         for( size_t k=0UL; k<kpos; k+=4UL )
         {
            const size_t j1( element->index() );
            const ET2    v1( element->value() );
            ++element;
            const size_t j2( element->index() );
            const ET2    v2( element->value() );
            ++element;
            const size_t j3( element->index() );
            const ET2    v3( element->value() );
            ++element;
            const size_t j4( element->index() );
            const ET2    v4( element->value() );
            ++element;

            BLAZE_INTERNAL_ASSERT( j1 < j2 && j2 < j3 && j3 < j4, "Invalid sparse matrix index detected" );

            const SIMDType xmm1( set( v1 ) );
            const SIMDType xmm2( set( v2 ) );
            const SIMDType xmm3( set( v3 ) );
            const SIMDType xmm4( set( v4 ) );

            const size_t ibegin( ( IsLower<MT4>::value )
                                 ?( ( IsStrictlyLower<MT4>::value )
                                    ?( ( LOW ? max(j,j1+1UL) : j1+1UL ) & size_t(-SIMDSIZE) )
                                    :( ( LOW ? max(j,j1) : j1 ) & size_t(-SIMDSIZE) ) )
                                 :( LOW ? ( j & size_t(-SIMDSIZE) ) : 0UL ) );
            const size_t iend( ( IsUpper<MT4>::value )
                               ?( ( IsStrictlyUpper<MT4>::value )
                                  ?( SYM || HERM || UPP ? max(j+1UL,j4) : j4 )
                                  :( SYM || HERM || UPP ? max(j,j4)+1UL : j4+1UL ) )
                               :( SYM || HERM || UPP ? j+1UL : A.rows() ) );
            BLAZE_INTERNAL_ASSERT( ibegin <= iend, "Invalid loop indices detected" );

            const size_t ipos( remainder ? ( iend & size_t(-SIMDSIZE) ) : iend );
            BLAZE_INTERNAL_ASSERT( !remainder || ( iend - ( iend % (SIMDSIZE) ) ) == ipos, "Invalid end calculation" );

            size_t i( ibegin );

            for( ; i<ipos; i+=SIMDSIZE ) {
               C.store( i, j, C.load(i,j) + A.load(i,j1) * xmm1 + A.load(i,j2) * xmm2 + A.load(i,j3) * xmm3 + A.load(i,j4) * xmm4 );
            }
            for( ; remainder && i<iend; ++i ) {
               C(i,j) += A(i,j1) * v1 + A(i,j2) * v2 + A(i,j3) * v3 + A(i,j4) * v4;
            }
         }

         for( ; element!=end; ++element )
         {
            const size_t j1( element->index() );
            const ET2    v1( element->value() );

            const SIMDType xmm1( set( v1 ) );

            const size_t ibegin( ( IsLower<MT4>::value )
                                 ?( ( IsStrictlyLower<MT4>::value )
                                    ?( ( LOW ? max(j,j1+1UL) : j1+1UL ) & size_t(-SIMDSIZE) )
                                    :( ( LOW ? max(j,j1) : j1 ) & size_t(-SIMDSIZE) ) )
                                 :( LOW ? ( j & size_t(-SIMDSIZE) ) : 0UL ) );
            const size_t iend( ( IsUpper<MT4>::value )
                               ?( ( IsStrictlyUpper<MT4>::value )
                                  ?( SYM || HERM || UPP ? max(j+1UL,j1) : j1 )
                                  :( SYM || HERM || UPP ? max(j,j1)+1UL : j1+1UL ) )
                               :( SYM || HERM || UPP ? j+1UL : A.rows() ) );
            BLAZE_INTERNAL_ASSERT( ibegin <= iend, "Invalid loop indices detected" );

            const size_t ipos( remainder ? ( iend & size_t(-SIMDSIZE) ) : iend );
            BLAZE_INTERNAL_ASSERT( !remainder || ( iend - ( iend % (SIMDSIZE) ) ) == ipos, "Invalid end calculation" );

            size_t i( ibegin );

            for( ; i<ipos; i+=SIMDSIZE ) {
               C.store( i, j, C.load(i,j) + A.load(i,j1) * xmm1 );
            }
            for( ; remainder && i<iend; ++i ) {
               C(i,j) += A(i,j1) * v1;
            }
         }
      }

      if( SYM || HERM ) {
         for( size_t j=0UL; j<B.columns(); ++j ) {
            for( size_t i=j+1UL; i<A.rows(); ++i ) {
               C(i,j) = HERM ? conj( C(j,i) ) : C(j,i);
            }
         }
      }
   }
   /*! \endcond */
   //**********************************************************************************************

   //**Assignment to sparse matrices***************************************************************
   /*! \cond BLAZE_INTERNAL */
   /*!\brief Assignment of a transpose dense matrix-transpose sparse matrix multiplication to a
   //        sparse matrix (\f$ A=B*C \f$).
   // \ingroup dense_matrix
   //
   // \param lhs The target left-hand side sparse matrix.
   // \param rhs The right-hand side multiplication expression to be assigned.
   // \return void
   //
   // This function implements the performance optimized assignment of a transpose dense matrix-
   // transpose sparse matrix multiplication expression to a sparse matrix.
   */
   template< typename MT  // Type of the target sparse matrix
           , bool SO >    // Storage order of the target sparse matrix
   friend inline void assign( SparseMatrix<MT,SO>& lhs, const TDMatTSMatMultExpr& rhs )
   {
      BLAZE_FUNCTION_TRACE;

      typedef IfTrue_< SO, ResultType, OppositeType >  TmpType;

      BLAZE_CONSTRAINT_MUST_BE_DENSE_MATRIX_TYPE( ResultType );
      BLAZE_CONSTRAINT_MUST_BE_DENSE_MATRIX_TYPE( OppositeType );
      BLAZE_CONSTRAINT_MUST_BE_COLUMN_MAJOR_MATRIX_TYPE( ResultType );
      BLAZE_CONSTRAINT_MUST_BE_ROW_MAJOR_MATRIX_TYPE( OppositeType );
      BLAZE_CONSTRAINT_MATRICES_MUST_HAVE_SAME_STORAGE_ORDER( MT, TmpType );
      BLAZE_CONSTRAINT_MUST_BE_REFERENCE_TYPE( CompositeType_<TmpType> );

      BLAZE_INTERNAL_ASSERT( (~lhs).rows()    == rhs.rows()   , "Invalid number of rows"    );
      BLAZE_INTERNAL_ASSERT( (~lhs).columns() == rhs.columns(), "Invalid number of columns" );

      const ForwardFunctor fwd;

      const TmpType tmp( serial( rhs ) );
      assign( ~lhs, fwd( tmp ) );
   }
   /*! \endcond */
   //**********************************************************************************************

   //**Addition assignment to dense matrices*******************************************************
   /*! \cond BLAZE_INTERNAL */
   /*!\brief Addition assignment of a transpose dense matrix-transpose sparse matrix multiplication
   //        to a dense matrix (\f$ A+=B*C \f$).
   // \ingroup dense_matrix
   //
   // \param lhs The target left-hand side dense matrix.
   // \param rhs The right-hand side multiplication expression to be added.
   // \return void
   //
   // This function implements the performance optimized addition assignment of a transpose dense
   // matrix-transpose sparse matrix multiplication expression to a dense matrix.
   */
   template< typename MT  // Type of the target dense matrix
           , bool SO >    // Storage order of the target dense matrix
   friend inline void addAssign( DenseMatrix<MT,SO>& lhs, const TDMatTSMatMultExpr& rhs )
   {
      BLAZE_FUNCTION_TRACE;

      BLAZE_INTERNAL_ASSERT( (~lhs).rows()    == rhs.rows()   , "Invalid number of rows"    );
      BLAZE_INTERNAL_ASSERT( (~lhs).columns() == rhs.columns(), "Invalid number of columns" );

      LT A( serial( rhs.lhs_ ) );  // Evaluation of the left-hand side dense matrix operand
      RT B( serial( rhs.rhs_ ) );  // Evaluation of the right-hand side sparse matrix operand

      BLAZE_INTERNAL_ASSERT( A.rows()    == rhs.lhs_.rows()   , "Invalid number of rows"    );
      BLAZE_INTERNAL_ASSERT( A.columns() == rhs.lhs_.columns(), "Invalid number of columns" );
      BLAZE_INTERNAL_ASSERT( B.rows()    == rhs.rhs_.rows()   , "Invalid number of rows"    );
      BLAZE_INTERNAL_ASSERT( B.columns() == rhs.rhs_.columns(), "Invalid number of columns" );
      BLAZE_INTERNAL_ASSERT( A.rows()    == (~lhs).rows()     , "Invalid number of rows"    );
      BLAZE_INTERNAL_ASSERT( B.columns() == (~lhs).columns()  , "Invalid number of columns" );

      TDMatTSMatMultExpr::selectAddAssignKernel( ~lhs, A, B );
   }
   /*! \endcond */
   //**********************************************************************************************

   //**Default addition assignment to dense matrices***********************************************
   /*! \cond BLAZE_INTERNAL */
   /*!\brief Default addition assignment of a transpose dense matrix-transpose sparse matrix
   //        multiplication to dense matrices (\f$ A+=B*C \f$).
   // \ingroup dense_matrix
   //
   // \param C The target left-hand side dense matrix.
   // \param A The left-hand side dense matrix operand.
   // \param B The right-hand side sparse matrix operand.
   // \return void
   //
   // This function implements the default addition assignment kernel for the transpose dense
   // matrix-transpose sparse matrix multiplication to a dense matrix.
   */
   template< typename MT3    // Type of the left-hand side target matrix
           , typename MT4    // Type of the left-hand side matrix operand
           , typename MT5 >  // Type of the right-hand side matrix operand
   static inline EnableIf_< UseDefaultKernel<MT3,MT4,MT5> >
      selectAddAssignKernel( MT3& C, const MT4& A, const MT5& B )
   {
      typedef ConstIterator_<MT5>  ConstIterator;

      const size_t block( Or< IsColumnMajorMatrix<MT3>, IsDiagonal<MT4> >::value ? A.rows() : 64UL );

      for( size_t ii=0UL; ii<A.rows(); ii+=block )
      {
         const size_t itmp( min( ii+block, A.rows() ) );

         for( size_t j=0UL; j<B.columns(); ++j )
         {
            ConstIterator element( B.begin(j) );
            const ConstIterator end( B.end(j) );

            for( ; element!=end; ++element )
            {
               const size_t j1( element->index() );

               if( IsDiagonal<MT4>::value )
               {
                  C(j1,j) += A(j1,j1) * element->value();
               }
               else
               {
                  const size_t ibegin( ( IsLower<MT4>::value )
                                       ?( ( LOW )
                                          ?( max( j, ii, ( IsStrictlyLower<MT4>::value ? j1+1UL : j1 ) ) )
                                          :( max( ii, ( IsStrictlyLower<MT4>::value ? j1+1UL : j1 ) ) ) )
                                       :( LOW ? max(j,ii) : ii ) );
                  const size_t iend( ( IsUpper<MT4>::value )
                                     ?( ( UPP )
                                        ?( min( j+1UL, itmp, ( IsStrictlyUpper<MT4>::value ? j1 : j1+1UL ) ) )
                                        :( min( itmp, ( IsStrictlyUpper<MT4>::value ? j1 : j1+1UL ) ) ) )
                                     :( UPP ? min(j+1UL,itmp) : itmp ) );

                  if( ( LOW || UPP || IsTriangular<MT4>::value ) && ( ibegin >= iend ) )
                     continue;

                  BLAZE_INTERNAL_ASSERT( ibegin <= iend, "Invalid loop indices detected" );

                  const size_t inum( iend - ibegin );
                  const size_t ipos( ibegin + ( inum & size_t(-4) ) );
                  BLAZE_INTERNAL_ASSERT( ( ibegin + inum - ( inum % 4UL ) ) == ipos, "Invalid end calculation" );

                  for( size_t i=ibegin; i<ipos; i+=4UL ) {
                     C(i    ,j) += A(i    ,j1) * element->value();
                     C(i+1UL,j) += A(i+1UL,j1) * element->value();
                     C(i+2UL,j) += A(i+2UL,j1) * element->value();
                     C(i+3UL,j) += A(i+3UL,j1) * element->value();
                  }
                  for( size_t i=ipos; i<iend; ++i ) {
                     C(i,j) += A(i,j1) * element->value();
                  }
               }
            }
         }
      }
   }
   /*! \endcond */
   //**********************************************************************************************

   //**Optimized addition assignment to dense matrices*********************************************
   /*! \cond BLAZE_INTERNAL */
   /*!\brief Optimized addition assignment of a transpose dense matrix-transpose sparse matrix
   //        multiplication to dense matrices (\f$ A+=B*C \f$).
   // \ingroup dense_matrix
   //
   // \param C The target left-hand side dense matrix.
   // \param A The left-hand side dense matrix operand.
   // \param B The right-hand side sparse matrix operand.
   // \return void
   //
   // This function implements the optimized addition assignment kernel for the transpose dense
   // matrix-transpose sparse matrix multiplication to a dense matrix.
   */
   template< typename MT3    // Type of the left-hand side target matrix
           , typename MT4    // Type of the left-hand side matrix operand
           , typename MT5 >  // Type of the right-hand side matrix operand
   static inline EnableIf_< UseOptimizedKernel<MT3,MT4,MT5> >
      selectAddAssignKernel( MT3& C, const MT4& A, const MT5& B )
   {
      typedef ConstIterator_<MT5>  ConstIterator;

      const size_t block( IsColumnMajorMatrix<MT3>::value ? A.rows() : 64UL );

      for( size_t ii=0UL; ii<A.rows(); ii+=block )
      {
         const size_t itmp( min( ii+block, A.rows() ) );

         for( size_t j=0UL; j<B.columns(); ++j )
         {
            const ConstIterator end( B.end(j) );
            ConstIterator element( B.begin(j) );

            const size_t nonzeros( B.nonZeros(j) );
            const size_t kpos( nonzeros & size_t(-4) );
            BLAZE_INTERNAL_ASSERT( ( nonzeros - ( nonzeros % 4UL ) ) == kpos, "Invalid end calculation" );

            for( size_t k=0UL; k<kpos; k+=4UL )
            {
               const size_t j1( element->index() );
               const ET2    v1( element->value() );
               ++element;
               const size_t j2( element->index() );
               const ET2    v2( element->value() );
               ++element;
               const size_t j3( element->index() );
               const ET2    v3( element->value() );
               ++element;
               const size_t j4( element->index() );
               const ET2    v4( element->value() );
               ++element;

               BLAZE_INTERNAL_ASSERT( j1 < j2 && j2 < j3 && j3 < j4, "Invalid sparse matrix index detected" );

               const size_t ibegin( ( IsLower<MT4>::value )
                                    ?( ( LOW )
                                       ?( max( j, ii, ( IsStrictlyLower<MT4>::value ? j1+1UL : j1 ) ) )
                                       :( max( ii, ( IsStrictlyLower<MT4>::value ? j1+1UL : j1 ) ) ) )
                                    :( LOW ? max(j,ii) : ii ) );
               const size_t iend( ( IsUpper<MT4>::value )
                                  ?( ( UPP )
                                     ?( min( j+1UL, itmp, ( IsStrictlyUpper<MT4>::value ? j4 : j4+1UL ) ) )
                                     :( min( itmp, ( IsStrictlyUpper<MT4>::value ? j4 : j4+1UL ) ) ) )
                                  :( UPP ? min(j+1UL,itmp) : itmp ) );

               if( ( LOW || UPP || IsTriangular<MT4>::value ) && ( ibegin >= iend ) )
                  continue;

               BLAZE_INTERNAL_ASSERT( ibegin <= iend, "Invalid loop indices detected" );

               const size_t inum( iend - ibegin );
               const size_t ipos( ibegin + ( inum & size_t(-4) ) );
               BLAZE_INTERNAL_ASSERT( ( ibegin + inum - ( inum % 4UL ) ) == ipos, "Invalid end calculation" );

               for( size_t i=ibegin; i<ipos; i+=4UL ) {
                  C(i    ,j) += A(i    ,j1) * v1 + A(i    ,j2) * v2 + A(i    ,j3) * v3 + A(i    ,j4) * v4;
                  C(i+1UL,j) += A(i+1UL,j1) * v1 + A(i+1UL,j2) * v2 + A(i+1UL,j3) * v3 + A(i+1UL,j4) * v4;
                  C(i+2UL,j) += A(i+2UL,j1) * v1 + A(i+2UL,j2) * v2 + A(i+2UL,j3) * v3 + A(i+2UL,j4) * v4;
                  C(i+3UL,j) += A(i+3UL,j1) * v1 + A(i+3UL,j2) * v2 + A(i+3UL,j3) * v3 + A(i+3UL,j4) * v4;
               }
               for( size_t i=ipos; i<iend; ++i ) {
                  C(i,j) += A(i,j1) * v1 + A(i,j2) * v2 + A(i,j3) * v3 + A(i,j4) * v4;
               }
            }

            for( ; element!=end; ++element )
            {
               const size_t j1( element->index() );
               const ET2    v1( element->value() );

               const size_t ibegin( ( IsLower<MT4>::value )
                                    ?( ( LOW )
                                       ?( max( j, ii, ( IsStrictlyLower<MT4>::value ? j1+1UL : j1 ) ) )
                                       :( max( ii, ( IsStrictlyLower<MT4>::value ? j1+1UL : j1 ) ) ) )
                                    :( LOW ? max(j,ii) : ii ) );
               const size_t iend( ( IsUpper<MT4>::value )
                                  ?( ( UPP )
                                     ?( min( j+1UL, itmp, ( IsStrictlyUpper<MT4>::value ? j1 : j1+1UL ) ) )
                                     :( min( itmp, ( IsStrictlyUpper<MT4>::value ? j1 : j1+1UL ) ) ) )
                                  :( UPP ? min(j+1UL,itmp) : itmp ) );

               if( ( LOW || UPP || IsTriangular<MT4>::value ) && ( ibegin >= iend ) )
                  continue;

               BLAZE_INTERNAL_ASSERT( ibegin <= iend, "Invalid loop indices detected" );

               const size_t inum( iend - ibegin );
               const size_t ipos( ibegin + ( inum & size_t(-4) ) );
               BLAZE_INTERNAL_ASSERT( ( ibegin + inum - ( inum % 4UL ) ) == ipos, "Invalid end calculation" );

               for( size_t i=ibegin; i<ipos; i+=4UL ) {
                  C(i    ,j) += A(i    ,j1) * v1;
                  C(i+1UL,j) += A(i+1UL,j1) * v1;
                  C(i+2UL,j) += A(i+2UL,j1) * v1;
                  C(i+3UL,j) += A(i+3UL,j1) * v1;
               }
               for( size_t i=ipos; i<iend; ++i ) {
                  C(i,j) += A(i,j1) * v1;
               }
            }
         }
      }
   }
   /*! \endcond */
   //**********************************************************************************************

   //**Vectorized addition assignment to column-major dense matrices*******************************
   /*! \cond BLAZE_INTERNAL */
   /*!\brief Vectorized addition assignment of a transpose dense matrix-transpose sparse matrix
   //        multiplication to column-major dense matrices (\f$ A+=B*C \f$).
   // \ingroup dense_matrix
   //
   // \param C The target left-hand side dense matrix.
   // \param A The left-hand side dense matrix operand.
   // \param B The right-hand side sparse matrix operand.
   // \return void
   //
   // This function implements the vectorized addition assignment kernel for the transpose dense
   // matrix-transpose sparse matrix multiplication to a column-major dense matrix.
   */
   template< typename MT3    // Type of the left-hand side target matrix
           , typename MT4    // Type of the left-hand side matrix operand
           , typename MT5 >  // Type of the right-hand side matrix operand
   static inline EnableIf_< UseVectorizedKernel<MT3,MT4,MT5> >
      selectAddAssignKernel( MT3& C, const MT4& A, const MT5& B )
   {
      typedef ConstIterator_<MT5>  ConstIterator;

      constexpr bool remainder( !IsPadded<MT3>::value || !IsPadded<MT4>::value );

      for( size_t j=0UL; j<B.columns(); ++j )
      {
         const ConstIterator end( B.end(j) );
         ConstIterator element( B.begin(j) );

         const size_t nonzeros( B.nonZeros(j) );
         const size_t kpos( nonzeros & size_t(-4) );
         BLAZE_INTERNAL_ASSERT( ( nonzeros - ( nonzeros % 4UL ) ) == kpos, "Invalid end calculation" );

         for( size_t k=0UL; k<kpos; k+=4UL )
         {
            const size_t j1( element->index() );
            const ET2    v1( element->value() );
            ++element;
            const size_t j2( element->index() );
            const ET2    v2( element->value() );
            ++element;
            const size_t j3( element->index() );
            const ET2    v3( element->value() );
            ++element;
            const size_t j4( element->index() );
            const ET2    v4( element->value() );
            ++element;

            BLAZE_INTERNAL_ASSERT( j1 < j2 && j2 < j3 && j3 < j4, "Invalid sparse matrix index detected" );

            const SIMDType xmm1( set( v1 ) );
            const SIMDType xmm2( set( v2 ) );
            const SIMDType xmm3( set( v3 ) );
            const SIMDType xmm4( set( v4 ) );

            const size_t ibegin( ( IsLower<MT4>::value )
                                 ?( ( IsStrictlyLower<MT4>::value )
                                    ?( ( LOW ? max(j,j1+1UL) : j1+1UL ) & size_t(-SIMDSIZE) )
                                    :( ( LOW ? max(j,j1) : j1 ) & size_t(-SIMDSIZE) ) )
                                 :( LOW ? ( j & size_t(-SIMDSIZE) ) : 0UL ) );
            const size_t iend( ( IsUpper<MT4>::value )
                               ?( ( IsStrictlyUpper<MT4>::value )
                                  ?( UPP ? max(j+1UL,j4) : j4 )
                                  :( UPP ? max(j,j4)+1UL : j4+1UL ) )
                               :( UPP ? j+1UL : A.rows() ) );
            BLAZE_INTERNAL_ASSERT( ibegin <= iend, "Invalid loop indices detected" );

            const size_t ipos( remainder ? ( iend & size_t(-SIMDSIZE) ) : iend );
            BLAZE_INTERNAL_ASSERT( !remainder || ( iend - ( iend % (SIMDSIZE) ) ) == ipos, "Invalid end calculation" );

            size_t i( ibegin );

            for( ; i<ipos; i+=SIMDSIZE ) {
               C.store( i, j, C.load(i,j) + A.load(i,j1) * xmm1 + A.load(i,j2) * xmm2 + A.load(i,j3) * xmm3 + A.load(i,j4) * xmm4 );
            }
            for( ; remainder && i<iend; ++i ) {
               C(i,j) += A(i,j1) * v1 + A(i,j2) * v2 + A(i,j3) * v3 + A(i,j4) * v4;
            }
         }

         for( ; element!=end; ++element )
         {
            const size_t j1( element->index() );
            const ET2    v1( element->value() );

            const SIMDType xmm1( set( v1 ) );

            const size_t ibegin( ( IsLower<MT4>::value )
                                 ?( ( IsStrictlyLower<MT4>::value )
                                    ?( ( LOW ? max(j,j1+1UL) : j1+1UL ) & size_t(-SIMDSIZE) )
                                    :( ( LOW ? max(j,j1) : j1 ) & size_t(-SIMDSIZE) ) )
                                 :( LOW ? ( j & size_t(-SIMDSIZE) ) : 0UL ) );
            const size_t iend( ( IsUpper<MT4>::value )
                               ?( ( IsStrictlyUpper<MT4>::value )
                                  ?( UPP ? max(j+1UL,j1) : j1 )
                                  :( UPP ? max(j,j1)+1UL : j1+1UL ) )
                               :( UPP ? j+1UL : A.rows() ) );
            BLAZE_INTERNAL_ASSERT( ibegin <= iend, "Invalid loop indices detected" );

            const size_t ipos( remainder ? ( iend & size_t(-SIMDSIZE) ) : iend );
            BLAZE_INTERNAL_ASSERT( !remainder || ( iend - ( iend % (SIMDSIZE) ) ) == ipos, "Invalid end calculation" );

            size_t i( ibegin );

            for( ; i<ipos; i+=SIMDSIZE ) {
               C.store( i, j, C.load(i,j) + A.load(i,j1) * xmm1 );
            }
            for( ; remainder && i<iend; ++i ) {
               C(i,j) += A(i,j1) * v1;
            }
         }
      }
   }
   /*! \endcond */
   //**********************************************************************************************

   //**Addition assignment to sparse matrices******************************************************
   // No special implementation for the addition assignment to sparse matrices.
   //**********************************************************************************************

   //**Subtraction assignment to dense matrices****************************************************
   /*! \cond BLAZE_INTERNAL */
   /*!\brief Subtraction assignment of a transpose dense matrix-transpose sparse matrix
   //        multiplication to a dense matrix (\f$ A-=B*C \f$).
   // \ingroup dense_matrix
   //
   // \param lhs The target left-hand side dense matrix.
   // \param rhs The right-hand side multiplication expression to be subtracted.
   // \return void
   //
   // This function implements the performance optimized subtraction assignment of a transpose
   // dense matrix-transpose sparse matrix multiplication expression to a dense matrix.
   */
   template< typename MT  // Type of the target dense matrix
           , bool SO >    // Storage order of the target dense matrix
   friend inline void subAssign( DenseMatrix<MT,SO>& lhs, const TDMatTSMatMultExpr& rhs )
   {
      BLAZE_FUNCTION_TRACE;

      BLAZE_INTERNAL_ASSERT( (~lhs).rows()    == rhs.rows()   , "Invalid number of rows"    );
      BLAZE_INTERNAL_ASSERT( (~lhs).columns() == rhs.columns(), "Invalid number of columns" );

      LT A( serial( rhs.lhs_ ) );  // Evaluation of the left-hand side dense matrix operand
      RT B( serial( rhs.rhs_ ) );  // Evaluation of the right-hand side sparse matrix operand

      BLAZE_INTERNAL_ASSERT( A.rows()    == rhs.lhs_.rows()   , "Invalid number of rows"    );
      BLAZE_INTERNAL_ASSERT( A.columns() == rhs.lhs_.columns(), "Invalid number of columns" );
      BLAZE_INTERNAL_ASSERT( B.rows()    == rhs.rhs_.rows()   , "Invalid number of rows"    );
      BLAZE_INTERNAL_ASSERT( B.columns() == rhs.rhs_.columns(), "Invalid number of columns" );
      BLAZE_INTERNAL_ASSERT( A.rows()    == (~lhs).rows()     , "Invalid number of rows"    );
      BLAZE_INTERNAL_ASSERT( B.columns() == (~lhs).columns()  , "Invalid number of columns" );

      TDMatTSMatMultExpr::selectSubAssignKernel( ~lhs, A, B );
   }
   /*! \endcond */
   //**********************************************************************************************

   //**Default subtraction assignment to dense matrices********************************************
   /*! \cond BLAZE_INTERNAL */
   /*!\brief Default subtraction assignment of a transpose dense matrix-transpose sparse matrix
   //        multiplication to matrices (\f$ A-=B*C \f$).
   // \ingroup dense_matrix
   //
   // \param C The target left-hand side dense matrix.
   // \param A The left-hand side dense matrix operand.
   // \param B The right-hand side sparse matrix operand.
   // \return void
   //
   // This function implements the default subtraction assignment kernel for the transpose dense
   // matrix-transpose sparse matrix multiplication to a dense matrix.
   */
   template< typename MT3    // Type of the left-hand side target matrix
           , typename MT4    // Type of the left-hand side matrix operand
           , typename MT5 >  // Type of the right-hand side matrix operand
   static inline EnableIf_< UseDefaultKernel<MT3,MT4,MT5> >
      selectSubAssignKernel( MT3& C, const MT4& A, const MT5& B )
   {
      typedef ConstIterator_<MT5>  ConstIterator;

      const size_t block( Or< IsColumnMajorMatrix<MT3>, IsDiagonal<MT4> >::value ? A.rows() : 64UL );

      for( size_t ii=0UL; ii<A.rows(); ii+=block )
      {
         const size_t itmp( min( ii+block, A.rows() ) );

         for( size_t j=0UL; j<B.columns(); ++j )
         {
            ConstIterator element( B.begin(j) );
            const ConstIterator end( B.end(j) );

            for( ; element!=end; ++element )
            {
               const size_t j1( element->index() );

               if( IsDiagonal<MT4>::value )
               {
                  C(j1,j) -= A(j1,j1) * element->value();
               }
               else
               {
                  const size_t ibegin( ( IsLower<MT4>::value )
                                       ?( ( LOW )
                                          ?( max( j, ii, ( IsStrictlyLower<MT4>::value ? j1+1UL : j1 ) ) )
                                          :( max( ii, ( IsStrictlyLower<MT4>::value ? j1+1UL : j1 ) ) ) )
                                       :( LOW ? max(j,ii) : ii ) );
                  const size_t iend( ( IsUpper<MT4>::value )
                                     ?( ( UPP )
                                        ?( min( j+1UL, itmp, ( IsStrictlyUpper<MT4>::value ? j1 : j1+1UL ) ) )
                                        :( min( itmp, ( IsStrictlyUpper<MT4>::value ? j1 : j1+1UL ) ) ) )
                                     :( UPP ? min(j+1UL,itmp) : itmp ) );

                  if( ( LOW || UPP || IsTriangular<MT4>::value ) && ( ibegin >= iend ) )
                     continue;

                  BLAZE_INTERNAL_ASSERT( ibegin <= iend, "Invalid loop indices detected" );

                  const size_t inum( iend - ibegin );
                  const size_t ipos( ibegin + ( inum & size_t(-4) ) );
                  BLAZE_INTERNAL_ASSERT( ( ibegin + inum - ( inum % 4UL ) ) == ipos, "Invalid end calculation" );

                  for( size_t i=ibegin; i<ipos; i+=4UL ) {
                     C(i    ,j) -= A(i    ,j1) * element->value();
                     C(i+1UL,j) -= A(i+1UL,j1) * element->value();
                     C(i+2UL,j) -= A(i+2UL,j1) * element->value();
                     C(i+3UL,j) -= A(i+3UL,j1) * element->value();
                  }
                  for( size_t i=ipos; i<iend; ++i ) {
                     C(i,j) -= A(i,j1) * element->value();
                  }
               }
            }
         }
      }
   }
   /*! \endcond */
   //**********************************************************************************************

   //**Optimized subtraction assignment to dense matrices******************************************
   /*! \cond BLAZE_INTERNAL */
   /*!\brief Optimized subtraction assignment of a transpose dense matrix-transpose sparse matrix
   //        multiplication to matrices (\f$ A-=B*C \f$).
   // \ingroup dense_matrix
   //
   // \param C The target left-hand side dense matrix.
   // \param A The left-hand side dense matrix operand.
   // \param B The right-hand side sparse matrix operand.
   // \return void
   //
   // This function implements the optimized subtraction assignment kernel for the transpose dense
   // matrix-transpose sparse matrix multiplication to a dense matrix.
   */
   template< typename MT3    // Type of the left-hand side target matrix
           , typename MT4    // Type of the left-hand side matrix operand
           , typename MT5 >  // Type of the right-hand side matrix operand
   static inline EnableIf_< UseOptimizedKernel<MT3,MT4,MT5> >
      selectSubAssignKernel( MT3& C, const MT4& A, const MT5& B )
   {
      typedef ConstIterator_<MT5>  ConstIterator;

      const size_t block( IsColumnMajorMatrix<MT3>::value ? A.rows() : 64UL );

      for( size_t ii=0UL; ii<A.rows(); ii+=block )
      {
         const size_t itmp( min( ii+block, A.rows() ) );

         for( size_t j=0UL; j<B.columns(); ++j )
         {
            const ConstIterator end( B.end(j) );
            ConstIterator element( B.begin(j) );

            const size_t nonzeros( B.nonZeros(j) );
            const size_t kpos( nonzeros & size_t(-4) );
            BLAZE_INTERNAL_ASSERT( ( nonzeros - ( nonzeros % 4UL ) ) == kpos, "Invalid end calculation" );

            for( size_t k=0UL; k<kpos; k+=4UL )
            {
               const size_t j1( element->index() );
               const ET2    v1( element->value() );
               ++element;
               const size_t j2( element->index() );
               const ET2    v2( element->value() );
               ++element;
               const size_t j3( element->index() );
               const ET2    v3( element->value() );
               ++element;
               const size_t j4( element->index() );
               const ET2    v4( element->value() );
               ++element;

               BLAZE_INTERNAL_ASSERT( j1 < j2 && j2 < j3 && j3 < j4, "Invalid sparse matrix index detected" );

               const size_t ibegin( ( IsLower<MT4>::value )
                                    ?( ( LOW )
                                       ?( max( j, ii, ( IsStrictlyLower<MT4>::value ? j1+1UL : j1 ) ) )
                                       :( max( ii, ( IsStrictlyLower<MT4>::value ? j1+1UL : j1 ) ) ) )
                                    :( LOW ? max(j,ii) : ii ) );
               const size_t iend( ( IsUpper<MT4>::value )
                                  ?( ( UPP )
                                     ?( min( j+1UL, itmp, ( IsStrictlyUpper<MT4>::value ? j4 : j4+1UL ) ) )
                                     :( min( itmp, ( IsStrictlyUpper<MT4>::value ? j4 : j4+1UL ) ) ) )
                                  :( UPP ? min(j+1UL,itmp) : itmp ) );

               if( ( LOW || UPP || IsTriangular<MT4>::value ) && ( ibegin >= iend ) )
                  continue;

               BLAZE_INTERNAL_ASSERT( ibegin <= iend, "Invalid loop indices detected" );

               const size_t inum( iend - ibegin );
               const size_t ipos( ibegin + ( inum & size_t(-4) ) );
               BLAZE_INTERNAL_ASSERT( ( ibegin + inum - ( inum % 4UL ) ) == ipos, "Invalid end calculation" );

               for( size_t i=ibegin; i<ipos; i+=4UL ) {
                  C(i    ,j) -= A(i    ,j1) * v1 + A(i    ,j2) * v2 + A(i    ,j3) * v3 + A(i    ,j4) * v4;
                  C(i+1UL,j) -= A(i+1UL,j1) * v1 + A(i+1UL,j2) * v2 + A(i+1UL,j3) * v3 + A(i+1UL,j4) * v4;
                  C(i+2UL,j) -= A(i+2UL,j1) * v1 + A(i+2UL,j2) * v2 + A(i+2UL,j3) * v3 + A(i+2UL,j4) * v4;
                  C(i+3UL,j) -= A(i+3UL,j1) * v1 + A(i+3UL,j2) * v2 + A(i+3UL,j3) * v3 + A(i+3UL,j4) * v4;
               }
               for( size_t i=ipos; i<iend; ++i ) {
                  C(i,j) -= A(i,j1) * v1 + A(i,j2) * v2 + A(i,j3) * v3 + A(i,j4) * v4;
               }
            }

            for( ; element!=end; ++element )
            {
               const size_t j1( element->index() );
               const ET2    v1( element->value() );

               const size_t ibegin( ( IsLower<MT4>::value )
                                    ?( ( LOW )
                                       ?( max( j, ii, ( IsStrictlyLower<MT4>::value ? j1+1UL : j1 ) ) )
                                       :( max( ii, ( IsStrictlyLower<MT4>::value ? j1+1UL : j1 ) ) ) )
                                    :( LOW ? max(j,ii) : ii ) );
               const size_t iend( ( IsUpper<MT4>::value )
                                  ?( ( UPP )
                                     ?( min( j+1UL, itmp, ( IsStrictlyUpper<MT4>::value ? j1 : j1+1UL ) ) )
                                     :( min( itmp, ( IsStrictlyUpper<MT4>::value ? j1 : j1+1UL ) ) ) )
                                  :( UPP ? min(j+1UL,itmp) : itmp ) );

               if( ( LOW || UPP || IsTriangular<MT4>::value ) && ( ibegin >= iend ) )
                  continue;

               BLAZE_INTERNAL_ASSERT( ibegin <= iend, "Invalid loop indices detected" );

               const size_t inum( iend - ibegin );
               const size_t ipos( ibegin + ( inum & size_t(-4) ) );
               BLAZE_INTERNAL_ASSERT( ( ibegin + inum - ( inum % 4UL ) ) == ipos, "Invalid end calculation" );

               for( size_t i=ibegin; i<ipos; i+=4UL ) {
                  C(i    ,j) -= A(i    ,j1) * v1;
                  C(i+1UL,j) -= A(i+1UL,j1) * v1;
                  C(i+2UL,j) -= A(i+2UL,j1) * v1;
                  C(i+3UL,j) -= A(i+3UL,j1) * v1;
               }
               for( size_t i=ipos; i<iend; ++i ) {
                  C(i,j) -= A(i,j1) * v1;
               }
            }
         }
      }
   }
   /*! \endcond */
   //**********************************************************************************************

   //**Vectorized subtraction assignment to column-major dense matrices****************************
   /*! \cond BLAZE_INTERNAL */
   /*!\brief Vectorized subtraction assignment of a transpose dense matrix-transpose sparse matrix
   //        multiplication to column-major matrices (\f$ A-=B*C \f$).
   // \ingroup dense_matrix
   //
   // \param C The target left-hand side dense matrix.
   // \param A The left-hand side dense matrix operand.
   // \param B The right-hand side sparse matrix operand.
   // \return void
   //
   // This function implements the vectorized subtraction assignment kernel for the transpose dense
   // matrix-transpose sparse matrix multiplication to a column-major dense matrix.
   */
   template< typename MT3    // Type of the left-hand side target matrix
           , typename MT4    // Type of the left-hand side matrix operand
           , typename MT5 >  // Type of the right-hand side matrix operand
   static inline EnableIf_< UseVectorizedKernel<MT3,MT4,MT5> >
      selectSubAssignKernel( MT3& C, const MT4& A, const MT5& B )
   {
      typedef ConstIterator_<MT5>  ConstIterator;

      constexpr bool remainder( !IsPadded<MT3>::value || !IsPadded<MT4>::value );

      for( size_t j=0UL; j<B.columns(); ++j )
      {
         const ConstIterator end( B.end(j) );
         ConstIterator element( B.begin(j) );

         const size_t nonzeros( B.nonZeros(j) );
         const size_t kpos( nonzeros & size_t(-4) );
         BLAZE_INTERNAL_ASSERT( ( nonzeros - ( nonzeros % 4UL ) ) == kpos, "Invalid end calculation" );

         for( size_t k=0UL; k<kpos; k+=4UL )
         {
            const size_t j1( element->index() );
            const ET2    v1( element->value() );
            ++element;
            const size_t j2( element->index() );
            const ET2    v2( element->value() );
            ++element;
            const size_t j3( element->index() );
            const ET2    v3( element->value() );
            ++element;
            const size_t j4( element->index() );
            const ET2    v4( element->value() );
            ++element;

            BLAZE_INTERNAL_ASSERT( j1 < j2 && j2 < j3 && j3 < j4, "Invalid sparse matrix index detected" );

            const SIMDType xmm1( set( v1 ) );
            const SIMDType xmm2( set( v2 ) );
            const SIMDType xmm3( set( v3 ) );
            const SIMDType xmm4( set( v4 ) );

            const size_t ibegin( ( IsLower<MT4>::value )
                                 ?( ( IsStrictlyLower<MT4>::value )
                                    ?( ( LOW ? max(j,j1+1UL) : j1+1UL ) & size_t(-SIMDSIZE) )
                                    :( ( LOW ? max(j,j1) : j1 ) & size_t(-SIMDSIZE) ) )
                                 :( LOW ? ( j & size_t(-SIMDSIZE) ) : 0UL ) );
            const size_t iend( ( IsUpper<MT4>::value )
                               ?( ( IsStrictlyUpper<MT4>::value )
                                  ?( UPP ? max(j+1UL,j4) : j4 )
                                  :( UPP ? max(j,j4)+1UL : j4+1UL ) )
                               :( UPP ? j+1UL : A.rows() ) );
            BLAZE_INTERNAL_ASSERT( ibegin <= iend, "Invalid loop indices detected" );

            const size_t ipos( remainder ? ( iend & size_t(-SIMDSIZE) ) : iend );
            BLAZE_INTERNAL_ASSERT( !remainder || ( iend - ( iend % (SIMDSIZE) ) ) == ipos, "Invalid end calculation" );

            size_t i( ibegin );

            for( ; i<ipos; i+=SIMDSIZE ) {
               C.store( i, j, C.load(i,j) - A.load(i,j1) * xmm1 - A.load(i,j2) * xmm2 - A.load(i,j3) * xmm3 - A.load(i,j4) * xmm4 );
            }
            for( ; remainder && i<iend; ++i ) {
               C(i,j) -= A(i,j1) * v1 + A(i,j2) * v2 + A(i,j3) * v3 + A(i,j4) * v4;
            }
         }

         for( ; element!=end; ++element )
         {
            const size_t j1( element->index() );
            const ET2    v1( element->value() );

            const SIMDType xmm1( set( v1 ) );

            const size_t ibegin( ( IsLower<MT4>::value )
                                 ?( ( IsStrictlyLower<MT4>::value )
                                    ?( ( LOW ? max(j,j1+1UL) : j1+1UL ) & size_t(-SIMDSIZE) )
                                    :( ( LOW ? max(j,j1) : j1 ) & size_t(-SIMDSIZE) ) )
                                 :( LOW ? ( j & size_t(-SIMDSIZE) ) : 0UL ) );
            const size_t iend( ( IsUpper<MT4>::value )
                               ?( ( IsStrictlyUpper<MT4>::value )
                                  ?( UPP ? max(j+1UL,j1) : j1 )
                                  :( UPP ? max(j,j1)+1UL : j1+1UL ) )
                               :( UPP ? j+1UL : A.rows() ) );
            BLAZE_INTERNAL_ASSERT( ibegin <= iend, "Invalid loop indices detected" );

            const size_t ipos( remainder ? ( iend & size_t(-SIMDSIZE) ) : iend );
            BLAZE_INTERNAL_ASSERT( !remainder || ( iend - ( iend % (SIMDSIZE) ) ) == ipos, "Invalid end calculation" );

            size_t i( ibegin );

            for( ; i<ipos; i+=SIMDSIZE ) {
               C.store( i, j, C.load(i,j) - A.load(i,j1) * xmm1 );
            }
            for( ; remainder && i<iend; ++i ) {
               C(i,j) -= A(i,j1) * v1;
            }
         }
      }
   }
   /*! \endcond */
   //**********************************************************************************************

   //**Subtraction assignment to sparse matrices***************************************************
   // No special implementation for the subtraction assignment to sparse matrices.
   //**********************************************************************************************

   //**Multiplication assignment to dense matrices*************************************************
   // No special implementation for the multiplication assignment to dense matrices.
   //**********************************************************************************************

   //**Multiplication assignment to sparse matrices************************************************
   // No special implementation for the multiplication assignment to sparse matrices.
   //**********************************************************************************************

   //**SMP assignment to dense matrices************************************************************
   /*! \cond BLAZE_INTERNAL */
   /*!\brief SMP assignment of a transpose dense matrix-transpose sparse matrix multiplication
   //        to a dense matrix (\f$ A=B*C \f$).
   // \ingroup dense_matrix
   //
   // \param lhs The target left-hand side dense matrix.
   // \param rhs The right-hand side multiplication expression to be assigned.
   // \return void
   //
   // This function implements the performance optimized SMP assignment of a transpose dense
   // matrix-transpose sparse matrix multiplication expression to a dense matrix. Due to the
   // explicit application of the SFINAE principle this function can only be selected by the
   // compiler in case either of the two matrix operands requires an intermediate evaluation.
   */
   template< typename MT  // Type of the target dense matrix
           , bool SO >    // Storage order of the target dense matrix
   friend inline EnableIf_< IsEvaluationRequired<MT,MT1,MT2> >
      smpAssign( DenseMatrix<MT,SO>& lhs, const TDMatTSMatMultExpr& rhs )
   {
      BLAZE_FUNCTION_TRACE;

      BLAZE_INTERNAL_ASSERT( (~lhs).rows()    == rhs.rows()   , "Invalid number of rows"    );
      BLAZE_INTERNAL_ASSERT( (~lhs).columns() == rhs.columns(), "Invalid number of columns" );

      LT A( rhs.lhs_ );  // Evaluation of the left-hand side dense matrix operand
      RT B( rhs.rhs_ );  // Evaluation of the right-hand side sparse matrix operand

      BLAZE_INTERNAL_ASSERT( A.rows()    == rhs.lhs_.rows()   , "Invalid number of rows"    );
      BLAZE_INTERNAL_ASSERT( A.columns() == rhs.lhs_.columns(), "Invalid number of columns" );
      BLAZE_INTERNAL_ASSERT( B.rows()    == rhs.rhs_.rows()   , "Invalid number of rows"    );
      BLAZE_INTERNAL_ASSERT( B.columns() == rhs.rhs_.columns(), "Invalid number of columns" );
      BLAZE_INTERNAL_ASSERT( A.rows()    == (~lhs).rows()     , "Invalid number of rows"    );
      BLAZE_INTERNAL_ASSERT( B.columns() == (~lhs).columns()  , "Invalid number of columns" );

      smpAssign( ~lhs, A * B );
   }
   /*! \endcond */
   //**********************************************************************************************

   //**SMP assignment to sparse matrices***********************************************************
   /*! \cond BLAZE_INTERNAL */
   /*!\brief SMP assignment of a transpose dense matrix-transpose sparse matrix multiplication
   //        to a sparse matrix (\f$ A=B*C \f$).
   // \ingroup dense_matrix
   //
   // \param lhs The target left-hand side sparse matrix.
   // \param rhs The right-hand side multiplication expression to be assigned.
   // \return void
   //
   // This function implements the performance optimized SMP assignment of a transpose dense
   // matrix-transpose sparse matrix multiplication expression to a sparse matrix. Due to the
   // explicit application of the SFINAE principle this function can only be selected by the
   // compiler in case either of the two matrix operands requires an intermediate evaluation.
   */
   template< typename MT  // Type of the target sparse matrix
           , bool SO >    // Storage order of the target sparse matrix
   friend inline EnableIf_< IsEvaluationRequired<MT,MT1,MT2> >
      smpAssign( SparseMatrix<MT,SO>& lhs, const TDMatTSMatMultExpr& rhs )
   {
      BLAZE_FUNCTION_TRACE;

      typedef IfTrue_< SO, ResultType, OppositeType >  TmpType;

      BLAZE_CONSTRAINT_MUST_BE_DENSE_MATRIX_TYPE( ResultType );
      BLAZE_CONSTRAINT_MUST_BE_DENSE_MATRIX_TYPE( OppositeType );
      BLAZE_CONSTRAINT_MUST_BE_COLUMN_MAJOR_MATRIX_TYPE( ResultType );
      BLAZE_CONSTRAINT_MUST_BE_ROW_MAJOR_MATRIX_TYPE( OppositeType );
      BLAZE_CONSTRAINT_MATRICES_MUST_HAVE_SAME_STORAGE_ORDER( MT, TmpType );
      BLAZE_CONSTRAINT_MUST_BE_REFERENCE_TYPE( CompositeType_<TmpType> );

      BLAZE_INTERNAL_ASSERT( (~lhs).rows()    == rhs.rows()   , "Invalid number of rows"    );
      BLAZE_INTERNAL_ASSERT( (~lhs).columns() == rhs.columns(), "Invalid number of columns" );

      const ForwardFunctor fwd;

      const TmpType tmp( rhs );
      smpAssign( ~lhs, fwd( tmp ) );
   }
   /*! \endcond */
   //**********************************************************************************************

   //**SMP addition assignment to dense matrices***************************************************
   /*! \cond BLAZE_INTERNAL */
   /*!\brief SMP addition assignment of a transpose dense matrix-transpose sparse matrix
   //        multiplication to a dense matrix (\f$ A+=B*C \f$).
   // \ingroup dense_matrix
   //
   // \param lhs The target left-hand side dense matrix.
   // \param rhs The right-hand side multiplication expression to be added.
   // \return void
   //
   // This function implements the performance optimized SMP addition assignment of a transpose
   // dense matrix-transpose sparse matrix multiplication expression to a dense matrix. Due to
   // the explicit application of the SFINAE principle this function can only be selected by the
   // compiler in case either of the two matrix operands requires an intermediate evaluation.
   */
   template< typename MT  // Type of the target dense matrix
           , bool SO >    // Storage order of the target dense matrix
   friend inline EnableIf_< IsEvaluationRequired<MT,MT1,MT2> >
      smpAddAssign( DenseMatrix<MT,SO>& lhs, const TDMatTSMatMultExpr& rhs )
   {
      BLAZE_FUNCTION_TRACE;

      BLAZE_INTERNAL_ASSERT( (~lhs).rows()    == rhs.rows()   , "Invalid number of rows"    );
      BLAZE_INTERNAL_ASSERT( (~lhs).columns() == rhs.columns(), "Invalid number of columns" );

      LT A( rhs.lhs_ );  // Evaluation of the left-hand side dense matrix operand
      RT B( rhs.rhs_ );  // Evaluation of the right-hand side sparse matrix operand

      BLAZE_INTERNAL_ASSERT( A.rows()    == rhs.lhs_.rows()   , "Invalid number of rows"    );
      BLAZE_INTERNAL_ASSERT( A.columns() == rhs.lhs_.columns(), "Invalid number of columns" );
      BLAZE_INTERNAL_ASSERT( B.rows()    == rhs.rhs_.rows()   , "Invalid number of rows"    );
      BLAZE_INTERNAL_ASSERT( B.columns() == rhs.rhs_.columns(), "Invalid number of columns" );
      BLAZE_INTERNAL_ASSERT( A.rows()    == (~lhs).rows()     , "Invalid number of rows"    );
      BLAZE_INTERNAL_ASSERT( B.columns() == (~lhs).columns()  , "Invalid number of columns" );

      smpAddAssign( ~lhs, A * B );
   }
   /*! \endcond */
   //**********************************************************************************************

   //**SMP addition assignment to sparse matrices**************************************************
   // No special implementation for the SMP addition assignment to sparse matrices.
   //**********************************************************************************************

   //**SMP subtraction assignment to dense matrices************************************************
   /*! \cond BLAZE_INTERNAL */
   /*!\brief SMP subtraction assignment of a transpose dense matrix-transpose sparse matrix
   //        multiplication to a dense matrix (\f$ A-=B*C \f$).
   // \ingroup dense_matrix
   //
   // \param lhs The target left-hand side dense matrix.
   // \param rhs The right-hand side multiplication expression to be subtracted.
   // \return void
   //
   // This function implements the performance optimized SMP subtraction assignment of a transpose
   // dense matrix-transpose sparse matrix multiplication expression to a dense matrix. Due to
   // the explicit application of the SFINAE principle this function can only be selected by the
   // compiler in case either of the two matrix operands requires an intermediate evaluation.
   */
   template< typename MT  // Type of the target dense matrix
           , bool SO >    // Storage order of the target dense matrix
   friend inline EnableIf_< IsEvaluationRequired<MT,MT1,MT2> >
      smpSubAssign( DenseMatrix<MT,SO>& lhs, const TDMatTSMatMultExpr& rhs )
   {
      BLAZE_FUNCTION_TRACE;

      BLAZE_INTERNAL_ASSERT( (~lhs).rows()    == rhs.rows()   , "Invalid number of rows"    );
      BLAZE_INTERNAL_ASSERT( (~lhs).columns() == rhs.columns(), "Invalid number of columns" );

      LT A( rhs.lhs_ );  // Evaluation of the left-hand side dense matrix operand
      RT B( rhs.rhs_ );  // Evaluation of the right-hand side sparse matrix operand

      BLAZE_INTERNAL_ASSERT( A.rows()    == rhs.lhs_.rows()   , "Invalid number of rows"    );
      BLAZE_INTERNAL_ASSERT( A.columns() == rhs.lhs_.columns(), "Invalid number of columns" );
      BLAZE_INTERNAL_ASSERT( B.rows()    == rhs.rhs_.rows()   , "Invalid number of rows"    );
      BLAZE_INTERNAL_ASSERT( B.columns() == rhs.rhs_.columns(), "Invalid number of columns" );
      BLAZE_INTERNAL_ASSERT( A.rows()    == (~lhs).rows()     , "Invalid number of rows"    );
      BLAZE_INTERNAL_ASSERT( B.columns() == (~lhs).columns()  , "Invalid number of columns" );

      smpSubAssign( ~lhs, A * B );
   }
   /*! \endcond */
   //**********************************************************************************************

   //**SMP subtraction assignment to sparse matrices***********************************************
   // No special implementation for the SMP subtraction assignment to sparse matrices.
   //**********************************************************************************************

   //**SMP multiplication assignment to dense matrices*********************************************
   // No special implementation for the SMP multiplication assignment to dense matrices.
   //**********************************************************************************************

   //**SMP multiplication assignment to sparse matrices********************************************
   // No special implementation for the SMP multiplication assignment to sparse matrices.
   //**********************************************************************************************

   //**Compile time checks*************************************************************************
   /*! \cond BLAZE_INTERNAL */
   BLAZE_CONSTRAINT_MUST_BE_DENSE_MATRIX_TYPE( MT1 );
   BLAZE_CONSTRAINT_MUST_BE_COLUMN_MAJOR_MATRIX_TYPE( MT1 );
   BLAZE_CONSTRAINT_MUST_BE_SPARSE_MATRIX_TYPE( MT2 );
   BLAZE_CONSTRAINT_MUST_BE_COLUMN_MAJOR_MATRIX_TYPE( MT2 );
   BLAZE_CONSTRAINT_MUST_FORM_VALID_MATMATMULTEXPR( MT1, MT2 );
   /*! \endcond */
   //**********************************************************************************************
};
//*************************************************************************************************




//=================================================================================================
//
//  GLOBAL BINARY ARITHMETIC OPERATORS
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Multiplication operator for the multiplication of a column-major dense matrix and a
//        column-major sparse matrix (\f$ A=B*C \f$).
// \ingroup dense_matrix
//
// \param lhs The left-hand side dense matrix for the multiplication.
// \param rhs The right-hand side sparse matrix for the multiplication.
// \return The resulting matrix.
// \exception std::invalid_argument Matrix sizes do not match.
//
// This operator represents the multiplication of a column-major dense matrix and a column-major
// sparse matrix:

   \code
   using blaze::columnMajor;

   blaze::DynamicMatrix<double,columnMajor> A, C;
   blaze::CompressedMatrix<double,columnMajor> B;
   // ... Resizing and initialization
   C = A * B;
   \endcode

// The operator returns an expression representing a dense matrix of the higher-order element
// type of the two involved matrix element types \a T1::ElementType and \a T2::ElementType.
// Both matrix types \a T1 and \a T2 as well as the two element types \a T1::ElementType and
// \a T2::ElementType have to be supported by the MultTrait class template.\n
// In case the current sizes of the two given matrices don't match, a \a std::invalid_argument
// is thrown.
*/
template< typename T1    // Type of the left-hand side dense matrix
        , typename T2 >  // Type of the right-hand side sparse matrix
inline const TDMatTSMatMultExpr<T1,T2,false,false,false,false>
   operator*( const DenseMatrix<T1,true>& lhs, const SparseMatrix<T2,true>& rhs )
{
   BLAZE_FUNCTION_TRACE;

   if( (~lhs).columns() != (~rhs).rows() ) {
      BLAZE_THROW_INVALID_ARGUMENT( "Matrix sizes do not match" );
   }

   return TDMatTSMatMultExpr<T1,T2,false,false,false,false>( ~lhs, ~rhs );
}
//*************************************************************************************************




//=================================================================================================
//
//  GLOBAL FUNCTIONS
//
//=================================================================================================

//*************************************************************************************************
/*! \cond BLAZE_INTERNAL */
/*!\brief Declares the given non-symmetric matrix multiplication expression as symmetric.
// \ingroup dense_matrix
//
// \param dm The input matrix multiplication expression.
// \return The redeclared dense matrix multiplication expression.
// \exception std::invalid_argument Invalid symmetric matrix specification.
//
// The \a declsym function declares the given non-symmetric matrix multiplication expression
// \a dm as symmetric. The function returns an expression representing the operation. In case
// the given expression does not represent a square matrix, a \a std::invalid_argument exception
// is thrown.\n
// The following example demonstrates the use of the \a declsym function:

   \code
   using blaze::columnMajor;

   blaze::DynamicMatrix<double,columnMajor> A, C;
   blaze::CompressedMatrix<double,columnMajor> B;
   // ... Resizing and initialization
   C = declsym( A * B );
   \endcode
*/
template< typename MT1  // Type of the left-hand side dense matrix
        , typename MT2  // Type of the right-hand side dense matrix
        , bool SF       // Symmetry flag
        , bool HF       // Hermitian flag
        , bool LF       // Lower flag
        , bool UF >     // Upper flag
inline const TDMatTSMatMultExpr<MT1,MT2,true,HF,LF,UF>
   declsym( const TDMatTSMatMultExpr<MT1,MT2,SF,HF,LF,UF>& dm )
{
   BLAZE_FUNCTION_TRACE;

   if( !isSquare( dm ) ) {
      BLAZE_THROW_INVALID_ARGUMENT( "Invalid symmetric matrix specification" );
   }

   return TDMatTSMatMultExpr<MT1,MT2,true,HF,LF,UF>( dm.leftOperand(), dm.rightOperand() );
}
/*! \endcond */
//*************************************************************************************************


//*************************************************************************************************
/*! \cond BLAZE_INTERNAL */
/*!\brief Declares the given non-Hermitian matrix multiplication expression as Hermitian.
// \ingroup dense_matrix
//
// \param dm The input matrix multiplication expression.
// \return The redeclared dense matrix multiplication expression.
// \exception std::invalid_argument Invalid Hermitian matrix specification.
//
// The \a declherm function declares the given non-Hermitian matrix multiplication expression
// \a dm as Hermitian. The function returns an expression representing the operation. In case
// the given expression does not represent a square matrix, a \a std::invalid_argument exception
// is thrown.\n
// The following example demonstrates the use of the \a declherm function:

   \code
   using blaze::columnMajor;

   blaze::DynamicMatrix<double,columnMajor> A, C;
   blaze::CompressedMatrix<double,columnMajor> B;
   // ... Resizing and initialization
   C = declherm( A * B );
   \endcode
*/
template< typename MT1  // Type of the left-hand side dense matrix
        , typename MT2  // Type of the right-hand side dense matrix
        , bool SF       // Symmetry flag
        , bool HF       // Hermitian flag
        , bool LF       // Lower flag
        , bool UF >     // Upper flag
inline const TDMatTSMatMultExpr<MT1,MT2,SF,true,LF,UF>
   declherm( const TDMatTSMatMultExpr<MT1,MT2,SF,HF,LF,UF>& dm )
{
   BLAZE_FUNCTION_TRACE;

   if( !isSquare( dm ) ) {
      BLAZE_THROW_INVALID_ARGUMENT( "Invalid Hermitian matrix specification" );
   }

   return TDMatTSMatMultExpr<MT1,MT2,SF,true,LF,UF>( dm.leftOperand(), dm.rightOperand() );
}
/*! \endcond */
//*************************************************************************************************


//*************************************************************************************************
/*! \cond BLAZE_INTERNAL */
/*!\brief Declares the given non-lower matrix multiplication expression as lower.
// \ingroup dense_matrix
//
// \param dm The input matrix multiplication expression.
// \return The redeclared dense matrix multiplication expression.
// \exception std::invalid_argument Invalid lower matrix specification.
//
// The \a decllow function declares the given non-lower matrix multiplication expression
// \a dm as lower. The function returns an expression representing the operation. In case
// the given expression does not represent a square matrix, a \a std::invalid_argument
// exception is thrown.\n
// The following example demonstrates the use of the \a decllow function:

   \code
   using blaze::columnMajor;

   blaze::DynamicMatrix<double,columnMajor> A, C;
   blaze::CompressedMatrix<double,columnMajor> B;
   // ... Resizing and initialization
   C = decllow( A * B );
   \endcode
*/
template< typename MT1  // Type of the left-hand side dense matrix
        , typename MT2  // Type of the right-hand side dense matrix
        , bool SF       // Symmetry flag
        , bool HF       // Hermitian flag
        , bool LF       // Lower flag
        , bool UF >     // Upper flag
inline const TDMatTSMatMultExpr<MT1,MT2,SF,HF,true,UF>
   decllow( const TDMatTSMatMultExpr<MT1,MT2,SF,HF,LF,UF>& dm )
{
   BLAZE_FUNCTION_TRACE;

   if( !isSquare( dm ) ) {
      BLAZE_THROW_INVALID_ARGUMENT( "Invalid lower matrix specification" );
   }

   return TDMatTSMatMultExpr<MT1,MT2,SF,HF,true,UF>( dm.leftOperand(), dm.rightOperand() );
}
/*! \endcond */
//*************************************************************************************************


//*************************************************************************************************
/*! \cond BLAZE_INTERNAL */
/*!\brief Declares the given non-upper matrix multiplication expression as upper.
// \ingroup dense_matrix
//
// \param dm The input matrix multiplication expression.
// \return The redeclared dense matrix multiplication expression.
// \exception std::invalid_argument Invalid upper matrix specification.
//
// The \a declupp function declares the given non-upper matrix multiplication expression
// \a dm as upper. The function returns an expression representing the operation. In case
// the given expression does not represent a square matrix, a \a std::invalid_argument
// exception is thrown.\n
// The following example demonstrates the use of the \a declupp function:

   \code
   using blaze::columnMajor;

   blaze::DynamicMatrix<double,columnMajor> A, C;
   blaze::CompressedMatrix<double,columnMajor> B;
   // ... Resizing and initialization
   C = declupp( A * B );
   \endcode
*/
template< typename MT1  // Type of the left-hand side dense matrix
        , typename MT2  // Type of the right-hand side dense matrix
        , bool SF       // Symmetry flag
        , bool HF       // Hermitian flag
        , bool LF       // Lower flag
        , bool UF >     // Upper flag
inline const TDMatTSMatMultExpr<MT1,MT2,SF,HF,LF,true>
   declupp( const TDMatTSMatMultExpr<MT1,MT2,SF,HF,LF,UF>& dm )
{
   BLAZE_FUNCTION_TRACE;

   if( !isSquare( dm ) ) {
      BLAZE_THROW_INVALID_ARGUMENT( "Invalid upper matrix specification" );
   }

   return TDMatTSMatMultExpr<MT1,MT2,SF,HF,LF,true>( dm.leftOperand(), dm.rightOperand() );
}
/*! \endcond */
//*************************************************************************************************


//*************************************************************************************************
/*! \cond BLAZE_INTERNAL */
/*!\brief Declares the given non-diagonal matrix multiplication expression as diagonal.
// \ingroup dense_matrix
//
// \param dm The input matrix multiplication expression.
// \return The redeclared dense matrix multiplication expression.
// \exception std::invalid_argument Invalid diagonal matrix specification.
//
// The \a decldiag function declares the given non-diagonal matrix multiplication expression
// \a dm as diagonal. The function returns an expression representing the operation. In case
// the given expression does not represent a square matrix, a \a std::invalid_argument exception
// is thrown.\n
// The following example demonstrates the use of the \a decldiag function:

   \code
   using blaze::columnMajor;

   blaze::DynamicMatrix<double,columnMajor> A, C;
   blaze::CompressedMatrix<double,columnMajor> B;
   // ... Resizing and initialization
   C = decldiag( A * B );
   \endcode
*/
template< typename MT1  // Type of the left-hand side dense matrix
        , typename MT2  // Type of the right-hand side dense matrix
        , bool SF       // Symmetry flag
        , bool HF       // Hermitian flag
        , bool LF       // Lower flag
        , bool UF >     // Upper flag
inline const TDMatTSMatMultExpr<MT1,MT2,SF,HF,true,true>
   decldiag( const TDMatTSMatMultExpr<MT1,MT2,SF,HF,LF,UF>& dm )
{
   BLAZE_FUNCTION_TRACE;

   if( !isSquare( dm ) ) {
      BLAZE_THROW_INVALID_ARGUMENT( "Invalid diagonal matrix specification" );
   }

   return TDMatTSMatMultExpr<MT1,MT2,SF,HF,true,true>( dm.leftOperand(), dm.rightOperand() );
}
/*! \endcond */
//*************************************************************************************************




//=================================================================================================
//
//  ROWS SPECIALIZATIONS
//
//=================================================================================================

//*************************************************************************************************
/*! \cond BLAZE_INTERNAL */
template< typename MT1, typename MT2, bool SF, bool HF, bool LF, bool UF >
struct Rows< TDMatTSMatMultExpr<MT1,MT2,SF,HF,LF,UF> > : public Rows<MT1>
{};
/*! \endcond */
//*************************************************************************************************




//=================================================================================================
//
//  COLUMNS SPECIALIZATIONS
//
//=================================================================================================

//*************************************************************************************************
/*! \cond BLAZE_INTERNAL */
template< typename MT1, typename MT2, bool SF, bool HF, bool LF, bool UF >
struct Columns< TDMatTSMatMultExpr<MT1,MT2,SF,HF,LF,UF> > : public Columns<MT2>
{};
/*! \endcond */
//*************************************************************************************************




//=================================================================================================
//
//  ISALIGNED SPECIALIZATIONS
//
//=================================================================================================

//*************************************************************************************************
/*! \cond BLAZE_INTERNAL */
template< typename MT1, typename MT2, bool SF, bool HF, bool LF, bool UF >
struct IsAligned< TDMatTSMatMultExpr<MT1,MT2,SF,HF,LF,UF> >
   : public BoolConstant< IsAligned<MT1>::value >
{};
/*! \endcond */
//*************************************************************************************************




//=================================================================================================
//
//  ISSYMMETRIC SPECIALIZATIONS
//
//=================================================================================================

//*************************************************************************************************
/*! \cond BLAZE_INTERNAL */
template< typename MT1, typename MT2, bool SF, bool HF, bool LF, bool UF >
struct IsSymmetric< TDMatTSMatMultExpr<MT1,MT2,SF,HF,LF,UF> >
   : public BoolConstant< Or< Bool<SF>
                            , And< Bool<HF>
                                 , IsBuiltin< ElementType_< TDMatTSMatMultExpr<MT1,MT2,false,true,false,false> > > >
                            , And< Bool<LF>, Bool<UF> > >::value >
{};
/*! \endcond */
//*************************************************************************************************




//=================================================================================================
//
//  ISHERMITIAN SPECIALIZATIONS
//
//=================================================================================================

//*************************************************************************************************
/*! \cond BLAZE_INTERNAL */
template< typename MT1, typename MT2, bool SF, bool LF, bool UF >
struct IsHermitian< TDMatTSMatMultExpr<MT1,MT2,SF,true,LF,UF> >
   : public TrueType
{};
/*! \endcond */
//*************************************************************************************************




//=================================================================================================
//
//  ISLOWER SPECIALIZATIONS
//
//=================================================================================================

//*************************************************************************************************
/*! \cond BLAZE_INTERNAL */
template< typename MT1, typename MT2, bool SF, bool HF, bool LF, bool UF >
struct IsLower< TDMatTSMatMultExpr<MT1,MT2,SF,HF,LF,UF> >
   : public BoolConstant< Or< Bool<LF>
                            , And< IsLower<MT1>, IsLower<MT2> >
                            , And< Or< Bool<SF>, Bool<HF> >
                                 , IsUpper<MT1>, IsUpper<MT2> > >::value >
{};
/*! \endcond */
//*************************************************************************************************




//=================================================================================================
//
//  ISUNILOWER SPECIALIZATIONS
//
//=================================================================================================

//*************************************************************************************************
/*! \cond BLAZE_INTERNAL */
template< typename MT1, typename MT2, bool SF, bool HF, bool LF, bool UF >
struct IsUniLower< TDMatTSMatMultExpr<MT1,MT2,SF,HF,LF,UF> >
   : public BoolConstant< Or< And< IsUniLower<MT1>, IsUniLower<MT2> >
                            , And< Or< Bool<SF>, Bool<HF> >
                                 , IsUniUpper<MT1>, IsUniUpper<MT2> > >::value >
{};
/*! \endcond */
//*************************************************************************************************




//=================================================================================================
//
//  ISSTRICTLYLOWER SPECIALIZATIONS
//
//=================================================================================================

//*************************************************************************************************
/*! \cond BLAZE_INTERNAL */
template< typename MT1, typename MT2, bool SF, bool HF, bool LF, bool UF >
struct IsStrictlyLower< TDMatTSMatMultExpr<MT1,MT2,SF,HF,LF,UF> >
   : public BoolConstant< Or< And< IsStrictlyLower<MT1>, IsLower<MT2> >
                            , And< IsStrictlyLower<MT2>, IsLower<MT1> >
                            , And< Or< Bool<SF>, Bool<HF> >
                                 , Or< And< IsStrictlyUpper<MT1>, IsUpper<MT2> >
                                     , And< IsStrictlyUpper<MT2>, IsUpper<MT1> > > > >::value >
{};
/*! \endcond */
//*************************************************************************************************




//=================================================================================================
//
//  ISUPPER SPECIALIZATIONS
//
//=================================================================================================

//*************************************************************************************************
/*! \cond BLAZE_INTERNAL */
template< typename MT1, typename MT2, bool SF, bool HF, bool LF, bool UF >
struct IsUpper< TDMatTSMatMultExpr<MT1,MT2,SF,HF,LF,UF> >
   : public BoolConstant< Or< Bool<UF>
                            , And< IsUpper<MT1>, IsUpper<MT2> >
                            , And< Or< Bool<SF>, Bool<HF> >
                                 , IsLower<MT1>, IsLower<MT2> > >::value >
{};
/*! \endcond */
//*************************************************************************************************




//=================================================================================================
//
//  ISUNIUPPER SPECIALIZATIONS
//
//=================================================================================================

//*************************************************************************************************
/*! \cond BLAZE_INTERNAL */
template< typename MT1, typename MT2, bool SF, bool HF, bool LF, bool UF >
struct IsUniUpper< TDMatTSMatMultExpr<MT1,MT2,SF,HF,LF,UF> >
   : public BoolConstant< Or< And< IsUniUpper<MT1>, IsUniUpper<MT2> >
                            , And< Or< Bool<SF>, Bool<HF> >
                                 , IsUniLower<MT1>, IsUniLower<MT2> > >::value >
{};
/*! \endcond */
//*************************************************************************************************




//=================================================================================================
//
//  ISSTRICTLYUPPER SPECIALIZATIONS
//
//=================================================================================================

//*************************************************************************************************
/*! \cond BLAZE_INTERNAL */
template< typename MT1, typename MT2, bool SF, bool HF, bool LF, bool UF >
struct IsStrictlyUpper< TDMatTSMatMultExpr<MT1,MT2,SF,HF,LF,UF> >
   : public BoolConstant< Or< And< IsStrictlyUpper<MT1>, IsUpper<MT2> >
                            , And< IsStrictlyUpper<MT2>, IsUpper<MT1> >
                            , And< Or< Bool<SF>, Bool<HF> >
                                 , Or< And< IsStrictlyLower<MT1>, IsLower<MT2> >
                                     , And< IsStrictlyLower<MT2>, IsLower<MT1> > > > >::value >
{};
/*! \endcond */
//*************************************************************************************************




//=================================================================================================
//
//  EXPRESSION TRAIT SPECIALIZATIONS
//
//=================================================================================================

//*************************************************************************************************
/*! \cond BLAZE_INTERNAL */
template< typename MT1, typename MT2, bool SF, bool HF, bool LF, bool UF, typename VT >
struct TDMatDVecMultExprTrait< TDMatTSMatMultExpr<MT1,MT2,SF,HF,LF,UF>, VT >
{
 public:
   //**********************************************************************************************
   using Type = If_< And< IsDenseMatrix<MT1>, IsColumnMajorMatrix<MT1>
                        , IsSparseMatrix<MT2>, IsColumnMajorMatrix<MT2>
                        , IsDenseVector<VT>, IsColumnVector<VT> >
                   , TDMatDVecMultExprTrait_< MT1, TSMatDVecMultExprTrait_<MT2,VT> >
                   , INVALID_TYPE >;
   //**********************************************************************************************
};
/*! \endcond */
//*************************************************************************************************


//*************************************************************************************************
/*! \cond BLAZE_INTERNAL */
template< typename MT1, typename MT2, bool SF, bool HF, bool LF, bool UF, typename VT >
struct TDMatSVecMultExprTrait< TDMatTSMatMultExpr<MT1,MT2,SF,HF,LF,UF>, VT >
{
 public:
   //**********************************************************************************************
   using Type = If_< And< IsDenseMatrix<MT1>, IsColumnMajorMatrix<MT1>
                        , IsSparseMatrix<MT2>, IsColumnMajorMatrix<MT2>
                        , IsSparseVector<VT>, IsColumnVector<VT> >
                   , TDMatDVecMultExprTrait_< MT1, TSMatDVecMultExprTrait_<MT2,VT> >
                   , INVALID_TYPE >;
   //**********************************************************************************************
};
/*! \endcond */
//*************************************************************************************************


//*************************************************************************************************
/*! \cond BLAZE_INTERNAL */
template< typename VT, typename MT1, typename MT2, bool SF, bool HF, bool LF, bool UF >
struct TDVecTDMatMultExprTrait< VT, TDMatTSMatMultExpr<MT1,MT2,SF,HF,LF,UF> >
{
 public:
   //**********************************************************************************************
   using Type = If_< And< IsDenseVector<VT>, IsRowVector<VT>
                        , IsDenseMatrix<MT1>, IsColumnMajorMatrix<MT1>
                        , IsSparseMatrix<MT2>, IsColumnMajorMatrix<MT2> >
                   , TDVecTSMatMultExprTrait_< TDVecTDMatMultExprTrait_<VT,MT1>, MT2 >
                   , INVALID_TYPE >;
   //**********************************************************************************************
};
/*! \endcond */
//*************************************************************************************************


//*************************************************************************************************
/*! \cond BLAZE_INTERNAL */
template< typename VT, typename MT1, typename MT2, bool SF, bool HF, bool LF, bool UF >
struct TSVecTDMatMultExprTrait< VT, TDMatTSMatMultExpr<MT1,MT2,SF,HF,LF,UF> >
{
 public:
   //**********************************************************************************************
   using Type = If_< And< IsSparseVector<VT>, IsRowVector<VT>
                        , IsDenseMatrix<MT1>, IsColumnMajorMatrix<MT1>
                        , IsSparseMatrix<MT2>, IsColumnMajorMatrix<MT2> >
                   , TDVecTSMatMultExprTrait_< TSVecTDMatMultExprTrait_<VT,MT1>, MT2 >
                   , INVALID_TYPE >;
   //**********************************************************************************************
};
/*! \endcond */
//*************************************************************************************************


//*************************************************************************************************
/*! \cond BLAZE_INTERNAL */
template< typename MT1, typename MT2, bool SF, bool HF, bool LF, bool UF >
struct TDMatDeclSymExprTrait< TDMatTSMatMultExpr<MT1,MT2,SF,HF,LF,UF> >
{
 public:
   //**********************************************************************************************
   using Type = If_< And< IsDenseMatrix<MT1>, IsColumnMajorMatrix<MT1>
                        , IsSparseMatrix<MT2>, IsColumnMajorMatrix<MT2> >
                   , TDMatTSMatMultExpr<MT1,MT2,true,HF,LF,UF>
                   , INVALID_TYPE >;
   //**********************************************************************************************
};
/*! \endcond */
//*************************************************************************************************


//*************************************************************************************************
/*! \cond BLAZE_INTERNAL */
template< typename MT1, typename MT2, bool SF, bool HF, bool LF, bool UF >
struct TDMatDeclHermExprTrait< TDMatTSMatMultExpr<MT1,MT2,SF,HF,LF,UF> >
{
 public:
   //**********************************************************************************************
   using Type = If_< And< IsDenseMatrix<MT1>, IsColumnMajorMatrix<MT1>
                        , IsSparseMatrix<MT2>, IsColumnMajorMatrix<MT2> >
                   , TDMatTSMatMultExpr<MT1,MT2,SF,true,LF,UF>
                   , INVALID_TYPE >;
   //**********************************************************************************************
};
/*! \endcond */
//*************************************************************************************************


//*************************************************************************************************
/*! \cond BLAZE_INTERNAL */
template< typename MT1, typename MT2, bool SF, bool HF, bool LF, bool UF >
struct TDMatDeclLowExprTrait< TDMatTSMatMultExpr<MT1,MT2,SF,HF,LF,UF> >
{
 public:
   //**********************************************************************************************
   using Type = If_< And< IsDenseMatrix<MT1>, IsColumnMajorMatrix<MT1>
                        , IsSparseMatrix<MT2>, IsColumnMajorMatrix<MT2> >
                   , TDMatTSMatMultExpr<MT1,MT2,SF,HF,true,UF>
                   , INVALID_TYPE >;
   //**********************************************************************************************
};
/*! \endcond */
//*************************************************************************************************


//*************************************************************************************************
/*! \cond BLAZE_INTERNAL */
template< typename MT1, typename MT2, bool SF, bool HF, bool LF, bool UF >
struct TDMatDeclUppExprTrait< TDMatTSMatMultExpr<MT1,MT2,SF,HF,LF,UF> >
{
 public:
   //**********************************************************************************************
   using Type = If_< And< IsDenseMatrix<MT1>, IsColumnMajorMatrix<MT1>
                        , IsSparseMatrix<MT2>, IsColumnMajorMatrix<MT2> >
                   , TDMatTSMatMultExpr<MT1,MT2,SF,HF,LF,true>
                   , INVALID_TYPE >;
   //**********************************************************************************************
};
/*! \endcond */
//*************************************************************************************************


//*************************************************************************************************
/*! \cond BLAZE_INTERNAL */
template< typename MT1, typename MT2, bool SF, bool HF, bool LF, bool UF >
struct TDMatDeclDiagExprTrait< TDMatTSMatMultExpr<MT1,MT2,SF,HF,LF,UF> >
{
 public:
   //**********************************************************************************************
   using Type = If_< And< IsDenseMatrix<MT1>, IsColumnMajorMatrix<MT1>
                        , IsSparseMatrix<MT2>, IsColumnMajorMatrix<MT2> >
                   , TDMatTSMatMultExpr<MT1,MT2,SF,HF,true,true>
                   , INVALID_TYPE >;
   //**********************************************************************************************
};
/*! \endcond */
//*************************************************************************************************


//*************************************************************************************************
/*! \cond BLAZE_INTERNAL */
template< typename MT1, typename MT2, bool SF, bool HF, bool LF, bool UF, bool AF >
struct SubmatrixExprTrait< TDMatTSMatMultExpr<MT1,MT2,SF,HF,LF,UF>, AF >
{
 public:
   //**********************************************************************************************
   using Type = MultExprTrait_< SubmatrixExprTrait_<const MT1,AF>
                              , SubmatrixExprTrait_<const MT2,AF> >;
   //**********************************************************************************************
};
/*! \endcond */
//*************************************************************************************************


//*************************************************************************************************
/*! \cond BLAZE_INTERNAL */
template< typename MT1, typename MT2, bool SF, bool HF, bool LF, bool UF >
struct RowExprTrait< TDMatTSMatMultExpr<MT1,MT2,SF,HF,LF,UF> >
{
 public:
   //**********************************************************************************************
   using Type = MultExprTrait_< RowExprTrait_<const MT1>, MT2 >;
   //**********************************************************************************************
};
/*! \endcond */
//*************************************************************************************************


//*************************************************************************************************
/*! \cond BLAZE_INTERNAL */
template< typename MT1, typename MT2, bool SF, bool HF, bool LF, bool UF >
struct ColumnExprTrait< TDMatTSMatMultExpr<MT1,MT2,SF,HF,LF,UF> >
{
 public:
   //**********************************************************************************************
   using Type = MultExprTrait_< MT1, ColumnExprTrait_<const MT2> >;
   //**********************************************************************************************
};
/*! \endcond */
//*************************************************************************************************

} // namespace blaze

#endif
