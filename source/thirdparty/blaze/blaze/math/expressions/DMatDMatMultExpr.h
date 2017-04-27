//=================================================================================================
/*!
//  \file blaze/math/expressions/DMatDMatMultExpr.h
//  \brief Header file for the dense matrix/dense matrix multiplication expression
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

#ifndef _BLAZE_MATH_EXPRESSIONS_DMATDMATMULTEXPR_H_
#define _BLAZE_MATH_EXPRESSIONS_DMATDMATMULTEXPR_H_


//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <blaze/math/blas/gemm.h>
#include <blaze/math/blas/trmm.h>
#include <blaze/math/Aliases.h>
#include <blaze/math/constraints/ColumnMajorMatrix.h>
#include <blaze/math/constraints/DenseMatrix.h>
#include <blaze/math/constraints/MatMatMultExpr.h>
#include <blaze/math/constraints/RowMajorMatrix.h>
#include <blaze/math/constraints/StorageOrder.h>
#include <blaze/math/constraints/Symmetric.h>
#include <blaze/math/dense/MMM.h>
#include <blaze/math/Exception.h>
#include <blaze/math/expressions/Computation.h>
#include <blaze/math/expressions/DenseMatrix.h>
#include <blaze/math/expressions/DenseVector.h>
#include <blaze/math/expressions/Forward.h>
#include <blaze/math/expressions/MatMatMultExpr.h>
#include <blaze/math/expressions/MatScalarMultExpr.h>
#include <blaze/math/expressions/SparseVector.h>
#include <blaze/math/Functions.h>
#include <blaze/math/functors/DeclDiag.h>
#include <blaze/math/functors/DeclHerm.h>
#include <blaze/math/functors/DeclLow.h>
#include <blaze/math/functors/DeclSym.h>
#include <blaze/math/functors/DeclUpp.h>
#include <blaze/math/functors/Noop.h>
#include <blaze/math/shims/Conjugate.h>
#include <blaze/math/shims/Reset.h>
#include <blaze/math/shims/Serial.h>
#include <blaze/math/SIMD.h>
#include <blaze/math/traits/ColumnExprTrait.h>
#include <blaze/math/traits/DMatDeclDiagExprTrait.h>
#include <blaze/math/traits/DMatDeclHermExprTrait.h>
#include <blaze/math/traits/DMatDeclLowExprTrait.h>
#include <blaze/math/traits/DMatDeclSymExprTrait.h>
#include <blaze/math/traits/DMatDeclUppExprTrait.h>
#include <blaze/math/traits/DMatDVecMultExprTrait.h>
#include <blaze/math/traits/DMatSVecMultExprTrait.h>
#include <blaze/math/traits/MultExprTrait.h>
#include <blaze/math/traits/MultTrait.h>
#include <blaze/math/traits/RowExprTrait.h>
#include <blaze/math/traits/SubmatrixExprTrait.h>
#include <blaze/math/traits/TDVecDMatMultExprTrait.h>
#include <blaze/math/traits/TSVecDMatMultExprTrait.h>
#include <blaze/math/typetraits/Columns.h>
#include <blaze/math/typetraits/HasConstDataAccess.h>
#include <blaze/math/typetraits/HasMutableDataAccess.h>
#include <blaze/math/typetraits/HasSIMDAdd.h>
#include <blaze/math/typetraits/HasSIMDMult.h>
#include <blaze/math/typetraits/IsAligned.h>
#include <blaze/math/typetraits/IsBLASCompatible.h>
#include <blaze/math/typetraits/IsColumnMajorMatrix.h>
#include <blaze/math/typetraits/IsColumnVector.h>
#include <blaze/math/typetraits/IsComputation.h>
#include <blaze/math/typetraits/IsDenseMatrix.h>
#include <blaze/math/typetraits/IsDenseVector.h>
#include <blaze/math/typetraits/IsDiagonal.h>
#include <blaze/math/typetraits/IsExpression.h>
#include <blaze/math/typetraits/IsLower.h>
#include <blaze/math/typetraits/IsResizable.h>
#include <blaze/math/typetraits/IsRowMajorMatrix.h>
#include <blaze/math/typetraits/IsRowVector.h>
#include <blaze/math/typetraits/IsSIMDCombinable.h>
#include <blaze/math/typetraits/IsSparseVector.h>
#include <blaze/math/typetraits/IsStrictlyLower.h>
#include <blaze/math/typetraits/IsStrictlyTriangular.h>
#include <blaze/math/typetraits/IsStrictlyUpper.h>
#include <blaze/math/typetraits/IsSymmetric.h>
#include <blaze/math/typetraits/IsTriangular.h>
#include <blaze/math/typetraits/IsUniLower.h>
#include <blaze/math/typetraits/IsUniUpper.h>
#include <blaze/math/typetraits/IsUpper.h>
#include <blaze/math/typetraits/RequiresEvaluation.h>
#include <blaze/math/typetraits/Rows.h>
#include <blaze/system/BLAS.h>
#include <blaze/system/Blocking.h>
#include <blaze/system/Debugging.h>
#include <blaze/system/Optimizations.h>
#include <blaze/system/Thresholds.h>
#include <blaze/util/Assert.h>
#include <blaze/util/Complex.h>
#include <blaze/util/constraints/Numeric.h>
#include <blaze/util/constraints/Reference.h>
#include <blaze/util/constraints/SameType.h>
#include <blaze/util/DisableIf.h>
#include <blaze/util/EnableIf.h>
#include <blaze/util/FunctionTrace.h>
#include <blaze/util/IntegralConstant.h>
#include <blaze/util/InvalidType.h>
#include <blaze/util/mpl/And.h>
#include <blaze/util/mpl/Bool.h>
#include <blaze/util/mpl/If.h>
#include <blaze/util/mpl/Not.h>
#include <blaze/util/mpl/Or.h>
#include <blaze/util/TrueType.h>
#include <blaze/util/Types.h>
#include <blaze/util/typetraits/IsBuiltin.h>
#include <blaze/util/typetraits/IsComplex.h>
#include <blaze/util/typetraits/IsComplexDouble.h>
#include <blaze/util/typetraits/IsComplexFloat.h>
#include <blaze/util/typetraits/IsDouble.h>
#include <blaze/util/typetraits/IsFloat.h>
#include <blaze/util/typetraits/IsIntegral.h>
#include <blaze/util/typetraits/IsNumeric.h>
#include <blaze/util/typetraits/IsSame.h>


namespace blaze {

//=================================================================================================
//
//  CLASS DMATDMATMULTEXPR
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Expression object for dense matrix-dense matrix multiplications.
// \ingroup dense_matrix_expression
//
// The DMatDMatMultExpr class represents the compile time expression for multiplications between
// row-major dense matrices.
*/
template< typename MT1  // Type of the left-hand side dense matrix
        , typename MT2  // Type of the right-hand side dense matrix
        , bool SF       // Symmetry flag
        , bool HF       // Hermitian flag
        , bool LF       // Lower flag
        , bool UF >     // Upper flag
class DMatDMatMultExpr : public DenseMatrix< DMatDMatMultExpr<MT1,MT2,SF,HF,LF,UF>, false >
                       , private MatMatMultExpr
                       , private Computation
{
 private:
   //**Type definitions****************************************************************************
   typedef ResultType_<MT1>     RT1;  //!< Result type of the left-hand side dense matrix expression.
   typedef ResultType_<MT2>     RT2;  //!< Result type of the right-hand side dense matrix expression.
   typedef ElementType_<RT1>    ET1;  //!< Element type of the left-hand side dense matrix expression.
   typedef ElementType_<RT2>    ET2;  //!< Element type of the right-hand side dense matrix expression.
   typedef CompositeType_<MT1>  CT1;  //!< Composite type of the left-hand side dense matrix expression.
   typedef CompositeType_<MT2>  CT2;  //!< Composite type of the right-hand side dense matrix expression.
   //**********************************************************************************************

   //**********************************************************************************************
   //! Compilation switch for the composite type of the left-hand side dense matrix expression.
   enum : bool { evaluateLeft = IsComputation<MT1>::value || RequiresEvaluation<MT1>::value };
   //**********************************************************************************************

   //**********************************************************************************************
   //! Compilation switch for the composite type of the right-hand side dense matrix expression.
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
   /*! The CanExploitSymmetry struct is a helper struct for the selection of the optimal
       evaluation strategy. In case the target matrix is column-major and either of the
       two matrix operands is symmetric, \a value is set to 1 and an optimized evaluation
       strategy is selected. Otherwise \a value is set to 0 and the default strategy is
       chosen. */
   template< typename T1, typename T2, typename T3 >
   struct CanExploitSymmetry {
      enum : bool { value = IsColumnMajorMatrix<T1>::value &&
                            ( IsSymmetric<T2>::value || IsSymmetric<T3>::value ) };
   };
   /*! \endcond */
   //**********************************************************************************************

   //**********************************************************************************************
   /*! \cond BLAZE_INTERNAL */
   //! Helper structure for the explicit application of the SFINAE principle.
   /*! The IsEvaluationRequired struct is a helper struct for the selection of the parallel
       evaluation strategy. In case either of the two matrix operands requires an intermediate
       evaluation, the nested \value will be set to 1, otherwise it will be 0. */
   template< typename T1, typename T2, typename T3 >
   struct IsEvaluationRequired {
      enum : bool { value = ( evaluateLeft || evaluateRight ) &&
                            !CanExploitSymmetry<T1,T2,T3>::value };
   };
   /*! \endcond */
   //**********************************************************************************************

   //**********************************************************************************************
   /*! \cond BLAZE_INTERNAL */
   //! Helper structure for the explicit application of the SFINAE principle.
   /*! In case the types of all three involved matrices are suited for a BLAS kernel, the nested
       \a value will be set to 1, otherwise it will be 0. */
   template< typename T1, typename T2, typename T3 >
   struct UseBlasKernel {
      enum : bool { value = BLAZE_BLAS_MODE && BLAZE_USE_BLAS_MATRIX_MATRIX_MULTIPLICATION &&
                            !SYM && !HERM && !LOW && !UPP &&
                            HasMutableDataAccess<T1>::value &&
                            HasConstDataAccess<T2>::value &&
                            HasConstDataAccess<T3>::value &&
                            !IsDiagonal<T2>::value && !IsDiagonal<T3>::value &&
                            T1::simdEnabled && T2::simdEnabled && T3::simdEnabled &&
                            IsBLASCompatible< ElementType_<T1> >::value &&
                            IsBLASCompatible< ElementType_<T2> >::value &&
                            IsBLASCompatible< ElementType_<T3> >::value &&
                            IsSame< ElementType_<T1>, ElementType_<T2> >::value &&
                            IsSame< ElementType_<T1>, ElementType_<T3> >::value };
   };
   /*! \endcond */
   //**********************************************************************************************

   //**********************************************************************************************
   /*! \cond BLAZE_INTERNAL */
   //! Helper structure for the explicit application of the SFINAE principle.
   /*! In case all three involved data types are suited for a vectorized computation of the
       matrix multiplication, the nested \value will be set to 1, otherwise it will be 0. */
   template< typename T1, typename T2, typename T3 >
   struct UseVectorizedDefaultKernel {
      enum : bool { value = useOptimizedKernels &&
                            !IsDiagonal<T2>::value && !IsDiagonal<T3>::value &&
                            T1::simdEnabled && T2::simdEnabled && T3::simdEnabled &&
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
   //! Type of this DMatDMatMultExpr instance.
   typedef DMatDMatMultExpr<MT1,MT2,SF,HF,LF,UF>  This;

   typedef MultTrait_<RT1,RT2>         ResultType;     //!< Result type for expression template evaluations.
   typedef OppositeType_<ResultType>   OppositeType;   //!< Result type with opposite storage order for expression template evaluations.
   typedef TransposeType_<ResultType>  TransposeType;  //!< Transpose type for expression template evaluations.
   typedef ElementType_<ResultType>    ElementType;    //!< Resulting element type.
   typedef SIMDTrait_<ElementType>     SIMDType;       //!< Resulting SIMD element type.
   typedef const ElementType           ReturnType;     //!< Return type for expression template evaluations.
   typedef const ResultType            CompositeType;  //!< Data type for composite expression templates.

   //! Composite type of the left-hand side dense matrix expression.
   typedef If_< IsExpression<MT1>, const MT1, const MT1& >  LeftOperand;

   //! Composite type of the right-hand side dense matrix expression.
   typedef If_< IsExpression<MT2>, const MT2, const MT2& >  RightOperand;

   //! Type for the assignment of the left-hand side dense matrix operand.
   typedef IfTrue_< evaluateLeft, const RT1, CT1 >  LT;

   //! Type for the assignment of the right-hand side dense matrix operand.
   typedef IfTrue_< evaluateRight, const RT2, CT2 >  RT;
   //**********************************************************************************************

   //**Compilation flags***************************************************************************
   //! Compilation switch for the expression template evaluation strategy.
   enum : bool { simdEnabled = !IsDiagonal<MT2>::value &&
                               MT1::simdEnabled && MT2::simdEnabled &&
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
   /*!\brief Constructor for the DMatDMatMultExpr class.
   //
   // \param lhs The left-hand side operand of the multiplication expression.
   // \param rhs The right-hand side operand of the multiplication expression.
   */
   explicit inline DMatDMatMultExpr( const MT1& lhs, const MT2& rhs ) noexcept
      : lhs_( lhs )  // Left-hand side dense matrix of the multiplication expression
      , rhs_( rhs )  // Right-hand side dense matrix of the multiplication expression
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
   /*!\brief Returns the left-hand side dense matrix operand.
   //
   // \return The left-hand side dense matrix operand.
   */
   inline LeftOperand leftOperand() const noexcept {
      return lhs_;
   }
   //**********************************************************************************************

   //**Right operand access************************************************************************
   /*!\brief Returns the right-hand side dense matrix operand.
   //
   // \return The right-hand side dense matrix operand.
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
      return ( lhs_.canAlias( alias ) || rhs_.canAlias( alias ) );
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
      return lhs_.isAligned() && rhs_.isAligned();
   }
   //**********************************************************************************************

   //**********************************************************************************************
   /*!\brief Returns whether the expression can be used in SMP assignments.
   //
   // \return \a true in case the expression can be used in SMP assignments, \a false if not.
   */
   inline bool canSMPAssign() const noexcept {
      return ( !BLAZE_BLAS_IS_PARALLEL ||
               ( rows() * columns() < DMATDMATMULT_THRESHOLD ) ) &&
             ( rows() * columns() >= SMP_DMATDMATMULT_THRESHOLD ) &&
             !IsDiagonal<MT1>::value && !IsDiagonal<MT2>::value;
   }
   //**********************************************************************************************

 private:
   //**Member variables****************************************************************************
   LeftOperand  lhs_;  //!< Left-hand side dense matrix of the multiplication expression.
   RightOperand rhs_;  //!< Right-hand side dense matrix of the multiplication expression.
   //**********************************************************************************************

   //**Assignment to dense matrices****************************************************************
   /*! \cond BLAZE_INTERNAL */
   /*!\brief Assignment of a dense matrix-dense matrix multiplication to a dense matrix
   //        (\f$ C=A*B \f$).
   // \ingroup dense_matrix
   //
   // \param lhs The target left-hand side dense matrix.
   // \param rhs The right-hand side multiplication expression to be assigned.
   // \return void
   //
   // This function implements the performance optimized assignment of a dense matrix-dense
   // matrix multiplication expression to a dense matrix.
   */
   template< typename MT  // Type of the target dense matrix
           , bool SO >    // Storage order of the target dense matrix
   friend inline DisableIf_< CanExploitSymmetry<MT,MT1,MT2> >
      assign( DenseMatrix<MT,SO>& lhs, const DMatDMatMultExpr& rhs )
   {
      BLAZE_FUNCTION_TRACE;

      BLAZE_INTERNAL_ASSERT( (~lhs).rows()    == rhs.rows()   , "Invalid number of rows"    );
      BLAZE_INTERNAL_ASSERT( (~lhs).columns() == rhs.columns(), "Invalid number of columns" );

      if( (~lhs).rows() == 0UL || (~lhs).columns() == 0UL ) {
         return;
      }
      else if( rhs.lhs_.columns() == 0UL ) {
         reset( ~lhs );
         return;
      }

      LT A( serial( rhs.lhs_ ) );  // Evaluation of the left-hand side dense matrix operand
      RT B( serial( rhs.rhs_ ) );  // Evaluation of the right-hand side dense matrix operand

      BLAZE_INTERNAL_ASSERT( A.rows()    == rhs.lhs_.rows()   , "Invalid number of rows"    );
      BLAZE_INTERNAL_ASSERT( A.columns() == rhs.lhs_.columns(), "Invalid number of columns" );
      BLAZE_INTERNAL_ASSERT( B.rows()    == rhs.rhs_.rows()   , "Invalid number of rows"    );
      BLAZE_INTERNAL_ASSERT( B.columns() == rhs.rhs_.columns(), "Invalid number of columns" );
      BLAZE_INTERNAL_ASSERT( A.rows()    == (~lhs).rows()     , "Invalid number of rows"    );
      BLAZE_INTERNAL_ASSERT( B.columns() == (~lhs).columns()  , "Invalid number of columns" );

      DMatDMatMultExpr::selectAssignKernel( ~lhs, A, B );
   }
   /*! \endcond */
   //**********************************************************************************************

   //**Assignment to dense matrices (kernel selection)*********************************************
   /*! \cond BLAZE_INTERNAL */
   /*!\brief Selection of the kernel for an assignment of a dense matrix-dense matrix
   //        multiplication to a dense matrix (\f$ C=A*B \f$).
   // \ingroup dense_matrix
   //
   // \param C The target left-hand side dense matrix.
   // \param A The left-hand side multiplication operand.
   // \param B The right-hand side multiplication operand.
   // \return void
   */
   template< typename MT3    // Type of the left-hand side target matrix
           , typename MT4    // Type of the left-hand side matrix operand
           , typename MT5 >  // Type of the right-hand side matrix operand
   static inline void selectAssignKernel( MT3& C, const MT4& A, const MT5& B )
   {
      if( ( IsDiagonal<MT5>::value ) ||
          ( !BLAZE_DEBUG_MODE && B.columns() <= SIMDSIZE*10UL ) ||
          ( C.rows() * C.columns() < DMATDMATMULT_THRESHOLD ) )
         selectSmallAssignKernel( C, A, B );
      else
         selectBlasAssignKernel( C, A, B );
   }
   /*! \endcond */
   //**********************************************************************************************

   //**Default assignment to dense matrices (general/general)**************************************
   /*! \cond BLAZE_INTERNAL */
   /*!\brief Default assignment of a general dense matrix-general dense matrix multiplication
   //        (\f$ C=A*B \f$).
   // \ingroup dense_matrix
   //
   // \param C The target left-hand side dense matrix.
   // \param A The left-hand side multiplication operand.
   // \param B The right-hand side multiplication operand.
   // \return void
   //
   // This function implements the default assignment of a general dense matrix-general dense
   // matrix multiplication expression to a dense matrix.
   */
   template< typename MT3    // Type of the left-hand side target matrix
           , typename MT4    // Type of the left-hand side matrix operand
           , typename MT5 >  // Type of the right-hand side matrix operand
   static inline EnableIf_< And< Not< IsDiagonal<MT4> >, Not< IsDiagonal<MT5> > > >
      selectDefaultAssignKernel( MT3& C, const MT4& A, const MT5& B )
   {
      const size_t M( A.rows()    );
      const size_t N( B.columns() );
      const size_t K( A.columns() );

      BLAZE_INTERNAL_ASSERT( !( SYM || HERM || LOW || UPP ) || ( M == N ), "Broken invariant detected" );

      for( size_t i=0UL; i<M; ++i )
      {
         const size_t kbegin( ( IsUpper<MT4>::value )
                              ?( IsStrictlyUpper<MT4>::value ? i+1UL : i )
                              :( 0UL ) );
         const size_t kend( ( IsLower<MT4>::value )
                            ?( IsStrictlyLower<MT4>::value ? i : i+1UL )
                            :( K ) );
         BLAZE_INTERNAL_ASSERT( kbegin <= kend, "Invalid loop indices detected" );

         if( IsStrictlyTriangular<MT4>::value && kbegin == kend ) {
            for( size_t j=0UL; j<N; ++j ) {
               reset( C(i,j) );
            }
            continue;
         }

         {
            const size_t jbegin( ( IsUpper<MT5>::value )
                                 ?( ( IsStrictlyUpper<MT5>::value )
                                    ?( UPP ? max(i,kbegin+1UL) : kbegin+1UL )
                                    :( UPP ? max(i,kbegin) : kbegin ) )
                                 :( UPP ? i : 0UL ) );
            const size_t jend( ( IsLower<MT5>::value )
                               ?( ( IsStrictlyLower<MT5>::value )
                                  ?( LOW ? min(i+1UL,kbegin) : kbegin )
                                  :( LOW ? min(i,kbegin)+1UL : kbegin+1UL ) )
                               :( LOW ? i+1UL : N ) );

            if( ( IsUpper<MT4>::value && IsUpper<MT5>::value ) || UPP ) {
               for( size_t j=0UL; j<jbegin; ++j ) {
                  reset( C(i,j) );
               }
            }
            else if( IsStrictlyUpper<MT5>::value ) {
               reset( C(i,0UL) );
            }
            for( size_t j=jbegin; j<jend; ++j ) {
               C(i,j) = A(i,kbegin) * B(kbegin,j);
            }
            if( ( IsLower<MT4>::value && IsLower<MT5>::value ) || LOW ) {
               for( size_t j=jend; j<N; ++j ) {
                  reset( C(i,j) );
               }
            }
            else if( IsStrictlyLower<MT5>::value ) {
               reset( C(i,N-1UL) );
            }
         }

         for( size_t k=kbegin+1UL; k<kend; ++k )
         {
            const size_t jbegin( ( IsUpper<MT5>::value )
                                 ?( ( IsStrictlyUpper<MT5>::value )
                                    ?( SYM || HERM || UPP ? max( i, k+1UL ) : k+1UL )
                                    :( SYM || HERM || UPP ? max( i, k ) : k ) )
                                 :( SYM || HERM || UPP ? i : 0UL ) );
            const size_t jend( ( IsLower<MT5>::value )
                               ?( ( IsStrictlyLower<MT5>::value )
                                  ?( LOW ? min(i+1UL,k-1UL) : k-1UL )
                                  :( LOW ? min(i+1UL,k) : k ) )
                               :( LOW ? i+1UL : N ) );

            if( ( SYM || HERM || LOW || UPP ) && ( jbegin > jend ) ) continue;
            BLAZE_INTERNAL_ASSERT( jbegin <= jend, "Invalid loop indices detected" );

            for( size_t j=jbegin; j<jend; ++j ) {
               C(i,j) += A(i,k) * B(k,j);
            }
            if( IsLower<MT5>::value ) {
               C(i,jend) = A(i,k) * B(k,jend);
            }
         }
      }

      if( SYM || HERM ) {
         for( size_t i=1UL; i<M; ++i ) {
            for( size_t j=0UL; j<i; ++j ) {
               C(i,j) = HERM ? conj( C(j,i) ) : C(j,i);
            }
         }
      }
   }
   /*! \endcond */
   //**********************************************************************************************

   //**Default assignment to dense matrices (general/diagonal)*************************************
   /*! \cond BLAZE_INTERNAL */
   /*!\brief Default assignment of a general dense matrix-diagonal dense matrix multiplication
   //        (\f$ C=A*B \f$).
   // \ingroup dense_matrix
   //
   // \param C The target left-hand side dense matrix.
   // \param A The left-hand side multiplication operand.
   // \param B The right-hand side multiplication operand.
   // \return void
   //
   // This function implements the default assignment of a general dense matrix-diagonal dense
   // matrix multiplication expression to a dense matrix.
   */
   template< typename MT3    // Type of the left-hand side target matrix
           , typename MT4    // Type of the left-hand side matrix operand
           , typename MT5 >  // Type of the right-hand side matrix operand
   static inline EnableIf_< And< Not< IsDiagonal<MT4> >, IsDiagonal<MT5> > >
      selectDefaultAssignKernel( MT3& C, const MT4& A, const MT5& B )
   {
      BLAZE_CONSTRAINT_MUST_BE_ROW_MAJOR_MATRIX_TYPE( MT3 );

      const size_t M( A.rows()    );
      const size_t N( B.columns() );

      for( size_t i=0UL; i<M; ++i )
      {
         const size_t jbegin( ( IsUpper<MT4>::value )
                              ?( IsStrictlyUpper<MT4>::value ? i+1UL : i )
                              :( 0UL ) );
         const size_t jend( ( IsLower<MT4>::value )
                            ?( IsStrictlyLower<MT4>::value ? i : i+1UL )
                            :( N ) );
         BLAZE_INTERNAL_ASSERT( jbegin <= jend, "Invalid loop indices detected" );

         if( IsUpper<MT4>::value ) {
            for( size_t j=0UL; j<jbegin; ++j ) {
               reset( C(i,j) );
            }
         }
         for( size_t j=jbegin; j<jend; ++j ) {
            C(i,j) = A(i,j) * B(j,j);
         }
         if( IsLower<MT4>::value ) {
            for( size_t j=jend; j<N; ++j ) {
               reset( C(i,j) );
            }
         }
      }
   }
   /*! \endcond */
   //**********************************************************************************************

   //**Default assignment to dense matrices (diagonal/general)*************************************
   /*! \cond BLAZE_INTERNAL */
   /*!\brief Default assignment of a diagonal dense matrix-general dense matrix multiplication
   //        (\f$ C=A*B \f$).
   // \ingroup dense_matrix
   //
   // \param C The target left-hand side dense matrix.
   // \param A The left-hand side multiplication operand.
   // \param B The right-hand side multiplication operand.
   // \return void
   //
   // This function implements the default assignment of a diagonal dense matrix-general dense
   // matrix multiplication expression to a dense matrix.
   */
   template< typename MT3    // Type of the left-hand side target matrix
           , typename MT4    // Type of the left-hand side matrix operand
           , typename MT5 >  // Type of the right-hand side matrix operand
   static inline EnableIf_< And< IsDiagonal<MT4>, Not< IsDiagonal<MT5> > > >
      selectDefaultAssignKernel( MT3& C, const MT4& A, const MT5& B )
   {
      BLAZE_CONSTRAINT_MUST_BE_ROW_MAJOR_MATRIX_TYPE( MT3 );

      const size_t M( A.rows()    );
      const size_t N( B.columns() );

      for( size_t i=0UL; i<M; ++i )
      {
         const size_t jbegin( ( IsUpper<MT5>::value )
                              ?( IsStrictlyUpper<MT5>::value ? i+1UL : i )
                              :( 0UL ) );
         const size_t jend( ( IsLower<MT5>::value )
                            ?( IsStrictlyLower<MT5>::value ? i : i+1UL )
                            :( N ) );
         BLAZE_INTERNAL_ASSERT( jbegin <= jend, "Invalid loop indices detected" );

         if( IsUpper<MT5>::value ) {
            for( size_t j=0UL; j<jbegin; ++j ) {
               reset( C(i,j) );
            }
         }
         for( size_t j=jbegin; j<jend; ++j ) {
            C(i,j) = A(i,i) * B(i,j);
         }
         if( IsLower<MT5>::value ) {
            for( size_t j=jend; j<N; ++j ) {
               reset( C(i,j) );
            }
         }
      }
   }
   /*! \endcond */
   //**********************************************************************************************

   //**Default assignment to dense matrices (diagonal/diagonal)************************************
   /*! \cond BLAZE_INTERNAL */
   /*!\brief Default assignment of a diagonal dense matrix-diagonal dense matrix multiplication
   //        (\f$ C=A*B \f$).
   // \ingroup dense_matrix
   //
   // \param C The target left-hand side dense matrix.
   // \param A The left-hand side multiplication operand.
   // \param B The right-hand side multiplication operand.
   // \return void
   //
   // This function implements the default assignment of a diagonal dense matrix-diagonal dense
   // matrix multiplication expression to a dense matrix.
   */
   template< typename MT3    // Type of the left-hand side target matrix
           , typename MT4    // Type of the left-hand side matrix operand
           , typename MT5 >  // Type of the right-hand side matrix operand
   static inline EnableIf_< And< IsDiagonal<MT4>, IsDiagonal<MT5> > >
      selectDefaultAssignKernel( MT3& C, const MT4& A, const MT5& B )
   {
      BLAZE_CONSTRAINT_MUST_BE_ROW_MAJOR_MATRIX_TYPE( MT3 );

      reset( C );

      for( size_t i=0UL; i<A.rows(); ++i ) {
         C(i,i) = A(i,i) * B(i,i);
      }
   }
   /*! \endcond */
   //**********************************************************************************************

   //**Default assignment to dense matrices (small matrices)***************************************
   /*! \cond BLAZE_INTERNAL */
   /*!\brief Default assignment of a small dense matrix-dense matrix multiplication (\f$ C=A*B \f$).
   // \ingroup dense_matrix
   //
   // \param C The target left-hand side dense matrix.
   // \param A The left-hand side multiplication operand.
   // \param B The right-hand side multiplication operand.
   // \return void
   //
   // This function relays to the default implementation of the assignment of a dense matrix-
   // dense matrix multiplication expression to a dense matrix.
   */
   template< typename MT3    // Type of the left-hand side target matrix
           , typename MT4    // Type of the left-hand side matrix operand
           , typename MT5 >  // Type of the right-hand side matrix operand
   static inline DisableIf_< UseVectorizedDefaultKernel<MT3,MT4,MT5> >
      selectSmallAssignKernel( MT3& C, const MT4& A, const MT5& B )
   {
      selectDefaultAssignKernel( C, A, B );
   }
   /*! \endcond */
   //**********************************************************************************************

   //**Vectorized default assignment to row-major dense matrices (small matrices)******************
   /*! \cond BLAZE_INTERNAL */
   /*!\brief Vectorized default assignment of a small dense matrix-dense matrix multiplication
   //        (\f$ C=A*B \f$).
   // \ingroup dense_matrix
   //
   // \param C The target left-hand side dense matrix.
   // \param A The left-hand side multiplication operand.
   // \param B The right-hand side multiplication operand.
   // \return void
   //
   // This function implements the vectorized default assignment of a dense matrix-dense matrix
   // multiplication expression to a row-major dense matrix. This kernel is optimized for small
   // matrices.
   */
   template< typename MT3    // Type of the left-hand side target matrix
           , typename MT4    // Type of the left-hand side matrix operand
           , typename MT5 >  // Type of the right-hand side matrix operand
   static inline EnableIf_< UseVectorizedDefaultKernel<MT3,MT4,MT5> >
      selectSmallAssignKernel( DenseMatrix<MT3,false>& C, const MT4& A, const MT5& B )
   {
      constexpr bool remainder( !IsPadded<MT3>::value || !IsPadded<MT5>::value );

      const size_t M( A.rows()    );
      const size_t N( B.columns() );
      const size_t K( A.columns() );

      BLAZE_INTERNAL_ASSERT( !( SYM || HERM || LOW || UPP ) || ( M == N ), "Broken invariant detected" );

      const size_t jpos( remainder ? ( N & size_t(-SIMDSIZE) ) : N );
      BLAZE_INTERNAL_ASSERT( !remainder || ( N - ( N % SIMDSIZE ) ) == jpos, "Invalid end calculation" );

      if( LOW && UPP && N > SIMDSIZE*3UL ) {
         reset( ~C );
      }

      {
         size_t j( 0UL );

         if( IsIntegral<ElementType>::value )
         {
            for( ; !SYM && !HERM && !LOW && !UPP && (j+SIMDSIZE*7UL) < jpos; j+=SIMDSIZE*8UL ) {
               for( size_t i=0UL; i<M; ++i )
               {
                  const size_t kbegin( ( IsUpper<MT4>::value )
                                       ?( ( IsLower<MT5>::value )
                                          ?( max( ( IsStrictlyUpper<MT4>::value ? i+1UL : i ), j ) )
                                          :( IsStrictlyUpper<MT4>::value ? i+1UL : i ) )
                                       :( IsLower<MT5>::value ? j : 0UL ) );
                  const size_t kend( ( IsLower<MT4>::value )
                                     ?( ( IsUpper<MT5>::value )
                                        ?( min( ( IsStrictlyLower<MT4>::value ? i : i+1UL ), j+SIMDSIZE*8UL, K ) )
                                        :( IsStrictlyLower<MT4>::value ? i : i+1UL ) )
                                     :( IsUpper<MT5>::value ? min( j+SIMDSIZE*8UL, K ) : K ) );

                  SIMDType xmm1, xmm2, xmm3, xmm4, xmm5, xmm6, xmm7, xmm8;

                  for( size_t k=kbegin; k<kend; ++k ) {
                     const SIMDType a1( set( A(i,k) ) );
                     xmm1 += a1 * B.load(k,j             );
                     xmm2 += a1 * B.load(k,j+SIMDSIZE    );
                     xmm3 += a1 * B.load(k,j+SIMDSIZE*2UL);
                     xmm4 += a1 * B.load(k,j+SIMDSIZE*3UL);
                     xmm5 += a1 * B.load(k,j+SIMDSIZE*4UL);
                     xmm6 += a1 * B.load(k,j+SIMDSIZE*5UL);
                     xmm7 += a1 * B.load(k,j+SIMDSIZE*6UL);
                     xmm8 += a1 * B.load(k,j+SIMDSIZE*7UL);
                  }

                  (~C).store( i, j             , xmm1 );
                  (~C).store( i, j+SIMDSIZE    , xmm2 );
                  (~C).store( i, j+SIMDSIZE*2UL, xmm3 );
                  (~C).store( i, j+SIMDSIZE*3UL, xmm4 );
                  (~C).store( i, j+SIMDSIZE*4UL, xmm5 );
                  (~C).store( i, j+SIMDSIZE*5UL, xmm6 );
                  (~C).store( i, j+SIMDSIZE*6UL, xmm7 );
                  (~C).store( i, j+SIMDSIZE*7UL, xmm8 );
               }
            }
         }

         for( ; !SYM && !HERM && !LOW && !UPP && (j+SIMDSIZE*4UL) < jpos; j+=SIMDSIZE*5UL )
         {
            size_t i( 0UL );

            for( ; (i+2UL) <= M; i+=2UL )
            {
               const size_t kbegin( ( IsUpper<MT4>::value )
                                    ?( ( IsLower<MT5>::value )
                                       ?( max( ( IsStrictlyUpper<MT4>::value ? i+1UL : i ), j ) )
                                       :( IsStrictlyUpper<MT4>::value ? i+1UL : i ) )
                                    :( IsLower<MT5>::value ? j : 0UL ) );
               const size_t kend( ( IsLower<MT4>::value )
                                  ?( ( IsUpper<MT5>::value )
                                     ?( min( ( IsStrictlyLower<MT4>::value ? i+1UL : i+2UL ), j+SIMDSIZE*5UL, K ) )
                                     :( IsStrictlyLower<MT4>::value ? i+1UL : i+2UL ) )
                                  :( IsUpper<MT5>::value ? min( j+SIMDSIZE*5UL, K ) : K ) );

               SIMDType xmm1, xmm2, xmm3, xmm4, xmm5, xmm6, xmm7, xmm8, xmm9, xmm10;

               for( size_t k=kbegin; k<kend; ++k ) {
                  const SIMDType a1( set( A(i    ,k) ) );
                  const SIMDType a2( set( A(i+1UL,k) ) );
                  const SIMDType b1( B.load(k,j             ) );
                  const SIMDType b2( B.load(k,j+SIMDSIZE    ) );
                  const SIMDType b3( B.load(k,j+SIMDSIZE*2UL) );
                  const SIMDType b4( B.load(k,j+SIMDSIZE*3UL) );
                  const SIMDType b5( B.load(k,j+SIMDSIZE*4UL) );
                  xmm1  += a1 * b1;
                  xmm2  += a1 * b2;
                  xmm3  += a1 * b3;
                  xmm4  += a1 * b4;
                  xmm5  += a1 * b5;
                  xmm6  += a2 * b1;
                  xmm7  += a2 * b2;
                  xmm8  += a2 * b3;
                  xmm9  += a2 * b4;
                  xmm10 += a2 * b5;
               }

               (~C).store( i    , j             , xmm1  );
               (~C).store( i    , j+SIMDSIZE    , xmm2  );
               (~C).store( i    , j+SIMDSIZE*2UL, xmm3  );
               (~C).store( i    , j+SIMDSIZE*3UL, xmm4  );
               (~C).store( i    , j+SIMDSIZE*4UL, xmm5  );
               (~C).store( i+1UL, j             , xmm6  );
               (~C).store( i+1UL, j+SIMDSIZE    , xmm7  );
               (~C).store( i+1UL, j+SIMDSIZE*2UL, xmm8  );
               (~C).store( i+1UL, j+SIMDSIZE*3UL, xmm9  );
               (~C).store( i+1UL, j+SIMDSIZE*4UL, xmm10 );
            }

            if( i < M )
            {
               const size_t kbegin( ( IsUpper<MT4>::value )
                                    ?( ( IsLower<MT5>::value )
                                       ?( max( ( IsStrictlyUpper<MT4>::value ? i+1UL : i ), j ) )
                                       :( IsStrictlyUpper<MT4>::value ? i+1UL : i ) )
                                    :( IsLower<MT5>::value ? j : 0UL ) );
               const size_t kend( ( IsUpper<MT5>::value )?( min( j+SIMDSIZE*5UL, K ) ):( K ) );

               SIMDType xmm1, xmm2, xmm3, xmm4, xmm5;

               for( size_t k=kbegin; k<kend; ++k ) {
                  const SIMDType a1( set( A(i,k) ) );
                  xmm1 += a1 * B.load(k,j             );
                  xmm2 += a1 * B.load(k,j+SIMDSIZE    );
                  xmm3 += a1 * B.load(k,j+SIMDSIZE*2UL);
                  xmm4 += a1 * B.load(k,j+SIMDSIZE*3UL);
                  xmm5 += a1 * B.load(k,j+SIMDSIZE*4UL);
               }

               (~C).store( i, j             , xmm1 );
               (~C).store( i, j+SIMDSIZE    , xmm2 );
               (~C).store( i, j+SIMDSIZE*2UL, xmm3 );
               (~C).store( i, j+SIMDSIZE*3UL, xmm4 );
               (~C).store( i, j+SIMDSIZE*4UL, xmm5 );
            }
         }

         for( ; !( LOW && UPP ) && (j+SIMDSIZE*3UL) < jpos; j+=SIMDSIZE*4UL )
         {
            const size_t iend( SYM || HERM || UPP ? min(j+SIMDSIZE*4UL,M) : M );
            size_t i( LOW ? j : 0UL );

            for( ; (i+2UL) <= iend; i+=2UL )
            {
               const size_t kbegin( ( IsUpper<MT4>::value )
                                    ?( ( IsLower<MT5>::value )
                                       ?( max( ( IsStrictlyUpper<MT4>::value ? i+1UL : i ), j ) )
                                       :( IsStrictlyUpper<MT4>::value ? i+1UL : i ) )
                                    :( IsLower<MT5>::value ? j : 0UL ) );
               const size_t kend( ( IsLower<MT4>::value )
                                  ?( ( IsUpper<MT5>::value )
                                     ?( min( ( IsStrictlyLower<MT4>::value ? i+1UL : i+2UL ), j+SIMDSIZE*4UL, K ) )
                                     :( IsStrictlyLower<MT4>::value ? i+1UL : i+2UL ) )
                                  :( IsUpper<MT5>::value ? min( j+SIMDSIZE*4UL, K ) : K ) );

               SIMDType xmm1, xmm2, xmm3, xmm4, xmm5, xmm6, xmm7, xmm8;

               for( size_t k=kbegin; k<kend; ++k ) {
                  const SIMDType a1( set( A(i    ,k) ) );
                  const SIMDType a2( set( A(i+1UL,k) ) );
                  const SIMDType b1( B.load(k,j             ) );
                  const SIMDType b2( B.load(k,j+SIMDSIZE    ) );
                  const SIMDType b3( B.load(k,j+SIMDSIZE*2UL) );
                  const SIMDType b4( B.load(k,j+SIMDSIZE*3UL) );
                  xmm1 += a1 * b1;
                  xmm2 += a1 * b2;
                  xmm3 += a1 * b3;
                  xmm4 += a1 * b4;
                  xmm5 += a2 * b1;
                  xmm6 += a2 * b2;
                  xmm7 += a2 * b3;
                  xmm8 += a2 * b4;
               }

               (~C).store( i    , j             , xmm1 );
               (~C).store( i    , j+SIMDSIZE    , xmm2 );
               (~C).store( i    , j+SIMDSIZE*2UL, xmm3 );
               (~C).store( i    , j+SIMDSIZE*3UL, xmm4 );
               (~C).store( i+1UL, j             , xmm5 );
               (~C).store( i+1UL, j+SIMDSIZE    , xmm6 );
               (~C).store( i+1UL, j+SIMDSIZE*2UL, xmm7 );
               (~C).store( i+1UL, j+SIMDSIZE*3UL, xmm8 );
            }

            if( i < iend )
            {
               const size_t kbegin( ( IsUpper<MT4>::value )
                                    ?( ( IsLower<MT5>::value )
                                       ?( max( ( IsStrictlyUpper<MT4>::value ? i+1UL : i ), j ) )
                                       :( IsStrictlyUpper<MT4>::value ? i+1UL : i ) )
                                    :( IsLower<MT5>::value ? j : 0UL ) );
               const size_t kend( ( IsUpper<MT5>::value )?( min( j+SIMDSIZE*4UL, K ) ):( K ) );

               SIMDType xmm1, xmm2, xmm3, xmm4;

               for( size_t k=kbegin; k<kend; ++k ) {
                  const SIMDType a1( set( A(i,k) ) );
                  xmm1 += a1 * B.load(k,j             );
                  xmm2 += a1 * B.load(k,j+SIMDSIZE    );
                  xmm3 += a1 * B.load(k,j+SIMDSIZE*2UL);
                  xmm4 += a1 * B.load(k,j+SIMDSIZE*3UL);
               }

               (~C).store( i, j             , xmm1 );
               (~C).store( i, j+SIMDSIZE    , xmm2 );
               (~C).store( i, j+SIMDSIZE*2UL, xmm3 );
               (~C).store( i, j+SIMDSIZE*3UL, xmm4 );
            }
         }

         for( ; (j+SIMDSIZE*2UL) < jpos; j+=SIMDSIZE*3UL )
         {
            const size_t iend( SYM || HERM || UPP ? min(j+SIMDSIZE*3UL,M) : M );
            size_t i( LOW ? j : 0UL );

            for( ; (i+2UL) <= iend; i+=2UL )
            {
               const size_t kbegin( ( IsUpper<MT4>::value )
                                    ?( ( IsLower<MT5>::value )
                                       ?( max( ( IsStrictlyUpper<MT4>::value ? i+1UL : i ), j ) )
                                       :( IsStrictlyUpper<MT4>::value ? i+1UL : i ) )
                                    :( IsLower<MT5>::value ? j : 0UL ) );
               const size_t kend( ( IsLower<MT4>::value )
                                  ?( ( IsUpper<MT5>::value )
                                     ?( min( ( IsStrictlyLower<MT4>::value ? i+1UL : i+2UL ), j+SIMDSIZE*3UL, K ) )
                                     :( IsStrictlyLower<MT4>::value ? i+1UL : i+2UL ) )
                                  :( IsUpper<MT5>::value ? min( j+SIMDSIZE*3UL, K ) : K ) );

               SIMDType xmm1, xmm2, xmm3, xmm4, xmm5, xmm6;

               for( size_t k=kbegin; k<kend; ++k ) {
                  const SIMDType a1( set( A(i    ,k) ) );
                  const SIMDType a2( set( A(i+1UL,k) ) );
                  const SIMDType b1( B.load(k,j             ) );
                  const SIMDType b2( B.load(k,j+SIMDSIZE    ) );
                  const SIMDType b3( B.load(k,j+SIMDSIZE*2UL) );
                  xmm1 += a1 * b1;
                  xmm2 += a1 * b2;
                  xmm3 += a1 * b3;
                  xmm4 += a2 * b1;
                  xmm5 += a2 * b2;
                  xmm6 += a2 * b3;
               }

               (~C).store( i    , j             , xmm1 );
               (~C).store( i    , j+SIMDSIZE    , xmm2 );
               (~C).store( i    , j+SIMDSIZE*2UL, xmm3 );
               (~C).store( i+1UL, j             , xmm4 );
               (~C).store( i+1UL, j+SIMDSIZE    , xmm5 );
               (~C).store( i+1UL, j+SIMDSIZE*2UL, xmm6 );
            }

            if( i < iend )
            {
               const size_t kbegin( ( IsUpper<MT4>::value )
                                    ?( ( IsLower<MT5>::value )
                                       ?( max( ( IsStrictlyUpper<MT4>::value ? i+1UL : i ), j ) )
                                       :( IsStrictlyUpper<MT4>::value ? i+1UL : i ) )
                                    :( IsLower<MT5>::value ? j : 0UL ) );
               const size_t kend( ( IsUpper<MT5>::value )?( min( j+SIMDSIZE*3UL, K ) ):( K ) );

               SIMDType xmm1, xmm2, xmm3;

               for( size_t k=kbegin; k<kend; ++k ) {
                  const SIMDType a1( set( A(i,k) ) );
                  xmm1 += a1 * B.load(k,j             );
                  xmm2 += a1 * B.load(k,j+SIMDSIZE    );
                  xmm3 += a1 * B.load(k,j+SIMDSIZE*2UL);
               }

               (~C).store( i, j             , xmm1 );
               (~C).store( i, j+SIMDSIZE    , xmm2 );
               (~C).store( i, j+SIMDSIZE*2UL, xmm3 );
            }
         }

         for( ; (j+SIMDSIZE) < jpos; j+=SIMDSIZE*2UL )
         {
            const size_t iend( SYM || HERM || UPP ? min(j+SIMDSIZE*2UL,M) : M );
            size_t i( LOW ? j : 0UL );

            for( ; (i+2UL) <= iend; i+=2UL )
            {
               const size_t kbegin( ( IsUpper<MT4>::value )
                                    ?( ( IsLower<MT5>::value )
                                       ?( max( ( IsStrictlyUpper<MT4>::value ? i+1UL : i ), j ) )
                                       :( IsStrictlyUpper<MT4>::value ? i+1UL : i ) )
                                    :( IsLower<MT5>::value ? j : 0UL ) );
               const size_t kend( ( IsLower<MT4>::value )
                                  ?( ( IsUpper<MT5>::value )
                                     ?( min( ( IsStrictlyLower<MT4>::value ? i+1UL : i+2UL ), j+SIMDSIZE*2UL, K ) )
                                     :( IsStrictlyLower<MT4>::value ? i+1UL : i+2UL ) )
                                  :( IsUpper<MT5>::value ? min( j+SIMDSIZE*2UL, K ) : K ) );

               SIMDType xmm1, xmm2, xmm3, xmm4;

               for( size_t k=kbegin; k<kend; ++k ) {
                  const SIMDType a1( set( A(i    ,k) ) );
                  const SIMDType a2( set( A(i+1UL,k) ) );
                  const SIMDType b1( B.load(k,j         ) );
                  const SIMDType b2( B.load(k,j+SIMDSIZE) );
                  xmm1 += a1 * b1;
                  xmm2 += a1 * b2;
                  xmm3 += a2 * b1;
                  xmm4 += a2 * b2;
               }

               (~C).store( i    , j         , xmm1 );
               (~C).store( i    , j+SIMDSIZE, xmm2 );
               (~C).store( i+1UL, j         , xmm3 );
               (~C).store( i+1UL, j+SIMDSIZE, xmm4 );
            }

            if( i < iend )
            {
               const size_t kbegin( ( IsUpper<MT4>::value )
                                    ?( ( IsLower<MT5>::value )
                                       ?( max( ( IsStrictlyUpper<MT4>::value ? i+1UL : i ), j ) )
                                       :( IsStrictlyUpper<MT4>::value ? i+1UL : i ) )
                                    :( IsLower<MT5>::value ? j : 0UL ) );
               const size_t kend( ( IsUpper<MT5>::value )?( min( j+SIMDSIZE*2UL, K ) ):( K ) );

               SIMDType xmm1, xmm2;

               for( size_t k=kbegin; k<kend; ++k ) {
                  const SIMDType a1( set( A(i,k) ) );
                  xmm1 += a1 * B.load(k,j         );
                  xmm2 += a1 * B.load(k,j+SIMDSIZE);
               }

               (~C).store( i, j         , xmm1 );
               (~C).store( i, j+SIMDSIZE, xmm2 );
            }
         }

         for( ; j<jpos; j+=SIMDSIZE )
         {
            const size_t iend( SYM || HERM || UPP ? min(j+SIMDSIZE,M) : M );
            size_t i( LOW ? j : 0UL );

            for( ; (i+2UL) <= iend; i+=2UL )
            {
               const size_t kbegin( ( IsUpper<MT4>::value )
                                    ?( ( IsLower<MT5>::value )
                                       ?( max( ( IsStrictlyUpper<MT4>::value ? i+1UL : i ), j ) )
                                       :( IsStrictlyUpper<MT4>::value ? i+1UL : i ) )
                                    :( IsLower<MT5>::value ? j : 0UL ) );
               const size_t kend( ( IsLower<MT4>::value )
                                  ?( IsStrictlyLower<MT4>::value ? i+1UL : i+2UL )
                                  :( K ) );

               SIMDType xmm1, xmm2;

               for( size_t k=kbegin; k<kend; ++k ) {
                  const SIMDType b1( B.load(k,j) );
                  xmm1 += set( A(i    ,k) ) * b1;
                  xmm2 += set( A(i+1UL,k) ) * b1;
               }

               (~C).store( i    , j, xmm1 );
               (~C).store( i+1UL, j, xmm2 );
            }

            if( i < iend )
            {
               const size_t kbegin( ( IsUpper<MT4>::value )
                                    ?( ( IsLower<MT5>::value )
                                       ?( max( ( IsStrictlyUpper<MT4>::value ? i+1UL : i ), j ) )
                                       :( IsStrictlyUpper<MT4>::value ? i+1UL : i ) )
                                    :( IsLower<MT5>::value ? j : 0UL ) );

               SIMDType xmm1;

               for( size_t k=kbegin; k<K; ++k ) {
                  xmm1 += set( A(i,k) ) * B.load(k,j);
               }

               (~C).store( i, j, xmm1 );
            }
         }

         for( ; remainder && j<N; ++j )
         {
            size_t i( LOW && UPP ? j : 0UL );

            for( ; (i+2UL) <= M; i+=2UL )
            {
               const size_t kbegin( ( IsUpper<MT4>::value )
                                    ?( ( IsLower<MT5>::value )
                                       ?( max( ( IsStrictlyUpper<MT4>::value ? i+1UL : i ), j ) )
                                       :( IsStrictlyUpper<MT4>::value ? i+1UL : i ) )
                                    :( IsLower<MT5>::value ? j : 0UL ) );
               const size_t kend( ( IsLower<MT4>::value )
                                  ?( IsStrictlyLower<MT4>::value ? i+1UL : i+2UL )
                                  :( K ) );

               ElementType value1 = ElementType();
               ElementType value2 = ElementType();

               for( size_t k=kbegin; k<kend; ++k ) {
                  value1 += A(i    ,k) * B(k,j);
                  value2 += A(i+1UL,k) * B(k,j);
               }

               (~C)(i    ,j) = value1;
               (~C)(i+1UL,j) = value2;
            }

            if( i < M )
            {
               const size_t kbegin( ( IsUpper<MT4>::value )
                                    ?( ( IsLower<MT5>::value )
                                       ?( max( ( IsStrictlyUpper<MT4>::value ? i+1UL : i ), j ) )
                                       :( IsStrictlyUpper<MT4>::value ? i+1UL : i ) )
                                    :( IsLower<MT5>::value ? j : 0UL ) );

               ElementType value = ElementType();

               for( size_t k=kbegin; k<K; ++k ) {
                  value += A(i,k) * B(k,j);
               }

               (~C)(i,j) = value;
            }
         }
      }

      if( ( SYM || HERM ) && ( N > SIMDSIZE*4UL ) ) {
         for( size_t i=SIMDSIZE*4UL; i<M; ++i ) {
            const size_t jend( ( SIMDSIZE*4UL ) * ( i / (SIMDSIZE*4UL) ) );
            for( size_t j=0UL; j<jend; ++j ) {
               (~C)(i,j) = HERM ? conj( (~C)(j,i) ) : (~C)(j,i);
            }
         }
      }
      else if( LOW && !UPP && N > SIMDSIZE*4UL ) {
         for( size_t j=SIMDSIZE*4UL; j<N; ++j ) {
            const size_t iend( ( SIMDSIZE*4UL ) * ( j / (SIMDSIZE*4UL) ) );
            for( size_t i=0UL; i<iend; ++i ) {
               reset( (~C)(i,j) );
            }
         }
      }
      else if( !LOW && UPP && N > SIMDSIZE*4UL ) {
         for( size_t i=SIMDSIZE*4UL; i<M; ++i ) {
            const size_t jend( ( SIMDSIZE*4UL ) * ( i / (SIMDSIZE*4UL) ) );
            for( size_t j=0UL; j<jend; ++j ) {
               reset( (~C)(i,j) );
            }
         }
      }
   }
   /*! \endcond */
   //**********************************************************************************************

   //**Vectorized default assignment to column-major dense matrices (small matrices)***************
   /*! \cond BLAZE_INTERNAL */
   /*!\brief Vectorized default assignment of a small dense matrix-dense matrix multiplication
   //        (\f$ C=A*B \f$).
   // \ingroup dense_matrix
   //
   // \param C The target left-hand side dense matrix.
   // \param A The left-hand side multiplication operand.
   // \param B The right-hand side multiplication operand.
   // \return void
   //
   // This function implements the vectorized default assignment of a dense matrix-dense matrix
   // multiplication expression to a column-major dense matrix. This kernel is optimized for small
   // matrices.
   */
   template< typename MT3    // Type of the left-hand side target matrix
           , typename MT4    // Type of the left-hand side matrix operand
           , typename MT5 >  // Type of the right-hand side matrix operand
   static inline EnableIf_< UseVectorizedDefaultKernel<MT3,MT4,MT5> >
      selectSmallAssignKernel( DenseMatrix<MT3,true>& C, const MT4& A, const MT5& B )
   {
      BLAZE_CONSTRAINT_MUST_NOT_BE_SYMMETRIC_MATRIX_TYPE( MT4 );
      BLAZE_CONSTRAINT_MUST_NOT_BE_SYMMETRIC_MATRIX_TYPE( MT5 );
      BLAZE_CONSTRAINT_MUST_BE_COLUMN_MAJOR_MATRIX_TYPE( OppositeType_<MT4> );
      BLAZE_CONSTRAINT_MUST_BE_COLUMN_MAJOR_MATRIX_TYPE( OppositeType_<MT5> );

      const ForwardFunctor fwd;

      if( !IsResizable<MT4>::value && IsResizable<MT5>::value ) {
         const OppositeType_<MT4> tmp( serial( A ) );
         assign( ~C, fwd( tmp * B ) );
      }
      else if( IsResizable<MT4>::value && !IsResizable<MT5>::value ) {
         const OppositeType_<MT5> tmp( serial( B ) );
         assign( ~C, fwd( A * tmp ) );
      }
      else if( A.rows() * A.columns() <= B.rows() * B.columns() ) {
         const OppositeType_<MT4> tmp( serial( A ) );
         assign( ~C, fwd( tmp * B ) );
      }
      else {
         const OppositeType_<MT5> tmp( serial( B ) );
         assign( ~C, fwd( A * tmp ) );
      }
   }
   /*! \endcond */
   //**********************************************************************************************

   //**Default assignment to dense matrices (large matrices)***************************************
   /*! \cond BLAZE_INTERNAL */
   /*!\brief Default assignment of a large dense matrix-dense matrix multiplication (\f$ C=A*B \f$).
   // \ingroup dense_matrix
   //
   // \param C The target left-hand side dense matrix.
   // \param A The left-hand side multiplication operand.
   // \param B The right-hand side multiplication operand.
   // \return void
   //
   // This function relays to the default implementation of the assignment of a dense matrix-
   // dense matrix multiplication expression to a dense matrix.
   */
   template< typename MT3    // Type of the left-hand side target matrix
           , typename MT4    // Type of the left-hand side matrix operand
           , typename MT5 >  // Type of the right-hand side matrix operand
   static inline DisableIf_< UseVectorizedDefaultKernel<MT3,MT4,MT5> >
      selectLargeAssignKernel( MT3& C, const MT4& A, const MT5& B )
   {
      selectDefaultAssignKernel( C, A, B );
   }
   /*! \endcond */
   //**********************************************************************************************

   //**Vectorized default assignment to dense matrices (large matrices)****************************
   /*! \cond BLAZE_INTERNAL */
   /*!\brief Vectorized default assignment of a large dense matrix-dense matrix multiplication
   //        (\f$ C=A*B \f$).
   // \ingroup dense_matrix
   //
   // \param C The target left-hand side dense matrix.
   // \param A The left-hand side multiplication operand.
   // \param B The right-hand side multiplication operand.
   // \return void
   //
   // This function implements the vectorized default assignment of a dense matrix-dense matrix
   // multiplication expression to a dense matrix. This kernel is optimized for large matrices.
   */
   template< typename MT3    // Type of the left-hand side target matrix
           , typename MT4    // Type of the left-hand side matrix operand
           , typename MT5 >  // Type of the right-hand side matrix operand
   static inline EnableIf_< UseVectorizedDefaultKernel<MT3,MT4,MT5> >
      selectLargeAssignKernel( MT3& C, const MT4& A, const MT5& B )
   {
      if( SYM )
         smmm( C, A, B, ElementType(1) );
      else if( HERM )
         hmmm( C, A, B, ElementType(1) );
      else if( LOW )
         lmmm( C, A, B, ElementType(1), ElementType(0) );
      else if( UPP )
         ummm( C, A, B, ElementType(1), ElementType(0) );
      else
         mmm( C, A, B, ElementType(1), ElementType(0) );
   }
   /*! \endcond */
   //**********************************************************************************************

   //**BLAS-based assignment to dense matrices (default)*******************************************
   /*! \cond BLAZE_INTERNAL */
   /*!\brief Default assignment of a dense matrix-dense matrix multiplication (\f$ C=A*B \f$).
   // \ingroup dense_matrix
   //
   // \param C The target left-hand side dense matrix.
   // \param A The left-hand side multiplication operand.
   // \param B The right-hand side multiplication operand.
   // \return void
   //
   // This function relays to the default implementation of the assignment of a large dense
   // matrix-dense matrix multiplication expression to a dense matrix.
   */
   template< typename MT3    // Type of the left-hand side target matrix
           , typename MT4    // Type of the left-hand side matrix operand
           , typename MT5 >  // Type of the right-hand side matrix operand
   static inline DisableIf_< UseBlasKernel<MT3,MT4,MT5> >
      selectBlasAssignKernel( MT3& C, const MT4& A, const MT5& B )
   {
      selectLargeAssignKernel( C, A, B );
   }
   /*! \endcond */
   //**********************************************************************************************

   //**BLAS-based assignment to dense matrices*****************************************************
#if BLAZE_BLAS_MODE && BLAZE_USE_BLAS_MATRIX_MATRIX_MULTIPLICATION
   /*! \cond BLAZE_INTERNAL */
   /*!\brief BLAS-based assignment of a dense matrix-dense matrix multiplication (\f$ C=A*B \f$).
   // \ingroup dense_matrix
   //
   // \param C The target left-hand side dense matrix.
   // \param A The left-hand side multiplication operand.
   // \param B The right-hand side multiplication operand.
   // \return void
   //
   // This function performs the dense matrix-dense matrix multiplication based on the according
   // BLAS functionality.
   */
   template< typename MT3    // Type of the left-hand side target matrix
           , typename MT4    // Type of the left-hand side matrix operand
           , typename MT5 >  // Type of the right-hand side matrix operand
   static inline EnableIf_< UseBlasKernel<MT3,MT4,MT5> >
      selectBlasAssignKernel( MT3& C, const MT4& A, const MT5& B )
   {
      typedef ElementType_<MT3>  ET;

      if( IsTriangular<MT4>::value ) {
         assign( C, B );
         trmm( C, A, CblasLeft, ( IsLower<MT4>::value )?( CblasLower ):( CblasUpper ), ET(1) );
      }
      else if( IsTriangular<MT5>::value ) {
         assign( C, A );
         trmm( C, B, CblasRight, ( IsLower<MT5>::value )?( CblasLower ):( CblasUpper ), ET(1) );
      }
      else {
         gemm( C, A, B, ET(1), ET(0) );
      }
   }
   /*! \endcond */
#endif
   //**********************************************************************************************

   //**Assignment to sparse matrices***************************************************************
   /*! \cond BLAZE_INTERNAL */
   /*!\brief Assignment of a dense matrix-dense matrix multiplication to a sparse matrix
   //        (\f$ C=A*B \f$).
   // \ingroup dense_matrix
   //
   // \param lhs The target left-hand side sparse matrix.
   // \param rhs The right-hand side multiplication expression to be assigned.
   // \return void
   //
   // This function implements the performance optimized assignment of a dense matrix-dense
   // matrix multiplication expression to a sparse matrix.
   */
   template< typename MT  // Type of the target sparse matrix
           , bool SO >    // Storage order of the target sparse matrix
   friend inline DisableIf_< CanExploitSymmetry<MT,MT1,MT2> >
      assign( SparseMatrix<MT,SO>& lhs, const DMatDMatMultExpr& rhs )
   {
      BLAZE_FUNCTION_TRACE;

      typedef IfTrue_< SO, OppositeType, ResultType >  TmpType;

      BLAZE_CONSTRAINT_MUST_BE_DENSE_MATRIX_TYPE( ResultType );
      BLAZE_CONSTRAINT_MUST_BE_DENSE_MATRIX_TYPE( OppositeType );
      BLAZE_CONSTRAINT_MUST_BE_ROW_MAJOR_MATRIX_TYPE( ResultType );
      BLAZE_CONSTRAINT_MUST_BE_COLUMN_MAJOR_MATRIX_TYPE( OppositeType );
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

   //**Restructuring assignment to column-major matrices*******************************************
   /*! \cond BLAZE_INTERNAL */
   /*!\brief Restructuring assignment of a dense matrix-dense matrix multiplication to a
   //        column-major matrix (\f$ C=A*B \f$).
   // \ingroup dense_matrix
   //
   // \param lhs The target left-hand side matrix.
   // \param rhs The right-hand side multiplication expression to be assigned.
   // \return void
   //
   // This function implements the symmetry-based restructuring assignment of a dense matrix-
   // dense matrix multiplication expression to a column-major matrix. Due to the explicit
   // application of the SFINAE principle this function can only be selected by the compiler
   // in case the symmetry of either of the two matrix operands can be exploited.
   */
   template< typename MT >  // Type of the target matrix
   friend inline EnableIf_< CanExploitSymmetry<MT,MT1,MT2> >
      assign( Matrix<MT,true>& lhs, const DMatDMatMultExpr& rhs )
   {
      BLAZE_FUNCTION_TRACE;

      BLAZE_CONSTRAINT_MUST_NOT_BE_SYMMETRIC_MATRIX_TYPE( MT );

      BLAZE_INTERNAL_ASSERT( (~lhs).rows()    == rhs.rows()   , "Invalid number of rows"    );
      BLAZE_INTERNAL_ASSERT( (~lhs).columns() == rhs.columns(), "Invalid number of columns" );

      const ForwardFunctor fwd;

      if( IsSymmetric<MT1>::value && IsSymmetric<MT2>::value )
         assign( ~lhs, fwd( trans( rhs.lhs_ ) * trans( rhs.rhs_ ) ) );
      else if( IsSymmetric<MT1>::value )
         assign( ~lhs, fwd( trans( rhs.lhs_ ) * rhs.rhs_ ) );
      else
         assign( ~lhs, fwd( rhs.lhs_ * trans( rhs.rhs_ ) ) );
   }
   /*! \endcond */
   //**********************************************************************************************

   //**Addition assignment to dense matrices*******************************************************
   /*! \cond BLAZE_INTERNAL */
   /*!\brief Addition assignment of a dense matrix-dense matrix multiplication to a dense matrix
   //        (\f$ C+=A*B \f$).
   // \ingroup dense_matrix
   //
   // \param lhs The target left-hand side dense matrix.
   // \param rhs The right-hand side multiplication expression to be added.
   // \return void
   //
   // This function implements the performance optimized addition assignment of a dense matrix-
   // dense matrix multiplication expression to a dense matrix.
   */
   template< typename MT  // Type of the target dense matrix
           , bool SO >    // Storage order of the target dense matrix
   friend inline DisableIf_< CanExploitSymmetry<MT,MT1,MT2> >
      addAssign( DenseMatrix<MT,SO>& lhs, const DMatDMatMultExpr& rhs )
   {
      BLAZE_FUNCTION_TRACE;

      BLAZE_INTERNAL_ASSERT( (~lhs).rows()    == rhs.rows()   , "Invalid number of rows"    );
      BLAZE_INTERNAL_ASSERT( (~lhs).columns() == rhs.columns(), "Invalid number of columns" );

      if( (~lhs).rows() == 0UL || (~lhs).columns() == 0UL || rhs.lhs_.columns() == 0UL ) {
         return;
      }

      LT A( serial( rhs.lhs_ ) );  // Evaluation of the left-hand side dense matrix operand
      RT B( serial( rhs.rhs_ ) );  // Evaluation of the right-hand side dense matrix operand

      BLAZE_INTERNAL_ASSERT( A.rows()    == rhs.lhs_.rows()   , "Invalid number of rows"    );
      BLAZE_INTERNAL_ASSERT( A.columns() == rhs.lhs_.columns(), "Invalid number of columns" );
      BLAZE_INTERNAL_ASSERT( B.rows()    == rhs.rhs_.rows()   , "Invalid number of rows"    );
      BLAZE_INTERNAL_ASSERT( B.columns() == rhs.rhs_.columns(), "Invalid number of columns" );
      BLAZE_INTERNAL_ASSERT( A.rows()    == (~lhs).rows()     , "Invalid number of rows"    );
      BLAZE_INTERNAL_ASSERT( B.columns() == (~lhs).columns()  , "Invalid number of columns" );

      DMatDMatMultExpr::selectAddAssignKernel( ~lhs, A, B );
   }
   /*! \endcond */
   //**********************************************************************************************

   //**Addition assignment to dense matrices (kernel selection)************************************
   /*! \cond BLAZE_INTERNAL */
   /*!\brief Selection of the kernel for an addition assignment of a dense matrix-dense matrix
   //        multiplication to a dense matrix (\f$ C+=A*B \f$).
   // \ingroup dense_matrix
   //
   // \param C The target left-hand side dense matrix.
   // \param A The left-hand side multiplication operand.
   // \param B The right-hand side multiplication operand.
   // \return void
   */
   template< typename MT3    // Type of the left-hand side target matrix
           , typename MT4    // Type of the left-hand side matrix operand
           , typename MT5 >  // Type of the right-hand side matrix operand
   static inline void selectAddAssignKernel( MT3& C, const MT4& A, const MT5& B )
   {
      if( ( IsDiagonal<MT5>::value ) ||
          ( !BLAZE_DEBUG_MODE && B.columns() <= SIMDSIZE*10UL ) ||
          ( C.rows() * C.columns() < DMATDMATMULT_THRESHOLD ) )
         selectSmallAddAssignKernel( C, A, B );
      else
         selectBlasAddAssignKernel( C, A, B );
   }
   /*! \endcond */
   //**********************************************************************************************

   //**Default addition assignment to dense matrices (general/general)*****************************
   /*! \cond BLAZE_INTERNAL */
   /*!\brief Default addition assignment of a general dense matrix-general dense matrix
   //        multiplication (\f$ C+=A*B \f$).
   // \ingroup dense_matrix
   //
   // \param C The target left-hand side dense matrix.
   // \param A The left-hand side multiplication operand.
   // \param B The right-hand side multiplication operand.
   // \return void
   //
   // This function implements the default addition assignment of a general dense matrix-general
   // dense matrix multiplication expression to a dense matrix.
   */
   template< typename MT3    // Type of the left-hand side target matrix
           , typename MT4    // Type of the left-hand side matrix operand
           , typename MT5 >  // Type of the right-hand side matrix operand
   static inline EnableIf_< And< Not< IsDiagonal<MT4> >, Not< IsDiagonal<MT5> > > >
      selectDefaultAddAssignKernel( MT3& C, const MT4& A, const MT5& B )
   {
      const size_t M( A.rows()    );
      const size_t N( B.columns() );
      const size_t K( A.columns() );

      BLAZE_INTERNAL_ASSERT( !( LOW || UPP ) || ( M == N ), "Broken invariant detected" );

      for( size_t i=0UL; i<M; ++i )
      {
         const size_t kbegin( ( IsUpper<MT4>::value )
                              ?( IsStrictlyUpper<MT4>::value ? i+1UL : i )
                              :( 0UL ) );
         const size_t kend( ( IsLower<MT4>::value )
                            ?( IsStrictlyLower<MT4>::value ? i : i+1UL )
                            :( K ) );
         BLAZE_INTERNAL_ASSERT( kbegin <= kend, "Invalid loop indices detected" );

         for( size_t k=kbegin; k<kend; ++k )
         {
            const size_t jbegin( ( IsUpper<MT5>::value )
                                 ?( ( IsStrictlyUpper<MT5>::value )
                                    ?( UPP ? max(i,k+1UL) : k+1UL )
                                    :( UPP ? max(i,k) : k ) )
                                 :( UPP ? i : 0UL ) );
            const size_t jend( ( IsLower<MT5>::value )
                               ?( ( IsStrictlyLower<MT5>::value )
                                  ?( LOW ? min(i+1UL,k) : k )
                                  :( LOW ? min(i,k)+1UL : k+1UL ) )
                               :( LOW ? i+1UL : N ) );

            if( ( LOW || UPP ) && ( jbegin >= jend ) ) continue;
            BLAZE_INTERNAL_ASSERT( jbegin <= jend, "Invalid loop indices detected" );

            const size_t jnum( jend - jbegin );
            const size_t jpos( jbegin + ( jnum & size_t(-2) ) );

            for( size_t j=jbegin; j<jpos; j+=2UL ) {
               C(i,j    ) += A(i,k) * B(k,j    );
               C(i,j+1UL) += A(i,k) * B(k,j+1UL);
            }
            if( jpos < jend ) {
               C(i,jpos) += A(i,k) * B(k,jpos);
            }
         }
      }
   }
   /*! \endcond */
   //**********************************************************************************************

   //**Default addition assignment to dense matrices (general/diagonal)****************************
   /*! \cond BLAZE_INTERNAL */
   /*!\brief Default addition assignment of a general dense matrix-diagonal dense matrix
   //        multiplication (\f$ C+=A*B \f$).
   // \ingroup dense_matrix
   //
   // \param C The target left-hand side dense matrix.
   // \param A The left-hand side multiplication operand.
   // \param B The right-hand side multiplication operand.
   // \return void
   //
   // This function implements the default addition assignment of a general dense matrix-diagonal
   // dense matrix multiplication expression to a dense matrix.
   */
   template< typename MT3    // Type of the left-hand side target matrix
           , typename MT4    // Type of the left-hand side matrix operand
           , typename MT5 >  // Type of the right-hand side matrix operand
   static inline EnableIf_< And< Not< IsDiagonal<MT4> >, IsDiagonal<MT5> > >
      selectDefaultAddAssignKernel( MT3& C, const MT4& A, const MT5& B )
   {
      BLAZE_CONSTRAINT_MUST_BE_ROW_MAJOR_MATRIX_TYPE( MT3 );

      const size_t M( A.rows()    );
      const size_t N( B.columns() );

      for( size_t i=0UL; i<M; ++i )
      {
         const size_t jbegin( ( IsUpper<MT4>::value )
                              ?( IsStrictlyUpper<MT4>::value ? i+1UL : i )
                              :( 0UL ) );
         const size_t jend( ( IsLower<MT4>::value )
                            ?( IsStrictlyLower<MT4>::value ? i : i+1UL )
                            :( N ) );
         BLAZE_INTERNAL_ASSERT( jbegin <= jend, "Invalid loop indices detected" );

         const size_t jnum( jend - jbegin );
         const size_t jpos( jbegin + ( jnum & size_t(-2) ) );

         for( size_t j=jbegin; j<jpos; j+=2UL ) {
            C(i,j    ) += A(i,j    ) * B(j    ,j    );
            C(i,j+1UL) += A(i,j+1UL) * B(j+1UL,j+1UL);
         }
         if( jpos < jend ) {
            C(i,jpos) += A(i,jpos) * B(jpos,jpos);
         }
      }
   }
   /*! \endcond */
   //**********************************************************************************************

   //**Default addition assignment to dense matrices (diagonal/general)****************************
   /*! \cond BLAZE_INTERNAL */
   /*!\brief Default addition assignment of a diagonal dense matrix-general dense matrix
   //        multiplication (\f$ C+=A*B \f$).
   // \ingroup dense_matrix
   //
   // \param C The target left-hand side dense matrix.
   // \param A The left-hand side multiplication operand.
   // \param B The right-hand side multiplication operand.
   // \return void
   //
   // This function implements the default addition assignment of a diagonal dense matrix-general
   // dense matrix multiplication expression to a dense matrix.
   */
   template< typename MT3    // Type of the left-hand side target matrix
           , typename MT4    // Type of the left-hand side matrix operand
           , typename MT5 >  // Type of the right-hand side matrix operand
   static inline EnableIf_< And< IsDiagonal<MT4>, Not< IsDiagonal<MT5> > > >
      selectDefaultAddAssignKernel( MT3& C, const MT4& A, const MT5& B )
   {
      BLAZE_CONSTRAINT_MUST_BE_ROW_MAJOR_MATRIX_TYPE( MT3 );

      const size_t M( A.rows()    );
      const size_t N( B.columns() );

      for( size_t i=0UL; i<M; ++i )
      {
         const size_t jbegin( ( IsUpper<MT5>::value )
                              ?( IsStrictlyUpper<MT5>::value ? i+1UL : i )
                              :( 0UL ) );
         const size_t jend( ( IsLower<MT5>::value )
                            ?( IsStrictlyLower<MT5>::value ? i : i+1UL )
                            :( N ) );
         BLAZE_INTERNAL_ASSERT( jbegin <= jend, "Invalid loop indices detected" );

         const size_t jnum( jend - jbegin );
         const size_t jpos( jbegin + ( jnum & size_t(-2) ) );

         for( size_t j=jbegin; j<jpos; j+=2UL ) {
            C(i,j    ) += A(i,i) * B(i,j    );
            C(i,j+1UL) += A(i,i) * B(i,j+1UL);
         }
         if( jpos < jend ) {
            C(i,jpos) += A(i,i) * B(i,jpos);
         }
      }
   }
   /*! \endcond */
   //**********************************************************************************************

   //**Default addition assignment to dense matrices (diagonal/diagonal)***************************
   /*! \cond BLAZE_INTERNAL */
   /*!\brief Default addition assignment of a diagonal dense matrix-diagonal dense matrix
   //        multiplication (\f$ C+=A*B \f$).
   // \ingroup dense_matrix
   //
   // \param C The target left-hand side dense matrix.
   // \param A The left-hand side multiplication operand.
   // \param B The right-hand side multiplication operand.
   // \return void
   //
   // This function implements the default addition assignment of a diagonal dense matrix-diagonal
   // dense matrix multiplication expression to a dense matrix.
   */
   template< typename MT3    // Type of the left-hand side target matrix
           , typename MT4    // Type of the left-hand side matrix operand
           , typename MT5 >  // Type of the right-hand side matrix operand
   static inline EnableIf_< And< IsDiagonal<MT4>, IsDiagonal<MT5> > >
      selectDefaultAddAssignKernel( MT3& C, const MT4& A, const MT5& B )
   {
      BLAZE_CONSTRAINT_MUST_BE_ROW_MAJOR_MATRIX_TYPE( MT3 );

      for( size_t i=0UL; i<A.rows(); ++i ) {
         C(i,i) += A(i,i) * B(i,i);
      }
   }
   /*! \endcond */
   //**********************************************************************************************

   //**Default addition assignment to dense matrices (small matrices)******************************
   /*! \cond BLAZE_INTERNAL */
   /*!\brief Default addition assignment of a small dense matrix-dense matrix multiplication
   //        (\f$ C+=A*B \f$).
   // \ingroup dense_matrix
   //
   // \param C The target left-hand side dense matrix.
   // \param A The left-hand side multiplication operand.
   // \param B The right-hand side multiplication operand.
   // \return void
   //
   // This function relays to the default implementation of the addition assignment of a dense
   // matrix-dense matrix multiplication expression to a dense matrix.
   */
   template< typename MT3    // Type of the left-hand side target matrix
           , typename MT4    // Type of the left-hand side matrix operand
           , typename MT5 >  // Type of the right-hand side matrix operand
   static inline DisableIf_< UseVectorizedDefaultKernel<MT3,MT4,MT5> >
      selectSmallAddAssignKernel( MT3& C, const MT4& A, const MT5& B )
   {
      selectDefaultAddAssignKernel( C, A, B );
   }
   /*! \endcond */
   //**********************************************************************************************

   //**Vectorized default addition assignment to row-major dense matrices (small matrices)*********
   /*! \cond BLAZE_INTERNAL */
   /*!\brief Vectorized default addition assignment of a small dense matrix-dense matrix
   //        multiplication (\f$ C+=A*B \f$).
   // \ingroup dense_matrix
   //
   // \param C The target left-hand side dense matrix.
   // \param A The left-hand side multiplication operand.
   // \param B The right-hand side multiplication operand.
   // \return void
   //
   // This function implements the vectorized default addition assignment of a dense matrix-dense
   // matrix multiplication expression to a row-major dense matrix. This kernel is optimized for
   // small matrices.
   */
   template< typename MT3    // Type of the left-hand side target matrix
           , typename MT4    // Type of the left-hand side matrix operand
           , typename MT5 >  // Type of the right-hand side matrix operand
   static inline EnableIf_< UseVectorizedDefaultKernel<MT3,MT4,MT5> >
      selectSmallAddAssignKernel( DenseMatrix<MT3,false>& C, const MT4& A, const MT5& B )
   {
      constexpr bool remainder( !IsPadded<MT3>::value || !IsPadded<MT5>::value );

      const size_t M( A.rows()    );
      const size_t N( B.columns() );
      const size_t K( A.columns() );

      BLAZE_INTERNAL_ASSERT( !( LOW || UPP ) || ( M == N ), "Broken invariant detected" );

      const size_t jpos( remainder ? ( N & size_t(-SIMDSIZE) ) : N );
      BLAZE_INTERNAL_ASSERT( !remainder || ( N - ( N % SIMDSIZE ) ) == jpos, "Invalid end calculation" );

      size_t j( 0UL );

      if( IsIntegral<ElementType>::value )
      {
         for( ; !LOW && !UPP && (j+SIMDSIZE*7UL) < jpos; j+=SIMDSIZE*8UL ) {
            for( size_t i=0UL; i<M; ++i )
            {
               const size_t kbegin( ( IsUpper<MT4>::value )
                                    ?( ( IsLower<MT5>::value )
                                       ?( max( ( IsStrictlyUpper<MT4>::value ? i+1UL : i ), j ) )
                                       :( IsStrictlyUpper<MT4>::value ? i+1UL : i ) )
                                    :( IsLower<MT5>::value ? j : 0UL ) );
               const size_t kend( ( IsLower<MT4>::value )
                                  ?( ( IsUpper<MT5>::value )
                                     ?( min( ( IsStrictlyLower<MT4>::value ? i : i+1UL ), j+SIMDSIZE*8UL, K ) )
                                     :( IsStrictlyLower<MT4>::value ? i : i+1UL ) )
                                  :( IsUpper<MT5>::value ? min( j+SIMDSIZE*8UL, K ) : K ) );

               SIMDType xmm1( (~C).load(i,j             ) );
               SIMDType xmm2( (~C).load(i,j+SIMDSIZE    ) );
               SIMDType xmm3( (~C).load(i,j+SIMDSIZE*2UL) );
               SIMDType xmm4( (~C).load(i,j+SIMDSIZE*3UL) );
               SIMDType xmm5( (~C).load(i,j+SIMDSIZE*4UL) );
               SIMDType xmm6( (~C).load(i,j+SIMDSIZE*5UL) );
               SIMDType xmm7( (~C).load(i,j+SIMDSIZE*6UL) );
               SIMDType xmm8( (~C).load(i,j+SIMDSIZE*7UL) );

               for( size_t k=kbegin; k<kend; ++k ) {
                  const SIMDType a1( set( A(i,k) ) );
                  xmm1 += a1 * B.load(k,j             );
                  xmm2 += a1 * B.load(k,j+SIMDSIZE    );
                  xmm3 += a1 * B.load(k,j+SIMDSIZE*2UL);
                  xmm4 += a1 * B.load(k,j+SIMDSIZE*3UL);
                  xmm5 += a1 * B.load(k,j+SIMDSIZE*4UL);
                  xmm6 += a1 * B.load(k,j+SIMDSIZE*5UL);
                  xmm7 += a1 * B.load(k,j+SIMDSIZE*6UL);
                  xmm8 += a1 * B.load(k,j+SIMDSIZE*7UL);
               }

               (~C).store( i, j             , xmm1 );
               (~C).store( i, j+SIMDSIZE    , xmm2 );
               (~C).store( i, j+SIMDSIZE*2UL, xmm3 );
               (~C).store( i, j+SIMDSIZE*3UL, xmm4 );
               (~C).store( i, j+SIMDSIZE*4UL, xmm5 );
               (~C).store( i, j+SIMDSIZE*5UL, xmm6 );
               (~C).store( i, j+SIMDSIZE*6UL, xmm7 );
               (~C).store( i, j+SIMDSIZE*7UL, xmm8 );
            }
         }
      }

      for( ; !LOW && !UPP && (j+SIMDSIZE*4UL) < jpos; j+=SIMDSIZE*5UL )
      {
         size_t i( 0UL );

         for( ; (i+2UL) <= M; i+=2UL )
         {
            const size_t kbegin( ( IsUpper<MT4>::value )
                                 ?( ( IsLower<MT5>::value )
                                    ?( max( ( IsStrictlyUpper<MT4>::value ? i+1UL : i ), j ) )
                                    :( IsStrictlyUpper<MT4>::value ? i+1UL : i ) )
                                 :( IsLower<MT5>::value ? j : 0UL ) );
            const size_t kend( ( IsLower<MT4>::value )
                               ?( ( IsUpper<MT5>::value )
                                  ?( min( ( IsStrictlyLower<MT4>::value ? i+1UL : i+2UL ), j+SIMDSIZE*5UL, K ) )
                                  :( IsStrictlyLower<MT4>::value ? i+1UL : i+2UL ) )
                               :( IsUpper<MT5>::value ? min( j+SIMDSIZE*5UL, K ) : K ) );

            SIMDType xmm1 ( (~C).load(i    ,j             ) );
            SIMDType xmm2 ( (~C).load(i    ,j+SIMDSIZE    ) );
            SIMDType xmm3 ( (~C).load(i    ,j+SIMDSIZE*2UL) );
            SIMDType xmm4 ( (~C).load(i    ,j+SIMDSIZE*3UL) );
            SIMDType xmm5 ( (~C).load(i    ,j+SIMDSIZE*4UL) );
            SIMDType xmm6 ( (~C).load(i+1UL,j             ) );
            SIMDType xmm7 ( (~C).load(i+1UL,j+SIMDSIZE    ) );
            SIMDType xmm8 ( (~C).load(i+1UL,j+SIMDSIZE*2UL) );
            SIMDType xmm9 ( (~C).load(i+1UL,j+SIMDSIZE*3UL) );
            SIMDType xmm10( (~C).load(i+1UL,j+SIMDSIZE*4UL) );

            for( size_t k=kbegin; k<kend; ++k ) {
               const SIMDType a1( set( A(i    ,k) ) );
               const SIMDType a2( set( A(i+1UL,k) ) );
               const SIMDType b1( B.load(k,j             ) );
               const SIMDType b2( B.load(k,j+SIMDSIZE    ) );
               const SIMDType b3( B.load(k,j+SIMDSIZE*2UL) );
               const SIMDType b4( B.load(k,j+SIMDSIZE*3UL) );
               const SIMDType b5( B.load(k,j+SIMDSIZE*4UL) );
               xmm1  += a1 * b1;
               xmm2  += a1 * b2;
               xmm3  += a1 * b3;
               xmm4  += a1 * b4;
               xmm5  += a1 * b5;
               xmm6  += a2 * b1;
               xmm7  += a2 * b2;
               xmm8  += a2 * b3;
               xmm9  += a2 * b4;
               xmm10 += a2 * b5;
            }

            (~C).store( i    , j             , xmm1  );
            (~C).store( i    , j+SIMDSIZE    , xmm2  );
            (~C).store( i    , j+SIMDSIZE*2UL, xmm3  );
            (~C).store( i    , j+SIMDSIZE*3UL, xmm4  );
            (~C).store( i    , j+SIMDSIZE*4UL, xmm5  );
            (~C).store( i+1UL, j             , xmm6  );
            (~C).store( i+1UL, j+SIMDSIZE    , xmm7  );
            (~C).store( i+1UL, j+SIMDSIZE*2UL, xmm8  );
            (~C).store( i+1UL, j+SIMDSIZE*3UL, xmm9  );
            (~C).store( i+1UL, j+SIMDSIZE*4UL, xmm10 );
         }

         if( i < M )
         {
            const size_t kbegin( ( IsUpper<MT4>::value )
                                 ?( ( IsLower<MT5>::value )
                                    ?( max( ( IsStrictlyUpper<MT4>::value ? i+1UL : i ), j ) )
                                    :( IsStrictlyUpper<MT4>::value ? i+1UL : i ) )
                                 :( IsLower<MT5>::value ? j : 0UL ) );
            const size_t kend( ( IsUpper<MT5>::value )?( min( j+SIMDSIZE*5UL, K ) ):( K ) );

            SIMDType xmm1( (~C).load(i,j             ) );
            SIMDType xmm2( (~C).load(i,j+SIMDSIZE    ) );
            SIMDType xmm3( (~C).load(i,j+SIMDSIZE*2UL) );
            SIMDType xmm4( (~C).load(i,j+SIMDSIZE*3UL) );
            SIMDType xmm5( (~C).load(i,j+SIMDSIZE*4UL) );

            for( size_t k=kbegin; k<kend; ++k ) {
               const SIMDType a1( set( A(i,k) ) );
               xmm1 += a1 * B.load(k,j             );
               xmm2 += a1 * B.load(k,j+SIMDSIZE    );
               xmm3 += a1 * B.load(k,j+SIMDSIZE*2UL);
               xmm4 += a1 * B.load(k,j+SIMDSIZE*3UL);
               xmm5 += a1 * B.load(k,j+SIMDSIZE*4UL);
            }

            (~C).store( i, j             , xmm1 );
            (~C).store( i, j+SIMDSIZE    , xmm2 );
            (~C).store( i, j+SIMDSIZE*2UL, xmm3 );
            (~C).store( i, j+SIMDSIZE*3UL, xmm4 );
            (~C).store( i, j+SIMDSIZE*4UL, xmm5 );
         }
      }

      for( ; !LOW && !UPP && (j+SIMDSIZE*3UL) < jpos; j+=SIMDSIZE*4UL )
      {
         size_t i( 0UL );

         for( ; (i+2UL) <= M; i+=2UL )
         {
            const size_t kbegin( ( IsUpper<MT4>::value )
                                 ?( ( IsLower<MT5>::value )
                                    ?( max( ( IsStrictlyUpper<MT4>::value ? i+1UL : i ), j ) )
                                    :( IsStrictlyUpper<MT4>::value ? i+1UL : i ) )
                                 :( IsLower<MT5>::value ? j : 0UL ) );
            const size_t kend( ( IsLower<MT4>::value )
                               ?( ( IsUpper<MT5>::value )
                                  ?( min( ( IsStrictlyLower<MT4>::value ? i+1UL : i+2UL ), j+SIMDSIZE*4UL, K ) )
                                  :( IsStrictlyLower<MT4>::value ? i+1UL : i+2UL ) )
                               :( IsUpper<MT5>::value ? min( j+SIMDSIZE*4UL, K ) : K ) );

            SIMDType xmm1( (~C).load(i    ,j             ) );
            SIMDType xmm2( (~C).load(i    ,j+SIMDSIZE    ) );
            SIMDType xmm3( (~C).load(i    ,j+SIMDSIZE*2UL) );
            SIMDType xmm4( (~C).load(i    ,j+SIMDSIZE*3UL) );
            SIMDType xmm5( (~C).load(i+1UL,j             ) );
            SIMDType xmm6( (~C).load(i+1UL,j+SIMDSIZE    ) );
            SIMDType xmm7( (~C).load(i+1UL,j+SIMDSIZE*2UL) );
            SIMDType xmm8( (~C).load(i+1UL,j+SIMDSIZE*3UL) );

            for( size_t k=kbegin; k<kend; ++k ) {
               const SIMDType a1( set( A(i    ,k) ) );
               const SIMDType a2( set( A(i+1UL,k) ) );
               const SIMDType b1( B.load(k,j             ) );
               const SIMDType b2( B.load(k,j+SIMDSIZE    ) );
               const SIMDType b3( B.load(k,j+SIMDSIZE*2UL) );
               const SIMDType b4( B.load(k,j+SIMDSIZE*3UL) );
               xmm1 += a1 * b1;
               xmm2 += a1 * b2;
               xmm3 += a1 * b3;
               xmm4 += a1 * b4;
               xmm5 += a2 * b1;
               xmm6 += a2 * b2;
               xmm7 += a2 * b3;
               xmm8 += a2 * b4;
            }

            (~C).store( i    , j             , xmm1 );
            (~C).store( i    , j+SIMDSIZE    , xmm2 );
            (~C).store( i    , j+SIMDSIZE*2UL, xmm3 );
            (~C).store( i    , j+SIMDSIZE*3UL, xmm4 );
            (~C).store( i+1UL, j             , xmm5 );
            (~C).store( i+1UL, j+SIMDSIZE    , xmm6 );
            (~C).store( i+1UL, j+SIMDSIZE*2UL, xmm7 );
            (~C).store( i+1UL, j+SIMDSIZE*3UL, xmm8 );
         }

         if( i < M )
         {
            const size_t kbegin( ( IsUpper<MT4>::value )
                                 ?( ( IsLower<MT5>::value )
                                    ?( max( ( IsStrictlyUpper<MT4>::value ? i+1UL : i ), j ) )
                                    :( IsStrictlyUpper<MT4>::value ? i+1UL : i ) )
                                 :( IsLower<MT5>::value ? j : 0UL ) );
            const size_t kend( ( IsUpper<MT5>::value )?( min( j+SIMDSIZE*4UL, K ) ):( K ) );

            SIMDType xmm1( (~C).load(i,j             ) );
            SIMDType xmm2( (~C).load(i,j+SIMDSIZE    ) );
            SIMDType xmm3( (~C).load(i,j+SIMDSIZE*2UL) );
            SIMDType xmm4( (~C).load(i,j+SIMDSIZE*3UL) );

            for( size_t k=kbegin; k<kend; ++k ) {
               const SIMDType a1( set( A(i,k) ) );
               xmm1 += a1 * B.load(k,j             );
               xmm2 += a1 * B.load(k,j+SIMDSIZE    );
               xmm3 += a1 * B.load(k,j+SIMDSIZE*2UL);
               xmm4 += a1 * B.load(k,j+SIMDSIZE*3UL);
            }

            (~C).store( i, j             , xmm1 );
            (~C).store( i, j+SIMDSIZE    , xmm2 );
            (~C).store( i, j+SIMDSIZE*2UL, xmm3 );
            (~C).store( i, j+SIMDSIZE*3UL, xmm4 );
         }
      }

      for( ; !LOW && !UPP && (j+SIMDSIZE*2UL) < jpos; j+=SIMDSIZE*3UL )
      {
         size_t i( 0UL );

         for( ; (i+2UL) <= M; i+=2UL )
         {
            const size_t kbegin( ( IsUpper<MT4>::value )
                                 ?( ( IsLower<MT5>::value )
                                    ?( max( ( IsStrictlyUpper<MT4>::value ? i+1UL : i ), j ) )
                                    :( IsStrictlyUpper<MT4>::value ? i+1UL : i ) )
                                 :( IsLower<MT5>::value ? j : 0UL ) );
            const size_t kend( ( IsLower<MT4>::value )
                               ?( ( IsUpper<MT5>::value )
                                  ?( min( ( IsStrictlyLower<MT4>::value ? i+1UL : i+2UL ), j+SIMDSIZE*3UL, K ) )
                                  :( IsStrictlyLower<MT4>::value ? i+1UL : i+2UL ) )
                               :( IsUpper<MT5>::value ? min( j+SIMDSIZE*3UL, K ) : K ) );

            SIMDType xmm1( (~C).load(i    ,j             ) );
            SIMDType xmm2( (~C).load(i    ,j+SIMDSIZE    ) );
            SIMDType xmm3( (~C).load(i    ,j+SIMDSIZE*2UL) );
            SIMDType xmm4( (~C).load(i+1UL,j             ) );
            SIMDType xmm5( (~C).load(i+1UL,j+SIMDSIZE    ) );
            SIMDType xmm6( (~C).load(i+1UL,j+SIMDSIZE*2UL) );

            for( size_t k=kbegin; k<kend; ++k ) {
               const SIMDType a1( set( A(i    ,k) ) );
               const SIMDType a2( set( A(i+1UL,k) ) );
               const SIMDType b1( B.load(k,j             ) );
               const SIMDType b2( B.load(k,j+SIMDSIZE    ) );
               const SIMDType b3( B.load(k,j+SIMDSIZE*2UL) );
               xmm1 += a1 * b1;
               xmm2 += a1 * b2;
               xmm3 += a1 * b3;
               xmm4 += a2 * b1;
               xmm5 += a2 * b2;
               xmm6 += a2 * b3;
            }

            (~C).store( i    , j             , xmm1 );
            (~C).store( i    , j+SIMDSIZE    , xmm2 );
            (~C).store( i    , j+SIMDSIZE*2UL, xmm3 );
            (~C).store( i+1UL, j             , xmm4 );
            (~C).store( i+1UL, j+SIMDSIZE    , xmm5 );
            (~C).store( i+1UL, j+SIMDSIZE*2UL, xmm6 );
         }

         if( i < M )
         {
            const size_t kbegin( ( IsUpper<MT4>::value )
                                 ?( ( IsLower<MT5>::value )
                                    ?( max( ( IsStrictlyUpper<MT4>::value ? i+1UL : i ), j ) )
                                    :( IsStrictlyUpper<MT4>::value ? i+1UL : i ) )
                                 :( IsLower<MT5>::value ? j : 0UL ) );
            const size_t kend( ( IsUpper<MT5>::value )?( min( j+SIMDSIZE*3UL, K ) ):( K ) );

            SIMDType xmm1( (~C).load(i,j             ) );
            SIMDType xmm2( (~C).load(i,j+SIMDSIZE    ) );
            SIMDType xmm3( (~C).load(i,j+SIMDSIZE*2UL) );

            for( size_t k=kbegin; k<kend; ++k ) {
               const SIMDType a1( set( A(i,k) ) );
               xmm1 += a1 * B.load(k,j             );
               xmm2 += a1 * B.load(k,j+SIMDSIZE    );
               xmm3 += a1 * B.load(k,j+SIMDSIZE*2UL);
            }

            (~C).store( i, j             , xmm1 );
            (~C).store( i, j+SIMDSIZE    , xmm2 );
            (~C).store( i, j+SIMDSIZE*2UL, xmm3 );
         }
      }

      for( ; !( LOW && UPP ) && (j+SIMDSIZE) < jpos; j+=SIMDSIZE*2UL )
      {
         const size_t iend( UPP ? min(j+SIMDSIZE*2UL,M) : M );
         size_t i( LOW ? j : 0UL );

         for( ; (i+2UL) <= iend; i+=2UL )
         {
            const size_t kbegin( ( IsUpper<MT4>::value )
                                 ?( ( IsLower<MT5>::value )
                                    ?( max( ( IsStrictlyUpper<MT4>::value ? i+1UL : i ), j ) )
                                    :( IsStrictlyUpper<MT4>::value ? i+1UL : i ) )
                                 :( IsLower<MT5>::value ? j : 0UL ) );
            const size_t kend( ( IsLower<MT4>::value )
                               ?( ( IsUpper<MT5>::value )
                                  ?( min( ( IsStrictlyLower<MT4>::value ? i+1UL : i+2UL ), j+SIMDSIZE*2UL, K ) )
                                  :( IsStrictlyLower<MT4>::value ? i+1UL : i+2UL ) )
                               :( IsUpper<MT5>::value ? min( j+SIMDSIZE*2UL, K ) : K ) );

            SIMDType xmm1( (~C).load(i    ,j         ) );
            SIMDType xmm2( (~C).load(i    ,j+SIMDSIZE) );
            SIMDType xmm3( (~C).load(i+1UL,j         ) );
            SIMDType xmm4( (~C).load(i+1UL,j+SIMDSIZE) );

            for( size_t k=kbegin; k<kend; ++k ) {
               const SIMDType a1( set( A(i    ,k) ) );
               const SIMDType a2( set( A(i+1UL,k) ) );
               const SIMDType b1( B.load(k,j         ) );
               const SIMDType b2( B.load(k,j+SIMDSIZE) );
               xmm1 += a1 * b1;
               xmm2 += a1 * b2;
               xmm3 += a2 * b1;
               xmm4 += a2 * b2;
            }

            (~C).store( i    , j         , xmm1 );
            (~C).store( i    , j+SIMDSIZE, xmm2 );
            (~C).store( i+1UL, j         , xmm3 );
            (~C).store( i+1UL, j+SIMDSIZE, xmm4 );
         }

         if( i < iend )
         {
            const size_t kbegin( ( IsUpper<MT4>::value )
                                 ?( ( IsLower<MT5>::value )
                                    ?( max( ( IsStrictlyUpper<MT4>::value ? i+1UL : i ), j ) )
                                    :( IsStrictlyUpper<MT4>::value ? i+1UL : i ) )
                                 :( IsLower<MT5>::value ? j : 0UL ) );
            const size_t kend( ( IsUpper<MT5>::value )?( min( j+SIMDSIZE*2UL, K ) ):( K ) );

            SIMDType xmm1( (~C).load(i,j         ) );
            SIMDType xmm2( (~C).load(i,j+SIMDSIZE) );

            for( size_t k=kbegin; k<kend; ++k ) {
               const SIMDType a1( set( A(i,k) ) );
               xmm1 += a1 * B.load(k,j         );
               xmm2 += a1 * B.load(k,j+SIMDSIZE);
            }

            (~C).store( i, j         , xmm1 );
            (~C).store( i, j+SIMDSIZE, xmm2 );
         }
      }

      for( ; j<jpos; j+=SIMDSIZE )
      {
         const size_t iend( LOW && UPP ? min(j+SIMDSIZE,M) : M );
         size_t i( LOW ? j : 0UL );

         for( ; (i+2UL) <= iend; i+=2UL )
         {
            const size_t kbegin( ( IsUpper<MT4>::value )
                                 ?( ( IsLower<MT5>::value )
                                    ?( max( ( IsStrictlyUpper<MT4>::value ? i+1UL : i ), j ) )
                                    :( IsStrictlyUpper<MT4>::value ? i+1UL : i ) )
                                 :( IsLower<MT5>::value ? j : 0UL ) );
            const size_t kend( ( IsLower<MT4>::value )
                               ?( IsStrictlyLower<MT4>::value ? i+1UL : i+2UL )
                               :( K ) );

            SIMDType xmm1( (~C).load(i    ,j) );
            SIMDType xmm2( (~C).load(i+1UL,j) );

            for( size_t k=kbegin; k<kend; ++k ) {
               const SIMDType b1( B.load(k,j) );
               xmm1 += set( A(i    ,k) ) * b1;
               xmm2 += set( A(i+1UL,k) ) * b1;
            }

            (~C).store( i    , j, xmm1 );
            (~C).store( i+1UL, j, xmm2 );
         }

         if( i < iend )
         {
            const size_t kbegin( ( IsUpper<MT4>::value )
                                 ?( ( IsLower<MT5>::value )
                                    ?( max( ( IsStrictlyUpper<MT4>::value ? i+1UL : i ), j ) )
                                    :( IsStrictlyUpper<MT4>::value ? i+1UL : i ) )
                                 :( IsLower<MT5>::value ? j : 0UL ) );

            SIMDType xmm1( (~C).load(i,j) );

            for( size_t k=kbegin; k<K; ++k ) {
               xmm1 += set( A(i,k) ) * B.load(k,j);
            }

            (~C).store( i, j, xmm1 );
         }
      }

      for( ; remainder && j<N; ++j )
      {
         const size_t iend( UPP ? j+1UL : M );
         size_t i( LOW ? j : 0UL );

         for( ; (i+2UL) <= iend; i+=2UL )
         {
            const size_t kbegin( ( IsUpper<MT4>::value )
                                 ?( ( IsLower<MT5>::value )
                                    ?( max( ( IsStrictlyUpper<MT4>::value ? i+1UL : i ), j ) )
                                    :( IsStrictlyUpper<MT4>::value ? i+1UL : i ) )
                                 :( IsLower<MT5>::value ? j : 0UL ) );
            const size_t kend( ( IsLower<MT4>::value )
                               ?( IsStrictlyLower<MT4>::value ? i+1UL : i+2UL )
                               :( K ) );

            ElementType value1( (~C)(i    ,j) );
            ElementType value2( (~C)(i+1UL,j) );;

            for( size_t k=kbegin; k<kend; ++k ) {
               value1 += A(i    ,k) * B(k,j);
               value2 += A(i+1UL,k) * B(k,j);
            }

            (~C)(i    ,j) = value1;
            (~C)(i+1UL,j) = value2;
         }

         if( i < iend )
         {
            const size_t kbegin( ( IsUpper<MT4>::value )
                                 ?( ( IsLower<MT5>::value )
                                    ?( max( ( IsStrictlyUpper<MT4>::value ? i+1UL : i ), j ) )
                                    :( IsStrictlyUpper<MT4>::value ? i+1UL : i ) )
                                 :( IsLower<MT5>::value ? j : 0UL ) );

            ElementType value( (~C)(i,j) );

            for( size_t k=kbegin; k<K; ++k ) {
               value += A(i,k) * B(k,j);
            }

            (~C)(i,j) = value;
         }
      }
   }
   /*! \endcond */
   //**********************************************************************************************

   //**Vectorized default addition assignment to column-major dense matrices (small matrices)******
   /*! \cond BLAZE_INTERNAL */
   /*!\brief Vectorized default addition assignment of a small dense matrix-dense matrix
   //        multiplication (\f$ C+=A*B \f$).
   // \ingroup dense_matrix
   //
   // \param C The target left-hand side dense matrix.
   // \param A The left-hand side multiplication operand.
   // \param B The right-hand side multiplication operand.
   // \return void
   //
   // This function implements the vectorized default addition assignment of a small dense
   // matrix-dense matrix multiplication expression to a column-major dense matrix. This
   // kernel is optimized for small matrices.
   */
   template< typename MT3    // Type of the left-hand side target matrix
           , typename MT4    // Type of the left-hand side matrix operand
           , typename MT5 >  // Type of the right-hand side matrix operand
   static inline EnableIf_< UseVectorizedDefaultKernel<MT3,MT4,MT5> >
      selectSmallAddAssignKernel( DenseMatrix<MT3,true>& C, const MT4& A, const MT5& B )
   {
      BLAZE_CONSTRAINT_MUST_NOT_BE_SYMMETRIC_MATRIX_TYPE( MT4 );
      BLAZE_CONSTRAINT_MUST_NOT_BE_SYMMETRIC_MATRIX_TYPE( MT5 );
      BLAZE_CONSTRAINT_MUST_BE_COLUMN_MAJOR_MATRIX_TYPE( OppositeType_<MT4> );
      BLAZE_CONSTRAINT_MUST_BE_COLUMN_MAJOR_MATRIX_TYPE( OppositeType_<MT5> );

      const ForwardFunctor fwd;

      if( !IsResizable<MT4>::value && IsResizable<MT5>::value ) {
         const OppositeType_<MT4> tmp( serial( A ) );
         addAssign( ~C, fwd( tmp * B ) );
      }
      else if( IsResizable<MT4>::value && !IsResizable<MT5>::value ) {
         const OppositeType_<MT5> tmp( serial( B ) );
         addAssign( ~C, fwd( A * tmp ) );
      }
      else if( A.rows() * A.columns() <= B.rows() * B.columns() ) {
         const OppositeType_<MT4> tmp( serial( A ) );
         addAssign( ~C, fwd( tmp * B ) );
      }
      else {
         const OppositeType_<MT5> tmp( serial( B ) );
         addAssign( ~C, fwd( A * tmp ) );
      }
   }
   /*! \endcond */
   //**********************************************************************************************

   //**Default addition assignment to dense matrices (large matrices)******************************
   /*! \cond BLAZE_INTERNAL */
   /*!\brief Default addition assignment of a large dense matrix-dense matrix multiplication
   //        (\f$ C+=A*B \f$).
   // \ingroup dense_matrix
   //
   // \param C The target left-hand side dense matrix.
   // \param A The left-hand side multiplication operand.
   // \param B The right-hand side multiplication operand.
   // \return void
   //
   // This function relays to the default implementation of the addition assignment of a dense
   // matrix-dense matrix multiplication expression to a dense matrix.
   */
   template< typename MT3    // Type of the left-hand side target matrix
           , typename MT4    // Type of the left-hand side matrix operand
           , typename MT5 >  // Type of the right-hand side matrix operand
   static inline DisableIf_< UseVectorizedDefaultKernel<MT3,MT4,MT5> >
      selectLargeAddAssignKernel( MT3& C, const MT4& A, const MT5& B )
   {
      selectDefaultAddAssignKernel( C, A, B );
   }
   /*! \endcond */
   //**********************************************************************************************

   //**Vectorized default addition assignment to dense matrices (large matrices)*******************
   /*! \cond BLAZE_INTERNAL */
   /*!\brief Vectorized default addition assignment of a large dense matrix-dense matrix
   //        multiplication (\f$ C+=A*B \f$).
   // \ingroup dense_matrix
   //
   // \param C The target left-hand side dense matrix.
   // \param A The left-hand side multiplication operand.
   // \param B The right-hand side multiplication operand.
   // \return void
   //
   // This function implements the vectorized default addition assignment of a dense matrix-
   // dense matrix multiplication expression to a dense matrix. This kernel is optimized for
   // large matrices.
   */
   template< typename MT3    // Type of the left-hand side target matrix
           , typename MT4    // Type of the left-hand side matrix operand
           , typename MT5 >  // Type of the right-hand side matrix operand
   static inline EnableIf_< UseVectorizedDefaultKernel<MT3,MT4,MT5> >
      selectLargeAddAssignKernel( MT3& C, const MT4& A, const MT5& B )
   {
      if( LOW )
         lmmm( C, A, B, ElementType(1), ElementType(1) );
      else if( UPP )
         ummm( C, A, B, ElementType(1), ElementType(1) );
      else
         mmm( C, A, B, ElementType(1), ElementType(1) );
   }
   /*! \endcond */
   //**********************************************************************************************

   //**BLAS-based addition assignment to dense matrices (default)**********************************
   /*! \cond BLAZE_INTERNAL */
   /*!\brief Default addition assignment of a dense matrix-dense matrix multiplication
   //        (\f$ C+=A*B \f$).
   // \ingroup dense_matrix
   //
   // \param C The target left-hand side dense matrix.
   // \param A The left-hand side multiplication operand.
   // \param B The right-hand side multiplication operand.
   // \return void
   //
   // This function relays to the default implementation of the addition assignment of a large
   // dense matrix-dense matrix multiplication expression to a dense matrix.
   */
   template< typename MT3    // Type of the left-hand side target matrix
           , typename MT4    // Type of the left-hand side matrix operand
           , typename MT5 >  // Type of the right-hand side matrix operand
   static inline DisableIf_< UseBlasKernel<MT3,MT4,MT5> >
      selectBlasAddAssignKernel( MT3& C, const MT4& A, const MT5& B )
   {
      selectLargeAddAssignKernel( C, A, B );
   }
   /*! \endcond */
   //**********************************************************************************************

   //**BLAS-based addition assignment to dense matrices********************************************
#if BLAZE_BLAS_MODE && BLAZE_USE_BLAS_MATRIX_MATRIX_MULTIPLICATION
   /*! \cond BLAZE_INTERNAL */
   /*!\brief BLAS-based addition assignment of a dense matrix-dense matrix multiplication
   //        (\f$ C+=A*B \f$).
   // \ingroup dense_matrix
   //
   // \param C The target left-hand side dense matrix.
   // \param A The left-hand side multiplication operand.
   // \param B The right-hand side multiplication operand.
   // \return void
   //
   // This function performs the dense matrix-dense matrix multiplication based on the according
   // BLAS functionality.
   */
   template< typename MT3    // Type of the left-hand side target matrix
           , typename MT4    // Type of the left-hand side matrix operand
           , typename MT5 >  // Type of the right-hand side matrix operand
   static inline EnableIf_< UseBlasKernel<MT3,MT4,MT5> >
      selectBlasAddAssignKernel( MT3& C, const MT4& A, const MT5& B )
   {
      typedef ElementType_<MT3>  ET;

      if( IsTriangular<MT4>::value ) {
         ResultType_<MT3> tmp( serial( B ) );
         trmm( tmp, A, CblasLeft, ( IsLower<MT4>::value )?( CblasLower ):( CblasUpper ), ET(1) );
         addAssign( C, tmp );
      }
      else if( IsTriangular<MT5>::value ) {
         ResultType_<MT3> tmp( serial( A ) );
         trmm( tmp, B, CblasRight, ( IsLower<MT5>::value )?( CblasLower ):( CblasUpper ), ET(1) );
         addAssign( C, tmp );
      }
      else {
         gemm( C, A, B, ET(1), ET(1) );
      }
   }
   /*! \endcond */
#endif
   //**********************************************************************************************

   //**Restructuring addition assignment to column-major matrices**********************************
   /*! \cond BLAZE_INTERNAL */
   /*!\brief Restructuring addition assignment of a dense matrix-dense matrix multiplication to a
   //        column-major matrix (\f$ C+=A*B \f$).
   // \ingroup dense_matrix
   //
   // \param lhs The target left-hand side matrix.
   // \param rhs The right-hand side multiplication expression to be added.
   // \return void
   //
   // This function implements the symmetry-based restructuring addition assignment of a dense
   // matrix-dense matrix multiplication expression to a column-major matrix. Due to the explicit
   // application of the SFINAE principle this function can only be selected by the compiler
   // in case the symmetry of either of the two matrix operands can be exploited.
   */
   template< typename MT >  // Type of the target matrix
   friend inline EnableIf_< CanExploitSymmetry<MT,MT1,MT2> >
      addAssign( Matrix<MT,true>& lhs, const DMatDMatMultExpr& rhs )
   {
      BLAZE_FUNCTION_TRACE;

      BLAZE_CONSTRAINT_MUST_NOT_BE_SYMMETRIC_MATRIX_TYPE( MT );

      BLAZE_INTERNAL_ASSERT( (~lhs).rows()    == rhs.rows()   , "Invalid number of rows"    );
      BLAZE_INTERNAL_ASSERT( (~lhs).columns() == rhs.columns(), "Invalid number of columns" );

      const ForwardFunctor fwd;

      if( IsSymmetric<MT1>::value && IsSymmetric<MT2>::value )
         addAssign( ~lhs, fwd( trans( rhs.lhs_ ) * trans( rhs.rhs_ ) ) );
      else if( IsSymmetric<MT1>::value )
         addAssign( ~lhs, fwd( trans( rhs.lhs_ ) * rhs.rhs_ ) );
      else
         addAssign( ~lhs, fwd( rhs.lhs_ * trans( rhs.rhs_ ) ) );
   }
   /*! \endcond */
   //**********************************************************************************************

   //**Addition assignment to sparse matrices******************************************************
   // No special implementation for the addition assignment to sparse matrices.
   //**********************************************************************************************

   //**Subtraction assignment to dense matrices****************************************************
   /*! \cond BLAZE_INTERNAL */
   /*!\brief Subtraction assignment of a dense matrix-dense matrix multiplication to a
   //        dense matrix (\f$ C-=A*B \f$).
   // \ingroup dense_matrix
   //
   // \param lhs The target left-hand side dense matrix.
   // \param rhs The right-hand side multiplication expression to be subtracted.
   // \return void
   //
   // This function implements the performance optimized subtraction assignment of a dense matrix-
   // dense matrix multiplication expression to a dense matrix.
   */
   template< typename MT  // Type of the target dense matrix
           , bool SO >    // Storage order of the target dense matrix
   friend inline DisableIf_< CanExploitSymmetry<MT,MT1,MT2> >
      subAssign( DenseMatrix<MT,SO>& lhs, const DMatDMatMultExpr& rhs )
   {
      BLAZE_FUNCTION_TRACE;

      BLAZE_INTERNAL_ASSERT( (~lhs).rows()    == rhs.rows()   , "Invalid number of rows"    );
      BLAZE_INTERNAL_ASSERT( (~lhs).columns() == rhs.columns(), "Invalid number of columns" );

      if( (~lhs).rows() == 0UL || (~lhs).columns() == 0UL || rhs.lhs_.columns() == 0UL ) {
         return;
      }

      LT A( serial( rhs.lhs_ ) );  // Evaluation of the left-hand side dense matrix operand
      RT B( serial( rhs.rhs_ ) );  // Evaluation of the right-hand side dense matrix operand

      BLAZE_INTERNAL_ASSERT( A.rows()    == rhs.lhs_.rows()   , "Invalid number of rows"    );
      BLAZE_INTERNAL_ASSERT( A.columns() == rhs.lhs_.columns(), "Invalid number of columns" );
      BLAZE_INTERNAL_ASSERT( B.rows()    == rhs.rhs_.rows()   , "Invalid number of rows"    );
      BLAZE_INTERNAL_ASSERT( B.columns() == rhs.rhs_.columns(), "Invalid number of columns" );
      BLAZE_INTERNAL_ASSERT( A.rows()    == (~lhs).rows()     , "Invalid number of rows"    );
      BLAZE_INTERNAL_ASSERT( B.columns() == (~lhs).columns()  , "Invalid number of columns" );

      DMatDMatMultExpr::selectSubAssignKernel( ~lhs, A, B );
   }
   /*! \endcond */
   //**********************************************************************************************

   //**Subtraction assignment to dense matrices (kernel selection)*********************************
   /*! \cond BLAZE_INTERNAL */
   /*!\brief Selection of the kernel for a subtraction assignment of a dense matrix-dense matrix
   //        multiplication to a dense matrix (\f$ C-=A*B \f$).
   // \ingroup dense_matrix
   //
   // \param C The target left-hand side dense matrix.
   // \param A The left-hand side multiplication operand.
   // \param B The right-hand side multiplication operand.
   // \return void
   */
   template< typename MT3    // Type of the left-hand side target matrix
           , typename MT4    // Type of the left-hand side matrix operand
           , typename MT5 >  // Type of the right-hand side matrix operand
   static inline void selectSubAssignKernel( MT3& C, const MT4& A, const MT5& B )
   {
      if( ( IsDiagonal<MT5>::value ) ||
          ( !BLAZE_DEBUG_MODE && B.columns() <= SIMDSIZE*10UL ) ||
          ( C.rows() * C.columns() < DMATDMATMULT_THRESHOLD ) )
         selectSmallSubAssignKernel( C, A, B );
      else
         selectBlasSubAssignKernel( C, A, B );
   }
   /*! \endcond */
   //**********************************************************************************************

   //**Default subtraction assignment to dense matrices (general/general)**************************
   /*! \cond BLAZE_INTERNAL */
   /*!\brief Default subtraction assignment of a general dense matrix-general dense matrix
   //        multiplication (\f$ C-=A*B \f$).
   // \ingroup dense_matrix
   //
   // \param C The target left-hand side dense matrix.
   // \param A The left-hand side multiplication operand.
   // \param B The right-hand side multiplication operand.
   // \return void
   //
   // This function implements the default subtraction assignment of a general dense matrix-
   // general dense matrix multiplication expression to a dense matrix.
   */
   template< typename MT3    // Type of the left-hand side target matrix
           , typename MT4    // Type of the left-hand side matrix operand
           , typename MT5 >  // Type of the right-hand side matrix operand
   static inline EnableIf_< And< Not< IsDiagonal<MT4> >, Not< IsDiagonal<MT5> > > >
      selectDefaultSubAssignKernel( MT3& C, const MT4& A, const MT5& B )
   {
      const size_t M( A.rows()    );
      const size_t N( B.columns() );
      const size_t K( A.columns() );

      BLAZE_INTERNAL_ASSERT( !( LOW || UPP ) || ( M == N ), "Broken invariant detected" );

      for( size_t i=0UL; i<M; ++i )
      {
         const size_t kbegin( ( IsUpper<MT4>::value )
                              ?( IsStrictlyUpper<MT4>::value ? i+1UL : i )
                              :( 0UL ) );
         const size_t kend( ( IsLower<MT4>::value )
                            ?( IsStrictlyLower<MT4>::value ? i : i+1UL )
                            :( K ) );
         BLAZE_INTERNAL_ASSERT( kbegin <= kend, "Invalid loop indices detected" );

         for( size_t k=kbegin; k<kend; ++k )
         {
            const size_t jbegin( ( IsUpper<MT5>::value )
                                 ?( ( IsStrictlyUpper<MT5>::value )
                                    ?( UPP ? max(i,k+1UL) : k+1UL )
                                    :( UPP ? max(i,k) : k ) )
                                 :( UPP ? i : 0UL ) );
            const size_t jend( ( IsLower<MT5>::value )
                               ?( ( IsStrictlyLower<MT5>::value )
                                  ?( LOW ? min(i+1UL,k) : k )
                                  :( LOW ? min(i,k)+1UL : k+1UL ) )
                               :( LOW ? i+1UL : N ) );

            if( ( LOW || UPP ) && ( jbegin >= jend ) ) continue;
            BLAZE_INTERNAL_ASSERT( jbegin <= jend, "Invalid loop indices detected" );

            const size_t jnum( jend - jbegin );
            const size_t jpos( jbegin + ( jnum & size_t(-2) ) );

            for( size_t j=jbegin; j<jpos; j+=2UL ) {
               C(i,j    ) -= A(i,k) * B(k,j    );
               C(i,j+1UL) -= A(i,k) * B(k,j+1UL);
            }
            if( jpos < jend ) {
               C(i,jpos) -= A(i,k) * B(k,jpos);
            }
         }
      }
   }
   /*! \endcond */
   //**********************************************************************************************

   //**Default subtraction assignment to dense matrices (general/diagonal)*************************
   /*! \cond BLAZE_INTERNAL */
   /*!\brief Default subtraction assignment of a general dense matrix-diagonal dense matrix
   //        multiplication (\f$ C-=A*B \f$).
   // \ingroup dense_matrix
   //
   // \param C The target left-hand side dense matrix.
   // \param A The left-hand side multiplication operand.
   // \param B The right-hand side multiplication operand.
   // \return void
   //
   // This function implements the default subtraction assignment of a general dense matrix-
   // diagonal dense matrix multiplication expression to a dense matrix.
   */
   template< typename MT3    // Type of the left-hand side target matrix
           , typename MT4    // Type of the left-hand side matrix operand
           , typename MT5 >  // Type of the right-hand side matrix operand
   static inline EnableIf_< And< Not< IsDiagonal<MT4> >, IsDiagonal<MT5> > >
      selectDefaultSubAssignKernel( MT3& C, const MT4& A, const MT5& B )
   {
      BLAZE_CONSTRAINT_MUST_BE_ROW_MAJOR_MATRIX_TYPE( MT3 );

      const size_t M( A.rows()    );
      const size_t N( B.columns() );

      for( size_t i=0UL; i<M; ++i )
      {
         const size_t jbegin( ( IsUpper<MT4>::value )
                              ?( IsStrictlyUpper<MT4>::value ? i+1UL : i )
                              :( 0UL ) );
         const size_t jend( ( IsLower<MT4>::value )
                            ?( IsStrictlyLower<MT4>::value ? i : i+1UL )
                            :( N ) );
         BLAZE_INTERNAL_ASSERT( jbegin <= jend, "Invalid loop indices detected" );

         const size_t jnum( jend - jbegin );
         const size_t jpos( jbegin + ( jnum & size_t(-2) ) );

         for( size_t j=jbegin; j<jpos; j+=2UL ) {
            C(i,j    ) -= A(i,j    ) * B(j    ,j    );
            C(i,j+1UL) -= A(i,j+1UL) * B(j+1UL,j+1UL);
         }
         if( jpos < jend ) {
            C(i,jpos) -= A(i,jpos) * B(jpos,jpos);
         }
      }
   }
   /*! \endcond */
   //**********************************************************************************************

   //**Default subtraction assignment to dense matrices (diagonal/general)*************************
   /*! \cond BLAZE_INTERNAL */
   /*!\brief Default subtraction assignment of a diagonal dense matrix-general dense matrix
   //        multiplication (\f$ C-=A*B \f$).
   // \ingroup dense_matrix
   //
   // \param C The target left-hand side dense matrix.
   // \param A The left-hand side multiplication operand.
   // \param B The right-hand side multiplication operand.
   // \return void
   //
   // This function implements the default subtraction assignment of a diagonal dense matrix-
   // general dense matrix multiplication expression to a dense matrix.
   */
   template< typename MT3    // Type of the left-hand side target matrix
           , typename MT4    // Type of the left-hand side matrix operand
           , typename MT5 >  // Type of the right-hand side matrix operand
   static inline EnableIf_< And< IsDiagonal<MT4>, Not< IsDiagonal<MT5> > > >
      selectDefaultSubAssignKernel( MT3& C, const MT4& A, const MT5& B )
   {
      BLAZE_CONSTRAINT_MUST_BE_ROW_MAJOR_MATRIX_TYPE( MT3 );

      const size_t M( A.rows()    );
      const size_t N( B.columns() );

      for( size_t i=0UL; i<M; ++i )
      {
         const size_t jbegin( ( IsUpper<MT5>::value )
                              ?( IsStrictlyUpper<MT5>::value ? i+1UL : i )
                              :( 0UL ) );
         const size_t jend( ( IsLower<MT5>::value )
                            ?( IsStrictlyLower<MT5>::value ? i : i+1UL )
                            :( N ) );
         BLAZE_INTERNAL_ASSERT( jbegin <= jend, "Invalid loop indices detected" );

         const size_t jnum( jend - jbegin );
         const size_t jpos( jbegin + ( jnum & size_t(-2) ) );

         for( size_t j=jbegin; j<jpos; j+=2UL ) {
            C(i,j    ) -= A(i,i) * B(i,j    );
            C(i,j+1UL) -= A(i,i) * B(i,j+1UL);
         }
         if( jpos < jend ) {
            C(i,jpos) -= A(i,i) * B(i,jpos);
         }
      }
   }
   /*! \endcond */
   //**********************************************************************************************

   //**Default subtraction assignment to dense matrices (diagonal/diagonal)************************
   /*! \cond BLAZE_INTERNAL */
   /*!\brief Default subtraction assignment of a diagonal dense matrix-diagonal dense matrix
   //        multiplication (\f$ C-=A*B \f$).
   // \ingroup dense_matrix
   //
   // \param C The target left-hand side dense matrix.
   // \param A The left-hand side multiplication operand.
   // \param B The right-hand side multiplication operand.
   // \return void
   //
   // This function implements the default subtraction assignment of a diagonal dense matrix-
   // diagonal dense matrix multiplication expression to a dense matrix.
   */
   template< typename MT3    // Type of the left-hand side target matrix
           , typename MT4    // Type of the left-hand side matrix operand
           , typename MT5 >  // Type of the right-hand side matrix operand
   static inline EnableIf_< And< IsDiagonal<MT4>, IsDiagonal<MT5> > >
      selectDefaultSubAssignKernel( MT3& C, const MT4& A, const MT5& B )
   {
      BLAZE_CONSTRAINT_MUST_BE_ROW_MAJOR_MATRIX_TYPE( MT3 );

      for( size_t i=0UL; i<A.rows(); ++i ) {
         C(i,i) -= A(i,i) * B(i,i);
      }
   }
   /*! \endcond */
   //**********************************************************************************************

   //**Default subtraction assignment to dense matrices (small matrices)***************************
   /*! \cond BLAZE_INTERNAL */
   /*!\brief Default subtraction assignment of a small dense matrix-dense matrix multiplication
   //        (\f$ C-=A*B \f$).
   // \ingroup dense_matrix
   //
   // \param C The target left-hand side dense matrix.
   // \param A The left-hand side multiplication operand.
   // \param B The right-hand side multiplication operand.
   // \return void
   //
   // This function relays to the default implementation of the subtraction assignment of a dense
   // matrix-dense matrix multiplication expression to a dense matrix.
   */
   template< typename MT3    // Type of the left-hand side target matrix
           , typename MT4    // Type of the left-hand side matrix operand
           , typename MT5 >  // Type of the right-hand side matrix operand
   static inline DisableIf_< UseVectorizedDefaultKernel<MT3,MT4,MT5> >
      selectSmallSubAssignKernel( MT3& C, const MT4& A, const MT5& B )
   {
      selectDefaultSubAssignKernel( C, A, B );
   }
   /*! \endcond */
   //**********************************************************************************************

   //**Vectorized default subtraction assignment to row-major dense matrices (small matrices)******
   /*! \cond BLAZE_INTERNAL */
   /*!\brief Vectorized default subtraction assignment of a small dense matrix-dense matrix
   //        multiplication (\f$ C-=A*B \f$).
   // \ingroup dense_matrix
   //
   // \param C The target left-hand side dense matrix.
   // \param A The left-hand side multiplication operand.
   // \param B The right-hand side multiplication operand.
   // \return void
   //
   // This function implements the vectorized default subtraction assignment of a dense matrix-
   // dense matrix multiplication expression to a row-major dense matrix. This kernel is optimized
   // for small matrices.
   */
   template< typename MT3    // Type of the left-hand side target matrix
           , typename MT4    // Type of the left-hand side matrix operand
           , typename MT5 >  // Type of the right-hand side matrix operand
   static inline EnableIf_< UseVectorizedDefaultKernel<MT3,MT4,MT5> >
      selectSmallSubAssignKernel( DenseMatrix<MT3,false>& C, const MT4& A, const MT5& B )
   {
      constexpr bool remainder( !IsPadded<MT3>::value || !IsPadded<MT5>::value );

      const size_t M( A.rows()    );
      const size_t N( B.columns() );
      const size_t K( A.columns() );

      BLAZE_INTERNAL_ASSERT( !( LOW || UPP ) || ( M == N ), "Broken invariant detected" );

      const size_t jpos( remainder ? ( N & size_t(-SIMDSIZE) ) : N );
      BLAZE_INTERNAL_ASSERT( !remainder || ( N - ( N % SIMDSIZE ) ) == jpos, "Invalid end calculation" );

      size_t j( 0UL );

      if( IsIntegral<ElementType>::value )
      {
         for( ; !LOW && !UPP && (j+SIMDSIZE*7UL) < jpos; j+=SIMDSIZE*8UL ) {
            for( size_t i=0UL; i<M; ++i )
            {
               const size_t kbegin( ( IsUpper<MT4>::value )
                                    ?( ( IsLower<MT5>::value )
                                       ?( max( ( IsStrictlyUpper<MT4>::value ? i+1UL : i ), j ) )
                                       :( IsStrictlyUpper<MT4>::value ? i+1UL : i ) )
                                    :( IsLower<MT5>::value ? j : 0UL ) );
               const size_t kend( ( IsLower<MT4>::value )
                                  ?( ( IsUpper<MT5>::value )
                                     ?( min( ( IsStrictlyLower<MT4>::value ? i : i+1UL ), j+SIMDSIZE*8UL, K ) )
                                     :( IsStrictlyLower<MT4>::value ? i : i+1UL ) )
                                  :( IsUpper<MT5>::value ? min( j+SIMDSIZE*8UL, K ) : K ) );

               SIMDType xmm1( (~C).load(i,j             ) );
               SIMDType xmm2( (~C).load(i,j+SIMDSIZE    ) );
               SIMDType xmm3( (~C).load(i,j+SIMDSIZE*2UL) );
               SIMDType xmm4( (~C).load(i,j+SIMDSIZE*3UL) );
               SIMDType xmm5( (~C).load(i,j+SIMDSIZE*4UL) );
               SIMDType xmm6( (~C).load(i,j+SIMDSIZE*5UL) );
               SIMDType xmm7( (~C).load(i,j+SIMDSIZE*6UL) );
               SIMDType xmm8( (~C).load(i,j+SIMDSIZE*7UL) );

               for( size_t k=kbegin; k<kend; ++k ) {
                  const SIMDType a1( set( A(i,k) ) );
                  xmm1 -= a1 * B.load(k,j             );
                  xmm2 -= a1 * B.load(k,j+SIMDSIZE    );
                  xmm3 -= a1 * B.load(k,j+SIMDSIZE*2UL);
                  xmm4 -= a1 * B.load(k,j+SIMDSIZE*3UL);
                  xmm5 -= a1 * B.load(k,j+SIMDSIZE*4UL);
                  xmm6 -= a1 * B.load(k,j+SIMDSIZE*5UL);
                  xmm7 -= a1 * B.load(k,j+SIMDSIZE*6UL);
                  xmm8 -= a1 * B.load(k,j+SIMDSIZE*7UL);
               }

               (~C).store( i, j             , xmm1 );
               (~C).store( i, j+SIMDSIZE    , xmm2 );
               (~C).store( i, j+SIMDSIZE*2UL, xmm3 );
               (~C).store( i, j+SIMDSIZE*3UL, xmm4 );
               (~C).store( i, j+SIMDSIZE*4UL, xmm5 );
               (~C).store( i, j+SIMDSIZE*5UL, xmm6 );
               (~C).store( i, j+SIMDSIZE*6UL, xmm7 );
               (~C).store( i, j+SIMDSIZE*7UL, xmm8 );
            }
         }
      }

      for( ; !LOW && !UPP && (j+SIMDSIZE*4UL) < jpos; j+=SIMDSIZE*5UL )
      {
         size_t i( 0UL );

         for( ; (i+2UL) <= M; i+=2UL )
         {
            const size_t kbegin( ( IsUpper<MT4>::value )
                                 ?( ( IsLower<MT5>::value )
                                    ?( max( ( IsStrictlyUpper<MT4>::value ? i+1UL : i ), j ) )
                                    :( IsStrictlyUpper<MT4>::value ? i+1UL : i ) )
                                 :( IsLower<MT5>::value ? j : 0UL ) );
            const size_t kend( ( IsLower<MT4>::value )
                               ?( ( IsUpper<MT5>::value )
                                  ?( min( ( IsStrictlyLower<MT4>::value ? i+1UL : i+2UL ), j+SIMDSIZE*5UL, K ) )
                                  :( IsStrictlyLower<MT4>::value ? i+1UL : i+2UL ) )
                               :( IsUpper<MT5>::value ? min( j+SIMDSIZE*5UL, K ) : K ) );

            SIMDType xmm1 ( (~C).load(i    ,j             ) );
            SIMDType xmm2 ( (~C).load(i    ,j+SIMDSIZE    ) );
            SIMDType xmm3 ( (~C).load(i    ,j+SIMDSIZE*2UL) );
            SIMDType xmm4 ( (~C).load(i    ,j+SIMDSIZE*3UL) );
            SIMDType xmm5 ( (~C).load(i    ,j+SIMDSIZE*4UL) );
            SIMDType xmm6 ( (~C).load(i+1UL,j             ) );
            SIMDType xmm7 ( (~C).load(i+1UL,j+SIMDSIZE    ) );
            SIMDType xmm8 ( (~C).load(i+1UL,j+SIMDSIZE*2UL) );
            SIMDType xmm9 ( (~C).load(i+1UL,j+SIMDSIZE*3UL) );
            SIMDType xmm10( (~C).load(i+1UL,j+SIMDSIZE*4UL) );

            for( size_t k=kbegin; k<kend; ++k ) {
               const SIMDType a1( set( A(i    ,k) ) );
               const SIMDType a2( set( A(i+1UL,k) ) );
               const SIMDType b1( B.load(k,j             ) );
               const SIMDType b2( B.load(k,j+SIMDSIZE    ) );
               const SIMDType b3( B.load(k,j+SIMDSIZE*2UL) );
               const SIMDType b4( B.load(k,j+SIMDSIZE*3UL) );
               const SIMDType b5( B.load(k,j+SIMDSIZE*4UL) );
               xmm1  -= a1 * b1;
               xmm2  -= a1 * b2;
               xmm3  -= a1 * b3;
               xmm4  -= a1 * b4;
               xmm5  -= a1 * b5;
               xmm6  -= a2 * b1;
               xmm7  -= a2 * b2;
               xmm8  -= a2 * b3;
               xmm9  -= a2 * b4;
               xmm10 -= a2 * b5;
            }

            (~C).store( i    , j             , xmm1  );
            (~C).store( i    , j+SIMDSIZE    , xmm2  );
            (~C).store( i    , j+SIMDSIZE*2UL, xmm3  );
            (~C).store( i    , j+SIMDSIZE*3UL, xmm4  );
            (~C).store( i    , j+SIMDSIZE*4UL, xmm5  );
            (~C).store( i+1UL, j             , xmm6  );
            (~C).store( i+1UL, j+SIMDSIZE    , xmm7  );
            (~C).store( i+1UL, j+SIMDSIZE*2UL, xmm8  );
            (~C).store( i+1UL, j+SIMDSIZE*3UL, xmm9  );
            (~C).store( i+1UL, j+SIMDSIZE*4UL, xmm10 );
         }

         if( i < M )
         {
            const size_t kbegin( ( IsUpper<MT4>::value )
                                 ?( ( IsLower<MT5>::value )
                                    ?( max( ( IsStrictlyUpper<MT4>::value ? i+1UL : i ), j ) )
                                    :( IsStrictlyUpper<MT4>::value ? i+1UL : i ) )
                                 :( IsLower<MT5>::value ? j : 0UL ) );
            const size_t kend( ( IsUpper<MT5>::value )?( min( j+SIMDSIZE*5UL, K ) ):( K ) );

            SIMDType xmm1( (~C).load(i,j             ) );
            SIMDType xmm2( (~C).load(i,j+SIMDSIZE    ) );
            SIMDType xmm3( (~C).load(i,j+SIMDSIZE*2UL) );
            SIMDType xmm4( (~C).load(i,j+SIMDSIZE*3UL) );
            SIMDType xmm5( (~C).load(i,j+SIMDSIZE*4UL) );

            for( size_t k=kbegin; k<kend; ++k ) {
               const SIMDType a1( set( A(i,k) ) );
               xmm1 -= a1 * B.load(k,j             );
               xmm2 -= a1 * B.load(k,j+SIMDSIZE    );
               xmm3 -= a1 * B.load(k,j+SIMDSIZE*2UL);
               xmm4 -= a1 * B.load(k,j+SIMDSIZE*3UL);
               xmm5 -= a1 * B.load(k,j+SIMDSIZE*4UL);
            }

            (~C).store( i, j             , xmm1 );
            (~C).store( i, j+SIMDSIZE    , xmm2 );
            (~C).store( i, j+SIMDSIZE*2UL, xmm3 );
            (~C).store( i, j+SIMDSIZE*3UL, xmm4 );
            (~C).store( i, j+SIMDSIZE*4UL, xmm5 );
         }
      }

      for( ; !LOW && !UPP && (j+SIMDSIZE*3UL) < jpos; j+=SIMDSIZE*4UL )
      {
         size_t i( 0UL );

         for( ; (i+2UL) <= M; i+=2UL )
         {
            const size_t kbegin( ( IsUpper<MT4>::value )
                                 ?( ( IsLower<MT5>::value )
                                    ?( max( ( IsStrictlyUpper<MT4>::value ? i+1UL : i ), j ) )
                                    :( IsStrictlyUpper<MT4>::value ? i+1UL : i ) )
                                 :( IsLower<MT5>::value ? j : 0UL ) );
            const size_t kend( ( IsLower<MT4>::value )
                               ?( ( IsUpper<MT5>::value )
                                  ?( min( ( IsStrictlyLower<MT4>::value ? i+1UL : i+2UL ), j+SIMDSIZE*4UL, K ) )
                                  :( IsStrictlyLower<MT4>::value ? i+1UL : i+2UL ) )
                               :( IsUpper<MT5>::value ? min( j+SIMDSIZE*4UL, K ) : K ) );

            SIMDType xmm1( (~C).load(i    ,j             ) );
            SIMDType xmm2( (~C).load(i    ,j+SIMDSIZE    ) );
            SIMDType xmm3( (~C).load(i    ,j+SIMDSIZE*2UL) );
            SIMDType xmm4( (~C).load(i    ,j+SIMDSIZE*3UL) );
            SIMDType xmm5( (~C).load(i+1UL,j             ) );
            SIMDType xmm6( (~C).load(i+1UL,j+SIMDSIZE    ) );
            SIMDType xmm7( (~C).load(i+1UL,j+SIMDSIZE*2UL) );
            SIMDType xmm8( (~C).load(i+1UL,j+SIMDSIZE*3UL) );

            for( size_t k=kbegin; k<kend; ++k ) {
               const SIMDType a1( set( A(i    ,k) ) );
               const SIMDType a2( set( A(i+1UL,k) ) );
               const SIMDType b1( B.load(k,j             ) );
               const SIMDType b2( B.load(k,j+SIMDSIZE    ) );
               const SIMDType b3( B.load(k,j+SIMDSIZE*2UL) );
               const SIMDType b4( B.load(k,j+SIMDSIZE*3UL) );
               xmm1 -= a1 * b1;
               xmm2 -= a1 * b2;
               xmm3 -= a1 * b3;
               xmm4 -= a1 * b4;
               xmm5 -= a2 * b1;
               xmm6 -= a2 * b2;
               xmm7 -= a2 * b3;
               xmm8 -= a2 * b4;
            }

            (~C).store( i    , j             , xmm1 );
            (~C).store( i    , j+SIMDSIZE    , xmm2 );
            (~C).store( i    , j+SIMDSIZE*2UL, xmm3 );
            (~C).store( i    , j+SIMDSIZE*3UL, xmm4 );
            (~C).store( i+1UL, j             , xmm5 );
            (~C).store( i+1UL, j+SIMDSIZE    , xmm6 );
            (~C).store( i+1UL, j+SIMDSIZE*2UL, xmm7 );
            (~C).store( i+1UL, j+SIMDSIZE*3UL, xmm8 );
         }

         if( i < M )
         {
            const size_t kbegin( ( IsUpper<MT4>::value )
                                 ?( ( IsLower<MT5>::value )
                                    ?( max( ( IsStrictlyUpper<MT4>::value ? i+1UL : i ), j ) )
                                    :( IsStrictlyUpper<MT4>::value ? i+1UL : i ) )
                                 :( IsLower<MT5>::value ? j : 0UL ) );
            const size_t kend( ( IsUpper<MT5>::value )?( min( j+SIMDSIZE*4UL, K ) ):( K ) );

            SIMDType xmm1( (~C).load(i,j             ) );
            SIMDType xmm2( (~C).load(i,j+SIMDSIZE    ) );
            SIMDType xmm3( (~C).load(i,j+SIMDSIZE*2UL) );
            SIMDType xmm4( (~C).load(i,j+SIMDSIZE*3UL) );

            for( size_t k=kbegin; k<kend; ++k ) {
               const SIMDType a1( set( A(i,k) ) );
               xmm1 -= a1 * B.load(k,j             );
               xmm2 -= a1 * B.load(k,j+SIMDSIZE    );
               xmm3 -= a1 * B.load(k,j+SIMDSIZE*2UL);
               xmm4 -= a1 * B.load(k,j+SIMDSIZE*3UL);
            }

            (~C).store( i, j             , xmm1 );
            (~C).store( i, j+SIMDSIZE    , xmm2 );
            (~C).store( i, j+SIMDSIZE*2UL, xmm3 );
            (~C).store( i, j+SIMDSIZE*3UL, xmm4 );
         }
      }

      for( ; !LOW && !UPP && (j+SIMDSIZE*2UL) < jpos; j+=SIMDSIZE*3UL )
      {
         size_t i( 0UL );

         for( ; (i+2UL) <= M; i+=2UL )
         {
            const size_t kbegin( ( IsUpper<MT4>::value )
                                 ?( ( IsLower<MT5>::value )
                                    ?( max( ( IsStrictlyUpper<MT4>::value ? i+1UL : i ), j ) )
                                    :( IsStrictlyUpper<MT4>::value ? i+1UL : i ) )
                                 :( IsLower<MT5>::value ? j : 0UL ) );
            const size_t kend( ( IsLower<MT4>::value )
                               ?( ( IsUpper<MT5>::value )
                                  ?( min( ( IsStrictlyLower<MT4>::value ? i+1UL : i+2UL ), j+SIMDSIZE*3UL, K ) )
                                  :( IsStrictlyLower<MT4>::value ? i+1UL : i+2UL ) )
                               :( IsUpper<MT5>::value ? min( j+SIMDSIZE*3UL, K ) : K ) );

            SIMDType xmm1( (~C).load(i    ,j             ) );
            SIMDType xmm2( (~C).load(i    ,j+SIMDSIZE    ) );
            SIMDType xmm3( (~C).load(i    ,j+SIMDSIZE*2UL) );
            SIMDType xmm4( (~C).load(i+1UL,j             ) );
            SIMDType xmm5( (~C).load(i+1UL,j+SIMDSIZE    ) );
            SIMDType xmm6( (~C).load(i+1UL,j+SIMDSIZE*2UL) );

            for( size_t k=kbegin; k<kend; ++k ) {
               const SIMDType a1( set( A(i    ,k) ) );
               const SIMDType a2( set( A(i+1UL,k) ) );
               const SIMDType b1( B.load(k,j             ) );
               const SIMDType b2( B.load(k,j+SIMDSIZE    ) );
               const SIMDType b3( B.load(k,j+SIMDSIZE*2UL) );
               xmm1 -= a1 * b1;
               xmm2 -= a1 * b2;
               xmm3 -= a1 * b3;
               xmm4 -= a2 * b1;
               xmm5 -= a2 * b2;
               xmm6 -= a2 * b3;
            }

            (~C).store( i    , j             , xmm1 );
            (~C).store( i    , j+SIMDSIZE    , xmm2 );
            (~C).store( i    , j+SIMDSIZE*2UL, xmm3 );
            (~C).store( i+1UL, j             , xmm4 );
            (~C).store( i+1UL, j+SIMDSIZE    , xmm5 );
            (~C).store( i+1UL, j+SIMDSIZE*2UL, xmm6 );
         }

         if( i < M )
         {
            const size_t kbegin( ( IsUpper<MT4>::value )
                                 ?( ( IsLower<MT5>::value )
                                    ?( max( ( IsStrictlyUpper<MT4>::value ? i+1UL : i ), j ) )
                                    :( IsStrictlyUpper<MT4>::value ? i+1UL : i ) )
                                 :( IsLower<MT5>::value ? j : 0UL ) );
            const size_t kend( ( IsUpper<MT5>::value )?( min( j+SIMDSIZE*3UL, K ) ):( K ) );

            SIMDType xmm1( (~C).load(i,j             ) );
            SIMDType xmm2( (~C).load(i,j+SIMDSIZE    ) );
            SIMDType xmm3( (~C).load(i,j+SIMDSIZE*2UL) );

            for( size_t k=kbegin; k<kend; ++k ) {
               const SIMDType a1( set( A(i,k) ) );
               xmm1 -= a1 * B.load(k,j             );
               xmm2 -= a1 * B.load(k,j+SIMDSIZE    );
               xmm3 -= a1 * B.load(k,j+SIMDSIZE*2UL);
            }

            (~C).store( i, j             , xmm1 );
            (~C).store( i, j+SIMDSIZE    , xmm2 );
            (~C).store( i, j+SIMDSIZE*2UL, xmm3 );
         }
      }

      for( ; !( LOW && UPP ) && (j+SIMDSIZE) < jpos; j+=SIMDSIZE*2UL )
      {
         const size_t iend( UPP ? min(j+SIMDSIZE*2UL,M) : M );
         size_t i( LOW ? j : 0UL );

         for( ; (i+2UL) <= iend; i+=2UL )
         {
            const size_t kbegin( ( IsUpper<MT4>::value )
                                 ?( ( IsLower<MT5>::value )
                                    ?( max( ( IsStrictlyUpper<MT4>::value ? i+1UL : i ), j ) )
                                    :( IsStrictlyUpper<MT4>::value ? i+1UL : i ) )
                                 :( IsLower<MT5>::value ? j : 0UL ) );
            const size_t kend( ( IsLower<MT4>::value )
                               ?( ( IsUpper<MT5>::value )
                                  ?( min( ( IsStrictlyLower<MT4>::value ? i+1UL : i+2UL ), j+SIMDSIZE*2UL, K ) )
                                  :( IsStrictlyLower<MT4>::value ? i+1UL : i+2UL ) )
                               :( IsUpper<MT5>::value ? min( j+SIMDSIZE*2UL, K ) : K ) );

            SIMDType xmm1( (~C).load(i    ,j         ) );
            SIMDType xmm2( (~C).load(i    ,j+SIMDSIZE) );
            SIMDType xmm3( (~C).load(i+1UL,j         ) );
            SIMDType xmm4( (~C).load(i+1UL,j+SIMDSIZE) );

            for( size_t k=kbegin; k<kend; ++k ) {
               const SIMDType a1( set( A(i    ,k) ) );
               const SIMDType a2( set( A(i+1UL,k) ) );
               const SIMDType b1( B.load(k,j         ) );
               const SIMDType b2( B.load(k,j+SIMDSIZE) );
               xmm1 -= a1 * b1;
               xmm2 -= a1 * b2;
               xmm3 -= a2 * b1;
               xmm4 -= a2 * b2;
            }

            (~C).store( i    , j         , xmm1 );
            (~C).store( i    , j+SIMDSIZE, xmm2 );
            (~C).store( i+1UL, j         , xmm3 );
            (~C).store( i+1UL, j+SIMDSIZE, xmm4 );
         }

         if( i < iend )
         {
            const size_t kbegin( ( IsUpper<MT4>::value )
                                 ?( ( IsLower<MT5>::value )
                                    ?( max( ( IsStrictlyUpper<MT4>::value ? i+1UL : i ), j ) )
                                    :( IsStrictlyUpper<MT4>::value ? i+1UL : i ) )
                                 :( IsLower<MT5>::value ? j : 0UL ) );
            const size_t kend( ( IsUpper<MT5>::value )?( min( j+SIMDSIZE*2UL, K ) ):( K ) );

            SIMDType xmm1( (~C).load(i,j         ) );
            SIMDType xmm2( (~C).load(i,j+SIMDSIZE) );

            for( size_t k=kbegin; k<kend; ++k ) {
               const SIMDType a1( set( A(i,k) ) );
               xmm1 -= a1 * B.load(k,j         );
               xmm2 -= a1 * B.load(k,j+SIMDSIZE);
            }

            (~C).store( i, j         , xmm1 );
            (~C).store( i, j+SIMDSIZE, xmm2 );
         }
      }

      for( ; j<jpos; j+=SIMDSIZE )
      {
         const size_t iend( LOW && UPP ? min(j+SIMDSIZE,M) : M );
         size_t i( LOW ? j : 0UL );

         for( ; (i+2UL) <= iend; i+=2UL )
         {
            const size_t kbegin( ( IsUpper<MT4>::value )
                                 ?( ( IsLower<MT5>::value )
                                    ?( max( ( IsStrictlyUpper<MT4>::value ? i+1UL : i ), j ) )
                                    :( IsStrictlyUpper<MT4>::value ? i+1UL : i ) )
                                 :( IsLower<MT5>::value ? j : 0UL ) );
            const size_t kend( ( IsLower<MT4>::value )
                               ?( IsStrictlyLower<MT4>::value ? i+1UL : i+2UL )
                               :( K ) );

            SIMDType xmm1( (~C).load(i    ,j) );
            SIMDType xmm2( (~C).load(i+1UL,j) );

            for( size_t k=kbegin; k<kend; ++k ) {
               const SIMDType b1( B.load(k,j) );
               xmm1 -= set( A(i    ,k) ) * b1;
               xmm2 -= set( A(i+1UL,k) ) * b1;
            }

            (~C).store( i    , j, xmm1 );
            (~C).store( i+1UL, j, xmm2 );
         }

         if( i < iend )
         {
            const size_t kbegin( ( IsUpper<MT4>::value )
                                 ?( ( IsLower<MT5>::value )
                                    ?( max( ( IsStrictlyUpper<MT4>::value ? i+1UL : i ), j ) )
                                    :( IsStrictlyUpper<MT4>::value ? i+1UL : i ) )
                                 :( IsLower<MT5>::value ? j : 0UL ) );

            SIMDType xmm1( (~C).load(i,j) );

            for( size_t k=kbegin; k<K; ++k ) {
               xmm1 -= set( A(i,k) ) * B.load(k,j);
            }

            (~C).store( i, j, xmm1 );
         }
      }

      for( ; remainder && j<N; ++j )
      {
         const size_t iend( UPP ? j+1UL : M );
         size_t i( LOW ? j : 0UL );

         for( ; (i+2UL) <= iend; i+=2UL )
         {
            const size_t kbegin( ( IsUpper<MT4>::value )
                                 ?( ( IsLower<MT5>::value )
                                    ?( max( ( IsStrictlyUpper<MT4>::value ? i+1UL : i ), j ) )
                                    :( IsStrictlyUpper<MT4>::value ? i+1UL : i ) )
                                 :( IsLower<MT5>::value ? j : 0UL ) );
            const size_t kend( ( IsLower<MT4>::value )
                               ?( IsStrictlyLower<MT4>::value ? i+1UL : i+2UL )
                               :( K ) );

            ElementType value1( (~C)(i    ,j) );
            ElementType value2( (~C)(i+1UL,j) );

            for( size_t k=kbegin; k<kend; ++k ) {
               value1 -= A(i    ,k) * B(k,j);
               value2 -= A(i+1UL,k) * B(k,j);
            }

            (~C)(i    ,j) = value1;
            (~C)(i+1UL,j) = value2;
         }

         if( i < iend )
         {
            const size_t kbegin( ( IsUpper<MT4>::value )
                                 ?( ( IsLower<MT5>::value )
                                    ?( max( ( IsStrictlyUpper<MT4>::value ? i+1UL : i ), j ) )
                                    :( IsStrictlyUpper<MT4>::value ? i+1UL : i ) )
                                 :( IsLower<MT5>::value ? j : 0UL ) );

            ElementType value( (~C)(i,j) );

            for( size_t k=kbegin; k<K; ++k ) {
               value -= A(i,k) * B(k,j);
            }

            (~C)(i,j) = value;
         }
      }
   }
   /*! \endcond */
   //**********************************************************************************************

   //**Vectorized default subtraction assignment to column-major dense matrices (small matrices)***
   /*! \cond BLAZE_INTERNAL */
   /*!\brief Vectorized default subtraction assignment of a small dense matrix-dense matrix
   //        multiplication (\f$ C-=A*B \f$).
   // \ingroup dense_matrix
   //
   // \param C The target left-hand side dense matrix.
   // \param A The left-hand side multiplication operand.
   // \param B The right-hand side multiplication operand.
   // \return void
   //
   // This function implements the vectorized default subtraction assignment of a dense matrix-
   // dense matrix multiplication expression to a column-major dense matrix. This kernel is
   // optimized for small matrices.
   */
   template< typename MT3    // Type of the left-hand side target matrix
           , typename MT4    // Type of the left-hand side matrix operand
           , typename MT5 >  // Type of the right-hand side matrix operand
   static inline EnableIf_< UseVectorizedDefaultKernel<MT3,MT4,MT5> >
      selectSmallSubAssignKernel( DenseMatrix<MT3,true>& C, const MT4& A, const MT5& B )
   {
      BLAZE_CONSTRAINT_MUST_NOT_BE_SYMMETRIC_MATRIX_TYPE( MT4 );
      BLAZE_CONSTRAINT_MUST_NOT_BE_SYMMETRIC_MATRIX_TYPE( MT5 );
      BLAZE_CONSTRAINT_MUST_BE_COLUMN_MAJOR_MATRIX_TYPE( OppositeType_<MT4> );
      BLAZE_CONSTRAINT_MUST_BE_COLUMN_MAJOR_MATRIX_TYPE( OppositeType_<MT5> );

      const ForwardFunctor fwd;

      if( !IsResizable<MT4>::value && IsResizable<MT5>::value ) {
         const OppositeType_<MT4> tmp( serial( A ) );
         subAssign( ~C, fwd( tmp * B ) );
      }
      else if( IsResizable<MT4>::value && !IsResizable<MT5>::value ) {
         const OppositeType_<MT5> tmp( serial( B ) );
         subAssign( ~C, fwd( A * tmp ) );
      }
      else if( A.rows() * A.columns() <= B.rows() * B.columns() ) {
         const OppositeType_<MT4> tmp( serial( A ) );
         subAssign( ~C, fwd( tmp * B ) );
      }
      else {
         const OppositeType_<MT5> tmp( serial( B ) );
         subAssign( ~C, fwd( A * tmp ) );
      }
   }
   /*! \endcond */
   //**********************************************************************************************

   //**Default subtraction assignment to dense matrices (large matrices)***************************
   /*! \cond BLAZE_INTERNAL */
   /*!\brief Default subtraction assignment of a large dense matrix-dense matrix multiplication
   //        (\f$ C-=A*B \f$).
   // \ingroup dense_matrix
   //
   // \param C The target left-hand side dense matrix.
   // \param A The left-hand side multiplication operand.
   // \param B The right-hand side multiplication operand.
   // \return void
   //
   // This function relays to the default implementation of the subtraction assignment of a dense
   // matrix-dense matrix multiplication expression to a dense matrix.
   */
   template< typename MT3    // Type of the left-hand side target matrix
           , typename MT4    // Type of the left-hand side matrix operand
           , typename MT5 >  // Type of the right-hand side matrix operand
   static inline DisableIf_< UseVectorizedDefaultKernel<MT3,MT4,MT5> >
      selectLargeSubAssignKernel( MT3& C, const MT4& A, const MT5& B )
   {
      selectDefaultSubAssignKernel( C, A, B );
   }
   /*! \endcond */
   //**********************************************************************************************

   //**Vectorized default subtraction assignment to dense matrices (large matrices)****************
   /*! \cond BLAZE_INTERNAL */
   /*!\brief Vectorized default subtraction assignment of a large dense matrix-dense matrix
   //        multiplication (\f$ C-=A*B \f$).
   // \ingroup dense_matrix
   //
   // \param C The target left-hand side dense matrix.
   // \param A The left-hand side multiplication operand.
   // \param B The right-hand side multiplication operand.
   // \return void
   //
   // This function implements the vectorized default subtraction assignment of a dense matrix-
   // dense matrix multiplication expression to a dense matrix. This kernel is optimized for
   // large matrices.
   */
   template< typename MT3    // Type of the left-hand side target matrix
           , typename MT4    // Type of the left-hand side matrix operand
           , typename MT5 >  // Type of the right-hand side matrix operand
   static inline EnableIf_< UseVectorizedDefaultKernel<MT3,MT4,MT5> >
      selectLargeSubAssignKernel( MT3& C, const MT4& A, const MT5& B )
   {
      if( LOW )
         lmmm( C, A, B, ElementType(-1), ElementType(1) );
      else if( UPP )
         ummm( C, A, B, ElementType(-1), ElementType(1) );
      else
         mmm( C, A, B, ElementType(-1), ElementType(1) );
   }
   /*! \endcond */
   //**********************************************************************************************

   //**BLAS-based subtraction assignment to dense matrices (default)*******************************
   /*! \cond BLAZE_INTERNAL */
   /*!\brief Default subtraction assignment of a dense matrix-dense matrix multiplication
   //        (\f$ C-=A*B \f$).
   // \ingroup dense_matrix
   //
   // \param C The target left-hand side dense matrix.
   // \param A The left-hand side multiplication operand.
   // \param B The right-hand side multiplication operand.
   // \return void
   //
   // This function relays to the default implementation of the subtraction assignment of a large
   // dense matrix-dense matrix multiplication expression to a dense matrix.
   */
   template< typename MT3    // Type of the left-hand side target matrix
           , typename MT4    // Type of the left-hand side matrix operand
           , typename MT5 >  // Type of the right-hand side matrix operand
   static inline DisableIf_< UseBlasKernel<MT3,MT4,MT5> >
      selectBlasSubAssignKernel( MT3& C, const MT4& A, const MT5& B )
   {
      selectLargeSubAssignKernel( C, A, B );
   }
   /*! \endcond */
   //**********************************************************************************************

   //**BLAS-based subraction assignment to dense matrices******************************************
#if BLAZE_BLAS_MODE && BLAZE_USE_BLAS_MATRIX_MATRIX_MULTIPLICATION
   /*! \cond BLAZE_INTERNAL */
   /*!\brief BLAS-based subraction assignment of a dense matrix-dense matrix multiplication
   //        (\f$ C-=A*B \f$).
   // \ingroup dense_matrix
   //
   // \param C The target left-hand side dense matrix.
   // \param A The left-hand side multiplication operand.
   // \param B The right-hand side multiplication operand.
   // \return void
   //
   // This function performs the dense matrix-dense matrix multiplication based on the according
   // BLAS functionality.
   */
   template< typename MT3    // Type of the left-hand side target matrix
           , typename MT4    // Type of the left-hand side matrix operand
           , typename MT5 >  // Type of the right-hand side matrix operand
   static inline EnableIf_< UseBlasKernel<MT3,MT4,MT5> >
      selectBlasSubAssignKernel( MT3& C, const MT4& A, const MT5& B )
   {
      typedef ElementType_<MT3>  ET;

      if( IsTriangular<MT4>::value ) {
         ResultType_<MT3> tmp( serial( B ) );
         trmm( tmp, A, CblasLeft, ( IsLower<MT4>::value )?( CblasLower ):( CblasUpper ), ET(1) );
         subAssign( C, tmp );
      }
      else if( IsTriangular<MT5>::value ) {
         ResultType_<MT3> tmp( serial( A ) );
         trmm( tmp, B, CblasRight, ( IsLower<MT5>::value )?( CblasLower ):( CblasUpper ), ET(1) );
         subAssign( C, tmp );
      }
      else {
         gemm( C, A, B, ET(-1), ET(1) );
      }
   }
   /*! \endcond */
#endif
   //**********************************************************************************************

   //**Restructuring subtraction assignment to column-major matrices*******************************
   /*! \cond BLAZE_INTERNAL */
   /*!\brief Restructuring subtraction assignment of a dense matrix-dense matrix multiplication
   //        to a column-major matrix (\f$ C-=A*B \f$).
   // \ingroup dense_matrix
   //
   // \param lhs The target left-hand side matrix.
   // \param rhs The right-hand side multiplication expression to be subtracted.
   // \return void
   //
   // This function implements the symmetry-based restructuring subtraction assignment of a dense
   // matrix-dense matrix multiplication expression to a column-major matrix. Due to the explicit
   // application of the SFINAE principle this function can only be selected by the compiler in
   // case the symmetry of either of the two matrix operands can be exploited.
   */
   template< typename MT >  // Type of the target matrix
   friend inline EnableIf_< CanExploitSymmetry<MT,MT1,MT2> >
      subAssign( Matrix<MT,true>& lhs, const DMatDMatMultExpr& rhs )
   {
      BLAZE_FUNCTION_TRACE;

      BLAZE_CONSTRAINT_MUST_NOT_BE_SYMMETRIC_MATRIX_TYPE( MT );

      BLAZE_INTERNAL_ASSERT( (~lhs).rows()    == rhs.rows()   , "Invalid number of rows"    );
      BLAZE_INTERNAL_ASSERT( (~lhs).columns() == rhs.columns(), "Invalid number of columns" );

      const ForwardFunctor fwd;

      if( IsSymmetric<MT1>::value && IsSymmetric<MT2>::value )
         subAssign( ~lhs, fwd( trans( rhs.lhs_ ) * trans( rhs.rhs_ ) ) );
      else if( IsSymmetric<MT1>::value )
         subAssign( ~lhs, fwd( trans( rhs.lhs_ ) * rhs.rhs_ ) );
      else
         subAssign( ~lhs, fwd( rhs.lhs_ * trans( rhs.rhs_ ) ) );
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
   /*!\brief SMP assignment of a dense matrix-dense matrix multiplication to a dense matrix
   //        (\f$ C=A*B \f$).
   // \ingroup dense_matrix
   //
   // \param lhs The target left-hand side dense matrix.
   // \param rhs The right-hand side multiplication expression to be assigned.
   // \return void
   //
   // This function implements the performance optimized SMP assignment of a dense matrix-dense
   // matrix multiplication expression to a dense matrix. Due to the explicit application of the
   // SFINAE principle this function can only be selected by the compiler in case either of the
   // two matrix operands requires an intermediate evaluation and no symmetry can be exploited.
   */
   template< typename MT  // Type of the target dense matrix
           , bool SO >    // Storage order of the target dense matrix
   friend inline EnableIf_< IsEvaluationRequired<MT,MT1,MT2> >
      smpAssign( DenseMatrix<MT,SO>& lhs, const DMatDMatMultExpr& rhs )
   {
      BLAZE_FUNCTION_TRACE;

      BLAZE_INTERNAL_ASSERT( (~lhs).rows()    == rhs.rows()   , "Invalid number of rows"    );
      BLAZE_INTERNAL_ASSERT( (~lhs).columns() == rhs.columns(), "Invalid number of columns" );

      if( (~lhs).rows() == 0UL || (~lhs).columns() == 0UL ) {
         return;
      }
      else if( rhs.lhs_.columns() == 0UL ) {
         reset( ~lhs );
         return;
      }

      LT A( rhs.lhs_ );  // Evaluation of the left-hand side dense matrix operand
      RT B( rhs.rhs_ );  // Evaluation of the right-hand side dense matrix operand

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
   /*!\brief SMP assignment of a dense matrix-dense matrix multiplication to a sparse matrix
   //         (\f$ C=A*B \f$).
   // \ingroup dense_matrix
   //
   // \param lhs The target left-hand side sparse matrix.
   // \param rhs The right-hand side multiplication expression to be assigned.
   // \return void
   //
   // This function implements the performance optimized SMP assignment of a dense matrix-dense
   // matrix multiplication expression to a sparse matrix. Due to the explicit application of the
   // SFINAE principle this function can only be selected by the compiler in case either of the
   // two matrix operands requires an intermediate evaluation and no symmetry can be exploited.
   */
   template< typename MT  // Type of the target sparse matrix
           , bool SO >    // Storage order of the target sparse matrix
   friend inline EnableIf_< IsEvaluationRequired<MT,MT1,MT2> >
      smpAssign( SparseMatrix<MT,SO>& lhs, const DMatDMatMultExpr& rhs )
   {
      BLAZE_FUNCTION_TRACE;

      typedef IfTrue_< SO, OppositeType, ResultType >  TmpType;

      BLAZE_CONSTRAINT_MUST_BE_DENSE_MATRIX_TYPE( ResultType );
      BLAZE_CONSTRAINT_MUST_BE_DENSE_MATRIX_TYPE( OppositeType );
      BLAZE_CONSTRAINT_MUST_BE_ROW_MAJOR_MATRIX_TYPE( ResultType );
      BLAZE_CONSTRAINT_MUST_BE_COLUMN_MAJOR_MATRIX_TYPE( OppositeType );
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

   //**Restructuring SMP assignment to column-major matrices***************************************
   /*! \cond BLAZE_INTERNAL */
   /*!\brief Restructuring SMP assignment of a dense matrix-dense matrix multiplication to a
   //        column-major matrix (\f$ C=A*B \f$).
   // \ingroup dense_matrix
   //
   // \param lhs The target left-hand side matrix.
   // \param rhs The right-hand side multiplication expression to be assigned.
   // \return void
   //
   // This function implements the symmetry-based restructuring SMP assignment of a dense matrix-
   // dense matrix multiplication expression to a column-major matrix. Due to the explicit
   // application of the SFINAE principle this function can only be selected by the compiler in
   // case the symmetry of either of the two matrix operands can be exploited.
   */
   template< typename MT >  // Type of the target matrix
   friend inline EnableIf_< CanExploitSymmetry<MT,MT1,MT2> >
      smpAssign( Matrix<MT,true>& lhs, const DMatDMatMultExpr& rhs )
   {
      BLAZE_FUNCTION_TRACE;

      BLAZE_CONSTRAINT_MUST_NOT_BE_SYMMETRIC_MATRIX_TYPE( MT );

      BLAZE_INTERNAL_ASSERT( (~lhs).rows()    == rhs.rows()   , "Invalid number of rows"    );
      BLAZE_INTERNAL_ASSERT( (~lhs).columns() == rhs.columns(), "Invalid number of columns" );

      const ForwardFunctor fwd;

      if( IsSymmetric<MT1>::value && IsSymmetric<MT2>::value )
         smpAssign( ~lhs, fwd( trans( rhs.lhs_ ) * trans( rhs.rhs_ ) ) );
      else if( IsSymmetric<MT1>::value )
         smpAssign( ~lhs, fwd( trans( rhs.lhs_ ) * rhs.rhs_ ) );
      else
         smpAssign( ~lhs, fwd( rhs.lhs_ * trans( rhs.rhs_ ) ) );
   }
   /*! \endcond */
   //**********************************************************************************************

   //**SMP addition assignment to dense matrices***************************************************
   /*! \cond BLAZE_INTERNAL */
   /*!\brief SMP addition assignment of a dense matrix-dense matrix multiplication to a dense
   //        matrix (\f$ C+=A*B \f$).
   // \ingroup dense_matrix
   //
   // \param lhs The target left-hand side dense matrix.
   // \param rhs The right-hand side multiplication expression to be added.
   // \return void
   //
   // This function implements the performance optimized SMP addition assignment of a dense
   // matrix-dense matrix multiplication expression to a dense matrix. Due to the explicit
   // application of the SFINAE principle this function can only be selected by the compiler
   // in case either of the two matrix operands requires an intermediate evaluation and no
   // symmetry can be exploited.
   */
   template< typename MT  // Type of the target dense matrix
           , bool SO >    // Storage order of the target dense matrix
   friend inline EnableIf_< IsEvaluationRequired<MT,MT1,MT2> >
      smpAddAssign( DenseMatrix<MT,SO>& lhs, const DMatDMatMultExpr& rhs )
   {
      BLAZE_FUNCTION_TRACE;

      BLAZE_INTERNAL_ASSERT( (~lhs).rows()    == rhs.rows()   , "Invalid number of rows"    );
      BLAZE_INTERNAL_ASSERT( (~lhs).columns() == rhs.columns(), "Invalid number of columns" );

      if( (~lhs).rows() == 0UL || (~lhs).columns() == 0UL || rhs.lhs_.columns() == 0UL ) {
         return;
      }

      LT A( rhs.lhs_ );  // Evaluation of the left-hand side dense matrix operand
      RT B( rhs.rhs_ );  // Evaluation of the right-hand side dense matrix operand

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

   //**Restructuring SMP addition assignment to column-major matrices******************************
   /*! \cond BLAZE_INTERNAL */
   /*!\brief Restructuring SMP addition assignment of a dense matrix-dense matrix multiplication
   //        to a column-major matrix (\f$ C+=A*B \f$).
   // \ingroup dense_matrix
   //
   // \param lhs The target left-hand side matrix.
   // \param rhs The right-hand side multiplication expression to be added.
   // \return void
   //
   // This function implements the symmetry-based restructuring SMP addition assignment of a dense
   // matrix-dense matrix multiplication expression to a column-major matrix. Due to the explicit
   // application of the SFINAE principle this function can only be selected by the compiler in
   // case the symmetry of either of the two matrix operands can be exploited.
   */
   template< typename MT >  // Type of the target matrix
   friend inline EnableIf_< CanExploitSymmetry<MT,MT1,MT2> >
      smpAddAssign( Matrix<MT,true>& lhs, const DMatDMatMultExpr& rhs )
   {
      BLAZE_FUNCTION_TRACE;

      BLAZE_CONSTRAINT_MUST_NOT_BE_SYMMETRIC_MATRIX_TYPE( MT );

      BLAZE_INTERNAL_ASSERT( (~lhs).rows()    == rhs.rows()   , "Invalid number of rows"    );
      BLAZE_INTERNAL_ASSERT( (~lhs).columns() == rhs.columns(), "Invalid number of columns" );

      const ForwardFunctor fwd;

      if( IsSymmetric<MT1>::value && IsSymmetric<MT2>::value )
         smpAddAssign( ~lhs, fwd( trans( rhs.lhs_ ) * trans( rhs.rhs_ ) ) );
      else if( IsSymmetric<MT1>::value )
         smpAddAssign( ~lhs, fwd( trans( rhs.lhs_ ) * rhs.rhs_ ) );
      else
         smpAddAssign( ~lhs, fwd( rhs.lhs_ * trans( rhs.rhs_ ) ) );
   }
   /*! \endcond */
   //**********************************************************************************************

   //**SMP addition assignment to sparse matrices**************************************************
   // No special implementation for the SMP addition assignment to sparse matrices.
   //**********************************************************************************************

   //**SMP subtraction assignment to dense matrices************************************************
   /*! \cond BLAZE_INTERNAL */
   /*!\brief SMP subtraction assignment of a dense matrix-dense matrix multiplication to a
   //        dense matrix (\f$ C-=A*B \f$).
   // \ingroup dense_matrix
   //
   // \param lhs The target left-hand side dense matrix.
   // \param rhs The right-hand side multiplication expression to be subtracted.
   // \return void
   //
   // This function implements the performance optimized SMP subtraction assignment of a dense
   // matrix-dense matrix multiplication expression to a dense matrix. Due to the explicit
   // application of the SFINAE principle this function can only be selected by the compiler
   // in case either of the two matrix operands requires an intermediate evaluation and no
   // symmetry can be exploited.
   */
   template< typename MT  // Type of the target dense matrix
           , bool SO >    // Storage order of the target dense matrix
   friend inline EnableIf_< IsEvaluationRequired<MT,MT1,MT2> >
      smpSubAssign( DenseMatrix<MT,SO>& lhs, const DMatDMatMultExpr& rhs )
   {
      BLAZE_FUNCTION_TRACE;

      BLAZE_INTERNAL_ASSERT( (~lhs).rows()    == rhs.rows()   , "Invalid number of rows"    );
      BLAZE_INTERNAL_ASSERT( (~lhs).columns() == rhs.columns(), "Invalid number of columns" );

      if( (~lhs).rows() == 0UL || (~lhs).columns() == 0UL || rhs.lhs_.columns() == 0UL ) {
         return;
      }

      LT A( rhs.lhs_ );  // Evaluation of the left-hand side dense matrix operand
      RT B( rhs.rhs_ );  // Evaluation of the right-hand side dense matrix operand

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

   //**Restructuring SMP subtraction assignment to column-major matrices***************************
   /*! \cond BLAZE_INTERNAL */
   /*!\brief Restructuring SMP subtraction assignment of a dense matrix-dense matrix multiplication
   //        to a column-major matrix (\f$ C-=A*B \f$).
   // \ingroup dense_matrix
   //
   // \param lhs The target left-hand side matrix.
   // \param rhs The right-hand side multiplication expression to be subtracted.
   // \return void
   //
   // This function implements the symmetry-based restructuring SMP subtraction assignment of a
   // dense matrix-dense matrix multiplication expression to a column-major matrix. Due to the
   // explicit application of the SFINAE principle this function can only be selected by the
   // compiler in case the symmetry of either of the two matrix operands can be exploited.
   */
   template< typename MT >  // Type of the target matrix
   friend inline EnableIf_< CanExploitSymmetry<MT,MT1,MT2> >
      smpSubAssign( Matrix<MT,true>& lhs, const DMatDMatMultExpr& rhs )
   {
      BLAZE_FUNCTION_TRACE;

      BLAZE_CONSTRAINT_MUST_NOT_BE_SYMMETRIC_MATRIX_TYPE( MT );

      BLAZE_INTERNAL_ASSERT( (~lhs).rows()    == rhs.rows()   , "Invalid number of rows"    );
      BLAZE_INTERNAL_ASSERT( (~lhs).columns() == rhs.columns(), "Invalid number of columns" );

      const ForwardFunctor fwd;

      if( IsSymmetric<MT1>::value && IsSymmetric<MT2>::value )
         smpSubAssign( ~lhs, fwd( trans( rhs.lhs_ ) * trans( rhs.rhs_ ) ) );
      else if( IsSymmetric<MT1>::value )
         smpSubAssign( ~lhs, fwd( trans( rhs.lhs_ ) * rhs.rhs_ ) );
      else
         smpSubAssign( ~lhs, fwd( rhs.lhs_ * trans( rhs.rhs_ ) ) );
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
   BLAZE_CONSTRAINT_MUST_BE_ROW_MAJOR_MATRIX_TYPE( MT1 );
   BLAZE_CONSTRAINT_MUST_BE_DENSE_MATRIX_TYPE( MT2 );
   BLAZE_CONSTRAINT_MUST_BE_ROW_MAJOR_MATRIX_TYPE( MT2 );
   BLAZE_CONSTRAINT_MUST_FORM_VALID_MATMATMULTEXPR( MT1, MT2 );
   /*! \endcond */
   //**********************************************************************************************
};
//*************************************************************************************************




//=================================================================================================
//
//  DMATSCALARMULTEXPR SPECIALIZATION
//
//=================================================================================================

//*************************************************************************************************
/*! \cond BLAZE_INTERNAL */
/*!\brief Expression object for scaled dense matrix-dense matrix multiplications.
// \ingroup dense_matrix_expression
//
// This specialization of the DMatScalarMultExpr class represents the compile time expression
// for scaled multiplications between row-major dense matrices.
*/
template< typename MT1   // Type of the left-hand side dense matrix
        , typename MT2   // Type of the right-hand side dense matrix
        , bool SF        // Symmetry flag
        , bool HF        // Hermitian flag
        , bool LF        // Lower flag
        , bool UF        // Upper flag
        , typename ST >  // Type of the right-hand side scalar value
class DMatScalarMultExpr< DMatDMatMultExpr<MT1,MT2,SF,HF,LF,UF>, ST, false >
   : public DenseMatrix< DMatScalarMultExpr< DMatDMatMultExpr<MT1,MT2,SF,HF,LF,UF>, ST, false >, false >
   , private MatScalarMultExpr
   , private Computation
{
 private:
   //**Type definitions****************************************************************************
   //! Type of the dense matrix multiplication expression.
   typedef DMatDMatMultExpr<MT1,MT2,SF,HF,LF,UF>  MMM;

   typedef ResultType_<MMM>     RES;  //!< Result type of the dense matrix multiplication expression.
   typedef ResultType_<MT1>     RT1;  //!< Result type of the left-hand side dense matrix expression.
   typedef ResultType_<MT2>     RT2;  //!< Result type of the right-hand side dense matrix expression.
   typedef ElementType_<RT1>    ET1;  //!< Element type of the left-hand side dense matrix expression.
   typedef ElementType_<RT2>    ET2;  //!< Element type of the right-hand side dense matrix expression.
   typedef CompositeType_<MT1>  CT1;  //!< Composite type of the left-hand side dense matrix expression.
   typedef CompositeType_<MT2>  CT2;  //!< Composite type of the right-hand side dense matrix expression.
   //**********************************************************************************************

   //**********************************************************************************************
   //! Compilation switch for the composite type of the left-hand side dense matrix expression.
   enum : bool { evaluateLeft = IsComputation<MT1>::value || RequiresEvaluation<MT1>::value };
   //**********************************************************************************************

   //**********************************************************************************************
   //! Compilation switch for the composite type of the right-hand side dense matrix expression.
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
   //! Helper structure for the explicit application of the SFINAE principle.
   /*! The CanExploitSymmetry struct is a helper struct for the selection of the optimal
       evaluation strategy. In case the target matrix is column-major and either of the
       two matrix operands is symmetric, \a value is set to 1 and an optimized evaluation
       strategy is selected. Otherwise \a value is set to 0 and the default strategy is
       chosen. */
   template< typename T1, typename T2, typename T3 >
   struct CanExploitSymmetry {
      enum : bool { value = IsColumnMajorMatrix<T1>::value &&
                            ( IsSymmetric<T2>::value || IsSymmetric<T3>::value ) };
   };
   //**********************************************************************************************

   //**********************************************************************************************
   //! Helper structure for the explicit application of the SFINAE principle.
   /*! The IsEvaluationRequired struct is a helper struct for the selection of the parallel
       evaluation strategy. In case either of the two matrix operands requires an intermediate
       evaluation, the nested \value will be set to 1, otherwise it will be 0. */
   template< typename T1, typename T2, typename T3 >
   struct IsEvaluationRequired {
      enum : bool { value = ( evaluateLeft || evaluateRight ) &&
                            !CanExploitSymmetry<T1,T2,T3>::value };
   };
   //**********************************************************************************************

   //**********************************************************************************************
   //! Helper structure for the explicit application of the SFINAE principle.
   /*! In case the types of all three involved matrices and the scalar type are suited for a BLAS
       kernel, the nested \a value will be set to 1, otherwise it will be 0. */
   template< typename T1, typename T2, typename T3, typename T4 >
   struct UseBlasKernel {
      enum : bool { value = BLAZE_BLAS_MODE && BLAZE_USE_BLAS_MATRIX_MATRIX_MULTIPLICATION &&
                            !SYM && !HERM && !LOW && !UPP &&
                            HasMutableDataAccess<T1>::value &&
                            HasConstDataAccess<T2>::value &&
                            HasConstDataAccess<T3>::value &&
                            !IsDiagonal<T2>::value && !IsDiagonal<T3>::value &&
                            T1::simdEnabled && T2::simdEnabled && T3::simdEnabled &&
                            IsBLASCompatible< ElementType_<T1> >::value &&
                            IsBLASCompatible< ElementType_<T2> >::value &&
                            IsBLASCompatible< ElementType_<T3> >::value &&
                            IsSame< ElementType_<T1>, ElementType_<T2> >::value &&
                            IsSame< ElementType_<T1>, ElementType_<T3> >::value &&
                            !( IsBuiltin< ElementType_<T1> >::value && IsComplex<T4>::value ) };
   };
   //**********************************************************************************************

   //**********************************************************************************************
   //! Helper structure for the explicit application of the SFINAE principle.
   /*! In case all four involved data types are suited for a vectorized computation of the
       matrix multiplication, the nested \value will be set to 1, otherwise it will be 0. */
   template< typename T1, typename T2, typename T3, typename T4 >
   struct UseVectorizedDefaultKernel {
      enum : bool { value = useOptimizedKernels &&
                            !IsDiagonal<T3>::value &&
                            T1::simdEnabled && T2::simdEnabled && T3::simdEnabled &&
                            IsSIMDCombinable< ElementType_<T1>
                                            , ElementType_<T2>
                                            , ElementType_<T3>
                                            , T4 >::value &&
                            HasSIMDAdd< ElementType_<T2>, ElementType_<T3> >::value &&
                            HasSIMDMult< ElementType_<T2>, ElementType_<T3> >::value };
   };
   //**********************************************************************************************

   //**********************************************************************************************
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
   //**********************************************************************************************

 public:
   //**Type definitions****************************************************************************
   //! Type of this DMatScalarMultExpr instance.
   typedef DMatScalarMultExpr<MMM,ST,false>  This;

   typedef MultTrait_<RES,ST>          ResultType;     //!< Result type for expression template evaluations.
   typedef OppositeType_<ResultType>   OppositeType;   //!< Result type with opposite storage order for expression template evaluations.
   typedef TransposeType_<ResultType>  TransposeType;  //!< Transpose type for expression template evaluations.
   typedef ElementType_<ResultType>    ElementType;    //!< Resulting element type.
   typedef SIMDTrait_<ElementType>     SIMDType;       //!< Resulting SIMD element type.
   typedef const ElementType           ReturnType;     //!< Return type for expression template evaluations.
   typedef const ResultType            CompositeType;  //!< Data type for composite expression templates.

   //! Composite type of the left-hand side dense matrix expression.
   typedef const DMatDMatMultExpr<MT1,MT2,SF,HF,LF,UF>  LeftOperand;

   //! Composite type of the right-hand side scalar value.
   typedef ST  RightOperand;

   //! Type for the assignment of the left-hand side dense matrix operand.
   typedef IfTrue_< evaluateLeft, const RT1, CT1 >  LT;

   //! Type for the assignment of the right-hand side dense matrix operand.
   typedef IfTrue_< evaluateRight, const RT2, CT2 >  RT;
   //**********************************************************************************************

   //**Compilation flags***************************************************************************
   //! Compilation switch for the expression template evaluation strategy.
   enum : bool { simdEnabled = !IsDiagonal<MT2>::value &&
                               MT1::simdEnabled && MT2::simdEnabled &&
                               IsSIMDCombinable<ET1,ET2,ST>::value &&
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
   /*!\brief Constructor for the DMatScalarMultExpr class.
   //
   // \param matrix The left-hand side dense matrix of the multiplication expression.
   // \param scalar The right-hand side scalar of the multiplication expression.
   */
   explicit inline DMatScalarMultExpr( const MMM& matrix, ST scalar )
      : matrix_( matrix )  // Left-hand side dense matrix of the multiplication expression
      , scalar_( scalar )  // Right-hand side scalar of the multiplication expression
   {}
   //**********************************************************************************************

   //**Access operator*****************************************************************************
   /*!\brief 2D-access to the matrix elements.
   //
   // \param i Access index for the row. The index has to be in the range \f$[0..M-1]\f$.
   // \param j Access index for the column. The index has to be in the range \f$[0..N-1]\f$.
   // \return The resulting value.
   */
   inline ReturnType operator()( size_t i, size_t j ) const {
      BLAZE_INTERNAL_ASSERT( i < matrix_.rows()   , "Invalid row access index"    );
      BLAZE_INTERNAL_ASSERT( j < matrix_.columns(), "Invalid column access index" );
      return matrix_(i,j) * scalar_;
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
      if( i >= matrix_.rows() ) {
         BLAZE_THROW_OUT_OF_RANGE( "Invalid row access index" );
      }
      if( j >= matrix_.columns() ) {
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
   inline size_t rows() const {
      return matrix_.rows();
   }
   //**********************************************************************************************

   //**Columns function****************************************************************************
   /*!\brief Returns the current number of columns of the matrix.
   //
   // \return The number of columns of the matrix.
   */
   inline size_t columns() const {
      return matrix_.columns();
   }
   //**********************************************************************************************

   //**Left operand access*************************************************************************
   /*!\brief Returns the left-hand side dense matrix operand.
   //
   // \return The left-hand side dense matrix operand.
   */
   inline LeftOperand leftOperand() const {
      return matrix_;
   }
   //**********************************************************************************************

   //**Right operand access************************************************************************
   /*!\brief Returns the right-hand side scalar operand.
   //
   // \return The right-hand side scalar operand.
   */
   inline RightOperand rightOperand() const {
      return scalar_;
   }
   //**********************************************************************************************

   //**********************************************************************************************
   /*!\brief Returns whether the expression can alias with the given address \a alias.
   //
   // \param alias The alias to be checked.
   // \return \a true in case the expression can alias, \a false otherwise.
   */
   template< typename T >
   inline bool canAlias( const T* alias ) const {
      return matrix_.canAlias( alias );
   }
   //**********************************************************************************************

   //**********************************************************************************************
   /*!\brief Returns whether the expression is aliased with the given address \a alias.
   //
   // \param alias The alias to be checked.
   // \return \a true in case an alias effect is detected, \a false otherwise.
   */
   template< typename T >
   inline bool isAliased( const T* alias ) const {
      return matrix_.isAliased( alias );
   }
   //**********************************************************************************************

   //**********************************************************************************************
   /*!\brief Returns whether the operands of the expression are properly aligned in memory.
   //
   // \return \a true in case the operands are aligned, \a false if not.
   */
   inline bool isAligned() const {
      return matrix_.isAligned();
   }
   //**********************************************************************************************

   //**********************************************************************************************
   /*!\brief Returns whether the expression can be used in SMP assignments.
   //
   // \return \a true in case the expression can be used in SMP assignments, \a false if not.
   */
   inline bool canSMPAssign() const noexcept {
      return ( !BLAZE_BLAS_IS_PARALLEL ||
               ( rows() * columns() < DMATDMATMULT_THRESHOLD ) ) &&
             ( rows() * columns() >= SMP_DMATDMATMULT_THRESHOLD );
   }
   //**********************************************************************************************

 private:
   //**Member variables****************************************************************************
   LeftOperand  matrix_;  //!< Left-hand side dense matrix of the multiplication expression.
   RightOperand scalar_;  //!< Right-hand side scalar of the multiplication expression.
   //**********************************************************************************************

   //**Assignment to dense matrices****************************************************************
   /*!\brief Assignment of a scaled dense matrix-dense matrix multiplication to a dense matrix
   //        (\f$ C=s*A*B \f$).
   // \ingroup dense_matrix
   //
   // \param lhs The target left-hand side dense matrix.
   // \param rhs The right-hand side scaled multiplication expression to be assigned.
   // \return void
   //
   // This function implements the performance optimized assignment of a scaled dense matrix-
   // dense matrix multiplication expression to a dense matrix.
   */
   template< typename MT  // Type of the target dense matrix
           , bool SO >    // Storage order of the target dense matrix
   friend inline DisableIf_< CanExploitSymmetry<MT,MT1,MT2> >
      assign( DenseMatrix<MT,SO>& lhs, const DMatScalarMultExpr& rhs )
   {
      BLAZE_FUNCTION_TRACE;

      BLAZE_INTERNAL_ASSERT( (~lhs).rows()    == rhs.rows()   , "Invalid number of rows"    );
      BLAZE_INTERNAL_ASSERT( (~lhs).columns() == rhs.columns(), "Invalid number of columns" );

      LeftOperand_<MMM>  left ( rhs.matrix_.leftOperand()  );
      RightOperand_<MMM> right( rhs.matrix_.rightOperand() );

      if( (~lhs).rows() == 0UL || (~lhs).columns() == 0UL ) {
         return;
      }
      else if( left.columns() == 0UL ) {
         reset( ~lhs );
         return;
      }

      LT A( serial( left  ) );  // Evaluation of the left-hand side dense matrix operand
      RT B( serial( right ) );  // Evaluation of the right-hand side dense matrix operand

      BLAZE_INTERNAL_ASSERT( A.rows()    == left.rows()     , "Invalid number of rows"    );
      BLAZE_INTERNAL_ASSERT( A.columns() == left.columns()  , "Invalid number of columns" );
      BLAZE_INTERNAL_ASSERT( B.rows()    == right.rows()    , "Invalid number of rows"    );
      BLAZE_INTERNAL_ASSERT( B.columns() == right.columns() , "Invalid number of columns" );
      BLAZE_INTERNAL_ASSERT( A.rows()    == (~lhs).rows()   , "Invalid number of rows"    );
      BLAZE_INTERNAL_ASSERT( B.columns() == (~lhs).columns(), "Invalid number of columns" );

      DMatScalarMultExpr::selectAssignKernel( ~lhs, A, B, rhs.scalar_ );
   }
   //**********************************************************************************************

   //**Assignment to dense matrices (kernel selection)*********************************************
   /*!\brief Selection of the kernel for an assignment of a scaled dense matrix-dense matrix
   //        multiplication to a dense matrix (\f$ C=s*A*B \f$).
   // \ingroup dense_matrix
   //
   // \param C The target left-hand side dense matrix.
   // \param A The left-hand side multiplication operand.
   // \param B The right-hand side multiplication operand.
   // \param scalar The scaling factor.
   // \return void
   */
   template< typename MT3    // Type of the left-hand side target matrix
           , typename MT4    // Type of the left-hand side matrix operand
           , typename MT5    // Type of the right-hand side matrix operand
           , typename ST2 >  // Type of the scalar value
   static inline void selectAssignKernel( MT3& C, const MT4& A, const MT5& B, ST2 scalar )
   {
      if( ( IsDiagonal<MT5>::value ) ||
          ( !BLAZE_DEBUG_MODE && B.columns() <= SIMDSIZE*10UL ) ||
          ( C.rows() * C.columns() < DMATDMATMULT_THRESHOLD ) )
         selectSmallAssignKernel( C, A, B, scalar );
      else
         selectBlasAssignKernel( C, A, B, scalar );
   }
   //**********************************************************************************************

   //**Default assignment to dense matrices (general/general)**************************************
   /*!\brief Default assignment of a scaled general dense matrix-general dense matrix
   //        multiplication (\f$ C=s*A*B \f$).
   // \ingroup dense_matrix
   //
   // \param C The target left-hand side dense matrix.
   // \param A The left-hand side multiplication operand.
   // \param B The right-hand side multiplication operand.
   // \param scalar The scaling factor.
   // \return void
   //
   // This function implements the default assignment of a scaled general dense matrix-general
   // dense matrix multiplication expression to a dense matrix.
   */
   template< typename MT3    // Type of the left-hand side target matrix
           , typename MT4    // Type of the left-hand side matrix operand
           , typename MT5    // Type of the right-hand side matrix operand
           , typename ST2 >  // Type of the scalar value
   static inline EnableIf_< And< Not< IsDiagonal<MT4> >, Not< IsDiagonal<MT5> > > >
      selectDefaultAssignKernel( MT3& C, const MT4& A, const MT5& B, ST2 scalar )
   {
      const size_t M( A.rows()    );
      const size_t N( B.columns() );
      const size_t K( A.columns() );

      BLAZE_INTERNAL_ASSERT( !( SYM || HERM || LOW || UPP ) || ( M == N ), "Broken invariant detected" );

      for( size_t i=0UL; i<M; ++i )
      {
         const size_t kbegin( ( IsUpper<MT4>::value )
                              ?( IsStrictlyUpper<MT4>::value ? i+1UL : i )
                              :( 0UL ) );
         const size_t kend( ( IsLower<MT4>::value )
                            ?( IsStrictlyLower<MT4>::value ? i : i+1UL )
                            :( K ) );
         BLAZE_INTERNAL_ASSERT( kbegin <= kend, "Invalid loop indices detected" );

         if( IsStrictlyTriangular<MT4>::value && kbegin == kend ) {
            for( size_t j=0UL; j<N; ++j ) {
               reset( C(i,j) );
            }
            continue;
         }

         {
            const size_t jbegin( ( IsUpper<MT5>::value )
                                 ?( ( IsStrictlyUpper<MT5>::value )
                                    ?( UPP ? max(i,kbegin+1UL) : kbegin+1UL )
                                    :( UPP ? max(i,kbegin) : kbegin ) )
                                 :( UPP ? i : 0UL ) );
            const size_t jend( ( IsLower<MT5>::value )
                               ?( ( IsStrictlyLower<MT5>::value )
                                  ?( LOW ? min(i+1UL,kbegin) : kbegin )
                                  :( LOW ? min(i,kbegin)+1UL : kbegin+1UL ) )
                               :( LOW ? i+1UL : N ) );

            if( ( IsUpper<MT4>::value && IsUpper<MT5>::value ) || UPP ) {
               for( size_t j=0UL; j<jbegin; ++j ) {
                  reset( C(i,j) );
               }
            }
            else if( IsStrictlyUpper<MT5>::value ) {
               reset( C(i,0UL) );
            }
            for( size_t j=jbegin; j<jend; ++j ) {
               C(i,j) = A(i,kbegin) * B(kbegin,j);
            }
            if( ( IsLower<MT4>::value && IsLower<MT5>::value ) || LOW ) {
               for( size_t j=jend; j<N; ++j ) {
                  reset( C(i,j) );
               }
            }
            else if( IsStrictlyLower<MT5>::value ) {
               reset( C(i,N-1UL) );
            }
         }

         for( size_t k=kbegin+1UL; k<kend; ++k )
         {
            const size_t jbegin( ( IsUpper<MT5>::value )
                                 ?( ( IsStrictlyUpper<MT5>::value )
                                    ?( SYM || HERM || UPP ? max( i, k+1UL ) : k+1UL )
                                    :( SYM || HERM || UPP ? max( i, k ) : k ) )
                                 :( SYM || HERM || UPP ? i : 0UL ) );
            const size_t jend( ( IsLower<MT5>::value )
                               ?( ( IsStrictlyLower<MT5>::value )
                                  ?( LOW ? min(i+1UL,k-1UL) : k-1UL )
                                  :( LOW ? min(i+1UL,k) : k ) )
                               :( LOW ? i+1UL : N ) );

            if( ( SYM || HERM || LOW || UPP ) && ( jbegin > jend ) ) continue;
            BLAZE_INTERNAL_ASSERT( jbegin <= jend, "Invalid loop indices detected" );

            for( size_t j=jbegin; j<jend; ++j ) {
               C(i,j) += A(i,k) * B(k,j);
            }
            if( IsLower<MT5>::value ) {
               C(i,jend) = A(i,k) * B(k,jend);
            }
         }

         {
            const size_t jbegin( ( IsUpper<MT4>::value && IsUpper<MT5>::value )
                                 ?( IsStrictlyUpper<MT4>::value || IsStrictlyUpper<MT5>::value ? i+1UL : i )
                                 :( SYM || HERM || UPP ? i : 0UL ) );
            const size_t jend( ( IsLower<MT4>::value && IsLower<MT5>::value )
                               ?( IsStrictlyLower<MT4>::value || IsStrictlyLower<MT5>::value ? i : i+1UL )
                               :( LOW ? i+1UL : N ) );

            if( ( SYM || HERM || LOW || UPP ) && ( jbegin > jend ) ) continue;
            BLAZE_INTERNAL_ASSERT( jbegin <= jend, "Invalid loop indices detected" );

            for( size_t j=jbegin; j<jend; ++j ) {
               C(i,j) *= scalar;
            }
         }
      }

      if( SYM || HERM ) {
         for( size_t i=1UL; i<M; ++i ) {
            for( size_t j=0UL; j<i; ++j ) {
               C(i,j) = HERM ? conj( C(j,i) ) : C(j,i);
            }
         }
      }
   }
   //**********************************************************************************************

   //**Default assignment to dense matrices (general/diagonal)*************************************
   /*!\brief Default assignment of a scaled general dense matrix-diagonal dense matrix
   //        multiplication (\f$ C=s*A*B \f$).
   // \ingroup dense_matrix
   //
   // \param C The target left-hand side dense matrix.
   // \param A The left-hand side multiplication operand.
   // \param B The right-hand side multiplication operand.
   // \param scalar The scaling factor.
   // \return void
   //
   // This function implements the default assignment of a scaled general dense matrix-diagonal
   // dense matrix multiplication expression to a dense matrix.
   */
   template< typename MT3    // Type of the left-hand side target matrix
           , typename MT4    // Type of the left-hand side matrix operand
           , typename MT5    // Type of the right-hand side matrix operand
           , typename ST2 >  // Type of the scalar value
   static inline EnableIf_< And< Not< IsDiagonal<MT4> >, IsDiagonal<MT5> > >
      selectDefaultAssignKernel( MT3& C, const MT4& A, const MT5& B, ST2 scalar )
   {
      BLAZE_CONSTRAINT_MUST_BE_ROW_MAJOR_MATRIX_TYPE( MT3 );

      const size_t M( A.rows()    );
      const size_t N( B.columns() );

      for( size_t i=0UL; i<M; ++i )
      {
         const size_t jbegin( ( IsUpper<MT4>::value )
                              ?( IsStrictlyUpper<MT4>::value ? i+1UL : i )
                              :( 0UL ) );
         const size_t jend( ( IsLower<MT4>::value )
                            ?( IsStrictlyLower<MT4>::value ? i : i+1UL )
                            :( N ) );
         BLAZE_INTERNAL_ASSERT( jbegin <= jend, "Invalid loop indices detected" );

         if( IsUpper<MT4>::value ) {
            for( size_t j=0UL; j<jbegin; ++j ) {
               reset( C(i,j) );
            }
         }
         for( size_t j=jbegin; j<jend; ++j ) {
            C(i,j) = A(i,j) * B(j,j) * scalar;
         }
         if( IsLower<MT4>::value ) {
            for( size_t j=jend; j<N; ++j ) {
               reset( C(i,j) );
            }
         }
      }
   }
   //**********************************************************************************************

   //**Default assignment to dense matrices (diagonal/general)*************************************
   /*!\brief Default assignment of a scaled diagonal dense matrix-general dense matrix
   //        multiplication (\f$ C=s*A*B \f$).
   // \ingroup dense_matrix
   //
   // \param C The target left-hand side dense matrix.
   // \param A The left-hand side multiplication operand.
   // \param B The right-hand side multiplication operand.
   // \param scalar The scaling factor.
   // \return void
   //
   // This function implements the default assignment of a scaled diagonal dense matrix-general
   // dense matrix multiplication expression to a dense matrix.
   */
   template< typename MT3    // Type of the left-hand side target matrix
           , typename MT4    // Type of the left-hand side matrix operand
           , typename MT5    // Type of the right-hand side matrix operand
           , typename ST2 >  // Type of the scalar value
   static inline EnableIf_< And< IsDiagonal<MT4>, Not< IsDiagonal<MT5> > > >
      selectDefaultAssignKernel( MT3& C, const MT4& A, const MT5& B, ST2 scalar )
   {
      BLAZE_CONSTRAINT_MUST_BE_ROW_MAJOR_MATRIX_TYPE( MT3 );

      const size_t M( A.rows()    );
      const size_t N( B.columns() );

      for( size_t i=0UL; i<M; ++i )
      {
         const size_t jbegin( ( IsUpper<MT5>::value )
                              ?( IsStrictlyUpper<MT5>::value ? i+1UL : i )
                              :( 0UL ) );
         const size_t jend( ( IsLower<MT5>::value )
                            ?( IsStrictlyLower<MT5>::value ? i : i+1UL )
                            :( N ) );
         BLAZE_INTERNAL_ASSERT( jbegin <= jend, "Invalid loop indices detected" );

         if( IsUpper<MT5>::value ) {
            for( size_t j=0UL; j<jbegin; ++j ) {
               reset( C(i,j) );
            }
         }
         for( size_t j=jbegin; j<jend; ++j ) {
            C(i,j) = A(i,i) * B(i,j) * scalar;
         }
         if( IsLower<MT5>::value ) {
            for( size_t j=jend; j<N; ++j ) {
               reset( C(i,j) );
            }
         }
      }
   }
   //**********************************************************************************************

   //**Default assignment to dense matrices (diagonal/diagonal)************************************
   /*!\brief Default assignment of a scaled diagonal dense matrix-diagonal dense matrix
   //        multiplication (\f$ C=s*A*B \f$).
   // \ingroup dense_matrix
   //
   // \param C The target left-hand side dense matrix.
   // \param A The left-hand side multiplication operand.
   // \param B The right-hand side multiplication operand.
   // \param scalar The scaling factor.
   // \return void
   //
   // This function implements the default assignment of a scaled diagonal dense matrix-diagional
   // dense matrix multiplication expression to a dense matrix.
   */
   template< typename MT3    // Type of the left-hand side target matrix
           , typename MT4    // Type of the left-hand side matrix operand
           , typename MT5    // Type of the right-hand side matrix operand
           , typename ST2 >  // Type of the scalar value
   static inline EnableIf_< And< IsDiagonal<MT4>, IsDiagonal<MT5> > >
      selectDefaultAssignKernel( MT3& C, const MT4& A, const MT5& B, ST2 scalar )
   {
      BLAZE_CONSTRAINT_MUST_BE_ROW_MAJOR_MATRIX_TYPE( MT3 );

      reset( C );

      for( size_t i=0UL; i<A.rows(); ++i ) {
         C(i,i) = A(i,i) * B(i,i) * scalar;
      }
   }
   //**********************************************************************************************

   //**Default assignment to dense matrices (small matrices)***************************************
   /*!\brief Default assignment of a small scaled dense matrix-dense matrix multiplication
   //        (\f$ C=s*A*B \f$).
   // \ingroup dense_matrix
   //
   // \param C The target left-hand side dense matrix.
   // \param A The left-hand side multiplication operand.
   // \param B The right-hand side multiplication operand.
   // \param scalar The scaling factor.
   // \return void
   //
   // This function relays to the default implementation of the assignment of a scaled dense
   // matrix-dense matrix multiplication expression to a dense matrix.
   */
   template< typename MT3    // Type of the left-hand side target matrix
           , typename MT4    // Type of the left-hand side matrix operand
           , typename MT5    // Type of the right-hand side matrix operand
           , typename ST2 >  // Type of the scalar value
   static inline DisableIf_< UseVectorizedDefaultKernel<MT3,MT4,MT5,ST2> >
      selectSmallAssignKernel( MT3& C, const MT4& A, const MT5& B, ST2 scalar )
   {
      selectDefaultAssignKernel( C, A, B, scalar );
   }
   //**********************************************************************************************

   //**Vectorized default assignment to row-major dense matrices (small matrices)******************
   /*!\brief Vectorized default assignment of a small scaled dense matrix-dense matrix
   //        multiplication (\f$ C=s*A*B \f$).
   // \ingroup dense_matrix
   //
   // \param C The target left-hand side dense matrix.
   // \param A The left-hand side multiplication operand.
   // \param B The right-hand side multiplication operand.
   // \param scalar The scaling factor.
   // \return void
   //
   // This function implements the vectorized default assignment of a scaled dense matrix-dense
   // matrix multiplication expression to a row-major dense matrix. This kernel is optimized for
   // small matrices.
   */
   template< typename MT3    // Type of the left-hand side target matrix
           , typename MT4    // Type of the left-hand side matrix operand
           , typename MT5    // Type of the right-hand side matrix operand
           , typename ST2 >  // Type of the scalar value
   static inline EnableIf_< UseVectorizedDefaultKernel<MT3,MT4,MT5,ST2> >
      selectSmallAssignKernel( DenseMatrix<MT3,false>& C, const MT4& A, const MT5& B, ST2 scalar )
   {
      constexpr bool remainder( !IsPadded<MT3>::value || !IsPadded<MT5>::value );

      const size_t M( A.rows()    );
      const size_t N( B.columns() );
      const size_t K( A.columns() );

      BLAZE_INTERNAL_ASSERT( !( SYM || HERM || LOW || UPP ) || ( M == N ), "Broken invariant detected" );

      const size_t jpos( remainder ? ( N & size_t(-SIMDSIZE) ) : N );
      BLAZE_INTERNAL_ASSERT( !remainder || ( N - ( N % SIMDSIZE ) ) == jpos, "Invalid end calculation" );

      const SIMDType factor( set( scalar ) );

      if( LOW && UPP && N > SIMDSIZE*3UL ) {
         reset( ~C );
      }

      {
         size_t j( 0UL );

         if( IsIntegral<ElementType>::value )
         {
            for( ; !SYM && !HERM && !LOW && !UPP && (j+SIMDSIZE*7UL) < jpos; j+=SIMDSIZE*8UL ) {
               for( size_t i=0UL; i<M; ++i )
               {
                  const size_t kbegin( ( IsUpper<MT4>::value )
                                       ?( ( IsLower<MT5>::value )
                                          ?( max( ( IsStrictlyUpper<MT4>::value ? i+1UL : i ), j ) )
                                          :( IsStrictlyUpper<MT4>::value ? i+1UL : i ) )
                                       :( IsLower<MT5>::value ? j : 0UL ) );
                  const size_t kend( ( IsLower<MT4>::value )
                                     ?( ( IsUpper<MT5>::value )
                                        ?( min( ( IsStrictlyLower<MT4>::value ? i : i+1UL ), j+SIMDSIZE*8UL, K ) )
                                        :( IsStrictlyLower<MT4>::value ? i : i+1UL ) )
                                     :( IsUpper<MT5>::value ? min( j+SIMDSIZE*8UL, K ) : K ) );

                  SIMDType xmm1, xmm2, xmm3, xmm4, xmm5, xmm6, xmm7, xmm8;

                  for( size_t k=kbegin; k<kend; ++k ) {
                     const SIMDType a1( set( A(i,k) ) );
                     xmm1 += a1 * B.load(k,j             );
                     xmm2 += a1 * B.load(k,j+SIMDSIZE    );
                     xmm3 += a1 * B.load(k,j+SIMDSIZE*2UL);
                     xmm4 += a1 * B.load(k,j+SIMDSIZE*3UL);
                     xmm5 += a1 * B.load(k,j+SIMDSIZE*4UL);
                     xmm6 += a1 * B.load(k,j+SIMDSIZE*5UL);
                     xmm7 += a1 * B.load(k,j+SIMDSIZE*6UL);
                     xmm8 += a1 * B.load(k,j+SIMDSIZE*7UL);
                  }

                  (~C).store( i, j             , xmm1 * factor );
                  (~C).store( i, j+SIMDSIZE    , xmm2 * factor );
                  (~C).store( i, j+SIMDSIZE*2UL, xmm3 * factor );
                  (~C).store( i, j+SIMDSIZE*3UL, xmm4 * factor );
                  (~C).store( i, j+SIMDSIZE*4UL, xmm5 * factor );
                  (~C).store( i, j+SIMDSIZE*5UL, xmm6 * factor );
                  (~C).store( i, j+SIMDSIZE*6UL, xmm7 * factor );
                  (~C).store( i, j+SIMDSIZE*7UL, xmm8 * factor );
               }
            }
         }

         for( ; !SYM && !HERM && !LOW && !UPP && (j+SIMDSIZE*4UL) < jpos; j+=SIMDSIZE*5UL )
         {
            size_t i( 0UL );

            for( ; (i+2UL) <= M; i+=2UL )
            {
               const size_t kbegin( ( IsUpper<MT4>::value )
                                    ?( ( IsLower<MT5>::value )
                                       ?( max( ( IsStrictlyUpper<MT4>::value ? i+1UL : i ), j ) )
                                       :( IsStrictlyUpper<MT4>::value ? i+1UL : i ) )
                                    :( IsLower<MT5>::value ? j : 0UL ) );
               const size_t kend( ( IsLower<MT4>::value )
                                  ?( ( IsUpper<MT5>::value )
                                     ?( min( ( IsStrictlyLower<MT4>::value ? i+1UL : i+2UL ), j+SIMDSIZE*5UL, K ) )
                                     :( IsStrictlyLower<MT4>::value ? i+1UL : i+2UL ) )
                                  :( IsUpper<MT5>::value ? min( j+SIMDSIZE*5UL, K ) : K ) );

               SIMDType xmm1, xmm2, xmm3, xmm4, xmm5, xmm6, xmm7, xmm8, xmm9, xmm10;

               for( size_t k=kbegin; k<kend; ++k ) {
                  const SIMDType a1( set( A(i    ,k) ) );
                  const SIMDType a2( set( A(i+1UL,k) ) );
                  const SIMDType b1( B.load(k,j             ) );
                  const SIMDType b2( B.load(k,j+SIMDSIZE    ) );
                  const SIMDType b3( B.load(k,j+SIMDSIZE*2UL) );
                  const SIMDType b4( B.load(k,j+SIMDSIZE*3UL) );
                  const SIMDType b5( B.load(k,j+SIMDSIZE*4UL) );
                  xmm1  += a1 * b1;
                  xmm2  += a1 * b2;
                  xmm3  += a1 * b3;
                  xmm4  += a1 * b4;
                  xmm5  += a1 * b5;
                  xmm6  += a2 * b1;
                  xmm7  += a2 * b2;
                  xmm8  += a2 * b3;
                  xmm9  += a2 * b4;
                  xmm10 += a2 * b5;
               }

               (~C).store( i    , j             , xmm1  * factor );
               (~C).store( i    , j+SIMDSIZE    , xmm2  * factor );
               (~C).store( i    , j+SIMDSIZE*2UL, xmm3  * factor );
               (~C).store( i    , j+SIMDSIZE*3UL, xmm4  * factor );
               (~C).store( i    , j+SIMDSIZE*4UL, xmm5  * factor );
               (~C).store( i+1UL, j             , xmm6  * factor );
               (~C).store( i+1UL, j+SIMDSIZE    , xmm7  * factor );
               (~C).store( i+1UL, j+SIMDSIZE*2UL, xmm8  * factor );
               (~C).store( i+1UL, j+SIMDSIZE*3UL, xmm9  * factor );
               (~C).store( i+1UL, j+SIMDSIZE*4UL, xmm10 * factor );
            }

            if( i < M )
            {
               const size_t kbegin( ( IsUpper<MT4>::value )
                                    ?( ( IsLower<MT5>::value )
                                       ?( max( ( IsStrictlyUpper<MT4>::value ? i+1UL : i ), j ) )
                                       :( IsStrictlyUpper<MT4>::value ? i+1UL : i ) )
                                    :( IsLower<MT5>::value ? j : 0UL ) );
               const size_t kend( ( IsUpper<MT5>::value )?( min( j+SIMDSIZE*5UL, K ) ):( K ) );

               SIMDType xmm1, xmm2, xmm3, xmm4, xmm5;

               for( size_t k=kbegin; k<kend; ++k ) {
                  const SIMDType a1( set( A(i,k) ) );
                  xmm1 += a1 * B.load(k,j             );
                  xmm2 += a1 * B.load(k,j+SIMDSIZE    );
                  xmm3 += a1 * B.load(k,j+SIMDSIZE*2UL);
                  xmm4 += a1 * B.load(k,j+SIMDSIZE*3UL);
                  xmm5 += a1 * B.load(k,j+SIMDSIZE*4UL);
               }

               (~C).store( i, j             , xmm1 * factor );
               (~C).store( i, j+SIMDSIZE    , xmm2 * factor );
               (~C).store( i, j+SIMDSIZE*2UL, xmm3 * factor );
               (~C).store( i, j+SIMDSIZE*3UL, xmm4 * factor );
               (~C).store( i, j+SIMDSIZE*4UL, xmm5 * factor );
            }
         }

         for( ; !( LOW && UPP ) && (j+SIMDSIZE*3UL) < jpos; j+=SIMDSIZE*4UL )
         {
            const size_t iend( SYM || HERM || UPP ? min(j+SIMDSIZE*4UL,M) : M );
            size_t i( LOW ? j : 0UL );

            for( ; (i+2UL) <= iend; i+=2UL )
            {
               const size_t kbegin( ( IsUpper<MT4>::value )
                                    ?( ( IsLower<MT5>::value )
                                       ?( max( ( IsStrictlyUpper<MT4>::value ? i+1UL : i ), j ) )
                                       :( IsStrictlyUpper<MT4>::value ? i+1UL : i ) )
                                    :( IsLower<MT5>::value ? j : 0UL ) );
               const size_t kend( ( IsLower<MT4>::value )
                                  ?( ( IsUpper<MT5>::value )
                                     ?( min( ( IsStrictlyLower<MT4>::value ? i+1UL : i+2UL ), j+SIMDSIZE*4UL, K ) )
                                     :( IsStrictlyLower<MT4>::value ? i+1UL : i+2UL ) )
                                  :( IsUpper<MT5>::value ? min( j+SIMDSIZE*4UL, K ) : K ) );

               SIMDType xmm1, xmm2, xmm3, xmm4, xmm5, xmm6, xmm7, xmm8;

               for( size_t k=kbegin; k<kend; ++k ) {
                  const SIMDType a1( set( A(i    ,k) ) );
                  const SIMDType a2( set( A(i+1UL,k) ) );
                  const SIMDType b1( B.load(k,j             ) );
                  const SIMDType b2( B.load(k,j+SIMDSIZE    ) );
                  const SIMDType b3( B.load(k,j+SIMDSIZE*2UL) );
                  const SIMDType b4( B.load(k,j+SIMDSIZE*3UL) );
                  xmm1 += a1 * b1;
                  xmm2 += a1 * b2;
                  xmm3 += a1 * b3;
                  xmm4 += a1 * b4;
                  xmm5 += a2 * b1;
                  xmm6 += a2 * b2;
                  xmm7 += a2 * b3;
                  xmm8 += a2 * b4;
               }

               (~C).store( i    , j             , xmm1 * factor );
               (~C).store( i    , j+SIMDSIZE    , xmm2 * factor );
               (~C).store( i    , j+SIMDSIZE*2UL, xmm3 * factor );
               (~C).store( i    , j+SIMDSIZE*3UL, xmm4 * factor );
               (~C).store( i+1UL, j             , xmm5 * factor );
               (~C).store( i+1UL, j+SIMDSIZE    , xmm6 * factor );
               (~C).store( i+1UL, j+SIMDSIZE*2UL, xmm7 * factor );
               (~C).store( i+1UL, j+SIMDSIZE*3UL, xmm8 * factor );
            }

            if( i < iend )
            {
               const size_t kbegin( ( IsUpper<MT4>::value )
                                    ?( ( IsLower<MT5>::value )
                                       ?( max( ( IsStrictlyUpper<MT4>::value ? i+1UL : i ), j ) )
                                       :( IsStrictlyUpper<MT4>::value ? i+1UL : i ) )
                                    :( IsLower<MT5>::value ? j : 0UL ) );
               const size_t kend( ( IsUpper<MT5>::value )?( min( j+SIMDSIZE*4UL, K ) ):( K ) );

               SIMDType xmm1, xmm2, xmm3, xmm4;

               for( size_t k=kbegin; k<kend; ++k ) {
                  const SIMDType a1( set( A(i,k) ) );
                  xmm1 += a1 * B.load(k,j             );
                  xmm2 += a1 * B.load(k,j+SIMDSIZE    );
                  xmm3 += a1 * B.load(k,j+SIMDSIZE*2UL);
                  xmm4 += a1 * B.load(k,j+SIMDSIZE*3UL);
               }

               (~C).store( i, j             , xmm1 * factor );
               (~C).store( i, j+SIMDSIZE    , xmm2 * factor );
               (~C).store( i, j+SIMDSIZE*2UL, xmm3 * factor );
               (~C).store( i, j+SIMDSIZE*3UL, xmm4 * factor );
            }
         }

         for( ; (j+SIMDSIZE*2UL) < jpos; j+=SIMDSIZE*3UL )
         {
            const size_t iend( SYM || HERM || UPP ? min(j+SIMDSIZE*3UL,M) : M );
            size_t i( LOW ? j : 0UL );

            for( ; (i+2UL) <= iend; i+=2UL )
            {
               const size_t kbegin( ( IsUpper<MT4>::value )
                                    ?( ( IsLower<MT5>::value )
                                       ?( max( ( IsStrictlyUpper<MT4>::value ? i+1UL : i ), j ) )
                                       :( IsStrictlyUpper<MT4>::value ? i+1UL : i ) )
                                    :( IsLower<MT5>::value ? j : 0UL ) );
               const size_t kend( ( IsLower<MT4>::value )
                                  ?( ( IsUpper<MT5>::value )
                                     ?( min( ( IsStrictlyLower<MT4>::value ? i+1UL : i+2UL ), j+SIMDSIZE*3UL, K ) )
                                     :( IsStrictlyLower<MT4>::value ? i+1UL : i+2UL ) )
                                  :( IsUpper<MT5>::value ? min( j+SIMDSIZE*3UL, K ) : K ) );

               SIMDType xmm1, xmm2, xmm3, xmm4, xmm5, xmm6;

               for( size_t k=kbegin; k<kend; ++k ) {
                  const SIMDType a1( set( A(i    ,k) ) );
                  const SIMDType a2( set( A(i+1UL,k) ) );
                  const SIMDType b1( B.load(k,j             ) );
                  const SIMDType b2( B.load(k,j+SIMDSIZE    ) );
                  const SIMDType b3( B.load(k,j+SIMDSIZE*2UL) );
                  xmm1 += a1 * b1;
                  xmm2 += a1 * b2;
                  xmm3 += a1 * b3;
                  xmm4 += a2 * b1;
                  xmm5 += a2 * b2;
                  xmm6 += a2 * b3;
               }

               (~C).store( i    , j             , xmm1 * factor );
               (~C).store( i    , j+SIMDSIZE    , xmm2 * factor );
               (~C).store( i    , j+SIMDSIZE*2UL, xmm3 * factor );
               (~C).store( i+1UL, j             , xmm4 * factor );
               (~C).store( i+1UL, j+SIMDSIZE    , xmm5 * factor );
               (~C).store( i+1UL, j+SIMDSIZE*2UL, xmm6 * factor );
            }

            if( i < iend )
            {
               const size_t kbegin( ( IsUpper<MT4>::value )
                                    ?( ( IsLower<MT5>::value )
                                       ?( max( ( IsStrictlyUpper<MT4>::value ? i+1UL : i ), j ) )
                                       :( IsStrictlyUpper<MT4>::value ? i+1UL : i ) )
                                    :( IsLower<MT5>::value ? j : 0UL ) );
               const size_t kend( ( IsUpper<MT5>::value )?( min( j+SIMDSIZE*3UL, K ) ):( K ) );

               SIMDType xmm1, xmm2, xmm3;

               for( size_t k=kbegin; k<kend; ++k ) {
                  const SIMDType a1( set( A(i,k) ) );
                  xmm1 += a1 * B.load(k,j             );
                  xmm2 += a1 * B.load(k,j+SIMDSIZE    );
                  xmm3 += a1 * B.load(k,j+SIMDSIZE*2UL);
               }

               (~C).store( i, j             , xmm1 * factor );
               (~C).store( i, j+SIMDSIZE    , xmm2 * factor );
               (~C).store( i, j+SIMDSIZE*2UL, xmm3 * factor );
            }
         }

         for( ; (j+SIMDSIZE) < jpos; j+=SIMDSIZE*2UL )
         {
            const size_t iend( SYM || HERM || UPP ? min(j+SIMDSIZE*2UL,M) : M );
            size_t i( LOW ? j : 0UL );

            for( ; (i+2UL) <= iend; i+=2UL )
            {
               const size_t kbegin( ( IsUpper<MT4>::value )
                                    ?( ( IsLower<MT5>::value )
                                       ?( max( ( IsStrictlyUpper<MT4>::value ? i+1UL : i ), j ) )
                                       :( IsStrictlyUpper<MT4>::value ? i+1UL : i ) )
                                    :( IsLower<MT5>::value ? j : 0UL ) );
               const size_t kend( ( IsLower<MT4>::value )
                                  ?( ( IsUpper<MT5>::value )
                                     ?( min( ( IsStrictlyLower<MT4>::value ? i+1UL : i+2UL ), j+SIMDSIZE*2UL, K ) )
                                     :( IsStrictlyLower<MT4>::value ? i+1UL : i+2UL ) )
                                  :( IsUpper<MT5>::value ? min( j+SIMDSIZE*2UL, K ) : K ) );

               SIMDType xmm1, xmm2, xmm3, xmm4;

               for( size_t k=kbegin; k<kend; ++k ) {
                  const SIMDType a1( set( A(i    ,k) ) );
                  const SIMDType a2( set( A(i+1UL,k) ) );
                  const SIMDType b1( B.load(k,j         ) );
                  const SIMDType b2( B.load(k,j+SIMDSIZE) );
                  xmm1 += a1 * b1;
                  xmm2 += a1 * b2;
                  xmm3 += a2 * b1;
                  xmm4 += a2 * b2;
               }

               (~C).store( i    , j         , xmm1 * factor );
               (~C).store( i    , j+SIMDSIZE, xmm2 * factor );
               (~C).store( i+1UL, j         , xmm3 * factor );
               (~C).store( i+1UL, j+SIMDSIZE, xmm4 * factor );
            }

            if( i < iend )
            {
               const size_t kbegin( ( IsUpper<MT4>::value )
                                    ?( ( IsLower<MT5>::value )
                                       ?( max( ( IsStrictlyUpper<MT4>::value ? i+1UL : i ), j ) )
                                       :( IsStrictlyUpper<MT4>::value ? i+1UL : i ) )
                                    :( IsLower<MT5>::value ? j : 0UL ) );
               const size_t kend( ( IsUpper<MT5>::value )?( min( j+SIMDSIZE*2UL, K ) ):( K ) );

               SIMDType xmm1, xmm2;

               for( size_t k=kbegin; k<kend; ++k ) {
                  const SIMDType a1( set( A(i,k) ) );
                  xmm1 += a1 * B.load(k,j         );
                  xmm2 += a1 * B.load(k,j+SIMDSIZE);
               }

               (~C).store( i, j         , xmm1 * factor );
               (~C).store( i, j+SIMDSIZE, xmm2 * factor );
            }
         }

         for( ; j<jpos; j+=SIMDSIZE )
         {
            const size_t iend( SYM || HERM || UPP ? min(j+SIMDSIZE,M) : M );
            size_t i( LOW ? j : 0UL );

            for( ; (i+2UL) <= iend; i+=2UL )
            {
               const size_t kbegin( ( IsUpper<MT4>::value )
                                    ?( ( IsLower<MT5>::value )
                                       ?( max( ( IsStrictlyUpper<MT4>::value ? i+1UL : i ), j ) )
                                       :( IsStrictlyUpper<MT4>::value ? i+1UL : i ) )
                                    :( IsLower<MT5>::value ? j : 0UL ) );
               const size_t kend( ( IsLower<MT4>::value )
                                  ?( IsStrictlyLower<MT4>::value ? i+1UL : i+2UL )
                                  :( K ) );

               SIMDType xmm1, xmm2;

               for( size_t k=kbegin; k<kend; ++k ) {
                  const SIMDType b1( B.load(k,j) );
                  xmm1 += set( A(i    ,k) ) * b1;
                  xmm2 += set( A(i+1UL,k) ) * b1;
               }

               (~C).store( i    , j, xmm1 * factor );
               (~C).store( i+1UL, j, xmm2 * factor );
            }

            if( i < iend )
            {
               const size_t kbegin( ( IsUpper<MT4>::value )
                                    ?( ( IsLower<MT5>::value )
                                       ?( max( ( IsStrictlyUpper<MT4>::value ? i+1UL : i ), j ) )
                                       :( IsStrictlyUpper<MT4>::value ? i+1UL : i ) )
                                    :( IsLower<MT5>::value ? j : 0UL ) );

               SIMDType xmm1;

               for( size_t k=kbegin; k<K; ++k ) {
                  xmm1 += set( A(i,k) ) * B.load(k,j);
               }

               (~C).store( i, j, xmm1 * factor );
            }
         }

         for( ; remainder && j<N; ++j )
         {
            size_t i( LOW && UPP ? j : 0UL );

            for( ; (i+2UL) <= M; i+=2UL )
            {
               const size_t kbegin( ( IsUpper<MT4>::value )
                                    ?( ( IsLower<MT5>::value )
                                       ?( max( ( IsStrictlyUpper<MT4>::value ? i+1UL : i ), j ) )
                                       :( IsStrictlyUpper<MT4>::value ? i+1UL : i ) )
                                    :( IsLower<MT5>::value ? j : 0UL ) );
               const size_t kend( ( IsLower<MT4>::value )
                                  ?( IsStrictlyLower<MT4>::value ? i+1UL : i+2UL )
                                  :( K ) );

               ElementType value1 = ElementType();
               ElementType value2 = ElementType();

               for( size_t k=kbegin; k<kend; ++k ) {
                  value1 += A(i    ,k) * B(k,j);
                  value2 += A(i+1UL,k) * B(k,j);
               }

               (~C)(i    ,j) = value1 * scalar;
               (~C)(i+1UL,j) = value2 * scalar;
            }

            if( i < M )
            {
               const size_t kbegin( ( IsUpper<MT4>::value )
                                    ?( ( IsLower<MT5>::value )
                                       ?( max( ( IsStrictlyUpper<MT4>::value ? i+1UL : i ), j ) )
                                       :( IsStrictlyUpper<MT4>::value ? i+1UL : i ) )
                                    :( IsLower<MT5>::value ? j : 0UL ) );

               ElementType value = ElementType();

               for( size_t k=kbegin; k<K; ++k ) {
                  value += A(i,k) * B(k,j);
               }

               (~C)(i,j) = value * scalar;
            }
         }
      }

      if( ( SYM || HERM ) && ( N > SIMDSIZE*4UL ) ) {
         for( size_t i=SIMDSIZE*4UL; i<M; ++i ) {
            const size_t jend( ( SIMDSIZE*4UL ) * ( i / (SIMDSIZE*4UL) ) );
            for( size_t j=0UL; j<jend; ++j ) {
               (~C)(i,j) = HERM ? conj( (~C)(j,i) ) : (~C)(j,i);
            }
         }
      }
      else if( LOW && !UPP && N > SIMDSIZE*4UL ) {
         for( size_t j=SIMDSIZE*4UL; j<N; ++j ) {
            const size_t iend( ( SIMDSIZE*4UL ) * ( j / (SIMDSIZE*4UL) ) );
            for( size_t i=0UL; i<iend; ++i ) {
               reset( (~C)(i,j) );
            }
         }
      }
      else if( !LOW && UPP && N > SIMDSIZE*4UL ) {
         for( size_t i=SIMDSIZE*4UL; i<M; ++i ) {
            const size_t jend( ( SIMDSIZE*4UL ) * ( i / (SIMDSIZE*4UL) ) );
            for( size_t j=0UL; j<jend; ++j ) {
               reset( (~C)(i,j) );
            }
         }
      }
   }
   //**********************************************************************************************

   //**Vectorized default assignment to column-major dense matrices (small matrices)***************
   /*!\brief Vectorized default assignment of a small scaled dense matrix-dense matrix
   //        multiplication (\f$ C=s*A*B \f$).
   // \ingroup dense_matrix
   //
   // \param C The target left-hand side dense matrix.
   // \param A The left-hand side multiplication operand.
   // \param B The right-hand side multiplication operand.
   // \param scalar The scaling factor.
   // \return void
   //
   // This function implements the vectorized default assignment of a small scaled dense matrix-
   // dense matrix multiplication expression to a column-major dense matrix. This kernel is
   // optimized for small matrices.
   */
   template< typename MT3    // Type of the left-hand side target matrix
           , typename MT4    // Type of the left-hand side matrix operand
           , typename MT5    // Type of the right-hand side matrix operand
           , typename ST2 >  // Type of the scalar value
   static inline EnableIf_< UseVectorizedDefaultKernel<MT3,MT4,MT5,ST2> >
      selectSmallAssignKernel( DenseMatrix<MT3,true>& C, const MT4& A, const MT5& B, ST2 scalar )
   {
      BLAZE_CONSTRAINT_MUST_NOT_BE_SYMMETRIC_MATRIX_TYPE( MT4 );
      BLAZE_CONSTRAINT_MUST_NOT_BE_SYMMETRIC_MATRIX_TYPE( MT5 );
      BLAZE_CONSTRAINT_MUST_BE_COLUMN_MAJOR_MATRIX_TYPE( OppositeType_<MT4> );
      BLAZE_CONSTRAINT_MUST_BE_COLUMN_MAJOR_MATRIX_TYPE( OppositeType_<MT5> );

      const ForwardFunctor fwd;

      if( !IsResizable<MT4>::value && IsResizable<MT5>::value ) {
         const OppositeType_<MT4> tmp( serial( A ) );
         assign( ~C, fwd( tmp * B ) * scalar );
      }
      else if( IsResizable<MT4>::value && !IsResizable<MT5>::value ) {
         const OppositeType_<MT5> tmp( serial( B ) );
         assign( ~C, fwd( A * tmp ) * scalar );
      }
      else if( A.rows() * A.columns() <= B.rows() * B.columns() ) {
         const OppositeType_<MT4> tmp( serial( A ) );
         assign( ~C, fwd( tmp * B ) * scalar );
      }
      else {
         const OppositeType_<MT5> tmp( serial( B ) );
         assign( ~C, fwd( A * tmp ) * scalar );
      }
   }
   //**********************************************************************************************

   //**Default assignment to dense matrices (large matrices)***************************************
   /*!\brief Default assignment of a large scaled dense matrix-dense matrix multiplication
   //        (\f$ C=s*A*B \f$).
   // \ingroup dense_matrix
   //
   // \param C The target left-hand side dense matrix.
   // \param A The left-hand side multiplication operand.
   // \param B The right-hand side multiplication operand.
   // \param scalar The scaling factor.
   // \return void
   //
   // This function relays to the default implementation of the assignment of a scaled dense
   // matrix-dense matrix multiplication expression to a dense matrix.
   */
   template< typename MT3    // Type of the left-hand side target matrix
           , typename MT4    // Type of the left-hand side matrix operand
           , typename MT5    // Type of the right-hand side matrix operand
           , typename ST2 >  // Type of the scalar value
   static inline DisableIf_< UseVectorizedDefaultKernel<MT3,MT4,MT5,ST2> >
      selectLargeAssignKernel( MT3& C, const MT4& A, const MT5& B, ST2 scalar )
   {
      selectDefaultAssignKernel( C, A, B, scalar );
   }
   //**********************************************************************************************

   //**Vectorized default assignment to dense matrices (large matrices)****************************
   /*!\brief Vectorized default assignment of a large scaled dense matrix-dense matrix
   //        multiplication (\f$ C=s*A*B \f$).
   // \ingroup dense_matrix
   //
   // \param C The target left-hand side dense matrix.
   // \param A The left-hand side multiplication operand.
   // \param B The right-hand side multiplication operand.
   // \param scalar The scaling factor.
   // \return void
   //
   // This function implements the vectorized default assignment of a scaled dense matrix-dense
   // matrix multiplication expression to a dense matrix. This kernel is optimized for large
   // matrices.
   */
   template< typename MT3    // Type of the left-hand side target matrix
           , typename MT4    // Type of the left-hand side matrix operand
           , typename MT5    // Type of the right-hand side matrix operand
           , typename ST2 >  // Type of the scalar value
   static inline EnableIf_< UseVectorizedDefaultKernel<MT3,MT4,MT5,ST2> >
      selectLargeAssignKernel( MT3& C, const MT4& A, const MT5& B, ST2 scalar )
   {
      if( SYM )
         smmm( C, A, B, scalar );
      else if( HERM )
         hmmm( C, A, B, scalar );
      else if( LOW )
         lmmm( C, A, B, scalar, ST2(0) );
      else if( UPP )
         ummm( C, A, B, scalar, ST2(0) );
      else
         mmm( C, A, B, scalar, ST2(0) );
   }
   //**********************************************************************************************

   //**BLAS-based assignment to dense matrices (default)*******************************************
   /*!\brief Default assignment of a scaled dense matrix-dense matrix multiplication
   //        (\f$ C=s*A*B \f$).
   // \ingroup dense_matrix
   //
   // \param C The target left-hand side dense matrix.
   // \param A The left-hand side multiplication operand.
   // \param B The right-hand side multiplication operand.
   // \param scalar The scaling factor.
   // \return void
   //
   // This function relays to the default implementation of the assignment of a large scaled
   // dense matrix-dense matrix multiplication expression to a dense matrix.
   */
   template< typename MT3    // Type of the left-hand side target matrix
           , typename MT4    // Type of the left-hand side matrix operand
           , typename MT5    // Type of the right-hand side matrix operand
           , typename ST2 >  // Type of the scalar value
   static inline DisableIf_< UseBlasKernel<MT3,MT4,MT5,ST2> >
      selectBlasAssignKernel( MT3& C, const MT4& A, const MT5& B, ST2 scalar )
   {
      selectLargeAssignKernel( C, A, B, scalar );
   }
   //**********************************************************************************************

   //**BLAS-based assignment to dense matrices*****************************************************
#if BLAZE_BLAS_MODE && BLAZE_USE_BLAS_MATRIX_MATRIX_MULTIPLICATION
   /*!\brief BLAS-based assignment of a scaled dense matrix-dense matrix multiplication
   //        (\f$ C=s*A*B \f$).
   // \ingroup dense_matrix
   //
   // \param C The target left-hand side dense matrix.
   // \param A The left-hand side multiplication operand.
   // \param B The right-hand side multiplication operand.
   // \param scalar The scaling factor.
   // \return void
   //
   // This function performs the scaled dense matrix-dense matrix multiplication based on the
   // according BLAS functionality.
   */
   template< typename MT3    // Type of the left-hand side target matrix
           , typename MT4    // Type of the left-hand side matrix operand
           , typename MT5    // Type of the right-hand side matrix operand
           , typename ST2 >  // Type of the scalar value
   static inline EnableIf_< UseBlasKernel<MT3,MT4,MT5,ST2> >
      selectBlasAssignKernel( MT3& C, const MT4& A, const MT5& B, ST2 scalar )
   {
      typedef ElementType_<MT3>  ET;

      if( IsTriangular<MT4>::value ) {
         assign( C, B );
         trmm( C, A, CblasLeft, ( IsLower<MT4>::value )?( CblasLower ):( CblasUpper ), ET(scalar) );
      }
      else if( IsTriangular<MT5>::value ) {
         assign( C, A );
         trmm( C, B, CblasRight, ( IsLower<MT5>::value )?( CblasLower ):( CblasUpper ), ET(scalar) );
      }
      else {
         gemm( C, A, B, ET(scalar), ET(0) );
      }
   }
#endif
   //**********************************************************************************************

   //**Assignment to sparse matrices***************************************************************
   /*!\brief Assignment of a scaled dense matrix-dense matrix multiplication to a sparse matrix
   //        (\f$ C=s*A*B \f$).
   // \ingroup dense_matrix
   //
   // \param lhs The target left-hand side sparse matrix.
   // \param rhs The right-hand side scaled multiplication expression to be assigned.
   // \return void
   //
   // This function implements the performance optimized assignment of a scaled dense matrix-
   // dense matrix multiplication expression to a sparse matrix.
   */
   template< typename MT  // Type of the target sparse matrix
           , bool SO >    // Storage order of the target sparse matrix
   friend inline DisableIf_< CanExploitSymmetry<MT,MT1,MT2> >
      assign( SparseMatrix<MT,SO>& lhs, const DMatScalarMultExpr& rhs )
   {
      BLAZE_FUNCTION_TRACE;

      typedef IfTrue_< SO, OppositeType, ResultType >  TmpType;

      BLAZE_CONSTRAINT_MUST_BE_DENSE_MATRIX_TYPE( ResultType );
      BLAZE_CONSTRAINT_MUST_BE_DENSE_MATRIX_TYPE( OppositeType );
      BLAZE_CONSTRAINT_MUST_BE_ROW_MAJOR_MATRIX_TYPE( ResultType );
      BLAZE_CONSTRAINT_MUST_BE_COLUMN_MAJOR_MATRIX_TYPE( OppositeType );
      BLAZE_CONSTRAINT_MATRICES_MUST_HAVE_SAME_STORAGE_ORDER( MT, TmpType );
      BLAZE_CONSTRAINT_MUST_BE_REFERENCE_TYPE( CompositeType_<TmpType> );

      BLAZE_INTERNAL_ASSERT( (~lhs).rows()    == rhs.rows()   , "Invalid number of rows"    );
      BLAZE_INTERNAL_ASSERT( (~lhs).columns() == rhs.columns(), "Invalid number of columns" );

      const ForwardFunctor fwd;

      const TmpType tmp( serial( rhs ) );
      assign( ~lhs, fwd( tmp ) );
   }
   //**********************************************************************************************

   //**Restructuring assignment to column-major matrices*******************************************
   /*!\brief Restructuring assignment of a scaled dense matrix-dense matrix multiplication to a
   //        column-major matrix (\f$ C=s*A*B \f$).
   // \ingroup dense_matrix
   //
   // \param lhs The target left-hand side matrix.
   // \param rhs The right-hand side scaled multiplication expression to be assigned.
   // \return void
   //
   // This function implements the symmetry-based restructuring assignment of a scaled dense
   // matrix-dense matrix multiplication expression to a column-major matrix. Due to the explicit
   // application of the SFINAE principle this function can only be selected by the compiler in
   // case the symmetry of either of the two matrix operands can be exploited.
   */
   template< typename MT >  // Type of the target matrix
   friend inline EnableIf_< CanExploitSymmetry<MT,MT1,MT2> >
      assign( Matrix<MT,true>& lhs, const DMatScalarMultExpr& rhs )
   {
      BLAZE_FUNCTION_TRACE;

      BLAZE_CONSTRAINT_MUST_NOT_BE_SYMMETRIC_MATRIX_TYPE( MT );

      BLAZE_INTERNAL_ASSERT( (~lhs).rows()    == rhs.rows()   , "Invalid number of rows"    );
      BLAZE_INTERNAL_ASSERT( (~lhs).columns() == rhs.columns(), "Invalid number of columns" );

      const ForwardFunctor fwd;

      LeftOperand_<MMM>  left ( rhs.matrix_.leftOperand()  );
      RightOperand_<MMM> right( rhs.matrix_.rightOperand() );

      if( IsSymmetric<MT1>::value && IsSymmetric<MT2>::value )
         assign( ~lhs, fwd( trans( left ) * trans( right ) ) * rhs.scalar_ );
      else if( IsSymmetric<MT1>::value )
         assign( ~lhs, fwd( trans( left ) * right ) * rhs.scalar_ );
      else
         assign( ~lhs, fwd( left * trans( right ) ) * rhs.scalar_ );
   }
   //**********************************************************************************************

   //**Addition assignment to dense matrices*******************************************************
   /*!\brief Addition assignment of a scaled dense matrix-dense matrix multiplication to a
   //        dense matrix (\f$ C+=s*A*B \f$).
   // \ingroup dense_matrix
   //
   // \param lhs The target left-hand side dense matrix.
   // \param rhs The right-hand side scaled multiplication expression to be added.
   // \return void
   //
   // This function implements the performance optimized addition assignment of a scaled dense
   // matrix-dense matrix multiplication expression to a dense matrix.
   */
   template< typename MT  // Type of the target dense matrix
           , bool SO >    // Storage order of the target dense matrix
   friend inline DisableIf_< CanExploitSymmetry<MT,MT1,MT2> >
      addAssign( DenseMatrix<MT,SO>& lhs, const DMatScalarMultExpr& rhs )
   {
      BLAZE_FUNCTION_TRACE;

      BLAZE_INTERNAL_ASSERT( (~lhs).rows()    == rhs.rows()   , "Invalid number of rows"    );
      BLAZE_INTERNAL_ASSERT( (~lhs).columns() == rhs.columns(), "Invalid number of columns" );

      LeftOperand_<MMM>  left ( rhs.matrix_.leftOperand()  );
      RightOperand_<MMM> right( rhs.matrix_.rightOperand() );

      if( (~lhs).rows() == 0UL || (~lhs).columns() == 0UL || left.columns() == 0UL ) {
         return;
      }

      LT A( serial( left  ) );  // Evaluation of the left-hand side dense matrix operand
      RT B( serial( right ) );  // Evaluation of the right-hand side dense matrix operand

      BLAZE_INTERNAL_ASSERT( A.rows()    == left.rows()     , "Invalid number of rows"    );
      BLAZE_INTERNAL_ASSERT( A.columns() == left.columns()  , "Invalid number of columns" );
      BLAZE_INTERNAL_ASSERT( B.rows()    == right.rows()    , "Invalid number of rows"    );
      BLAZE_INTERNAL_ASSERT( B.columns() == right.columns() , "Invalid number of columns" );
      BLAZE_INTERNAL_ASSERT( A.rows()    == (~lhs).rows()   , "Invalid number of rows"    );
      BLAZE_INTERNAL_ASSERT( B.columns() == (~lhs).columns(), "Invalid number of columns" );

      DMatScalarMultExpr::selectAddAssignKernel( ~lhs, A, B, rhs.scalar_ );
   }
   //**********************************************************************************************

   //**Addition assignment to dense matrices (kernel selection)************************************
   /*!\brief Selection of the kernel for an addition assignment of a scaled dense matrix-dense
   //        matrix multiplication to a dense matrix (\f$ C+=s*A*B \f$).
   // \ingroup dense_matrix
   //
   // \param C The target left-hand side dense matrix.
   // \param A The left-hand side multiplication operand.
   // \param B The right-hand side multiplication operand.
   // \param scalar The scaling factor.
   // \return void
   */
   template< typename MT3    // Type of the left-hand side target matrix
           , typename MT4    // Type of the left-hand side matrix operand
           , typename MT5    // Type of the right-hand side matrix operand
           , typename ST2 >  // Type of the scalar value
   static inline void selectAddAssignKernel( MT3& C, const MT4& A, const MT5& B, ST2 scalar )
   {
      if( ( IsDiagonal<MT5>::value ) ||
          ( !BLAZE_DEBUG_MODE && B.columns() <= SIMDSIZE*10UL ) ||
          ( C.rows() * C.columns() < DMATDMATMULT_THRESHOLD ) )
         selectSmallAddAssignKernel( C, A, B, scalar );
      else
         selectBlasAddAssignKernel( C, A, B, scalar );
   }
   //**********************************************************************************************

   //**Default addition assignment to dense matrices (general/general)*****************************
   /*!\brief Default addition assignment of a scaled general dense matrix-general dense matrix
   //        multiplication (\f$ C+=s*A*B \f$).
   // \ingroup dense_matrix
   //
   // \param C The target left-hand side dense matrix.
   // \param A The left-hand side multiplication operand.
   // \param B The right-hand side multiplication operand.
   // \param scalar The scaling factor.
   // \return void
   //
   // This function implements the default addition assignment of a scaled dense matrix-dense
   // matrix multiplication expression to a dense matrix.
   */
   template< typename MT3    // Type of the left-hand side target matrix
           , typename MT4    // Type of the left-hand side matrix operand
           , typename MT5    // Type of the right-hand side matrix operand
           , typename ST2 >  // Type of the scalar value
   static inline EnableIf_< And< Not< IsDiagonal<MT4> >, Not< IsDiagonal<MT5> > > >
      selectDefaultAddAssignKernel( MT3& C, const MT4& A, const MT5& B, ST2 scalar )
   {
      const ResultType tmp( serial( A * B * scalar ) );
      addAssign( C, tmp );
   }
   //**********************************************************************************************

   //**Default addition assignment to dense matrices (general/diagonal)****************************
   /*!\brief Default addition assignment of a scaled general dense matrix-diagonal dense matrix
   //        multiplication (\f$ C+=s*A*B \f$).
   // \ingroup dense_matrix
   //
   // \param C The target left-hand side dense matrix.
   // \param A The left-hand side multiplication operand.
   // \param B The right-hand side multiplication operand.
   // \param scalar The scaling factor.
   // \return void
   //
   // This function implements the default addition assignment of a scaled general dense matrix-
   // diagonal dense matrix multiplication expression to a dense matrix.
   */
   template< typename MT3    // Type of the left-hand side target matrix
           , typename MT4    // Type of the left-hand side matrix operand
           , typename MT5    // Type of the right-hand side matrix operand
           , typename ST2 >  // Type of the scalar value
   static inline EnableIf_< And< Not< IsDiagonal<MT4> >, IsDiagonal<MT5> > >
      selectDefaultAddAssignKernel( MT3& C, const MT4& A, const MT5& B, ST2 scalar )
   {
      BLAZE_CONSTRAINT_MUST_BE_ROW_MAJOR_MATRIX_TYPE( MT3 );

      const size_t M( A.rows()    );
      const size_t N( B.columns() );

      for( size_t i=0UL; i<M; ++i )
      {
         const size_t jbegin( ( IsUpper<MT4>::value )
                              ?( IsStrictlyUpper<MT4>::value ? i+1UL : i )
                              :( 0UL ) );
         const size_t jend( ( IsLower<MT4>::value )
                            ?( IsStrictlyLower<MT4>::value ? i : i+1UL )
                            :( N ) );
         BLAZE_INTERNAL_ASSERT( jbegin <= jend, "Invalid loop indices detected" );

         const size_t jnum( jend - jbegin );
         const size_t jpos( jbegin + ( jnum & size_t(-2) ) );

         for( size_t j=jbegin; j<jpos; j+=2UL ) {
            C(i,j    ) += A(i,j    ) * B(j    ,j    ) * scalar;
            C(i,j+1UL) += A(i,j+1UL) * B(j+1UL,j+1UL) * scalar;
         }
         if( jpos < jend ) {
            C(i,jpos) += A(i,jpos) * B(jpos,jpos) * scalar;
         }
      }
   }
   //**********************************************************************************************

   //**Default addition assignment to dense matrices (diagonal/general)****************************
   /*!\brief Default addition assignment of a scaled diagonal dense matrix-general dense matrix
   //        multiplication (\f$ C+=s*A*B \f$).
   // \ingroup dense_matrix
   //
   // \param C The target left-hand side dense matrix.
   // \param A The left-hand side multiplication operand.
   // \param B The right-hand side multiplication operand.
   // \param scalar The scaling factor.
   // \return void
   //
   // This function implements the default addition assignment of a scaled diagonal dense matrix-
   // general dense matrix multiplication expression to a dense matrix.
   */
   template< typename MT3    // Type of the left-hand side target matrix
           , typename MT4    // Type of the left-hand side matrix operand
           , typename MT5    // Type of the right-hand side matrix operand
           , typename ST2 >  // Type of the scalar value
   static inline EnableIf_< And< IsDiagonal<MT4>, Not< IsDiagonal<MT5> > > >
      selectDefaultAddAssignKernel( MT3& C, const MT4& A, const MT5& B, ST2 scalar )
   {
      BLAZE_CONSTRAINT_MUST_BE_ROW_MAJOR_MATRIX_TYPE( MT3 );

      const size_t M( A.rows()    );
      const size_t N( B.columns() );

      for( size_t i=0UL; i<M; ++i )
      {
         const size_t jbegin( ( IsUpper<MT5>::value )
                              ?( IsStrictlyUpper<MT5>::value ? i+1UL : i )
                              :( 0UL ) );
         const size_t jend( ( IsLower<MT5>::value )
                            ?( IsStrictlyLower<MT5>::value ? i : i+1UL )
                            :( N ) );
         BLAZE_INTERNAL_ASSERT( jbegin <= jend, "Invalid loop indices detected" );

         const size_t jnum( jend - jbegin );
         const size_t jpos( jbegin + ( jnum & size_t(-2) ) );

         for( size_t j=jbegin; j<jpos; j+=2UL ) {
            C(i,j    ) += A(i,i) * B(i,j    ) * scalar;
            C(i,j+1UL) += A(i,i) * B(i,j+1UL) * scalar;
         }
         if( jpos < jend ) {
            C(i,jpos) += A(i,i) * B(i,jpos) * scalar;
         }
      }
   }
   //**********************************************************************************************

   //**Default addition assignment to dense matrices (diagonal/diagonal)***************************
   /*!\brief Default addition assignment of a scaled diagonal dense matrix-diagonal dense matrix
   //        multiplication (\f$ C+=s*A*B \f$).
   // \ingroup dense_matrix
   //
   // \param C The target left-hand side dense matrix.
   // \param A The left-hand side multiplication operand.
   // \param B The right-hand side multiplication operand.
   // \param scalar The scaling factor.
   // \return void
   //
   // This function implements the default addition assignment of a scaled diagonal dense matrix-
   // diagonal dense matrix multiplication expression to a dense matrix.
   */
   template< typename MT3    // Type of the left-hand side target matrix
           , typename MT4    // Type of the left-hand side matrix operand
           , typename MT5    // Type of the right-hand side matrix operand
           , typename ST2 >  // Type of the scalar value
   static inline EnableIf_< And< IsDiagonal<MT4>, IsDiagonal<MT5> > >
      selectDefaultAddAssignKernel( MT3& C, const MT4& A, const MT5& B, ST2 scalar )
   {
      BLAZE_CONSTRAINT_MUST_BE_ROW_MAJOR_MATRIX_TYPE( MT3 );

      for( size_t i=0UL; i<A.rows(); ++i ) {
         C(i,i) += A(i,i) * B(i,i) * scalar;
      }
   }
   //**********************************************************************************************

   //**Default addition assignment to dense matrices (small matrices)******************************
   /*!\brief Default addition assignment of a small scaled dense matrix-dense matrix multiplication
   //        (\f$ C+=s*A*B \f$).
   // \ingroup dense_matrix
   //
   // \param C The target left-hand side dense matrix.
   // \param A The left-hand side multiplication operand.
   // \param B The right-hand side multiplication operand.
   // \param scalar The scaling factor.
   // \return void
   //
   // This function relays to the default implementation of the addition assignment of a scaled
   // dense matrix-dense matrix multiplication expression to a dense matrix.
   */
   template< typename MT3    // Type of the left-hand side target matrix
           , typename MT4    // Type of the left-hand side matrix operand
           , typename MT5    // Type of the right-hand side matrix operand
           , typename ST2 >  // Type of the scalar value
   static inline DisableIf_< UseVectorizedDefaultKernel<MT3,MT4,MT5,ST2> >
      selectSmallAddAssignKernel( MT3& C, const MT4& A, const MT5& B, ST2 scalar )
   {
      selectDefaultAddAssignKernel( C, A, B, scalar );
   }
   //**********************************************************************************************

   //**Vectorized default addition assignment to row-major dense matrices (small matrices)*********
   /*!\brief Vectorized default addition assignment of a small scaled dense matrix-dense matrix
   //        multiplication (\f$ C+=s*A*B \f$).
   // \ingroup dense_matrix
   //
   // \param C The target left-hand side dense matrix.
   // \param A The left-hand side multiplication operand.
   // \param B The right-hand side multiplication operand.
   // \param scalar The scaling factor.
   // \return void
   //
   // This function implements the vectorized default addition assignment of a scaled dense
   // matrix-dense matrix multiplication expression to a row-major dense matrix. This kernel
   // is optimized for small matrices.
   */
   template< typename MT3    // Type of the left-hand side target matrix
           , typename MT4    // Type of the left-hand side matrix operand
           , typename MT5    // Type of the right-hand side matrix operand
           , typename ST2 >  // Type of the scalar value
   static inline EnableIf_< UseVectorizedDefaultKernel<MT3,MT4,MT5,ST2> >
      selectSmallAddAssignKernel( DenseMatrix<MT3,false>& C, const MT4& A, const MT5& B, ST2 scalar )
   {
      constexpr bool remainder( !IsPadded<MT3>::value || !IsPadded<MT5>::value );

      const size_t M( A.rows()    );
      const size_t N( B.columns() );
      const size_t K( A.columns() );

      BLAZE_INTERNAL_ASSERT( !( LOW || UPP ) || ( M == N ), "Broken invariant detected" );

      const size_t jpos( remainder ? ( N & size_t(-SIMDSIZE) ) : N );
      BLAZE_INTERNAL_ASSERT( !remainder || ( N - ( N % SIMDSIZE ) ) == jpos, "Invalid end calculation" );

      const SIMDType factor( set( scalar ) );

      size_t j( 0UL );

      if( IsIntegral<ElementType>::value )
      {
         for( ; !LOW && !UPP && (j+SIMDSIZE*7UL) < jpos; j+=SIMDSIZE*8UL ) {
            for( size_t i=0UL; i<M; ++i )
            {
               const size_t kbegin( ( IsUpper<MT4>::value )
                                    ?( ( IsLower<MT5>::value )
                                       ?( max( ( IsStrictlyUpper<MT4>::value ? i+1UL : i ), j ) )
                                       :( IsStrictlyUpper<MT4>::value ? i+1UL : i ) )
                                    :( IsLower<MT5>::value ? j : 0UL ) );
               const size_t kend( ( IsLower<MT4>::value )
                                  ?( ( IsUpper<MT5>::value )
                                     ?( min( ( IsStrictlyLower<MT4>::value ? i : i+1UL ), j+SIMDSIZE*8UL, K ) )
                                     :( IsStrictlyLower<MT4>::value ? i : i+1UL ) )
                                  :( IsUpper<MT5>::value ? min( j+SIMDSIZE*8UL, K ) : K ) );

               SIMDType xmm1, xmm2, xmm3, xmm4, xmm5, xmm6, xmm7, xmm8;

               for( size_t k=kbegin; k<kend; ++k ) {
                  const SIMDType a1( set( A(i,k) ) );
                  xmm1 += a1 * B.load(k,j             );
                  xmm2 += a1 * B.load(k,j+SIMDSIZE    );
                  xmm3 += a1 * B.load(k,j+SIMDSIZE*2UL);
                  xmm4 += a1 * B.load(k,j+SIMDSIZE*3UL);
                  xmm5 += a1 * B.load(k,j+SIMDSIZE*4UL);
                  xmm6 += a1 * B.load(k,j+SIMDSIZE*5UL);
                  xmm7 += a1 * B.load(k,j+SIMDSIZE*6UL);
                  xmm8 += a1 * B.load(k,j+SIMDSIZE*7UL);
               }

               (~C).store( i, j             , (~C).load(i,j             ) + xmm1 * factor );
               (~C).store( i, j+SIMDSIZE    , (~C).load(i,j+SIMDSIZE    ) + xmm2 * factor );
               (~C).store( i, j+SIMDSIZE*2UL, (~C).load(i,j+SIMDSIZE*2UL) + xmm3 * factor );
               (~C).store( i, j+SIMDSIZE*3UL, (~C).load(i,j+SIMDSIZE*3UL) + xmm4 * factor );
               (~C).store( i, j+SIMDSIZE*4UL, (~C).load(i,j+SIMDSIZE*4UL) + xmm5 * factor );
               (~C).store( i, j+SIMDSIZE*5UL, (~C).load(i,j+SIMDSIZE*5UL) + xmm6 * factor );
               (~C).store( i, j+SIMDSIZE*6UL, (~C).load(i,j+SIMDSIZE*6UL) + xmm7 * factor );
               (~C).store( i, j+SIMDSIZE*7UL, (~C).load(i,j+SIMDSIZE*7UL) + xmm8 * factor );
            }
         }
      }

      for( ; !LOW && !UPP && (j+SIMDSIZE*4UL) < jpos; j+=SIMDSIZE*5UL )
      {
         size_t i( 0UL );

         for( ; (i+2UL) <= M; i+=2UL )
         {
            const size_t kbegin( ( IsUpper<MT4>::value )
                                 ?( ( IsLower<MT5>::value )
                                    ?( max( ( IsStrictlyUpper<MT4>::value ? i+1UL : i ), j ) )
                                    :( IsStrictlyUpper<MT4>::value ? i+1UL : i ) )
                                 :( IsLower<MT5>::value ? j : 0UL ) );
            const size_t kend( ( IsLower<MT4>::value )
                               ?( ( IsUpper<MT5>::value )
                                  ?( min( ( IsStrictlyLower<MT4>::value ? i+1UL : i+2UL ), j+SIMDSIZE*5UL, K ) )
                                  :( IsStrictlyLower<MT4>::value ? i+1UL : i+2UL ) )
                               :( IsUpper<MT5>::value ? min( j+SIMDSIZE*5UL, K ) : K ) );

            SIMDType xmm1, xmm2, xmm3, xmm4, xmm5, xmm6, xmm7, xmm8, xmm9, xmm10;

            for( size_t k=kbegin; k<kend; ++k ) {
               const SIMDType a1( set( A(i    ,k) ) );
               const SIMDType a2( set( A(i+1UL,k) ) );
               const SIMDType b1( B.load(k,j             ) );
               const SIMDType b2( B.load(k,j+SIMDSIZE    ) );
               const SIMDType b3( B.load(k,j+SIMDSIZE*2UL) );
               const SIMDType b4( B.load(k,j+SIMDSIZE*3UL) );
               const SIMDType b5( B.load(k,j+SIMDSIZE*4UL) );
               xmm1  += a1 * b1;
               xmm2  += a1 * b2;
               xmm3  += a1 * b3;
               xmm4  += a1 * b4;
               xmm5  += a1 * b5;
               xmm6  += a2 * b1;
               xmm7  += a2 * b2;
               xmm8  += a2 * b3;
               xmm9  += a2 * b4;
               xmm10 += a2 * b5;
            }

            (~C).store( i    , j             , (~C).load(i    ,j             ) + xmm1  * factor );
            (~C).store( i    , j+SIMDSIZE    , (~C).load(i    ,j+SIMDSIZE    ) + xmm2  * factor );
            (~C).store( i    , j+SIMDSIZE*2UL, (~C).load(i    ,j+SIMDSIZE*2UL) + xmm3  * factor );
            (~C).store( i    , j+SIMDSIZE*3UL, (~C).load(i    ,j+SIMDSIZE*3UL) + xmm4  * factor );
            (~C).store( i    , j+SIMDSIZE*4UL, (~C).load(i    ,j+SIMDSIZE*4UL) + xmm5  * factor );
            (~C).store( i+1UL, j             , (~C).load(i+1UL,j             ) + xmm6  * factor );
            (~C).store( i+1UL, j+SIMDSIZE    , (~C).load(i+1UL,j+SIMDSIZE    ) + xmm7  * factor );
            (~C).store( i+1UL, j+SIMDSIZE*2UL, (~C).load(i+1UL,j+SIMDSIZE*2UL) + xmm8  * factor );
            (~C).store( i+1UL, j+SIMDSIZE*3UL, (~C).load(i+1UL,j+SIMDSIZE*3UL) + xmm9  * factor );
            (~C).store( i+1UL, j+SIMDSIZE*4UL, (~C).load(i+1UL,j+SIMDSIZE*4UL) + xmm10 * factor );
         }

         if( i < M )
         {
            const size_t kbegin( ( IsUpper<MT4>::value )
                                 ?( ( IsLower<MT5>::value )
                                    ?( max( ( IsStrictlyUpper<MT4>::value ? i+1UL : i ), j ) )
                                    :( IsStrictlyUpper<MT4>::value ? i+1UL : i ) )
                                 :( IsLower<MT5>::value ? j : 0UL ) );
            const size_t kend( ( IsUpper<MT5>::value )?( min( j+SIMDSIZE*5UL, K ) ):( K ) );

            SIMDType xmm1, xmm2, xmm3, xmm4, xmm5;

            for( size_t k=kbegin; k<kend; ++k ) {
               const SIMDType a1( set( A(i,k) ) );
               xmm1 += a1 * B.load(k,j             );
               xmm2 += a1 * B.load(k,j+SIMDSIZE    );
               xmm3 += a1 * B.load(k,j+SIMDSIZE*2UL);
               xmm4 += a1 * B.load(k,j+SIMDSIZE*3UL);
               xmm5 += a1 * B.load(k,j+SIMDSIZE*4UL);
            }

            (~C).store( i, j             , (~C).load(i,j             ) + xmm1 * factor );
            (~C).store( i, j+SIMDSIZE    , (~C).load(i,j+SIMDSIZE    ) + xmm2 * factor );
            (~C).store( i, j+SIMDSIZE*2UL, (~C).load(i,j+SIMDSIZE*2UL) + xmm3 * factor );
            (~C).store( i, j+SIMDSIZE*3UL, (~C).load(i,j+SIMDSIZE*3UL) + xmm4 * factor );
            (~C).store( i, j+SIMDSIZE*4UL, (~C).load(i,j+SIMDSIZE*4UL) + xmm5 * factor );
         }
      }

      for( ; !LOW && !UPP && (j+SIMDSIZE*3UL) < jpos; j+=SIMDSIZE*4UL )
      {
         size_t i( 0UL );

         for( ; (i+2UL) <= M; i+=2UL )
         {
            const size_t kbegin( ( IsUpper<MT4>::value )
                                 ?( ( IsLower<MT5>::value )
                                    ?( max( ( IsStrictlyUpper<MT4>::value ? i+1UL : i ), j ) )
                                    :( IsStrictlyUpper<MT4>::value ? i+1UL : i ) )
                                 :( IsLower<MT5>::value ? j : 0UL ) );
            const size_t kend( ( IsLower<MT4>::value )
                               ?( ( IsUpper<MT5>::value )
                                  ?( min( ( IsStrictlyLower<MT4>::value ? i+1UL : i+2UL ), j+SIMDSIZE*4UL, K ) )
                                  :( IsStrictlyLower<MT4>::value ? i+1UL : i+2UL ) )
                               :( IsUpper<MT5>::value ? min( j+SIMDSIZE*4UL, K ) : K ) );

            SIMDType xmm1, xmm2, xmm3, xmm4, xmm5, xmm6, xmm7, xmm8;

            for( size_t k=kbegin; k<kend; ++k ) {
               const SIMDType a1( set( A(i    ,k) ) );
               const SIMDType a2( set( A(i+1UL,k) ) );
               const SIMDType b1( B.load(k,j             ) );
               const SIMDType b2( B.load(k,j+SIMDSIZE    ) );
               const SIMDType b3( B.load(k,j+SIMDSIZE*2UL) );
               const SIMDType b4( B.load(k,j+SIMDSIZE*3UL) );
               xmm1 += a1 * b1;
               xmm2 += a1 * b2;
               xmm3 += a1 * b3;
               xmm4 += a1 * b4;
               xmm5 += a2 * b1;
               xmm6 += a2 * b2;
               xmm7 += a2 * b3;
               xmm8 += a2 * b4;
            }

            (~C).store( i    , j             , (~C).load(i    ,j             ) + xmm1 * factor );
            (~C).store( i    , j+SIMDSIZE    , (~C).load(i    ,j+SIMDSIZE    ) + xmm2 * factor );
            (~C).store( i    , j+SIMDSIZE*2UL, (~C).load(i    ,j+SIMDSIZE*2UL) + xmm3 * factor );
            (~C).store( i    , j+SIMDSIZE*3UL, (~C).load(i    ,j+SIMDSIZE*3UL) + xmm4 * factor );
            (~C).store( i+1UL, j             , (~C).load(i+1UL,j             ) + xmm5 * factor );
            (~C).store( i+1UL, j+SIMDSIZE    , (~C).load(i+1UL,j+SIMDSIZE    ) + xmm6 * factor );
            (~C).store( i+1UL, j+SIMDSIZE*2UL, (~C).load(i+1UL,j+SIMDSIZE*2UL) + xmm7 * factor );
            (~C).store( i+1UL, j+SIMDSIZE*3UL, (~C).load(i+1UL,j+SIMDSIZE*3UL) + xmm8 * factor );
         }

         if( i < M )
         {
            const size_t kbegin( ( IsUpper<MT4>::value )
                                 ?( ( IsLower<MT5>::value )
                                    ?( max( ( IsStrictlyUpper<MT4>::value ? i+1UL : i ), j ) )
                                    :( IsStrictlyUpper<MT4>::value ? i+1UL : i ) )
                                 :( IsLower<MT5>::value ? j : 0UL ) );
            const size_t kend( ( IsUpper<MT5>::value )?( min( j+SIMDSIZE*4UL, K ) ):( K ) );

            SIMDType xmm1, xmm2, xmm3, xmm4;

            for( size_t k=kbegin; k<kend; ++k ) {
               const SIMDType a1( set( A(i,k) ) );
               xmm1 += a1 * B.load(k,j             );
               xmm2 += a1 * B.load(k,j+SIMDSIZE    );
               xmm3 += a1 * B.load(k,j+SIMDSIZE*2UL);
               xmm4 += a1 * B.load(k,j+SIMDSIZE*3UL);
            }

            (~C).store( i, j             , (~C).load(i,j             ) + xmm1 * factor );
            (~C).store( i, j+SIMDSIZE    , (~C).load(i,j+SIMDSIZE    ) + xmm2 * factor );
            (~C).store( i, j+SIMDSIZE*2UL, (~C).load(i,j+SIMDSIZE*2UL) + xmm3 * factor );
            (~C).store( i, j+SIMDSIZE*3UL, (~C).load(i,j+SIMDSIZE*3UL) + xmm4 * factor );
         }
      }

      for( ; !LOW && !UPP && (j+SIMDSIZE*2UL) < jpos; j+=SIMDSIZE*3UL )
      {
         size_t i( 0UL );

         for( ; (i+2UL) <= M; i+=2UL )
         {
            const size_t kbegin( ( IsUpper<MT4>::value )
                                 ?( ( IsLower<MT5>::value )
                                    ?( max( ( IsStrictlyUpper<MT4>::value ? i+1UL : i ), j ) )
                                    :( IsStrictlyUpper<MT4>::value ? i+1UL : i ) )
                                 :( IsLower<MT5>::value ? j : 0UL ) );
            const size_t kend( ( IsLower<MT4>::value )
                               ?( ( IsUpper<MT5>::value )
                                  ?( min( ( IsStrictlyLower<MT4>::value ? i+1UL : i+2UL ), j+SIMDSIZE*3UL, K ) )
                                  :( IsStrictlyLower<MT4>::value ? i+1UL : i+2UL ) )
                               :( IsUpper<MT5>::value ? min( j+SIMDSIZE*3UL, K ) : K ) );

            SIMDType xmm1, xmm2, xmm3, xmm4, xmm5, xmm6;

            for( size_t k=kbegin; k<kend; ++k ) {
               const SIMDType a1( set( A(i    ,k) ) );
               const SIMDType a2( set( A(i+1UL,k) ) );
               const SIMDType b1( B.load(k,j             ) );
               const SIMDType b2( B.load(k,j+SIMDSIZE    ) );
               const SIMDType b3( B.load(k,j+SIMDSIZE*2UL) );
               xmm1 += a1 * b1;
               xmm2 += a1 * b2;
               xmm3 += a1 * b3;
               xmm4 += a2 * b1;
               xmm5 += a2 * b2;
               xmm6 += a2 * b3;
            }

            (~C).store( i    , j             , (~C).load(i    ,j             ) + xmm1 * factor );
            (~C).store( i    , j+SIMDSIZE    , (~C).load(i    ,j+SIMDSIZE    ) + xmm2 * factor );
            (~C).store( i    , j+SIMDSIZE*2UL, (~C).load(i    ,j+SIMDSIZE*2UL) + xmm3 * factor );
            (~C).store( i+1UL, j             , (~C).load(i+1UL,j             ) + xmm4 * factor );
            (~C).store( i+1UL, j+SIMDSIZE    , (~C).load(i+1UL,j+SIMDSIZE    ) + xmm5 * factor );
            (~C).store( i+1UL, j+SIMDSIZE*2UL, (~C).load(i+1UL,j+SIMDSIZE*2UL) + xmm6 * factor );
         }

         if( i < M )
         {
            const size_t kbegin( ( IsUpper<MT4>::value )
                                 ?( ( IsLower<MT5>::value )
                                    ?( max( ( IsStrictlyUpper<MT4>::value ? i+1UL : i ), j ) )
                                    :( IsStrictlyUpper<MT4>::value ? i+1UL : i ) )
                                 :( IsLower<MT5>::value ? j : 0UL ) );
            const size_t kend( ( IsUpper<MT5>::value )?( min( j+SIMDSIZE*3UL, K ) ):( K ) );

            SIMDType xmm1, xmm2, xmm3;

            for( size_t k=kbegin; k<kend; ++k ) {
               const SIMDType a1( set( A(i,k) ) );
               xmm1 += a1 * B.load(k,j             );
               xmm2 += a1 * B.load(k,j+SIMDSIZE    );
               xmm3 += a1 * B.load(k,j+SIMDSIZE*2UL);
            }

            (~C).store( i, j             , (~C).load(i,j             ) + xmm1 * factor );
            (~C).store( i, j+SIMDSIZE    , (~C).load(i,j+SIMDSIZE    ) + xmm2 * factor );
            (~C).store( i, j+SIMDSIZE*2UL, (~C).load(i,j+SIMDSIZE*2UL) + xmm3 * factor );
         }
      }

      for( ; !( LOW && UPP ) && (j+SIMDSIZE) < jpos; j+=SIMDSIZE*2UL )
      {
         const size_t iend( UPP ? min(j+SIMDSIZE*2UL,M) : M );
         size_t i( LOW ? j : 0UL );

         for( ; (i+2UL) <= iend; i+=2UL )
         {
            const size_t kbegin( ( IsUpper<MT4>::value )
                                 ?( ( IsLower<MT5>::value )
                                    ?( max( ( IsStrictlyUpper<MT4>::value ? i+1UL : i ), j ) )
                                    :( IsStrictlyUpper<MT4>::value ? i+1UL : i ) )
                                 :( IsLower<MT5>::value ? j : 0UL ) );
            const size_t kend( ( IsLower<MT4>::value )
                               ?( ( IsUpper<MT5>::value )
                                  ?( min( ( IsStrictlyLower<MT4>::value ? i+1UL : i+2UL ), j+SIMDSIZE*2UL, K ) )
                                  :( IsStrictlyLower<MT4>::value ? i+1UL : i+2UL ) )
                               :( IsUpper<MT5>::value ? min( j+SIMDSIZE*2UL, K ) : K ) );

            SIMDType xmm1, xmm2, xmm3, xmm4;

            for( size_t k=kbegin; k<kend; ++k ) {
               const SIMDType a1( set( A(i    ,k) ) );
               const SIMDType a2( set( A(i+1UL,k) ) );
               const SIMDType b1( B.load(k,j         ) );
               const SIMDType b2( B.load(k,j+SIMDSIZE) );
               xmm1 += a1 * b1;
               xmm2 += a1 * b2;
               xmm3 += a2 * b1;
               xmm4 += a2 * b2;
            }

            (~C).store( i    , j         , (~C).load(i    ,j         ) + xmm1 * factor );
            (~C).store( i    , j+SIMDSIZE, (~C).load(i    ,j+SIMDSIZE) + xmm2 * factor );
            (~C).store( i+1UL, j         , (~C).load(i+1UL,j         ) + xmm3 * factor );
            (~C).store( i+1UL, j+SIMDSIZE, (~C).load(i+1UL,j+SIMDSIZE) + xmm4 * factor );
         }

         if( i < iend )
         {
            const size_t kbegin( ( IsUpper<MT4>::value )
                                 ?( ( IsLower<MT5>::value )
                                    ?( max( ( IsStrictlyUpper<MT4>::value ? i+1UL : i ), j ) )
                                    :( IsStrictlyUpper<MT4>::value ? i+1UL : i ) )
                                 :( IsLower<MT5>::value ? j : 0UL ) );
            const size_t kend( ( IsUpper<MT5>::value )?( min( j+SIMDSIZE*2UL, K ) ):( K ) );

            SIMDType xmm1, xmm2;

            for( size_t k=kbegin; k<kend; ++k ) {
               const SIMDType a1( set( A(i,k) ) );
               xmm1 += a1 * B.load(k,j         );
               xmm2 += a1 * B.load(k,j+SIMDSIZE);
            }

            (~C).store( i, j         , (~C).load(i,j         ) + xmm1 * factor );
            (~C).store( i, j+SIMDSIZE, (~C).load(i,j+SIMDSIZE) + xmm2 * factor );
         }
      }

      for( ; j<jpos; j+=SIMDSIZE )
      {
         const size_t iend( LOW && UPP ? min(j+SIMDSIZE,M) : M );
         size_t i( LOW ? j : 0UL );

         for( ; (i+2UL) <= iend; i+=2UL )
         {
            const size_t kbegin( ( IsUpper<MT4>::value )
                                 ?( ( IsLower<MT5>::value )
                                    ?( max( ( IsStrictlyUpper<MT4>::value ? i+1UL : i ), j ) )
                                    :( IsStrictlyUpper<MT4>::value ? i+1UL : i ) )
                                 :( IsLower<MT5>::value ? j : 0UL ) );
            const size_t kend( ( IsLower<MT4>::value )
                               ?( IsStrictlyLower<MT4>::value ? i+1UL : i+2UL )
                               :( K ) );

            SIMDType xmm1, xmm2;

            for( size_t k=kbegin; k<kend; ++k ) {
               const SIMDType b1( B.load(k,j) );
               xmm1 += set( A(i    ,k) ) * b1;
               xmm2 += set( A(i+1UL,k) ) * b1;
            }

            (~C).store( i    , j, (~C).load(i    ,j) + xmm1 * factor );
            (~C).store( i+1UL, j, (~C).load(i+1UL,j) + xmm2 * factor );
         }

         if( i < iend )
         {
            const size_t kbegin( ( IsUpper<MT4>::value )
                                 ?( ( IsLower<MT5>::value )
                                    ?( max( ( IsStrictlyUpper<MT4>::value ? i+1UL : i ), j ) )
                                    :( IsStrictlyUpper<MT4>::value ? i+1UL : i ) )
                                 :( IsLower<MT5>::value ? j : 0UL ) );

            SIMDType xmm1;

            for( size_t k=kbegin; k<K; ++k ) {
               xmm1 += set( A(i,k) ) * B.load(k,j);
            }

            (~C).store( i, j, (~C).load(i,j) + xmm1 * factor );
         }
      }

      for( ; remainder && j<N; ++j )
      {
         const size_t iend( UPP ? j+1UL : M );
         size_t i( LOW ? j : 0UL );

         for( ; (i+2UL) <= iend; i+=2UL )
         {
            const size_t kbegin( ( IsUpper<MT4>::value )
                                 ?( ( IsLower<MT5>::value )
                                    ?( max( ( IsStrictlyUpper<MT4>::value ? i+1UL : i ), j ) )
                                    :( IsStrictlyUpper<MT4>::value ? i+1UL : i ) )
                                 :( IsLower<MT5>::value ? j : 0UL ) );
            const size_t kend( ( IsLower<MT4>::value )
                               ?( IsStrictlyLower<MT4>::value ? i+1UL : i+2UL )
                               :( K ) );

            ElementType value1 = ElementType();
            ElementType value2 = ElementType();

            for( size_t k=kbegin; k<kend; ++k ) {
               value1 += A(i    ,k) * B(k,j);
               value2 += A(i+1UL,k) * B(k,j);
            }

            (~C)(i    ,j) += value1 * scalar;
            (~C)(i+1UL,j) += value2 * scalar;
         }

         if( i < iend )
         {
            const size_t kbegin( ( IsUpper<MT4>::value )
                                 ?( ( IsLower<MT5>::value )
                                    ?( max( ( IsStrictlyUpper<MT4>::value ? i+1UL : i ), j ) )
                                    :( IsStrictlyUpper<MT4>::value ? i+1UL : i ) )
                                 :( IsLower<MT5>::value ? j : 0UL ) );

            ElementType value = ElementType();

            for( size_t k=kbegin; k<K; ++k ) {
               value += A(i,k) * B(k,j);
            }

            (~C)(i,j) += value * scalar;
         }
      }
   }
   //**********************************************************************************************

   //**Vectorized default addition assignment to column-major dense matrices (small matrices)******
   /*!\brief Vectorized default addition assignment of a small scaled dense matrix-dense matrix
   //        multiplication (\f$ C+=s*A*B \f$).
   // \ingroup dense_matrix
   //
   // \param C The target left-hand side dense matrix.
   // \param A The left-hand side multiplication operand.
   // \param B The right-hand side multiplication operand.
   // \param scalar The scaling factor.
   // \return void
   //
   // This function implements the vectorized default addition assignment of a scaled dense
   // matrix-dense matrix multiplication expression to a column-major dense matrix. This
   // kernel is optimized for small matrices.
   */
   template< typename MT3    // Type of the left-hand side target matrix
           , typename MT4    // Type of the left-hand side matrix operand
           , typename MT5    // Type of the right-hand side matrix operand
           , typename ST2 >  // Type of the scalar value
   static inline EnableIf_< UseVectorizedDefaultKernel<MT3,MT4,MT5,ST2> >
      selectSmallAddAssignKernel( DenseMatrix<MT3,true>& C, const MT4& A, const MT5& B, ST2 scalar )
   {
      BLAZE_CONSTRAINT_MUST_NOT_BE_SYMMETRIC_MATRIX_TYPE( MT4 );
      BLAZE_CONSTRAINT_MUST_NOT_BE_SYMMETRIC_MATRIX_TYPE( MT5 );
      BLAZE_CONSTRAINT_MUST_BE_COLUMN_MAJOR_MATRIX_TYPE( OppositeType_<MT4> );
      BLAZE_CONSTRAINT_MUST_BE_COLUMN_MAJOR_MATRIX_TYPE( OppositeType_<MT5> );

      const ForwardFunctor fwd;

      if( !IsResizable<MT4>::value && IsResizable<MT5>::value ) {
         const OppositeType_<MT4> tmp( serial( A ) );
         addAssign( ~C, fwd( tmp * B ) * scalar );
      }
      else if( IsResizable<MT4>::value && !IsResizable<MT5>::value ) {
         const OppositeType_<MT5> tmp( serial( B ) );
         addAssign( ~C, fwd( A * tmp ) * scalar );
      }
      else if( A.rows() * A.columns() <= B.rows() * B.columns() ) {
         const OppositeType_<MT4> tmp( serial( A ) );
         addAssign( ~C, fwd( tmp * B ) * scalar );
      }
      else {
         const OppositeType_<MT5> tmp( serial( B ) );
         addAssign( ~C, fwd( A * tmp ) * scalar );
      }
   }
   //**********************************************************************************************

   //**Default addition assignment to dense matrices (large matrices)******************************
   /*!\brief Default addition assignment of a large scaled dense matrix-dense matrix multiplication
   //        (\f$ C+=s*A*B \f$).
   // \ingroup dense_matrix
   //
   // \param C The target left-hand side dense matrix.
   // \param A The left-hand side multiplication operand.
   // \param B The right-hand side multiplication operand.
   // \param scalar The scaling factor.
   // \return void
   //
   // This function relays to the default implementation of the addition assignment of a scaled
   // dense matrix-dense matrix multiplication expression to a dense matrix.
   */
   template< typename MT3    // Type of the left-hand side target matrix
           , typename MT4    // Type of the left-hand side matrix operand
           , typename MT5    // Type of the right-hand side matrix operand
           , typename ST2 >  // Type of the scalar value
   static inline DisableIf_< UseVectorizedDefaultKernel<MT3,MT4,MT5,ST2> >
      selectLargeAddAssignKernel( MT3& C, const MT4& A, const MT5& B, ST2 scalar )
   {
      selectDefaultAddAssignKernel( C, A, B, scalar );
   }
   //**********************************************************************************************

   //**Vectorized default addition assignment to dense matrices (large matrices)*******************
   /*!\brief Vectorized default addition assignment of a large scaled dense matrix-dense matrix
   //        multiplication (\f$ C+=s*A*B \f$).
   // \ingroup dense_matrix
   //
   // \param C The target left-hand side dense matrix.
   // \param A The left-hand side multiplication operand.
   // \param B The right-hand side multiplication operand.
   // \param scalar The scaling factor.
   // \return void
   //
   // This function implements the vectorized default addition assignment of a scaled dense
   // matrix-dense matrix multiplication expression to a dense matrix. This kernel is optimized
   // for large matrices.
   */
   template< typename MT3    // Type of the left-hand side target matrix
           , typename MT4    // Type of the left-hand side matrix operand
           , typename MT5    // Type of the right-hand side matrix operand
           , typename ST2 >  // Type of the scalar value
   static inline EnableIf_< UseVectorizedDefaultKernel<MT3,MT4,MT5,ST2> >
      selectLargeAddAssignKernel( MT3& C, const MT4& A, const MT5& B, ST2 scalar )
   {
      if( LOW )
         lmmm( C, A, B, scalar, ST2(1) );
      else if( UPP )
         ummm( C, A, B, scalar, ST2(1) );
      else
         mmm( C, A, B, scalar, ST2(1) );
   }
   //**********************************************************************************************

   //**BLAS-based addition assignment to dense matrices (default)**********************************
   /*!\brief Default addition assignment of a scaled dense matrix-dense matrix multiplication
   //        (\f$ C+=s*A*B \f$).
   // \ingroup dense_matrix
   //
   // \param C The target left-hand side dense matrix.
   // \param A The left-hand side multiplication operand.
   // \param B The right-hand side multiplication operand.
   // \param scalar The scaling factor.
   // \return void
   //
   // This function relays to the default implementation of the addition assignment of a large
   // scaled dense matrix-dense matrix multiplication expression to a dense matrix.
   */
   template< typename MT3    // Type of the left-hand side target matrix
           , typename MT4    // Type of the left-hand side matrix operand
           , typename MT5    // Type of the right-hand side matrix operand
           , typename ST2 >  // Type of the scalar value
   static inline DisableIf_< UseBlasKernel<MT3,MT4,MT5,ST2> >
      selectBlasAddAssignKernel( MT3& C, const MT4& A, const MT5& B, ST2 scalar )
   {
      selectLargeAddAssignKernel( C, A, B, scalar );
   }
   //**********************************************************************************************

   //**BLAS-based addition assignment to dense matrices********************************************
#if BLAZE_BLAS_MODE && BLAZE_USE_BLAS_MATRIX_MATRIX_MULTIPLICATION
   /*!\brief BLAS-based addition assignment of a scaled dense matrix-dense matrix multiplication
   //        (\f$ C+=s*A*B \f$).
   // \ingroup dense_matrix
   //
   // \param C The target left-hand side dense matrix.
   // \param A The left-hand side multiplication operand.
   // \param B The right-hand side multiplication operand.
   // \param scalar The scaling factor.
   // \return void
   //
   // This function performs the scaled dense matrix-dense matrix multiplication based on the
   // according BLAS functionality.
   */
   template< typename MT3    // Type of the left-hand side target matrix
           , typename MT4    // Type of the left-hand side matrix operand
           , typename MT5    // Type of the right-hand side matrix operand
           , typename ST2 >  // Type of the scalar value
   static inline EnableIf_< UseBlasKernel<MT3,MT4,MT5,ST2> >
      selectBlasAddAssignKernel( MT3& C, const MT4& A, const MT5& B, ST2 scalar )
   {
      typedef ElementType_<MT3>  ET;

      if( IsTriangular<MT4>::value ) {
         ResultType_<MT3> tmp( serial( B ) );
         trmm( tmp, A, CblasLeft, ( IsLower<MT4>::value )?( CblasLower ):( CblasUpper ), ET(scalar) );
         addAssign( C, tmp );
      }
      else if( IsTriangular<MT5>::value ) {
         ResultType_<MT3> tmp( serial( A ) );
         trmm( tmp, B, CblasRight, ( IsLower<MT5>::value )?( CblasLower ):( CblasUpper ), ET(scalar) );
         addAssign( C, tmp );
      }
      else {
         gemm( C, A, B, ET(scalar), ET(1) );
      }
   }
#endif
   //**********************************************************************************************

   //**Restructuring addition assignment to column-major matrices**********************************
   /*!\brief Restructuring addition assignment of a scaled dense matrix-dense matrix multiplication
   //        to a column-major matrix (\f$ C+=s*A*B \f$).
   // \ingroup dense_matrix
   //
   // \param lhs The target left-hand side matrix.
   // \param rhs The right-hand side scaled multiplication expression to be added.
   // \return void
   //
   // This function implements the symmetry-based restructuring addition assignment of a scaled
   // dense matrix-dense matrix multiplication expression to a column-major matrix. Due to the
   // explicit application of the SFINAE principle this function can only be selected by the
   // compiler in case the symmetry of either of the two matrix operands can be exploited.
   */
   template< typename MT >  // Type of the target matrix
   friend inline EnableIf_< CanExploitSymmetry<MT,MT1,MT2> >
      addAssign( Matrix<MT,true>& lhs, const DMatScalarMultExpr& rhs )
   {
      BLAZE_FUNCTION_TRACE;

      BLAZE_CONSTRAINT_MUST_NOT_BE_SYMMETRIC_MATRIX_TYPE( MT );

      BLAZE_INTERNAL_ASSERT( (~lhs).rows()    == rhs.rows()   , "Invalid number of rows"    );
      BLAZE_INTERNAL_ASSERT( (~lhs).columns() == rhs.columns(), "Invalid number of columns" );

      const ForwardFunctor fwd;

      LeftOperand_<MMM>  left ( rhs.matrix_.leftOperand()  );
      RightOperand_<MMM> right( rhs.matrix_.rightOperand() );

      if( IsSymmetric<MT1>::value && IsSymmetric<MT2>::value )
         addAssign( ~lhs, fwd( trans( left ) * trans( right ) ) * rhs.scalar_ );
      else if( IsSymmetric<MT1>::value )
         addAssign( ~lhs, fwd( trans( left ) * right ) * rhs.scalar_ );
      else
         addAssign( ~lhs, fwd( left * trans( right ) ) * rhs.scalar_ );
   }
   //**********************************************************************************************

   //**Addition assignment to sparse matrices******************************************************
   // No special implementation for the addition assignment to sparse matrices.
   //**********************************************************************************************

   //**Subtraction assignment to dense matrices****************************************************
   /*!\brief Subtraction assignment of a scaled dense matrix-dense matrix multiplication to a
   //        dense matrix (\f$ C-=s*A*B \f$).
   // \ingroup dense_matrix
   //
   // \param lhs The target left-hand side dense matrix.
   // \param rhs The right-hand side scaled multiplication expression to be subtracted.
   // \return void
   //
   // This function implements the performance optimized subtraction assignment of a scaled dense
   // matrix-dense matrix multiplication expression to a dense matrix.
   */
   template< typename MT  // Type of the target dense matrix
           , bool SO >    // Storage order of the target dense matrix
   friend inline DisableIf_< CanExploitSymmetry<MT,MT1,MT2> >
      subAssign( DenseMatrix<MT,SO>& lhs, const DMatScalarMultExpr& rhs )
   {
      BLAZE_FUNCTION_TRACE;

      BLAZE_INTERNAL_ASSERT( (~lhs).rows()    == rhs.rows()   , "Invalid number of rows"    );
      BLAZE_INTERNAL_ASSERT( (~lhs).columns() == rhs.columns(), "Invalid number of columns" );

      LeftOperand_<MMM>  left ( rhs.matrix_.leftOperand()  );
      RightOperand_<MMM> right( rhs.matrix_.rightOperand() );

      if( (~lhs).rows() == 0UL || (~lhs).columns() == 0UL || left.columns() == 0UL ) {
         return;
      }

      LT A( serial( left  ) );  // Evaluation of the left-hand side dense matrix operand
      RT B( serial( right ) );  // Evaluation of the right-hand side dense matrix operand

      BLAZE_INTERNAL_ASSERT( A.rows()    == left.rows()     , "Invalid number of rows"    );
      BLAZE_INTERNAL_ASSERT( A.columns() == left.columns()  , "Invalid number of columns" );
      BLAZE_INTERNAL_ASSERT( B.rows()    == right.rows()    , "Invalid number of rows"    );
      BLAZE_INTERNAL_ASSERT( B.columns() == right.columns() , "Invalid number of columns" );
      BLAZE_INTERNAL_ASSERT( A.rows()    == (~lhs).rows()   , "Invalid number of rows"    );
      BLAZE_INTERNAL_ASSERT( B.columns() == (~lhs).columns(), "Invalid number of columns" );

      DMatScalarMultExpr::selectSubAssignKernel( ~lhs, A, B, rhs.scalar_ );
   }
   //**********************************************************************************************

   //**Subtraction assignment to dense matrices (kernel selection)*********************************
   /*!\brief Selection of the kernel for a subtraction assignment of a scaled dense matrix-dense
   //        matrix multiplication to a dense matrix (\f$ C-=s*A*B \f$).
   // \ingroup dense_matrix
   //
   // \param C The target left-hand side dense matrix.
   // \param A The left-hand side multiplication operand.
   // \param B The right-hand side multiplication operand.
   // \param scalar The scaling factor.
   // \return void
   */
   template< typename MT3    // Type of the left-hand side target matrix
           , typename MT4    // Type of the left-hand side matrix operand
           , typename MT5    // Type of the right-hand side matrix operand
           , typename ST2 >  // Type of the scalar value
   static inline void selectSubAssignKernel( MT3& C, const MT4& A, const MT5& B, ST2 scalar )
   {
      if( ( IsDiagonal<MT5>::value ) ||
          ( !BLAZE_DEBUG_MODE && B.columns() <= SIMDSIZE*10UL ) ||
          ( C.rows() * C.columns() < DMATDMATMULT_THRESHOLD ) )
         selectSmallSubAssignKernel( C, A, B, scalar );
      else
         selectBlasSubAssignKernel( C, A, B, scalar );
   }
   //**********************************************************************************************

   //**Default subtraction assignment to dense matrices (general/general)**************************
   /*!\brief Default subtraction assignment of a scaled general dense matrix-general dense matrix
   //        multiplication (\f$ C-=s*A*B \f$).
   // \ingroup dense_matrix
   //
   // \param C The target left-hand side dense matrix.
   // \param A The left-hand side multiplication operand.
   // \param B The right-hand side multiplication operand.
   // \param scalar The scaling factor.
   // \return void
   //
   // This function implements the default subtraction assignment of a scaled general dense
   // matrix-general dense matrix multiplication expression to a dense matrix.
   */
   template< typename MT3    // Type of the left-hand side target matrix
           , typename MT4    // Type of the left-hand side matrix operand
           , typename MT5    // Type of the right-hand side matrix operand
           , typename ST2 >  // Type of the scalar value
   static inline EnableIf_< And< Not< IsDiagonal<MT4> >, Not< IsDiagonal<MT5> > > >
      selectDefaultSubAssignKernel( MT3& C, const MT4& A, const MT5& B, ST2 scalar )
   {
      const ResultType tmp( serial( A * B * scalar ) );
      subAssign( C, tmp );
   }
   //**********************************************************************************************

   //**Default subtraction assignment to dense matrices (general/diagonal)*************************
   /*!\brief Default subtraction assignment of a scaled general dense matrix-diagonal dense matrix
   //        multiplication (\f$ C-=s*A*B \f$).
   // \ingroup dense_matrix
   //
   // \param C The target left-hand side dense matrix.
   // \param A The left-hand side multiplication operand.
   // \param B The right-hand side multiplication operand.
   // \param scalar The scaling factor.
   // \return void
   //
   // This function implements the default subtraction assignment of a scaled general dense
   // matrix-diagonal dense matrix multiplication expression to a dense matrix.
   */
   template< typename MT3    // Type of the left-hand side target matrix
           , typename MT4    // Type of the left-hand side matrix operand
           , typename MT5    // Type of the right-hand side matrix operand
           , typename ST2 >  // Type of the scalar value
   static inline EnableIf_< And< Not< IsDiagonal<MT4> >, IsDiagonal<MT5> > >
      selectDefaultSubAssignKernel( MT3& C, const MT4& A, const MT5& B, ST2 scalar )
   {
      BLAZE_CONSTRAINT_MUST_BE_ROW_MAJOR_MATRIX_TYPE( MT3 );

      const size_t M( A.rows()    );
      const size_t N( B.columns() );

      for( size_t i=0UL; i<M; ++i )
      {
         const size_t jbegin( ( IsUpper<MT4>::value )
                              ?( IsStrictlyUpper<MT4>::value ? i+1UL : i )
                              :( 0UL ) );
         const size_t jend( ( IsLower<MT4>::value )
                            ?( IsStrictlyLower<MT4>::value ? i : i+1UL )
                            :( N ) );
         BLAZE_INTERNAL_ASSERT( jbegin <= jend, "Invalid loop indices detected" );

         const size_t jnum( jend - jbegin );
         const size_t jpos( jbegin + ( jnum & size_t(-2) ) );

         for( size_t j=jbegin; j<jpos; j+=2UL ) {
            C(i,j    ) -= A(i,j    ) * B(j    ,j    ) * scalar;
            C(i,j+1UL) -= A(i,j+1UL) * B(j+1UL,j+1UL) * scalar;
         }
         if( jpos < jend ) {
            C(i,jpos) -= A(i,jpos) * B(jpos,jpos) * scalar;
         }
      }
   }
   //**********************************************************************************************

   //**Default subtraction assignment to dense matrices (diagonal/general)*************************
   /*!\brief Default subtraction assignment of a scaled diagonal dense matrix-general dense matrix
   //        multiplication (\f$ C-=s*A*B \f$).
   // \ingroup dense_matrix
   //
   // \param C The target left-hand side dense matrix.
   // \param A The left-hand side multiplication operand.
   // \param B The right-hand side multiplication operand.
   // \param scalar The scaling factor.
   // \return void
   //
   // This function implements the default subtraction assignment of a scaled diagonal dense
   // matrix-general dense matrix multiplication expression to a dense matrix.
   */
   template< typename MT3    // Type of the left-hand side target matrix
           , typename MT4    // Type of the left-hand side matrix operand
           , typename MT5    // Type of the right-hand side matrix operand
           , typename ST2 >  // Type of the scalar value
   static inline EnableIf_< And< IsDiagonal<MT4>, Not< IsDiagonal<MT5> > > >
      selectDefaultSubAssignKernel( MT3& C, const MT4& A, const MT5& B, ST2 scalar )
   {
      BLAZE_CONSTRAINT_MUST_BE_ROW_MAJOR_MATRIX_TYPE( MT3 );

      const size_t M( A.rows()    );
      const size_t N( B.columns() );

      for( size_t i=0UL; i<M; ++i )
      {
         const size_t jbegin( ( IsUpper<MT5>::value )
                              ?( IsStrictlyUpper<MT5>::value ? i+1UL : i )
                              :( 0UL ) );
         const size_t jend( ( IsLower<MT5>::value )
                            ?( IsStrictlyLower<MT5>::value ? i : i+1UL )
                            :( N ) );
         BLAZE_INTERNAL_ASSERT( jbegin <= jend, "Invalid loop indices detected" );

         const size_t jnum( jend - jbegin );
         const size_t jpos( jbegin + ( jnum & size_t(-2) ) );

         for( size_t j=jbegin; j<jpos; j+=2UL ) {
            C(i,j    ) -= A(i,i) * B(i,j    ) * scalar;
            C(i,j+1UL) -= A(i,i) * B(i,j+1UL) * scalar;
         }
         if( jpos < jend ) {
            C(i,jpos) -= A(i,i) * B(i,jpos) * scalar;
         }
      }
   }
   //**********************************************************************************************

   //**Default subtraction assignment to dense matrices (diagonal/diagonal)************************
   /*!\brief Default subtraction assignment of a scaled diagonal dense matrix-diagonal dense
   //        matrix multiplication (\f$ C-=s*A*B \f$).
   // \ingroup dense_matrix
   //
   // \param C The target left-hand side dense matrix.
   // \param A The left-hand side multiplication operand.
   // \param B The right-hand side multiplication operand.
   // \param scalar The scaling factor.
   // \return void
   //
   // This function implements the default subtraction assignment of a scaled diagonal dense
   // matrix-diagonal dense matrix multiplication expression to a dense matrix.
   */
   template< typename MT3    // Type of the left-hand side target matrix
           , typename MT4    // Type of the left-hand side matrix operand
           , typename MT5    // Type of the right-hand side matrix operand
           , typename ST2 >  // Type of the scalar value
   static inline EnableIf_< And< IsDiagonal<MT4>, IsDiagonal<MT5> > >
      selectDefaultSubAssignKernel( MT3& C, const MT4& A, const MT5& B, ST2 scalar )
   {
      BLAZE_CONSTRAINT_MUST_BE_ROW_MAJOR_MATRIX_TYPE( MT3 );

      for( size_t i=0UL; i<A.rows(); ++i ) {
         C(i,i) -= A(i,i) * B(i,i) * scalar;
      }
   }
   //**********************************************************************************************

   //**Default subtraction assignment to dense matrices (small matrices)***************************
   /*!\brief Default subtraction assignment of a small scaled dense matrix-dense matrix
   //        multiplication (\f$ C-=s*A*B \f$).
   // \ingroup dense_matrix
   //
   // \param C The target left-hand side dense matrix.
   // \param A The left-hand side multiplication operand.
   // \param B The right-hand side multiplication operand.
   // \param scalar The scaling factor.
   // \return void
   //
   // This function relays to the default implementation of the subtraction assignment of a scaled
   // dense matrix-dense matrix multiplication expression to a dense matrix.
   */
   template< typename MT3    // Type of the left-hand side target matrix
           , typename MT4    // Type of the left-hand side matrix operand
           , typename MT5    // Type of the right-hand side matrix operand
           , typename ST2 >  // Type of the scalar value
   static inline DisableIf_< UseVectorizedDefaultKernel<MT3,MT4,MT5,ST2> >
      selectSmallSubAssignKernel( MT3& C, const MT4& A, const MT5& B, ST2 scalar )
   {
      selectDefaultSubAssignKernel( C, A, B, scalar );
   }
   //**********************************************************************************************

   //**Vectorized default subtraction assignment to row-major dense matrices (small matrices)******
   /*!\brief Vectorized default subtraction assignment of a small scaled dense matrix-dense matrix
   //        multiplication (\f$ C-=s*A*B \f$).
   // \ingroup dense_matrix
   //
   // \param C The target left-hand side dense matrix.
   // \param A The left-hand side multiplication operand.
   // \param B The right-hand side multiplication operand.
   // \param scalar The scaling factor.
   // \return void
   //
   // This function implements the vectorized default subtraction assignment of a scaled dense
   // matrix-dense matrix multiplication expression to a row-major dense matrix. This kernel is
   // optimized for small matrices.
   */
   template< typename MT3    // Type of the left-hand side target matrix
           , typename MT4    // Type of the left-hand side matrix operand
           , typename MT5    // Type of the right-hand side matrix operand
           , typename ST2 >  // Type of the scalar value
   static inline EnableIf_< UseVectorizedDefaultKernel<MT3,MT4,MT5,ST2> >
      selectSmallSubAssignKernel( DenseMatrix<MT3,false>& C, const MT4& A, const MT5& B, ST2 scalar )
   {
      constexpr bool remainder( !IsPadded<MT3>::value || !IsPadded<MT5>::value );

      const size_t M( A.rows()    );
      const size_t N( B.columns() );
      const size_t K( A.columns() );

      BLAZE_INTERNAL_ASSERT( !( LOW || UPP ) || ( M == N ), "Broken invariant detected" );

      const size_t jpos( remainder ? ( N & size_t(-SIMDSIZE) ) : N );
      BLAZE_INTERNAL_ASSERT( !remainder || ( N - ( N % SIMDSIZE ) ) == jpos, "Invalid end calculation" );

      const SIMDType factor( set( scalar ) );

      size_t j( 0UL );

      if( IsIntegral<ElementType>::value )
      {
         for( ; !LOW && !UPP && (j+SIMDSIZE*7UL) < jpos; j+=SIMDSIZE*8UL ) {
            for( size_t i=0UL; i<M; ++i )
            {
               const size_t kbegin( ( IsUpper<MT4>::value )
                                    ?( ( IsLower<MT5>::value )
                                       ?( max( ( IsStrictlyUpper<MT4>::value ? i+1UL : i ), j ) )
                                       :( IsStrictlyUpper<MT4>::value ? i+1UL : i ) )
                                    :( IsLower<MT5>::value ? j : 0UL ) );
               const size_t kend( ( IsLower<MT4>::value )
                                  ?( ( IsUpper<MT5>::value )
                                     ?( min( ( IsStrictlyLower<MT4>::value ? i : i+1UL ), j+SIMDSIZE*8UL, K ) )
                                     :( IsStrictlyLower<MT4>::value ? i : i+1UL ) )
                                  :( IsUpper<MT5>::value ? min( j+SIMDSIZE*8UL, K ) : K ) );

               SIMDType xmm1, xmm2, xmm3, xmm4, xmm5, xmm6, xmm7, xmm8;

               for( size_t k=kbegin; k<kend; ++k ) {
                  const SIMDType a1( set( A(i,k) ) );
                  xmm1 += a1 * B.load(k,j             );
                  xmm2 += a1 * B.load(k,j+SIMDSIZE    );
                  xmm3 += a1 * B.load(k,j+SIMDSIZE*2UL);
                  xmm4 += a1 * B.load(k,j+SIMDSIZE*3UL);
                  xmm5 += a1 * B.load(k,j+SIMDSIZE*4UL);
                  xmm6 += a1 * B.load(k,j+SIMDSIZE*5UL);
                  xmm7 += a1 * B.load(k,j+SIMDSIZE*6UL);
                  xmm8 += a1 * B.load(k,j+SIMDSIZE*7UL);
               }

               (~C).store( i, j             , (~C).load(i,j             ) - xmm1 * factor );
               (~C).store( i, j+SIMDSIZE    , (~C).load(i,j+SIMDSIZE    ) - xmm2 * factor );
               (~C).store( i, j+SIMDSIZE*2UL, (~C).load(i,j+SIMDSIZE*2UL) - xmm3 * factor );
               (~C).store( i, j+SIMDSIZE*3UL, (~C).load(i,j+SIMDSIZE*3UL) - xmm4 * factor );
               (~C).store( i, j+SIMDSIZE*4UL, (~C).load(i,j+SIMDSIZE*4UL) - xmm5 * factor );
               (~C).store( i, j+SIMDSIZE*5UL, (~C).load(i,j+SIMDSIZE*5UL) - xmm6 * factor );
               (~C).store( i, j+SIMDSIZE*6UL, (~C).load(i,j+SIMDSIZE*6UL) - xmm7 * factor );
               (~C).store( i, j+SIMDSIZE*7UL, (~C).load(i,j+SIMDSIZE*7UL) - xmm8 * factor );
            }
         }
      }

      for( ; !LOW && !UPP && (j+SIMDSIZE*4UL) < jpos; j+=SIMDSIZE*5UL )
      {
         size_t i( 0UL );

         for( ; (i+2UL) <= M; i+=2UL )
         {
            const size_t kbegin( ( IsUpper<MT4>::value )
                                 ?( ( IsLower<MT5>::value )
                                    ?( max( ( IsStrictlyUpper<MT4>::value ? i+1UL : i ), j ) )
                                    :( IsStrictlyUpper<MT4>::value ? i+1UL : i ) )
                                 :( IsLower<MT5>::value ? j : 0UL ) );
            const size_t kend( ( IsLower<MT4>::value )
                               ?( ( IsUpper<MT5>::value )
                                  ?( min( ( IsStrictlyLower<MT4>::value ? i+1UL : i+2UL ), j+SIMDSIZE*5UL, K ) )
                                  :( IsStrictlyLower<MT4>::value ? i+1UL : i+2UL ) )
                               :( IsUpper<MT5>::value ? min( j+SIMDSIZE*5UL, K ) : K ) );

            SIMDType xmm1, xmm2, xmm3, xmm4, xmm5, xmm6, xmm7, xmm8, xmm9, xmm10;

            for( size_t k=kbegin; k<kend; ++k ) {
               const SIMDType a1( set( A(i    ,k) ) );
               const SIMDType a2( set( A(i+1UL,k) ) );
               const SIMDType b1( B.load(k,j             ) );
               const SIMDType b2( B.load(k,j+SIMDSIZE    ) );
               const SIMDType b3( B.load(k,j+SIMDSIZE*2UL) );
               const SIMDType b4( B.load(k,j+SIMDSIZE*3UL) );
               const SIMDType b5( B.load(k,j+SIMDSIZE*4UL) );
               xmm1  += a1 * b1;
               xmm2  += a1 * b2;
               xmm3  += a1 * b3;
               xmm4  += a1 * b4;
               xmm5  += a1 * b5;
               xmm6  += a2 * b1;
               xmm7  += a2 * b2;
               xmm8  += a2 * b3;
               xmm9  += a2 * b4;
               xmm10 += a2 * b5;
            }

            (~C).store( i    , j             , (~C).load(i    ,j             ) - xmm1  * factor );
            (~C).store( i    , j+SIMDSIZE    , (~C).load(i    ,j+SIMDSIZE    ) - xmm2  * factor );
            (~C).store( i    , j+SIMDSIZE*2UL, (~C).load(i    ,j+SIMDSIZE*2UL) - xmm3  * factor );
            (~C).store( i    , j+SIMDSIZE*3UL, (~C).load(i    ,j+SIMDSIZE*3UL) - xmm4  * factor );
            (~C).store( i    , j+SIMDSIZE*4UL, (~C).load(i    ,j+SIMDSIZE*4UL) - xmm5  * factor );
            (~C).store( i+1UL, j             , (~C).load(i+1UL,j             ) - xmm6  * factor );
            (~C).store( i+1UL, j+SIMDSIZE    , (~C).load(i+1UL,j+SIMDSIZE    ) - xmm7  * factor );
            (~C).store( i+1UL, j+SIMDSIZE*2UL, (~C).load(i+1UL,j+SIMDSIZE*2UL) - xmm8  * factor );
            (~C).store( i+1UL, j+SIMDSIZE*3UL, (~C).load(i+1UL,j+SIMDSIZE*3UL) - xmm9  * factor );
            (~C).store( i+1UL, j+SIMDSIZE*4UL, (~C).load(i+1UL,j+SIMDSIZE*4UL) - xmm10 * factor );
         }

         if( i < M )
         {
            const size_t kbegin( ( IsUpper<MT4>::value )
                                 ?( ( IsLower<MT5>::value )
                                    ?( max( ( IsStrictlyUpper<MT4>::value ? i+1UL : i ), j ) )
                                    :( IsStrictlyUpper<MT4>::value ? i+1UL : i ) )
                                 :( IsLower<MT5>::value ? j : 0UL ) );
            const size_t kend( ( IsUpper<MT5>::value )?( min( j+SIMDSIZE*5UL, K ) ):( K ) );

            SIMDType xmm1, xmm2, xmm3, xmm4, xmm5;

            for( size_t k=kbegin; k<kend; ++k ) {
               const SIMDType a1( set( A(i,k) ) );
               xmm1 += a1 * B.load(k,j             );
               xmm2 += a1 * B.load(k,j+SIMDSIZE    );
               xmm3 += a1 * B.load(k,j+SIMDSIZE*2UL);
               xmm4 += a1 * B.load(k,j+SIMDSIZE*3UL);
               xmm5 += a1 * B.load(k,j+SIMDSIZE*4UL);
            }

            (~C).store( i, j             , (~C).load(i,j             ) - xmm1 * factor );
            (~C).store( i, j+SIMDSIZE    , (~C).load(i,j+SIMDSIZE    ) - xmm2 * factor );
            (~C).store( i, j+SIMDSIZE*2UL, (~C).load(i,j+SIMDSIZE*2UL) - xmm3 * factor );
            (~C).store( i, j+SIMDSIZE*3UL, (~C).load(i,j+SIMDSIZE*3UL) - xmm4 * factor );
            (~C).store( i, j+SIMDSIZE*4UL, (~C).load(i,j+SIMDSIZE*4UL) - xmm5 * factor );
         }
      }

      for( ; !LOW && !UPP && (j+SIMDSIZE*3UL) < jpos; j+=SIMDSIZE*4UL )
      {
         size_t i( 0UL );

         for( ; (i+2UL) <= M; i+=2UL )
         {
            const size_t kbegin( ( IsUpper<MT4>::value )
                                 ?( ( IsLower<MT5>::value )
                                    ?( max( ( IsStrictlyUpper<MT4>::value ? i+1UL : i ), j ) )
                                    :( IsStrictlyUpper<MT4>::value ? i+1UL : i ) )
                                 :( IsLower<MT5>::value ? j : 0UL ) );
            const size_t kend( ( IsLower<MT4>::value )
                               ?( ( IsUpper<MT5>::value )
                                  ?( min( ( IsStrictlyLower<MT4>::value ? i+1UL : i+2UL ), j+SIMDSIZE*4UL, K ) )
                                  :( IsStrictlyLower<MT4>::value ? i+1UL : i+2UL ) )
                               :( IsUpper<MT5>::value ? min( j+SIMDSIZE*4UL, K ) : K ) );

            SIMDType xmm1, xmm2, xmm3, xmm4, xmm5, xmm6, xmm7, xmm8;

            for( size_t k=kbegin; k<kend; ++k ) {
               const SIMDType a1( set( A(i    ,k) ) );
               const SIMDType a2( set( A(i+1UL,k) ) );
               const SIMDType b1( B.load(k,j             ) );
               const SIMDType b2( B.load(k,j+SIMDSIZE    ) );
               const SIMDType b3( B.load(k,j+SIMDSIZE*2UL) );
               const SIMDType b4( B.load(k,j+SIMDSIZE*3UL) );
               xmm1 += a1 * b1;
               xmm2 += a1 * b2;
               xmm3 += a1 * b3;
               xmm4 += a1 * b4;
               xmm5 += a2 * b1;
               xmm6 += a2 * b2;
               xmm7 += a2 * b3;
               xmm8 += a2 * b4;
            }

            (~C).store( i    , j             , (~C).load(i    ,j             ) - xmm1 * factor );
            (~C).store( i    , j+SIMDSIZE    , (~C).load(i    ,j+SIMDSIZE    ) - xmm2 * factor );
            (~C).store( i    , j+SIMDSIZE*2UL, (~C).load(i    ,j+SIMDSIZE*2UL) - xmm3 * factor );
            (~C).store( i    , j+SIMDSIZE*3UL, (~C).load(i    ,j+SIMDSIZE*3UL) - xmm4 * factor );
            (~C).store( i+1UL, j             , (~C).load(i+1UL,j             ) - xmm5 * factor );
            (~C).store( i+1UL, j+SIMDSIZE    , (~C).load(i+1UL,j+SIMDSIZE    ) - xmm6 * factor );
            (~C).store( i+1UL, j+SIMDSIZE*2UL, (~C).load(i+1UL,j+SIMDSIZE*2UL) - xmm7 * factor );
            (~C).store( i+1UL, j+SIMDSIZE*3UL, (~C).load(i+1UL,j+SIMDSIZE*3UL) - xmm8 * factor );
         }

         if( i < M )
         {
            const size_t kbegin( ( IsUpper<MT4>::value )
                                 ?( ( IsLower<MT5>::value )
                                    ?( max( ( IsStrictlyUpper<MT4>::value ? i+1UL : i ), j ) )
                                    :( IsStrictlyUpper<MT4>::value ? i+1UL : i ) )
                                 :( IsLower<MT5>::value ? j : 0UL ) );
            const size_t kend( ( IsUpper<MT5>::value )?( min( j+SIMDSIZE*4UL, K ) ):( K ) );

            SIMDType xmm1, xmm2, xmm3, xmm4;

            for( size_t k=kbegin; k<kend; ++k ) {
               const SIMDType a1( set( A(i,k) ) );
               xmm1 += a1 * B.load(k,j             );
               xmm2 += a1 * B.load(k,j+SIMDSIZE    );
               xmm3 += a1 * B.load(k,j+SIMDSIZE*2UL);
               xmm4 += a1 * B.load(k,j+SIMDSIZE*3UL);
            }

            (~C).store( i, j             , (~C).load(i,j             ) - xmm1 * factor );
            (~C).store( i, j+SIMDSIZE    , (~C).load(i,j+SIMDSIZE    ) - xmm2 * factor );
            (~C).store( i, j+SIMDSIZE*2UL, (~C).load(i,j+SIMDSIZE*2UL) - xmm3 * factor );
            (~C).store( i, j+SIMDSIZE*3UL, (~C).load(i,j+SIMDSIZE*3UL) - xmm4 * factor );
         }
      }

      for( ; !LOW && !UPP && (j+SIMDSIZE*2UL) < jpos; j+=SIMDSIZE*3UL )
      {
         size_t i( 0UL );

         for( ; (i+2UL) <= M; i+=2UL )
         {
            const size_t kbegin( ( IsUpper<MT4>::value )
                                 ?( ( IsLower<MT5>::value )
                                    ?( max( ( IsStrictlyUpper<MT4>::value ? i+1UL : i ), j ) )
                                    :( IsStrictlyUpper<MT4>::value ? i+1UL : i ) )
                                 :( IsLower<MT5>::value ? j : 0UL ) );
            const size_t kend( ( IsLower<MT4>::value )
                               ?( ( IsUpper<MT5>::value )
                                  ?( min( ( IsStrictlyLower<MT4>::value ? i+1UL : i+2UL ), j+SIMDSIZE*3UL, K ) )
                                  :( IsStrictlyLower<MT4>::value ? i+1UL : i+2UL ) )
                               :( IsUpper<MT5>::value ? min( j+SIMDSIZE*3UL, K ) : K ) );

            SIMDType xmm1, xmm2, xmm3, xmm4, xmm5, xmm6;

            for( size_t k=kbegin; k<kend; ++k ) {
               const SIMDType a1( set( A(i    ,k) ) );
               const SIMDType a2( set( A(i+1UL,k) ) );
               const SIMDType b1( B.load(k,j             ) );
               const SIMDType b2( B.load(k,j+SIMDSIZE    ) );
               const SIMDType b3( B.load(k,j+SIMDSIZE*2UL) );
               xmm1 += a1 * b1;
               xmm2 += a1 * b2;
               xmm3 += a1 * b3;
               xmm4 += a2 * b1;
               xmm5 += a2 * b2;
               xmm6 += a2 * b3;
            }

            (~C).store( i    , j             , (~C).load(i    ,j             ) - xmm1 * factor );
            (~C).store( i    , j+SIMDSIZE    , (~C).load(i    ,j+SIMDSIZE    ) - xmm2 * factor );
            (~C).store( i    , j+SIMDSIZE*2UL, (~C).load(i    ,j+SIMDSIZE*2UL) - xmm3 * factor );
            (~C).store( i+1UL, j             , (~C).load(i+1UL,j             ) - xmm4 * factor );
            (~C).store( i+1UL, j+SIMDSIZE    , (~C).load(i+1UL,j+SIMDSIZE    ) - xmm5 * factor );
            (~C).store( i+1UL, j+SIMDSIZE*2UL, (~C).load(i+1UL,j+SIMDSIZE*2UL) - xmm6 * factor );
         }

         if( i < M )
         {
            const size_t kbegin( ( IsUpper<MT4>::value )
                                 ?( ( IsLower<MT5>::value )
                                    ?( max( ( IsStrictlyUpper<MT4>::value ? i+1UL : i ), j ) )
                                    :( IsStrictlyUpper<MT4>::value ? i+1UL : i ) )
                                 :( IsLower<MT5>::value ? j : 0UL ) );
            const size_t kend( ( IsUpper<MT5>::value )?( min( j+SIMDSIZE*3UL, K ) ):( K ) );

            SIMDType xmm1, xmm2, xmm3;

            for( size_t k=kbegin; k<kend; ++k ) {
               const SIMDType a1( set( A(i,k) ) );
               xmm1 += a1 * B.load(k,j             );
               xmm2 += a1 * B.load(k,j+SIMDSIZE    );
               xmm3 += a1 * B.load(k,j+SIMDSIZE*2UL);
            }

            (~C).store( i, j             , (~C).load(i,j             ) - xmm1 * factor );
            (~C).store( i, j+SIMDSIZE    , (~C).load(i,j+SIMDSIZE    ) - xmm2 * factor );
            (~C).store( i, j+SIMDSIZE*2UL, (~C).load(i,j+SIMDSIZE*2UL) - xmm3 * factor );
         }
      }

      for( ; !( LOW && UPP ) && (j+SIMDSIZE) < jpos; j+=SIMDSIZE*2UL )
      {
         const size_t iend( UPP ? min(j+SIMDSIZE*2UL,M) : M );
         size_t i( LOW ? j : 0UL );

         for( ; (i+2UL) <= iend; i+=2UL )
         {
            const size_t kbegin( ( IsUpper<MT4>::value )
                                 ?( ( IsLower<MT5>::value )
                                    ?( max( ( IsStrictlyUpper<MT4>::value ? i+1UL : i ), j ) )
                                    :( IsStrictlyUpper<MT4>::value ? i+1UL : i ) )
                                 :( IsLower<MT5>::value ? j : 0UL ) );
            const size_t kend( ( IsLower<MT4>::value )
                               ?( ( IsUpper<MT5>::value )
                                  ?( min( ( IsStrictlyLower<MT4>::value ? i+1UL : i+2UL ), j+SIMDSIZE*2UL, K ) )
                                  :( IsStrictlyLower<MT4>::value ? i+1UL : i+2UL ) )
                               :( IsUpper<MT5>::value ? min( j+SIMDSIZE*2UL, K ) : K ) );

            SIMDType xmm1, xmm2, xmm3, xmm4;

            for( size_t k=kbegin; k<kend; ++k ) {
               const SIMDType a1( set( A(i    ,k) ) );
               const SIMDType a2( set( A(i+1UL,k) ) );
               const SIMDType b1( B.load(k,j         ) );
               const SIMDType b2( B.load(k,j+SIMDSIZE) );
               xmm1 += a1 * b1;
               xmm2 += a1 * b2;
               xmm3 += a2 * b1;
               xmm4 += a2 * b2;
            }

            (~C).store( i    , j         , (~C).load(i    ,j         ) - xmm1 * factor );
            (~C).store( i    , j+SIMDSIZE, (~C).load(i    ,j+SIMDSIZE) - xmm2 * factor );
            (~C).store( i+1UL, j         , (~C).load(i+1UL,j         ) - xmm3 * factor );
            (~C).store( i+1UL, j+SIMDSIZE, (~C).load(i+1UL,j+SIMDSIZE) - xmm4 * factor );
         }

         if( i < iend )
         {
            const size_t kbegin( ( IsUpper<MT4>::value )
                                 ?( ( IsLower<MT5>::value )
                                    ?( max( ( IsStrictlyUpper<MT4>::value ? i+1UL : i ), j ) )
                                    :( IsStrictlyUpper<MT4>::value ? i+1UL : i ) )
                                 :( IsLower<MT5>::value ? j : 0UL ) );
            const size_t kend( ( IsUpper<MT5>::value )?( min( j+SIMDSIZE*2UL, K ) ):( K ) );

            SIMDType xmm1, xmm2;

            for( size_t k=kbegin; k<kend; ++k ) {
               const SIMDType a1( set( A(i,k) ) );
               xmm1 += a1 * B.load(k,j         );
               xmm2 += a1 * B.load(k,j+SIMDSIZE);
            }

            (~C).store( i, j         , (~C).load(i,j         ) - xmm1 * factor );
            (~C).store( i, j+SIMDSIZE, (~C).load(i,j+SIMDSIZE) - xmm2 * factor );
         }
      }

      for( ; j<jpos; j+=SIMDSIZE )
      {
         const size_t iend( LOW && UPP ? min(j+SIMDSIZE,M) : M );
         size_t i( LOW ? j : 0UL );

         for( ; (i+2UL) <= iend; i+=2UL )
         {
            const size_t kbegin( ( IsUpper<MT4>::value )
                                 ?( ( IsLower<MT5>::value )
                                    ?( max( ( IsStrictlyUpper<MT4>::value ? i+1UL : i ), j ) )
                                    :( IsStrictlyUpper<MT4>::value ? i+1UL : i ) )
                                 :( IsLower<MT5>::value ? j : 0UL ) );
            const size_t kend( ( IsLower<MT4>::value )
                               ?( IsStrictlyLower<MT4>::value ? i+1UL : i+2UL )
                               :( K ) );

            SIMDType xmm1, xmm2;

            for( size_t k=kbegin; k<kend; ++k ) {
               const SIMDType b1( B.load(k,j) );
               xmm1 += set( A(i    ,k) ) * b1;
               xmm2 += set( A(i+1UL,k) ) * b1;
            }

            (~C).store( i    , j, (~C).load(i    ,j) - xmm1 * factor );
            (~C).store( i+1UL, j, (~C).load(i+1UL,j) - xmm2 * factor );
         }

         if( i < iend )
         {
            const size_t kbegin( ( IsUpper<MT4>::value )
                                 ?( ( IsLower<MT5>::value )
                                    ?( max( ( IsStrictlyUpper<MT4>::value ? i+1UL : i ), j ) )
                                    :( IsStrictlyUpper<MT4>::value ? i+1UL : i ) )
                                 :( IsLower<MT5>::value ? j : 0UL ) );

            SIMDType xmm1;

            for( size_t k=kbegin; k<K; ++k ) {
               xmm1 += set( A(i,k) ) * B.load(k,j);
            }

            (~C).store( i, j, (~C).load(i,j) - xmm1 * factor );
         }
      }

      for( ; remainder && j<N; ++j )
      {
         const size_t iend( UPP ? j+1UL : M );
         size_t i( LOW ? j : 0UL );

         for( ; (i+2UL) <= iend; i+=2UL )
         {
            const size_t kbegin( ( IsUpper<MT4>::value )
                                 ?( ( IsLower<MT5>::value )
                                    ?( max( ( IsStrictlyUpper<MT4>::value ? i+1UL : i ), j ) )
                                    :( IsStrictlyUpper<MT4>::value ? i+1UL : i ) )
                                 :( IsLower<MT5>::value ? j : 0UL ) );
            const size_t kend( ( IsLower<MT4>::value )
                               ?( IsStrictlyLower<MT4>::value ? i+1UL : i+2UL )
                               :( K ) );

            ElementType value1 = ElementType();
            ElementType value2 = ElementType();

            for( size_t k=kbegin; k<kend; ++k ) {
               value1 += A(i    ,k) * B(k,j);
               value2 += A(i+1UL,k) * B(k,j);
            }

            (~C)(i    ,j) -= value1 * scalar;
            (~C)(i+1UL,j) -= value2 * scalar;
         }

         if( i < iend )
         {
            const size_t kbegin( ( IsUpper<MT4>::value )
                                 ?( ( IsLower<MT5>::value )
                                    ?( max( ( IsStrictlyUpper<MT4>::value ? i+1UL : i ), j ) )
                                    :( IsStrictlyUpper<MT4>::value ? i+1UL : i ) )
                                 :( IsLower<MT5>::value ? j : 0UL ) );

            ElementType value = ElementType();

            for( size_t k=kbegin; k<K; ++k ) {
               value += A(i,k) * B(k,j);
            }

            (~C)(i,j) -= value * scalar;
         }
      }
   }
   //**********************************************************************************************

   //**Vectorized default subtraction assignment to column-major dense matrices (small matrices)***
   /*!\brief Vectorized default subtraction assignment of a small scaled dense matrix-dense matrix
   //        multiplication (\f$ C-=s*A*B \f$).
   // \ingroup dense_matrix
   //
   // \param C The target left-hand side dense matrix.
   // \param A The left-hand side multiplication operand.
   // \param B The right-hand side multiplication operand.
   // \param scalar The scaling factor.
   // \return void
   //
   // This function implements the vectorized default subtraction assignment of a small scaled
   // dense matrix-dense matrix multiplication expression to a column-major dense matrix.
   */
   template< typename MT3    // Type of the left-hand side target matrix
           , typename MT4    // Type of the left-hand side matrix operand
           , typename MT5    // Type of the right-hand side matrix operand
           , typename ST2 >  // Type of the scalar value
   static inline EnableIf_< UseVectorizedDefaultKernel<MT3,MT4,MT5,ST2> >
      selectSmallSubAssignKernel( DenseMatrix<MT3,true>& C, const MT4& A, const MT5& B, ST2 scalar )
   {
      BLAZE_CONSTRAINT_MUST_NOT_BE_SYMMETRIC_MATRIX_TYPE( MT4 );
      BLAZE_CONSTRAINT_MUST_NOT_BE_SYMMETRIC_MATRIX_TYPE( MT5 );
      BLAZE_CONSTRAINT_MUST_BE_COLUMN_MAJOR_MATRIX_TYPE( OppositeType_<MT4> );
      BLAZE_CONSTRAINT_MUST_BE_COLUMN_MAJOR_MATRIX_TYPE( OppositeType_<MT5> );

      const ForwardFunctor fwd;

      if( !IsResizable<MT4>::value && IsResizable<MT5>::value ) {
         const OppositeType_<MT4> tmp( serial( A ) );
         subAssign( ~C, fwd( tmp * B ) * scalar );
      }
      else if( IsResizable<MT4>::value && !IsResizable<MT5>::value ) {
         const OppositeType_<MT5> tmp( serial( B ) );
         subAssign( ~C, fwd( A * tmp ) * scalar );
      }
      else if( A.rows() * A.columns() <= B.rows() * B.columns() ) {
         const OppositeType_<MT4> tmp( serial( A ) );
         subAssign( ~C, fwd( tmp * B ) * scalar );
      }
      else {
         const OppositeType_<MT5> tmp( serial( B ) );
         subAssign( ~C, fwd( A * tmp ) * scalar );
      }
   }
   //**********************************************************************************************

   //**Default subtraction assignment to dense matrices (large matrices)***************************
   /*!\brief Default subtraction assignment of a large scaled dense matrix-dense matrix
   //        multiplication (\f$ C-=s*A*B \f$).
   // \ingroup dense_matrix
   //
   // \param C The target left-hand side dense matrix.
   // \param A The left-hand side multiplication operand.
   // \param B The right-hand side multiplication operand.
   // \param scalar The scaling factor.
   // \return void
   //
   // This function relays to the default implementation of the subtraction assignment of a scaled
   // dense matrix-dense matrix multiplication expression to a dense matrix.
   */
   template< typename MT3    // Type of the left-hand side target matrix
           , typename MT4    // Type of the left-hand side matrix operand
           , typename MT5    // Type of the right-hand side matrix operand
           , typename ST2 >  // Type of the scalar value
   static inline DisableIf_< UseVectorizedDefaultKernel<MT3,MT4,MT5,ST2> >
      selectLargeSubAssignKernel( MT3& C, const MT4& A, const MT5& B, ST2 scalar )
   {
      selectDefaultSubAssignKernel( C, A, B, scalar );
   }
   //**********************************************************************************************

   //**Vectorized default subtraction assignment to dense matrices (large matrices)****************
   /*!\brief Vectorized default subtraction assignment of a large scaled dense matrix-dense matrix
   //        multiplication (\f$ C-=s*A*B \f$).
   // \ingroup dense_matrix
   //
   // \param C The target left-hand side dense matrix.
   // \param A The left-hand side multiplication operand.
   // \param B The right-hand side multiplication operand.
   // \param scalar The scaling factor.
   // \return void
   //
   // This function implements the vectorized default subtraction assignment of a scaled dense
   // matrix-dense matrix multiplication expression to a dense matrix. This kernel is optimized
   // for large matrices.
   */
   template< typename MT3    // Type of the left-hand side target matrix
           , typename MT4    // Type of the left-hand side matrix operand
           , typename MT5    // Type of the right-hand side matrix operand
           , typename ST2 >  // Type of the scalar value
   static inline EnableIf_< UseVectorizedDefaultKernel<MT3,MT4,MT5,ST2> >
      selectLargeSubAssignKernel( MT3& C, const MT4& A, const MT5& B, ST2 scalar )
   {
      if( LOW )
         lmmm( C, A, B, -scalar, ST2(1) );
      else if( UPP )
         ummm( C, A, B, -scalar, ST2(1) );
      else
         mmm( C, A, B, -scalar, ST2(1) );
   }
   //**********************************************************************************************

   //**BLAS-based subtraction assignment to dense matrices (default)*******************************
   /*!\brief Default subtraction assignment of a scaled dense matrix-dense matrix multiplication
   //        (\f$ C-=s*A*B \f$).
   // \ingroup dense_matrix
   //
   // \param C The target left-hand side dense matrix.
   // \param A The left-hand side multiplication operand.
   // \param B The right-hand side multiplication operand.
   // \param scalar The scaling factor.
   // \return void
   //
   // This function relays to the default implementation of the subtraction assignment of a large
   // scaled dense matrix-dense matrix multiplication expression to a dense matrix.
   */
   template< typename MT3    // Type of the left-hand side target matrix
           , typename MT4    // Type of the left-hand side matrix operand
           , typename MT5    // Type of the right-hand side matrix operand
           , typename ST2 >  // Type of the scalar value
   static inline DisableIf_< UseBlasKernel<MT3,MT4,MT5,ST2> >
      selectBlasSubAssignKernel( MT3& C, const MT4& A, const MT5& B, ST2 scalar )
   {
      selectLargeSubAssignKernel( C, A, B, scalar );
   }
   //**********************************************************************************************

   //**BLAS-based subraction assignment to dense matrices******************************************
#if BLAZE_BLAS_MODE && BLAZE_USE_BLAS_MATRIX_MATRIX_MULTIPLICATION
   /*!\brief BLAS-based subraction assignment of a scaled dense matrix-dense matrix multiplication
   //        (\f$ C-=s*A*B \f$).
   // \ingroup dense_matrix
   //
   // \param C The target left-hand side dense matrix.
   // \param A The left-hand side multiplication operand.
   // \param B The right-hand side multiplication operand.
   // \param scalar The scaling factor.
   // \return void
   //
   // This function performs the scaled dense matrix-dense matrix multiplication based on the
   // according BLAS functionality.
   */
   template< typename MT3    // Type of the left-hand side target matrix
           , typename MT4    // Type of the left-hand side matrix operand
           , typename MT5    // Type of the right-hand side matrix operand
           , typename ST2 >  // Type of the scalar value
   static inline EnableIf_< UseBlasKernel<MT3,MT4,MT5,ST2> >
      selectBlasSubAssignKernel( MT3& C, const MT4& A, const MT5& B, ST2 scalar )
   {
      typedef ElementType_<MT3>  ET;

      if( IsTriangular<MT4>::value ) {
         ResultType_<MT3> tmp( serial( B ) );
         trmm( tmp, A, CblasLeft, ( IsLower<MT4>::value )?( CblasLower ):( CblasUpper ), ET(scalar) );
         subAssign( C, tmp );
      }
      else if( IsTriangular<MT5>::value ) {
         ResultType_<MT3> tmp( serial( A ) );
         trmm( tmp, B, CblasRight, ( IsLower<MT5>::value )?( CblasLower ):( CblasUpper ), ET(scalar) );
         subAssign( C, tmp );
      }
      else {
         gemm( C, A, B, ET(-scalar), ET(1) );
      }
   }
#endif
   //**********************************************************************************************

   //**Restructuring subtraction assignment to column-major matrices*******************************
   /*!\brief Restructuring subtraction assignment of a scaled dense matrix-dense matrix
   //        multiplication to a column-major matrix (\f$ C-=s*A*B \f$).
   // \ingroup dense_matrix
   //
   // \param lhs The target left-hand side matrix.
   // \param rhs The right-hand side scaled multiplication expression to be subtracted.
   // \return void
   //
   // This function implements the symmetry-based restructuring subtraction assignment of a scaled
   // dense matrix-dense matrix multiplication expression to a column-major matrix. Due to the
   // explicit application of the SFINAE principle this function can only be selected by the
   // compiler in case the symmetry of either of the two matrix operands can be exploited.
   */
   template< typename MT >  // Type of the target matrix
   friend inline EnableIf_< CanExploitSymmetry<MT,MT1,MT2> >
      subAssign( Matrix<MT,true>& lhs, const DMatScalarMultExpr& rhs )
   {
      BLAZE_FUNCTION_TRACE;

      BLAZE_CONSTRAINT_MUST_NOT_BE_SYMMETRIC_MATRIX_TYPE( MT );

      BLAZE_INTERNAL_ASSERT( (~lhs).rows()    == rhs.rows()   , "Invalid number of rows"    );
      BLAZE_INTERNAL_ASSERT( (~lhs).columns() == rhs.columns(), "Invalid number of columns" );

      const ForwardFunctor fwd;

      LeftOperand_<MMM>  left ( rhs.matrix_.leftOperand()  );
      RightOperand_<MMM> right( rhs.matrix_.rightOperand() );

      if( IsSymmetric<MT1>::value && IsSymmetric<MT2>::value )
         subAssign( ~lhs, fwd( trans( left ) * trans( right ) ) * rhs.scalar_ );
      else if( IsSymmetric<MT1>::value )
         subAssign( ~lhs, fwd( trans( left ) * right ) * rhs.scalar_ );
      else
         subAssign( ~lhs, fwd( left * trans( right ) ) * rhs.scalar_ );
   }
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
   /*!\brief SMP assignment of a scaled dense matrix-dense matrix multiplication to a dense matrix
   //        (\f$ C=s*A*B \f$).
   // \ingroup dense_matrix
   //
   // \param lhs The target left-hand side dense matrix.
   // \param rhs The right-hand side scaled multiplication expression to be assigned.
   // \return void
   //
   // This function implements the performance optimized assignment of a scaled dense matrix-
   // dense matrix multiplication expression to a dense matrix. Due to the explicit application
   // of the SFINAE principle this function can only be selected by the compiler in case either
   // of the two matrix operands requires an intermediate evaluation and no symmetry can be
   // exploited.
   */
   template< typename MT  // Type of the target dense matrix
           , bool SO >    // Storage order of the target dense matrix
   friend inline EnableIf_< IsEvaluationRequired<MT,MT1,MT2> >
      smpAssign( DenseMatrix<MT,SO>& lhs, const DMatScalarMultExpr& rhs )
   {
      BLAZE_FUNCTION_TRACE;

      BLAZE_INTERNAL_ASSERT( (~lhs).rows()    == rhs.rows()   , "Invalid number of rows"    );
      BLAZE_INTERNAL_ASSERT( (~lhs).columns() == rhs.columns(), "Invalid number of columns" );

      LeftOperand_<MMM>  left ( rhs.matrix_.leftOperand()  );
      RightOperand_<MMM> right( rhs.matrix_.rightOperand() );

      if( (~lhs).rows() == 0UL || (~lhs).columns() == 0UL ) {
         return;
      }
      else if( left.columns() == 0UL ) {
         reset( ~lhs );
         return;
      }

      LT A( left  );  // Evaluation of the left-hand side dense matrix operand
      RT B( right );  // Evaluation of the right-hand side dense matrix operand

      BLAZE_INTERNAL_ASSERT( A.rows()    == left.rows()     , "Invalid number of rows"    );
      BLAZE_INTERNAL_ASSERT( A.columns() == left.columns()  , "Invalid number of columns" );
      BLAZE_INTERNAL_ASSERT( B.rows()    == right.rows()    , "Invalid number of rows"    );
      BLAZE_INTERNAL_ASSERT( B.columns() == right.columns() , "Invalid number of columns" );
      BLAZE_INTERNAL_ASSERT( A.rows()    == (~lhs).rows()   , "Invalid number of rows"    );
      BLAZE_INTERNAL_ASSERT( B.columns() == (~lhs).columns(), "Invalid number of columns" );

      smpAssign( ~lhs, A * B * rhs.scalar_ );
   }
   //**********************************************************************************************

   //**SMP assignment to sparse matrices***********************************************************
   /*!\brief SMP assignment of a scaled dense matrix-dense matrix multiplication to a sparse matrix
   //        (\f$ C=s*A*B \f$).
   // \ingroup dense_matrix
   //
   // \param lhs The target left-hand side sparse matrix.
   // \param rhs The right-hand side scaled multiplication expression to be assigned.
   // \return void
   //
   // This function implements the performance optimized SMP assignment of a scaled dense matrix-
   // dense matrix multiplication expression to a sparse matrix. Due to the explicit application
   // of the SFINAE principle this function can only be selected by the compiler in case either
   // of the two matrix operands requires an intermediate evaluation and no symmetry can be
   // exploited.
   */
   template< typename MT  // Type of the target sparse matrix
           , bool SO >    // Storage order of the target sparse matrix
   friend inline EnableIf_< IsEvaluationRequired<MT,MT1,MT2> >
      smpAssign( SparseMatrix<MT,SO>& lhs, const DMatScalarMultExpr& rhs )
   {
      BLAZE_FUNCTION_TRACE;

      typedef IfTrue_< SO, OppositeType, ResultType >  TmpType;

      BLAZE_CONSTRAINT_MUST_BE_DENSE_MATRIX_TYPE( ResultType );
      BLAZE_CONSTRAINT_MUST_BE_DENSE_MATRIX_TYPE( OppositeType );
      BLAZE_CONSTRAINT_MUST_BE_ROW_MAJOR_MATRIX_TYPE( ResultType );
      BLAZE_CONSTRAINT_MUST_BE_COLUMN_MAJOR_MATRIX_TYPE( OppositeType );
      BLAZE_CONSTRAINT_MATRICES_MUST_HAVE_SAME_STORAGE_ORDER( MT, TmpType );
      BLAZE_CONSTRAINT_MUST_BE_REFERENCE_TYPE( CompositeType_<TmpType> );

      BLAZE_INTERNAL_ASSERT( (~lhs).rows()    == rhs.rows()   , "Invalid number of rows"    );
      BLAZE_INTERNAL_ASSERT( (~lhs).columns() == rhs.columns(), "Invalid number of columns" );

      const ForwardFunctor fwd;

      const TmpType tmp( rhs );
      smpAssign( ~lhs, fwd( tmp ) );
   }
   //**********************************************************************************************

   //**Restructuring SMP assignment to column-major matrices***************************************
   /*!\brief Restructuring SMP assignment of a scaled dense matrix-dense matrix multiplication to
   //        a column-major matrix (\f$ C=s*A*B \f$).
   // \ingroup dense_matrix
   //
   // \param lhs The target left-hand side matrix.
   // \param rhs The right-hand side scaled multiplication expression to be assigned.
   // \return void
   //
   // This function implements the symmetry-based restructuring SMP assignment of a scaled dense
   // matrix-dense matrix multiplication expression to a column-major matrix. Due to the
   // explicit application of the SFINAE principle this function can only be selected by
   // the compiler in case the symmetry of either of the two matrix operands can be exploited.
   */
   template< typename MT >  // Type of the target matrix
   friend inline EnableIf_< CanExploitSymmetry<MT,MT1,MT2> >
      smpAssign( Matrix<MT,true>& lhs, const DMatScalarMultExpr& rhs )
   {
      BLAZE_FUNCTION_TRACE;

      BLAZE_CONSTRAINT_MUST_NOT_BE_SYMMETRIC_MATRIX_TYPE( MT );

      BLAZE_INTERNAL_ASSERT( (~lhs).rows()    == rhs.rows()   , "Invalid number of rows"    );
      BLAZE_INTERNAL_ASSERT( (~lhs).columns() == rhs.columns(), "Invalid number of columns" );

      const ForwardFunctor fwd;

      LeftOperand_<MMM>  left ( rhs.matrix_.leftOperand()  );
      RightOperand_<MMM> right( rhs.matrix_.rightOperand() );

      if( IsSymmetric<MT1>::value && IsSymmetric<MT2>::value )
         smpAssign( ~lhs, fwd( trans( left ) * trans( right ) ) * rhs.scalar_ );
      else if( IsSymmetric<MT1>::value )
         smpAssign( ~lhs, fwd( trans( left ) * right ) * rhs.scalar_ );
      else
         smpAssign( ~lhs, fwd( left * trans( right ) ) * rhs.scalar_ );
   }
   //**********************************************************************************************

   //**SMP addition assignment to dense matrices***************************************************
   /*!\brief SMP addition assignment of a scaled dense matrix-dense matrix multiplication to a
   //        dense matrix (\f$ C+=s*A*B \f$).
   // \ingroup dense_matrix
   //
   // \param lhs The target left-hand side dense matrix.
   // \param rhs The right-hand side scaled multiplication expression to be added.
   // \return void
   //
   // This function implements the performance optimized addition assignment of a scaled dense
   // matrix-dense matrix multiplication expression to a dense matrix. Due to the explicit
   // application of the SFINAE principle this function can only be selected by the compiler
   // in case either of the two matrix operands requires an intermediate evaluation and no
   // symmetry can be exploited.
   */
   template< typename MT  // Type of the target dense matrix
           , bool SO >    // Storage order of the target dense matrix
   friend inline EnableIf_< IsEvaluationRequired<MT,MT1,MT2> >
      smpAddAssign( DenseMatrix<MT,SO>& lhs, const DMatScalarMultExpr& rhs )
   {
      BLAZE_FUNCTION_TRACE;

      BLAZE_INTERNAL_ASSERT( (~lhs).rows()    == rhs.rows()   , "Invalid number of rows"    );
      BLAZE_INTERNAL_ASSERT( (~lhs).columns() == rhs.columns(), "Invalid number of columns" );

      LeftOperand_<MMM>  left ( rhs.matrix_.leftOperand()  );
      RightOperand_<MMM> right( rhs.matrix_.rightOperand() );

      if( (~lhs).rows() == 0UL || (~lhs).columns() == 0UL || left.columns() == 0UL ) {
         return;
      }

      LT A( left  );  // Evaluation of the left-hand side dense matrix operand
      RT B( right );  // Evaluation of the right-hand side dense matrix operand

      BLAZE_INTERNAL_ASSERT( A.rows()    == left.rows()     , "Invalid number of rows"    );
      BLAZE_INTERNAL_ASSERT( A.columns() == left.columns()  , "Invalid number of columns" );
      BLAZE_INTERNAL_ASSERT( B.rows()    == right.rows()    , "Invalid number of rows"    );
      BLAZE_INTERNAL_ASSERT( B.columns() == right.columns() , "Invalid number of columns" );
      BLAZE_INTERNAL_ASSERT( A.rows()    == (~lhs).rows()   , "Invalid number of rows"    );
      BLAZE_INTERNAL_ASSERT( B.columns() == (~lhs).columns(), "Invalid number of columns" );

      smpAddAssign( ~lhs, A * B * rhs.scalar_ );
   }
   //**********************************************************************************************

   //**Restructuring SMP addition assignment to column-major matrices******************************
   /*!\brief Restructuring SMP addition assignment of a scaled dense matrix-dense matrix
   //        multiplication to a column-major matrix (\f$ C+=s*A*B \f$).
   // \ingroup dense_matrix
   //
   // \param lhs The target left-hand side matrix.
   // \param rhs The right-hand side scaled multiplication expression to be added.
   // \return void
   //
   // This function implements the symmetry-based restructuring SMP addition assignment of a scaled
   // dense matrix-dense matrix multiplication expression to a column-major matrix. Due to the
   // explicit application of the SFINAE principle this function can only be selected by the
   // compiler in case the symmetry of either of the two matrix operands can be exploited.
   */
   template< typename MT >  // Type of the target matrix
   friend inline EnableIf_< CanExploitSymmetry<MT,MT1,MT2> >
      smpAddAssign( Matrix<MT,true>& lhs, const DMatScalarMultExpr& rhs )
   {
      BLAZE_FUNCTION_TRACE;

      BLAZE_CONSTRAINT_MUST_NOT_BE_SYMMETRIC_MATRIX_TYPE( MT );

      BLAZE_INTERNAL_ASSERT( (~lhs).rows()    == rhs.rows()   , "Invalid number of rows"    );
      BLAZE_INTERNAL_ASSERT( (~lhs).columns() == rhs.columns(), "Invalid number of columns" );

      const ForwardFunctor fwd;

      LeftOperand_<MMM>  left ( rhs.matrix_.leftOperand()  );
      RightOperand_<MMM> right( rhs.matrix_.rightOperand() );

      if( IsSymmetric<MT1>::value && IsSymmetric<MT2>::value )
         smpAddAssign( ~lhs, fwd( trans( left ) * trans( right ) ) * rhs.scalar_ );
      else if( IsSymmetric<MT1>::value )
         smpAddAssign( ~lhs, fwd( trans( left ) * right ) * rhs.scalar_ );
      else
         smpAddAssign( ~lhs, fwd( left * trans( right ) ) * rhs.scalar_ );
   }
   //**********************************************************************************************

   //**SMP addition assignment to sparse matrices**************************************************
   // No special implementation for the SMP addition assignment to sparse matrices.
   //**********************************************************************************************

   //**SMP subtraction assignment to dense matrices************************************************
   /*!\brief SMP subtraction assignment of a scaled dense matrix-dense matrix multiplication to a
   //        dense matrix (\f$ C-=s*A*B \f$).
   // \ingroup dense_matrix
   //
   // \param lhs The target left-hand side dense matrix.
   // \param rhs The right-hand side scaled multiplication expression to be subtracted.
   // \return void
   //
   // This function implements the performance optimized SMP subtraction assignment of a scaled
   // dense matrix-dense matrix multiplication expression to a dense matrix. Due to the explicit
   // application of the SFINAE principle this function can only be selected by the compiler
   // in case either of the two matrix operands requires an intermediate evaluation and no
   // symmetry can be exploited.
   */
   template< typename MT  // Type of the target dense matrix
           , bool SO >    // Storage order of the target dense matrix
   friend inline EnableIf_< IsEvaluationRequired<MT,MT1,MT2> >
      smpSubAssign( DenseMatrix<MT,SO>& lhs, const DMatScalarMultExpr& rhs )
   {
      BLAZE_FUNCTION_TRACE;

      BLAZE_INTERNAL_ASSERT( (~lhs).rows()    == rhs.rows()   , "Invalid number of rows"    );
      BLAZE_INTERNAL_ASSERT( (~lhs).columns() == rhs.columns(), "Invalid number of columns" );

      LeftOperand_<MMM>  left ( rhs.matrix_.leftOperand()  );
      RightOperand_<MMM> right( rhs.matrix_.rightOperand() );

      if( (~lhs).rows() == 0UL || (~lhs).columns() == 0UL || left.columns() == 0UL ) {
         return;
      }

      LT A( left  );  // Evaluation of the left-hand side dense matrix operand
      RT B( right );  // Evaluation of the right-hand side dense matrix operand

      BLAZE_INTERNAL_ASSERT( A.rows()    == left.rows()     , "Invalid number of rows"    );
      BLAZE_INTERNAL_ASSERT( A.columns() == left.columns()  , "Invalid number of columns" );
      BLAZE_INTERNAL_ASSERT( B.rows()    == right.rows()    , "Invalid number of rows"    );
      BLAZE_INTERNAL_ASSERT( B.columns() == right.columns() , "Invalid number of columns" );
      BLAZE_INTERNAL_ASSERT( A.rows()    == (~lhs).rows()   , "Invalid number of rows"    );
      BLAZE_INTERNAL_ASSERT( B.columns() == (~lhs).columns(), "Invalid number of columns" );

      smpSubAssign( ~lhs, A * B * rhs.scalar_ );
   }
   //**********************************************************************************************

   //**Restructuring SMP subtraction assignment to column-major matrices***************************
   /*!\brief Restructuring SMP subtraction assignment of a scaled dense matrix-dense matrix
   //        multiplication to a column-major matrix (\f$ C-=s*A*B \f$).
   // \ingroup dense_matrix
   //
   // \param lhs The target left-hand side matrix.
   // \param rhs The right-hand side scaled multiplication expression to be subtracted.
   // \return void
   //
   // This function implements the symmetry-based restructuring SMP subtraction assignment of a
   // scaled dense matrix-dense matrix multiplication expression to a column-major matrix. Due
   // to the explicit application of the SFINAE principle this function can only be selected by
   // the compiler in case the symmetry of either of the two matrix operands can be exploited.
   */
   template< typename MT >  // Type of the target matrix
   friend inline EnableIf_< CanExploitSymmetry<MT,MT1,MT2> >
      smpSubAssign( Matrix<MT,true>& lhs, const DMatScalarMultExpr& rhs )
   {
      BLAZE_FUNCTION_TRACE;

      BLAZE_CONSTRAINT_MUST_NOT_BE_SYMMETRIC_MATRIX_TYPE( MT );

      BLAZE_INTERNAL_ASSERT( (~lhs).rows()    == rhs.rows()   , "Invalid number of rows"    );
      BLAZE_INTERNAL_ASSERT( (~lhs).columns() == rhs.columns(), "Invalid number of columns" );

      const ForwardFunctor fwd;

      LeftOperand_<MMM>  left ( rhs.matrix_.leftOperand()  );
      RightOperand_<MMM> right( rhs.matrix_.rightOperand() );

      if( IsSymmetric<MT1>::value && IsSymmetric<MT2>::value )
         smpSubAssign( ~lhs, fwd( trans( left ) * trans( right ) ) * rhs.scalar_ );
      else if( IsSymmetric<MT1>::value )
         smpSubAssign( ~lhs, fwd( trans( left ) * right ) * rhs.scalar_ );
      else
         smpSubAssign( ~lhs, fwd( left * trans( right ) ) * rhs.scalar_ );
   }
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
   BLAZE_CONSTRAINT_MUST_BE_DENSE_MATRIX_TYPE( MMM );
   BLAZE_CONSTRAINT_MUST_BE_ROW_MAJOR_MATRIX_TYPE( MMM );
   BLAZE_CONSTRAINT_MUST_BE_DENSE_MATRIX_TYPE( MT1 );
   BLAZE_CONSTRAINT_MUST_BE_ROW_MAJOR_MATRIX_TYPE( MT1 );
   BLAZE_CONSTRAINT_MUST_BE_DENSE_MATRIX_TYPE( MT2 );
   BLAZE_CONSTRAINT_MUST_BE_ROW_MAJOR_MATRIX_TYPE( MT2 );
   BLAZE_CONSTRAINT_MUST_BE_NUMERIC_TYPE( ST );
   BLAZE_CONSTRAINT_MUST_BE_SAME_TYPE( ST, RightOperand );
   //**********************************************************************************************
};
/*! \endcond */
//*************************************************************************************************




//=================================================================================================
//
//  GLOBAL BINARY ARITHMETIC OPERATORS
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Multiplication operator for the multiplication of two row-major dense matrices
//        (\f$ A=B*C \f$).
// \ingroup dense_matrix
//
// \param lhs The left-hand side matrix for the multiplication.
// \param rhs The right-hand side matrix for the multiplication.
// \return The resulting matrix.
// \exception std::invalid_argument Matrix sizes do not match.
//
// This operator represents the multiplication of two row-major dense matrices:

   \code
   using blaze::rowMajor;

   blaze::DynamicMatrix<double,rowMajor> A, B, C;
   // ... Resizing and initialization
   C = A * B;
   \endcode

// The operator returns an expression representing a dense matrix of the higher-order element
// type of the two involved matrix element types \a T1::ElementType and \a T2::ElementType.
// Both matrix types \a T1 and \a T2 as well as the two element types \a T1::ElementType and
// \a T2::ElementType have to be supported by the MultTrait class template.\n
// In case the current number of columns of \a lhs and the current number of rows of \a rhs
// don't match, a \a std::invalid_argument is thrown.
*/
template< typename T1    // Type of the left-hand side dense matrix
        , typename T2 >  // Type of the right-hand side dense matrix
inline const DMatDMatMultExpr<T1,T2,false,false,false,false>
   operator*( const DenseMatrix<T1,false>& lhs, const DenseMatrix<T2,false>& rhs )
{
   BLAZE_FUNCTION_TRACE;

   if( (~lhs).columns() != (~rhs).rows() ) {
      BLAZE_THROW_INVALID_ARGUMENT( "Matrix sizes do not match" );
   }

   return DMatDMatMultExpr<T1,T2,false,false,false,false>( ~lhs, ~rhs );
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
   using blaze::rowMajor;

   blaze::DynamicMatrix<double,rowMajor> A, B, C;
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
inline const DMatDMatMultExpr<MT1,MT2,true,HF,LF,UF>
   declsym( const DMatDMatMultExpr<MT1,MT2,SF,HF,LF,UF>& dm )
{
   BLAZE_FUNCTION_TRACE;

   if( !isSquare( dm ) ) {
      BLAZE_THROW_INVALID_ARGUMENT( "Invalid symmetric matrix specification" );
   }

   return DMatDMatMultExpr<MT1,MT2,true,HF,LF,UF>( dm.leftOperand(), dm.rightOperand() );
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
   using blaze::rowMajor;

   blaze::DynamicMatrix<double,rowMajor> A, B, C;
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
inline const DMatDMatMultExpr<MT1,MT2,SF,true,LF,UF>
   declherm( const DMatDMatMultExpr<MT1,MT2,SF,HF,LF,UF>& dm )
{
   BLAZE_FUNCTION_TRACE;

   if( !isSquare( dm ) ) {
      BLAZE_THROW_INVALID_ARGUMENT( "Invalid Hermitian matrix specification" );
   }

   return DMatDMatMultExpr<MT1,MT2,SF,true,LF,UF>( dm.leftOperand(), dm.rightOperand() );
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
   using blaze::rowMajor;

   blaze::DynamicMatrix<double,rowMajor> A, B, C;
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
inline const DMatDMatMultExpr<MT1,MT2,SF,HF,true,UF>
   decllow( const DMatDMatMultExpr<MT1,MT2,SF,HF,LF,UF>& dm )
{
   BLAZE_FUNCTION_TRACE;

   if( !isSquare( dm ) ) {
      BLAZE_THROW_INVALID_ARGUMENT( "Invalid lower matrix specification" );
   }

   return DMatDMatMultExpr<MT1,MT2,SF,HF,true,UF>( dm.leftOperand(), dm.rightOperand() );
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
   using blaze::rowMajor;

   blaze::DynamicMatrix<double,rowMajor> A, B, C;
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
inline const DMatDMatMultExpr<MT1,MT2,SF,HF,LF,true>
   declupp( const DMatDMatMultExpr<MT1,MT2,SF,HF,LF,UF>& dm )
{
   BLAZE_FUNCTION_TRACE;

   if( !isSquare( dm ) ) {
      BLAZE_THROW_INVALID_ARGUMENT( "Invalid upper matrix specification" );
   }

   return DMatDMatMultExpr<MT1,MT2,SF,HF,LF,true>( dm.leftOperand(), dm.rightOperand() );
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
   using blaze::rowMajor;

   blaze::DynamicMatrix<double,rowMajor> A, B, C;
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
inline const DMatDMatMultExpr<MT1,MT2,SF,HF,true,true>
   decldiag( const DMatDMatMultExpr<MT1,MT2,SF,HF,LF,UF>& dm )
{
   BLAZE_FUNCTION_TRACE;

   if( !isSquare( dm ) ) {
      BLAZE_THROW_INVALID_ARGUMENT( "Invalid diagonal matrix specification" );
   }

   return DMatDMatMultExpr<MT1,MT2,SF,HF,true,true>( dm.leftOperand(), dm.rightOperand() );
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
struct Rows< DMatDMatMultExpr<MT1,MT2,SF,HF,LF,UF> > : public Rows<MT1>
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
struct Columns< DMatDMatMultExpr<MT1,MT2,SF,HF,LF,UF> > : public Columns<MT2>
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
struct IsAligned< DMatDMatMultExpr<MT1,MT2,SF,HF,LF,UF> >
   : public BoolConstant< And< IsAligned<MT1>, IsAligned<MT2> >::value >
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
struct IsSymmetric< DMatDMatMultExpr<MT1,MT2,SF,HF,LF,UF> >
   : public BoolConstant< Or< Bool<SF>
                            , And< Bool<HF>
                                 , IsBuiltin< ElementType_< DMatDMatMultExpr<MT1,MT2,false,true,false,false> > > >
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
struct IsHermitian< DMatDMatMultExpr<MT1,MT2,SF,true,LF,UF> >
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
struct IsLower< DMatDMatMultExpr<MT1,MT2,SF,HF,LF,UF> >
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
struct IsUniLower< DMatDMatMultExpr<MT1,MT2,SF,HF,LF,UF> >
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
struct IsStrictlyLower< DMatDMatMultExpr<MT1,MT2,SF,HF,LF,UF> >
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
struct IsUpper< DMatDMatMultExpr<MT1,MT2,SF,HF,LF,UF> >
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
struct IsUniUpper< DMatDMatMultExpr<MT1,MT2,SF,HF,LF,UF> >
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
struct IsStrictlyUpper< DMatDMatMultExpr<MT1,MT2,SF,HF,LF,UF> >
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
struct DMatDVecMultExprTrait< DMatDMatMultExpr<MT1,MT2,SF,HF,LF,UF>, VT >
{
 public:
   //**********************************************************************************************
   using Type = If_< And< IsDenseMatrix<MT1>, IsRowMajorMatrix<MT1>
                        , IsDenseMatrix<MT2>, IsRowMajorMatrix<MT2>
                        , IsDenseVector<VT>, IsColumnVector<VT> >
                   , DMatDVecMultExprTrait_< MT1, DMatDVecMultExprTrait_<MT2,VT> >
                   , INVALID_TYPE >;
   //**********************************************************************************************
};
/*! \endcond */
//*************************************************************************************************


//*************************************************************************************************
/*! \cond BLAZE_INTERNAL */
template< typename MT1, typename MT2, bool SF, bool HF, bool LF, bool UF, typename VT >
struct DMatSVecMultExprTrait< DMatDMatMultExpr<MT1,MT2,SF,HF,LF,UF>, VT >
{
 public:
   //**********************************************************************************************
   using Type = If_< And< IsDenseMatrix<MT1>, IsRowMajorMatrix<MT1>
                        , IsDenseMatrix<MT2>, IsRowMajorMatrix<MT2>
                        , IsSparseVector<VT>, IsColumnVector<VT> >
                   , DMatDVecMultExprTrait_< MT1, DMatSVecMultExprTrait_<MT2,VT> >
                   , INVALID_TYPE >;
   //**********************************************************************************************
};
/*! \endcond */
//*************************************************************************************************


//*************************************************************************************************
/*! \cond BLAZE_INTERNAL */
template< typename VT, typename MT1, typename MT2, bool SF, bool HF, bool LF, bool UF >
struct TDVecDMatMultExprTrait< VT, DMatDMatMultExpr<MT1,MT2,SF,HF,LF,UF> >
{
 public:
   //**********************************************************************************************
   using Type = If_< And< IsDenseVector<VT>, IsRowVector<VT>
                        , IsDenseMatrix<MT1>, IsRowMajorMatrix<MT1>
                        , IsDenseMatrix<MT2>, IsRowMajorMatrix<MT2> >
                   , TDVecDMatMultExprTrait_< TDVecDMatMultExprTrait_<VT,MT1>, MT2 >
                   , INVALID_TYPE >;
   //**********************************************************************************************
};
/*! \endcond */
//*************************************************************************************************


//*************************************************************************************************
/*! \cond BLAZE_INTERNAL */
template< typename VT, typename MT1, typename MT2, bool SF, bool HF, bool LF, bool UF >
struct TSVecDMatMultExprTrait< VT, DMatDMatMultExpr<MT1,MT2,SF,HF,LF,UF> >
{
 public:
   //**********************************************************************************************
   using Type = If_< And< IsSparseVector<VT>, IsRowVector<VT>
                        , IsDenseMatrix<MT1>, IsRowMajorMatrix<MT1>
                        , IsDenseMatrix<MT2>, IsRowMajorMatrix<MT2> >
                   , TDVecDMatMultExprTrait_< TSVecDMatMultExprTrait_<VT,MT1>, MT2 >
                   , INVALID_TYPE >;
   //**********************************************************************************************
};
/*! \endcond */
//*************************************************************************************************


//*************************************************************************************************
/*! \cond BLAZE_INTERNAL */
template< typename MT1, typename MT2, bool SF, bool HF, bool LF, bool UF >
struct DMatDeclSymExprTrait< DMatDMatMultExpr<MT1,MT2,SF,HF,LF,UF> >
{
 public:
   //**********************************************************************************************
   using Type = If_< And< IsDenseMatrix<MT1>, IsRowMajorMatrix<MT1>
                        , IsDenseMatrix<MT2>, IsRowMajorMatrix<MT2> >
                   , DMatDMatMultExpr<MT1,MT2,true,HF,LF,UF>
                   , INVALID_TYPE >;
   //**********************************************************************************************
};
/*! \endcond */
//*************************************************************************************************


//*************************************************************************************************
/*! \cond BLAZE_INTERNAL */
template< typename MT1, typename MT2, bool SF, bool HF, bool LF, bool UF >
struct DMatDeclHermExprTrait< DMatDMatMultExpr<MT1,MT2,SF,HF,LF,UF> >
{
 public:
   //**********************************************************************************************
   using Type = If_< And< IsDenseMatrix<MT1>, IsRowMajorMatrix<MT1>
                        , IsDenseMatrix<MT2>, IsRowMajorMatrix<MT2> >
                   , DMatDMatMultExpr<MT1,MT2,SF,true,LF,UF>
                   , INVALID_TYPE >;
   //**********************************************************************************************
};
/*! \endcond */
//*************************************************************************************************


//*************************************************************************************************
/*! \cond BLAZE_INTERNAL */
template< typename MT1, typename MT2, bool SF, bool HF, bool LF, bool UF >
struct DMatDeclLowExprTrait< DMatDMatMultExpr<MT1,MT2,SF,HF,LF,UF> >
{
 public:
   //**********************************************************************************************
   using Type = If_< And< IsDenseMatrix<MT1>, IsRowMajorMatrix<MT1>
                        , IsDenseMatrix<MT2>, IsRowMajorMatrix<MT2> >
                   , DMatDMatMultExpr<MT1,MT2,SF,HF,true,UF>
                   , INVALID_TYPE >;
   //**********************************************************************************************
};
/*! \endcond */
//*************************************************************************************************


//*************************************************************************************************
/*! \cond BLAZE_INTERNAL */
template< typename MT1, typename MT2, bool SF, bool HF, bool LF, bool UF >
struct DMatDeclUppExprTrait< DMatDMatMultExpr<MT1,MT2,SF,HF,LF,UF> >
{
 public:
   //**********************************************************************************************
   using Type = If_< And< IsDenseMatrix<MT1>, IsRowMajorMatrix<MT1>
                        , IsDenseMatrix<MT2>, IsRowMajorMatrix<MT2> >
                   , DMatDMatMultExpr<MT1,MT2,SF,HF,LF,true>
                   , INVALID_TYPE >;
   //**********************************************************************************************
};
/*! \endcond */
//*************************************************************************************************


//*************************************************************************************************
/*! \cond BLAZE_INTERNAL */
template< typename MT1, typename MT2, bool SF, bool HF, bool LF, bool UF >
struct DMatDeclDiagExprTrait< DMatDMatMultExpr<MT1,MT2,SF,HF,LF,UF> >
{
 public:
   //**********************************************************************************************
   using Type = If_< And< IsDenseMatrix<MT1>, IsRowMajorMatrix<MT1>
                        , IsDenseMatrix<MT2>, IsRowMajorMatrix<MT2> >
                   , DMatDMatMultExpr<MT1,MT2,SF,HF,true,true>
                   , INVALID_TYPE >;
   //**********************************************************************************************
};
/*! \endcond */
//*************************************************************************************************


//*************************************************************************************************
/*! \cond BLAZE_INTERNAL */
template< typename MT1, typename MT2, bool SF, bool HF, bool LF, bool UF, bool AF >
struct SubmatrixExprTrait< DMatDMatMultExpr<MT1,MT2,SF,HF,LF,UF>, AF >
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
struct RowExprTrait< DMatDMatMultExpr<MT1,MT2,SF,HF,LF,UF> >
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
struct ColumnExprTrait< DMatDMatMultExpr<MT1,MT2,SF,HF,LF,UF> >
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
