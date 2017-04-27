//=================================================================================================
/*!
//  \file blaze/math/typetraits/LowType.h
//  \brief Header file for the LowType type trait
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

#ifndef _BLAZE_MATH_TYPETRAITS_LOWTYPE_H_
#define _BLAZE_MATH_TYPETRAITS_LOWTYPE_H_


//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <cstddef>
#include <blaze/util/Complex.h>
#include <blaze/util/InvalidType.h>
#include <blaze/util/mpl/If.h>
#include <blaze/util/mpl/Or.h>
#include <blaze/util/typetraits/Decay.h>
#include <blaze/util/typetraits/IsConst.h>
#include <blaze/util/typetraits/IsReference.h>
#include <blaze/util/typetraits/IsVolatile.h>


namespace blaze {

//=================================================================================================
//
//  CLASS DEFINITION
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Base template for the LowType type trait.
// \ingroup math_traits
//
// \section lowtype_general General
//
// The LowType class template determines the less significant, submissive data type of the two
// given data types \a T1 and \a T2. For instance, in case both \a T1 and \a T2 are built-in data
// types, the nested type \a Type is set to the smaller or unsigned data type. In case no lower
// data type can be selected, \a Type is set to \a INVALID_TYPE. Note that cv-qualifiers and
// reference modifiers are generally ignored.
//
// Per default, the LowType template provides specializations for the following built-in data
// types:
//
// <ul>
//    <li>Integral types</li>
//    <ul>
//       <li>unsigned char, signed char, char, wchar_t</li>
//       <li>unsigned short, short</li>
//       <li>unsigned int, int</li>
//       <li>unsigned long, long</li>
//       <li>std::size_t, std::ptrdiff_t (for certain 64-bit compilers)</li>
//    </ul>
//    <li>Floating point types</li>
//    <ul>
//       <li>float</li>
//       <li>double</li>
//       <li>long double</li>
//    </ul>
// </ul>
//
// Additionally, the Blaze library provides specializations for the following user-defined
// arithmetic types, wherever a less significant data type can be selected:
//
// <ul>
//    <li>std::complex</li>
//    <li>blaze::StaticVector</li>
//    <li>blaze::HybridVector</li>
//    <li>blaze::DynamicVector</li>
//    <li>blaze::CompressedVector</li>
//    <li>blaze::StaticMatrix</li>
//    <li>blaze::HybridMatrix</li>
//    <li>blaze::DynamicMatrix</li>
//    <li>blaze::CompressedMatrix</li>
//    <li>blaze::SymmetricMatrix</li>
//    <li>blaze::HermitianMatrix</li>
//    <li>blaze::LowerMatrix</li>
//    <li>blaze::UniLowerMatrix</li>
//    <li>blaze::StrictlyLowerMatrix</li>
//    <li>blaze::UpperMatrix</li>
//    <li>blaze::UniUpperMatrix</li>
//    <li>blaze::StrictlyUpperMatrix</li>
//    <li>blaze::DiagonalMatrix</li>
// </ul>
//
//
// \n \section lowtype_specializations Creating custom specializations
//
// It is possible to specialize the LowType template for additional user-defined data types.
// The following example shows the according specialization for two dynamic column vectors:

   \code
   template< typename T1, typename T2 >
   struct LowType< DynamicVector<T1,false>, DynamicVector<T2,false> >
   {
      typedef DynamicVector< typename LowType<T1,T2>::Type, false >  Type;
   };
   \endcode
*/
template< typename T1, typename T2 >
struct LowType
{
 private:
   //**********************************************************************************************
   /*! \cond BLAZE_INTERNAL */
   struct Failure {
      using Type = INVALID_TYPE;
   };
   /*! \endcond */
   //**********************************************************************************************

   //**********************************************************************************************
   /*! \cond BLAZE_INTERNAL */
   using Helper = LowType< Decay_<T1>, Decay_<T2> >;
   /*! \endcond */
   //**********************************************************************************************

 public:
   //**********************************************************************************************
   /*! \cond BLAZE_INTERNAL */
   using Type = typename If_< Or< IsConst<T1>, IsVolatile<T1>, IsReference<T1>
                                , IsConst<T2>, IsVolatile<T2>, IsReference<T2> >
                            , Helper
                            , Failure >::Type;
   /*! \endcond */
   //**********************************************************************************************
};
//*************************************************************************************************


//*************************************************************************************************
/*!\brief Auxiliary alias declaration for the LowType type trait.
// \ingroup type_traits
//
// The LowType_ alias declaration provides a convenient shortcut to access the nested \a Type of
// the LowType class template. For instance, given the types \a T1 and \a T2 the following two
// type definitions are identical:

   \code
   using Type1 = typename LowType<T1,T2>::Type;
   using Type2 = LowType_<T1,T2>;
   \endcode
*/
template< typename T1, typename T2 >
using LowType_ = typename LowType<T1,T2>::Type;
//*************************************************************************************************




//=================================================================================================
//
//  LOWTYPE SPECIALIZATION FOR IDENTICAL TYPES
//
//=================================================================================================

//*************************************************************************************************
/*! \cond BLAZE_INTERNAL */
/*!\brief Specialization for two identical types.
// \ingroup math_traits
//
// This specialization of the LowType class template handles the special case that the two
// given types are identical. In this case, the nested type \a Type is set to the given type
// \a T (ignoring cv-qualifiers and reference modifiers).
*/
template< typename T >
struct LowType<T,T>
{
   //**********************************************************************************************
   using Type = Decay_<T>;
   //**********************************************************************************************
};
/*! \endcond */
//*************************************************************************************************




//=================================================================================================
//
//  LOWTYPE SPECIALIZATION MACRO
//
//=================================================================================================

//*************************************************************************************************
/*! \cond BLAZE_INTERNAL */
/*!\brief Macro for the creation of LowType specializations for the built-in data types.
// \ingroup math_traits
//
// This macro is used for the setup of the LowType specializations for the built-in data types.
*/
#define BLAZE_CREATE_BUILTIN_LOWTYPE_SPECIALIZATION(T1,T2,LOW) \
   template<> \
   struct LowType< T1, T2 > \
   { \
      using Type = LOW; \
   }
/*! \endcond */
//*************************************************************************************************


//*************************************************************************************************
/*! \cond BLAZE_INTERNAL */
/*!\brief Macro for the creation of LowType specializations for the complex data type.
// \ingroup math_traits
//
// This macro is used for the setup of the LowType specializations for the complex data type.
*/
#define BLAZE_CREATE_COMPLEX_LOWTYPE_SPECIALIZATION( T1 ) \
   template< typename T2 > \
   struct LowType< T1, complex<T2> > \
   { \
      using Type = T1;  \
   }; \
   template< typename T2 > \
   struct LowType< complex<T2>, T1 > \
   { \
      using Type = T1;  \
   }
/*! \endcond */
//*************************************************************************************************




//=================================================================================================
//
//  UNSIGNED CHAR SPECIALIZATIONS
//
//=================================================================================================

//*************************************************************************************************
/*! \cond BLAZE_INTERNAL */
//                                           Type 1          Type 2          Low type
BLAZE_CREATE_BUILTIN_LOWTYPE_SPECIALIZATION( unsigned char , unsigned char , unsigned char  );
BLAZE_CREATE_BUILTIN_LOWTYPE_SPECIALIZATION( unsigned char , char          , unsigned char  );
BLAZE_CREATE_BUILTIN_LOWTYPE_SPECIALIZATION( unsigned char , signed char   , unsigned char  );
BLAZE_CREATE_BUILTIN_LOWTYPE_SPECIALIZATION( unsigned char , wchar_t       , unsigned char  );
BLAZE_CREATE_BUILTIN_LOWTYPE_SPECIALIZATION( unsigned char , unsigned short, unsigned char  );
BLAZE_CREATE_BUILTIN_LOWTYPE_SPECIALIZATION( unsigned char , short         , unsigned char  );
BLAZE_CREATE_BUILTIN_LOWTYPE_SPECIALIZATION( unsigned char , unsigned int  , unsigned char  );
BLAZE_CREATE_BUILTIN_LOWTYPE_SPECIALIZATION( unsigned char , int           , unsigned char  );
BLAZE_CREATE_BUILTIN_LOWTYPE_SPECIALIZATION( unsigned char , unsigned long , unsigned char  );
BLAZE_CREATE_BUILTIN_LOWTYPE_SPECIALIZATION( unsigned char , long          , unsigned char  );
#if defined(_WIN64)
BLAZE_CREATE_BUILTIN_LOWTYPE_SPECIALIZATION( unsigned char , std::size_t   , unsigned char  );
BLAZE_CREATE_BUILTIN_LOWTYPE_SPECIALIZATION( unsigned char , std::ptrdiff_t, unsigned char  );
#endif
BLAZE_CREATE_BUILTIN_LOWTYPE_SPECIALIZATION( unsigned char , float         , unsigned char  );
BLAZE_CREATE_BUILTIN_LOWTYPE_SPECIALIZATION( unsigned char , double        , unsigned char  );
BLAZE_CREATE_BUILTIN_LOWTYPE_SPECIALIZATION( unsigned char , long double   , unsigned char  );
/*! \endcond */
//*************************************************************************************************




//=================================================================================================
//
//  CHAR SPECIALIZATIONS
//
//=================================================================================================

//*************************************************************************************************
/*! \cond BLAZE_INTERNAL */
//                                           Type 1          Type 2          Low type
BLAZE_CREATE_BUILTIN_LOWTYPE_SPECIALIZATION( char          , unsigned char , unsigned char  );
BLAZE_CREATE_BUILTIN_LOWTYPE_SPECIALIZATION( char          , char          , char           );
BLAZE_CREATE_BUILTIN_LOWTYPE_SPECIALIZATION( char          , signed char   , char           );
BLAZE_CREATE_BUILTIN_LOWTYPE_SPECIALIZATION( char          , wchar_t       , char           );
BLAZE_CREATE_BUILTIN_LOWTYPE_SPECIALIZATION( char          , unsigned short, char           );
BLAZE_CREATE_BUILTIN_LOWTYPE_SPECIALIZATION( char          , short         , char           );
BLAZE_CREATE_BUILTIN_LOWTYPE_SPECIALIZATION( char          , unsigned int  , char           );
BLAZE_CREATE_BUILTIN_LOWTYPE_SPECIALIZATION( char          , int           , char           );
BLAZE_CREATE_BUILTIN_LOWTYPE_SPECIALIZATION( char          , unsigned long , char           );
BLAZE_CREATE_BUILTIN_LOWTYPE_SPECIALIZATION( char          , long          , char           );
#if defined(_WIN64)
BLAZE_CREATE_BUILTIN_LOWTYPE_SPECIALIZATION( char          , std::size_t   , char           );
BLAZE_CREATE_BUILTIN_LOWTYPE_SPECIALIZATION( char          , std::ptrdiff_t, char           );
#endif
BLAZE_CREATE_BUILTIN_LOWTYPE_SPECIALIZATION( char          , float         , char           );
BLAZE_CREATE_BUILTIN_LOWTYPE_SPECIALIZATION( char          , double        , char           );
BLAZE_CREATE_BUILTIN_LOWTYPE_SPECIALIZATION( char          , long double   , char           );
/*! \endcond */
//*************************************************************************************************




//=================================================================================================
//
//  SIGNED CHAR SPECIALIZATIONS
//
//=================================================================================================

//*************************************************************************************************
/*! \cond BLAZE_INTERNAL */
//                                           Type 1          Type 2          Low type
BLAZE_CREATE_BUILTIN_LOWTYPE_SPECIALIZATION( signed char   , unsigned char , unsigned char  );
BLAZE_CREATE_BUILTIN_LOWTYPE_SPECIALIZATION( signed char   , char          , char           );
BLAZE_CREATE_BUILTIN_LOWTYPE_SPECIALIZATION( signed char   , signed char   , signed char    );
BLAZE_CREATE_BUILTIN_LOWTYPE_SPECIALIZATION( signed char   , wchar_t       , signed char    );
BLAZE_CREATE_BUILTIN_LOWTYPE_SPECIALIZATION( signed char   , unsigned short, signed char    );
BLAZE_CREATE_BUILTIN_LOWTYPE_SPECIALIZATION( signed char   , short         , signed char    );
BLAZE_CREATE_BUILTIN_LOWTYPE_SPECIALIZATION( signed char   , unsigned int  , signed char    );
BLAZE_CREATE_BUILTIN_LOWTYPE_SPECIALIZATION( signed char   , int           , signed char    );
BLAZE_CREATE_BUILTIN_LOWTYPE_SPECIALIZATION( signed char   , unsigned long , signed char    );
BLAZE_CREATE_BUILTIN_LOWTYPE_SPECIALIZATION( signed char   , long          , signed char    );
#if defined(_WIN64)
BLAZE_CREATE_BUILTIN_LOWTYPE_SPECIALIZATION( signed char   , std::size_t   , signed char    );
BLAZE_CREATE_BUILTIN_LOWTYPE_SPECIALIZATION( signed char   , std::ptrdiff_t, signed char    );
#endif
BLAZE_CREATE_BUILTIN_LOWTYPE_SPECIALIZATION( signed char   , float         , signed char    );
BLAZE_CREATE_BUILTIN_LOWTYPE_SPECIALIZATION( signed char   , double        , signed char    );
BLAZE_CREATE_BUILTIN_LOWTYPE_SPECIALIZATION( signed char   , long double   , signed char    );
/*! \endcond */
//*************************************************************************************************




//=================================================================================================
//
//  WCHAR_T SPECIALIZATIONS
//
//=================================================================================================

//*************************************************************************************************
/*! \cond BLAZE_INTERNAL */
//                                           Type 1          Type 2          Low type
BLAZE_CREATE_BUILTIN_LOWTYPE_SPECIALIZATION( wchar_t       , unsigned char , unsigned char  );
BLAZE_CREATE_BUILTIN_LOWTYPE_SPECIALIZATION( wchar_t       , char          , char           );
BLAZE_CREATE_BUILTIN_LOWTYPE_SPECIALIZATION( wchar_t       , signed char   , signed char    );
BLAZE_CREATE_BUILTIN_LOWTYPE_SPECIALIZATION( wchar_t       , wchar_t       , wchar_t        );
BLAZE_CREATE_BUILTIN_LOWTYPE_SPECIALIZATION( wchar_t       , unsigned short, wchar_t        );
BLAZE_CREATE_BUILTIN_LOWTYPE_SPECIALIZATION( wchar_t       , short         , wchar_t        );
BLAZE_CREATE_BUILTIN_LOWTYPE_SPECIALIZATION( wchar_t       , unsigned int  , wchar_t        );
BLAZE_CREATE_BUILTIN_LOWTYPE_SPECIALIZATION( wchar_t       , int           , wchar_t        );
BLAZE_CREATE_BUILTIN_LOWTYPE_SPECIALIZATION( wchar_t       , unsigned long , wchar_t        );
BLAZE_CREATE_BUILTIN_LOWTYPE_SPECIALIZATION( wchar_t       , long          , wchar_t        );
#if defined(_WIN64)
BLAZE_CREATE_BUILTIN_LOWTYPE_SPECIALIZATION( wchar_t       , std::size_t   , wchar_t        );
BLAZE_CREATE_BUILTIN_LOWTYPE_SPECIALIZATION( wchar_t       , std::ptrdiff_t, wchar_t        );
#endif
BLAZE_CREATE_BUILTIN_LOWTYPE_SPECIALIZATION( wchar_t       , float         , wchar_t        );
BLAZE_CREATE_BUILTIN_LOWTYPE_SPECIALIZATION( wchar_t       , double        , wchar_t        );
BLAZE_CREATE_BUILTIN_LOWTYPE_SPECIALIZATION( wchar_t       , long double   , wchar_t        );
/*! \endcond */
//*************************************************************************************************




//=================================================================================================
//
//  UNSIGNED SHORT SPECIALIZATIONS
//
//=================================================================================================

//*************************************************************************************************
/*! \cond BLAZE_INTERNAL */
//                                           Type 1          Type 2          Low type
BLAZE_CREATE_BUILTIN_LOWTYPE_SPECIALIZATION( unsigned short, unsigned char , unsigned char  );
BLAZE_CREATE_BUILTIN_LOWTYPE_SPECIALIZATION( unsigned short, char          , char           );
BLAZE_CREATE_BUILTIN_LOWTYPE_SPECIALIZATION( unsigned short, signed char   , signed char    );
BLAZE_CREATE_BUILTIN_LOWTYPE_SPECIALIZATION( unsigned short, wchar_t       , wchar_t        );
BLAZE_CREATE_BUILTIN_LOWTYPE_SPECIALIZATION( unsigned short, unsigned short, unsigned short );
BLAZE_CREATE_BUILTIN_LOWTYPE_SPECIALIZATION( unsigned short, short         , unsigned short );
BLAZE_CREATE_BUILTIN_LOWTYPE_SPECIALIZATION( unsigned short, unsigned int  , unsigned short );
BLAZE_CREATE_BUILTIN_LOWTYPE_SPECIALIZATION( unsigned short, int           , unsigned short );
BLAZE_CREATE_BUILTIN_LOWTYPE_SPECIALIZATION( unsigned short, unsigned long , unsigned short );
BLAZE_CREATE_BUILTIN_LOWTYPE_SPECIALIZATION( unsigned short, long          , unsigned short );
#if defined(_WIN64)
BLAZE_CREATE_BUILTIN_LOWTYPE_SPECIALIZATION( unsigned short, std::size_t   , unsigned short );
BLAZE_CREATE_BUILTIN_LOWTYPE_SPECIALIZATION( unsigned short, std::ptrdiff_t, unsigned short );
#endif
BLAZE_CREATE_BUILTIN_LOWTYPE_SPECIALIZATION( unsigned short, float         , unsigned short );
BLAZE_CREATE_BUILTIN_LOWTYPE_SPECIALIZATION( unsigned short, double        , unsigned short );
BLAZE_CREATE_BUILTIN_LOWTYPE_SPECIALIZATION( unsigned short, long double   , unsigned short );
/*! \endcond */
//*************************************************************************************************




//=================================================================================================
//
//  SHORT SPECIALIZATIONS
//
//=================================================================================================

//*************************************************************************************************
/*! \cond BLAZE_INTERNAL */
//                                           Type 1          Type 2          Low type
BLAZE_CREATE_BUILTIN_LOWTYPE_SPECIALIZATION( short         , unsigned char , unsigned char  );
BLAZE_CREATE_BUILTIN_LOWTYPE_SPECIALIZATION( short         , char          , char           );
BLAZE_CREATE_BUILTIN_LOWTYPE_SPECIALIZATION( short         , signed char   , signed char    );
BLAZE_CREATE_BUILTIN_LOWTYPE_SPECIALIZATION( short         , wchar_t       , wchar_t        );
BLAZE_CREATE_BUILTIN_LOWTYPE_SPECIALIZATION( short         , unsigned short, unsigned short );
BLAZE_CREATE_BUILTIN_LOWTYPE_SPECIALIZATION( short         , short         , short          );
BLAZE_CREATE_BUILTIN_LOWTYPE_SPECIALIZATION( short         , unsigned int  , short          );
BLAZE_CREATE_BUILTIN_LOWTYPE_SPECIALIZATION( short         , int           , short          );
BLAZE_CREATE_BUILTIN_LOWTYPE_SPECIALIZATION( short         , unsigned long , short          );
BLAZE_CREATE_BUILTIN_LOWTYPE_SPECIALIZATION( short         , long          , short          );
#if defined(_WIN64)
BLAZE_CREATE_BUILTIN_LOWTYPE_SPECIALIZATION( short         , std::size_t   , short          );
BLAZE_CREATE_BUILTIN_LOWTYPE_SPECIALIZATION( short         , std::ptrdiff_t, short          );
#endif
BLAZE_CREATE_BUILTIN_LOWTYPE_SPECIALIZATION( short         , float         , short          );
BLAZE_CREATE_BUILTIN_LOWTYPE_SPECIALIZATION( short         , double        , short          );
BLAZE_CREATE_BUILTIN_LOWTYPE_SPECIALIZATION( short         , long double   , short          );
/*! \endcond */
//*************************************************************************************************




//=================================================================================================
//
//  UNSIGNED INT SPECIALIZATIONS
//
//=================================================================================================

//*************************************************************************************************
/*! \cond BLAZE_INTERNAL */
//                                           Type 1          Type 2          Low type
BLAZE_CREATE_BUILTIN_LOWTYPE_SPECIALIZATION( unsigned int  , unsigned char , unsigned char  );
BLAZE_CREATE_BUILTIN_LOWTYPE_SPECIALIZATION( unsigned int  , char          , char           );
BLAZE_CREATE_BUILTIN_LOWTYPE_SPECIALIZATION( unsigned int  , signed char   , signed char    );
BLAZE_CREATE_BUILTIN_LOWTYPE_SPECIALIZATION( unsigned int  , wchar_t       , wchar_t        );
BLAZE_CREATE_BUILTIN_LOWTYPE_SPECIALIZATION( unsigned int  , unsigned short, unsigned short );
BLAZE_CREATE_BUILTIN_LOWTYPE_SPECIALIZATION( unsigned int  , short         , short          );
BLAZE_CREATE_BUILTIN_LOWTYPE_SPECIALIZATION( unsigned int  , unsigned int  , unsigned int   );
BLAZE_CREATE_BUILTIN_LOWTYPE_SPECIALIZATION( unsigned int  , int           , unsigned int   );
BLAZE_CREATE_BUILTIN_LOWTYPE_SPECIALIZATION( unsigned int  , unsigned long , unsigned int   );
BLAZE_CREATE_BUILTIN_LOWTYPE_SPECIALIZATION( unsigned int  , long          , unsigned int   );
#if defined(_WIN64)
BLAZE_CREATE_BUILTIN_LOWTYPE_SPECIALIZATION( unsigned int  , std::size_t   , unsigned int   );
BLAZE_CREATE_BUILTIN_LOWTYPE_SPECIALIZATION( unsigned int  , std::ptrdiff_t, unsigned int   );
#endif
BLAZE_CREATE_BUILTIN_LOWTYPE_SPECIALIZATION( unsigned int  , float         , unsigned int   );
BLAZE_CREATE_BUILTIN_LOWTYPE_SPECIALIZATION( unsigned int  , double        , unsigned int   );
BLAZE_CREATE_BUILTIN_LOWTYPE_SPECIALIZATION( unsigned int  , long double   , unsigned int   );
/*! \endcond */
//*************************************************************************************************




//=================================================================================================
//
//  INT SPECIALIZATIONS
//
//=================================================================================================

//*************************************************************************************************
/*! \cond BLAZE_INTERNAL */
//                                           Type 1          Type 2          Low type
BLAZE_CREATE_BUILTIN_LOWTYPE_SPECIALIZATION( int           , unsigned char , unsigned char  );
BLAZE_CREATE_BUILTIN_LOWTYPE_SPECIALIZATION( int           , char          , wchar_t        );
BLAZE_CREATE_BUILTIN_LOWTYPE_SPECIALIZATION( int           , signed char   , signed char    );
BLAZE_CREATE_BUILTIN_LOWTYPE_SPECIALIZATION( int           , wchar_t       , wchar_t        );
BLAZE_CREATE_BUILTIN_LOWTYPE_SPECIALIZATION( int           , unsigned short, unsigned short );
BLAZE_CREATE_BUILTIN_LOWTYPE_SPECIALIZATION( int           , short         , short          );
BLAZE_CREATE_BUILTIN_LOWTYPE_SPECIALIZATION( int           , unsigned int  , unsigned int   );
BLAZE_CREATE_BUILTIN_LOWTYPE_SPECIALIZATION( int           , int           , int            );
BLAZE_CREATE_BUILTIN_LOWTYPE_SPECIALIZATION( int           , unsigned long , int            );
BLAZE_CREATE_BUILTIN_LOWTYPE_SPECIALIZATION( int           , long          , int            );
#if defined(_WIN64)
BLAZE_CREATE_BUILTIN_LOWTYPE_SPECIALIZATION( int           , std::size_t   , int            );
BLAZE_CREATE_BUILTIN_LOWTYPE_SPECIALIZATION( int           , std::ptrdiff_t, int            );
#endif
BLAZE_CREATE_BUILTIN_LOWTYPE_SPECIALIZATION( int           , float         , int            );
BLAZE_CREATE_BUILTIN_LOWTYPE_SPECIALIZATION( int           , double        , int            );
BLAZE_CREATE_BUILTIN_LOWTYPE_SPECIALIZATION( int           , long double   , int            );
/*! \endcond */
//*************************************************************************************************




//=================================================================================================
//
//  UNSIGNED LONG SPECIALIZATIONS
//
//=================================================================================================

//*************************************************************************************************
/*! \cond BLAZE_INTERNAL */
//                                           Type 1          Type 2          Low type
BLAZE_CREATE_BUILTIN_LOWTYPE_SPECIALIZATION( unsigned long , unsigned char , unsigned char  );
BLAZE_CREATE_BUILTIN_LOWTYPE_SPECIALIZATION( unsigned long , char          , char           );
BLAZE_CREATE_BUILTIN_LOWTYPE_SPECIALIZATION( unsigned long , signed char   , signed char    );
BLAZE_CREATE_BUILTIN_LOWTYPE_SPECIALIZATION( unsigned long , wchar_t       , wchar_t        );
BLAZE_CREATE_BUILTIN_LOWTYPE_SPECIALIZATION( unsigned long , unsigned short, unsigned short );
BLAZE_CREATE_BUILTIN_LOWTYPE_SPECIALIZATION( unsigned long , short         , short          );
BLAZE_CREATE_BUILTIN_LOWTYPE_SPECIALIZATION( unsigned long , unsigned int  , unsigned int   );
BLAZE_CREATE_BUILTIN_LOWTYPE_SPECIALIZATION( unsigned long , int           , int            );
BLAZE_CREATE_BUILTIN_LOWTYPE_SPECIALIZATION( unsigned long , unsigned long , unsigned long  );
BLAZE_CREATE_BUILTIN_LOWTYPE_SPECIALIZATION( unsigned long , long          , unsigned long  );
#if defined(_WIN64)
BLAZE_CREATE_BUILTIN_LOWTYPE_SPECIALIZATION( unsigned long , std::size_t   , unsigned long  );
BLAZE_CREATE_BUILTIN_LOWTYPE_SPECIALIZATION( unsigned long , std::ptrdiff_t, unsigned long  );
#endif
BLAZE_CREATE_BUILTIN_LOWTYPE_SPECIALIZATION( unsigned long , float         , unsigned long  );
BLAZE_CREATE_BUILTIN_LOWTYPE_SPECIALIZATION( unsigned long , double        , unsigned long  );
BLAZE_CREATE_BUILTIN_LOWTYPE_SPECIALIZATION( unsigned long , long double   , unsigned long  );
/*! \endcond */
//*************************************************************************************************




//=================================================================================================
//
//  LONG SPECIALIZATIONS
//
//=================================================================================================

//*************************************************************************************************
/*! \cond BLAZE_INTERNAL */
//                                           Type 1          Type 2          Low type
BLAZE_CREATE_BUILTIN_LOWTYPE_SPECIALIZATION( long          , unsigned char , unsigned char  );
BLAZE_CREATE_BUILTIN_LOWTYPE_SPECIALIZATION( long          , char          , char           );
BLAZE_CREATE_BUILTIN_LOWTYPE_SPECIALIZATION( long          , signed char   , signed char    );
BLAZE_CREATE_BUILTIN_LOWTYPE_SPECIALIZATION( long          , wchar_t       , wchar_t        );
BLAZE_CREATE_BUILTIN_LOWTYPE_SPECIALIZATION( long          , unsigned short, unsigned short );
BLAZE_CREATE_BUILTIN_LOWTYPE_SPECIALIZATION( long          , short         , short          );
BLAZE_CREATE_BUILTIN_LOWTYPE_SPECIALIZATION( long          , unsigned int  , unsigned int   );
BLAZE_CREATE_BUILTIN_LOWTYPE_SPECIALIZATION( long          , int           , int            );
BLAZE_CREATE_BUILTIN_LOWTYPE_SPECIALIZATION( long          , unsigned long , unsigned long  );
BLAZE_CREATE_BUILTIN_LOWTYPE_SPECIALIZATION( long          , long          , long           );
#if defined(_WIN64)
BLAZE_CREATE_BUILTIN_LOWTYPE_SPECIALIZATION( long          , std::size_t   , long           );
BLAZE_CREATE_BUILTIN_LOWTYPE_SPECIALIZATION( long          , std::ptrdiff_t, long           );
#endif
BLAZE_CREATE_BUILTIN_LOWTYPE_SPECIALIZATION( long          , float         , long           );
BLAZE_CREATE_BUILTIN_LOWTYPE_SPECIALIZATION( long          , double        , long           );
BLAZE_CREATE_BUILTIN_LOWTYPE_SPECIALIZATION( long          , long double   , long           );
/*! \endcond */
//*************************************************************************************************




//=================================================================================================
//
//  SIZE_T SPECIALIZATIONS
//
//=================================================================================================

//*************************************************************************************************
#if defined(_WIN64)
/*! \cond BLAZE_INTERNAL */
//                                           Type 1          Type 2          Low type
BLAZE_CREATE_BUILTIN_LOWTYPE_SPECIALIZATION( std::size_t   , unsigned char , unsigned char  );
BLAZE_CREATE_BUILTIN_LOWTYPE_SPECIALIZATION( std::size_t   , char          , char           );
BLAZE_CREATE_BUILTIN_LOWTYPE_SPECIALIZATION( std::size_t   , signed char   , signed char    );
BLAZE_CREATE_BUILTIN_LOWTYPE_SPECIALIZATION( std::size_t   , wchar_t       , wchar_t        );
BLAZE_CREATE_BUILTIN_LOWTYPE_SPECIALIZATION( std::size_t   , unsigned short, unsigned short );
BLAZE_CREATE_BUILTIN_LOWTYPE_SPECIALIZATION( std::size_t   , short         , short          );
BLAZE_CREATE_BUILTIN_LOWTYPE_SPECIALIZATION( std::size_t   , unsigned int  , unsigned int   );
BLAZE_CREATE_BUILTIN_LOWTYPE_SPECIALIZATION( std::size_t   , int           , int            );
BLAZE_CREATE_BUILTIN_LOWTYPE_SPECIALIZATION( std::size_t   , unsigned long , unsigned long  );
BLAZE_CREATE_BUILTIN_LOWTYPE_SPECIALIZATION( std::size_t   , long          , long           );
BLAZE_CREATE_BUILTIN_LOWTYPE_SPECIALIZATION( std::size_t   , std::size_t   , std::size_t    );
BLAZE_CREATE_BUILTIN_LOWTYPE_SPECIALIZATION( std::size_t   , std::ptrdiff_t, std::size_t    );
BLAZE_CREATE_BUILTIN_LOWTYPE_SPECIALIZATION( std::size_t   , float         , std::size_t    );
BLAZE_CREATE_BUILTIN_LOWTYPE_SPECIALIZATION( std::size_t   , double        , std::size_t    );
BLAZE_CREATE_BUILTIN_LOWTYPE_SPECIALIZATION( std::size_t   , long double   , std::size_t    );
/*! \endcond */
#endif
//*************************************************************************************************




//=================================================================================================
//
//  PTRDIFF_T SPECIALIZATIONS
//
//=================================================================================================

//*************************************************************************************************
#if defined(_WIN64)
/*! \cond BLAZE_INTERNAL */
//                                           Type 1          Type 2          Low type
BLAZE_CREATE_BUILTIN_LOWTYPE_SPECIALIZATION( std::ptrdiff_t, unsigned char , unsigned char  );
BLAZE_CREATE_BUILTIN_LOWTYPE_SPECIALIZATION( std::ptrdiff_t, char          , char           );
BLAZE_CREATE_BUILTIN_LOWTYPE_SPECIALIZATION( std::ptrdiff_t, signed char   , signed char    );
BLAZE_CREATE_BUILTIN_LOWTYPE_SPECIALIZATION( std::ptrdiff_t, wchar_t       , wchar_t        );
BLAZE_CREATE_BUILTIN_LOWTYPE_SPECIALIZATION( std::ptrdiff_t, unsigned short, unsigned short );
BLAZE_CREATE_BUILTIN_LOWTYPE_SPECIALIZATION( std::ptrdiff_t, short         , short          );
BLAZE_CREATE_BUILTIN_LOWTYPE_SPECIALIZATION( std::ptrdiff_t, unsigned int  , unsigned int   );
BLAZE_CREATE_BUILTIN_LOWTYPE_SPECIALIZATION( std::ptrdiff_t, int           , int            );
BLAZE_CREATE_BUILTIN_LOWTYPE_SPECIALIZATION( std::ptrdiff_t, unsigned long , unsigned long  );
BLAZE_CREATE_BUILTIN_LOWTYPE_SPECIALIZATION( std::ptrdiff_t, long          , long           );
BLAZE_CREATE_BUILTIN_LOWTYPE_SPECIALIZATION( std::ptrdiff_t, std::size_t   , std::size_t    );
BLAZE_CREATE_BUILTIN_LOWTYPE_SPECIALIZATION( std::ptrdiff_t, std::ptrdiff_t, std::ptrdiff_t );
BLAZE_CREATE_BUILTIN_LOWTYPE_SPECIALIZATION( std::ptrdiff_t, float         , std::ptrdiff_t );
BLAZE_CREATE_BUILTIN_LOWTYPE_SPECIALIZATION( std::ptrdiff_t, double        , std::ptrdiff_t );
BLAZE_CREATE_BUILTIN_LOWTYPE_SPECIALIZATION( std::ptrdiff_t, long double   , std::ptrdiff_t );
/*! \endcond */
#endif
//*************************************************************************************************




//=================================================================================================
//
//  FLOAT SPECIALIZATIONS
//
//=================================================================================================

//*************************************************************************************************
/*! \cond BLAZE_INTERNAL */
//                                           Type 1          Type 2          Low type
BLAZE_CREATE_BUILTIN_LOWTYPE_SPECIALIZATION( float         , unsigned char , unsigned char  );
BLAZE_CREATE_BUILTIN_LOWTYPE_SPECIALIZATION( float         , char          , char           );
BLAZE_CREATE_BUILTIN_LOWTYPE_SPECIALIZATION( float         , signed char   , signed char    );
BLAZE_CREATE_BUILTIN_LOWTYPE_SPECIALIZATION( float         , wchar_t       , wchar_t        );
BLAZE_CREATE_BUILTIN_LOWTYPE_SPECIALIZATION( float         , unsigned short, unsigned short );
BLAZE_CREATE_BUILTIN_LOWTYPE_SPECIALIZATION( float         , short         , short          );
BLAZE_CREATE_BUILTIN_LOWTYPE_SPECIALIZATION( float         , unsigned int  , unsigned int   );
BLAZE_CREATE_BUILTIN_LOWTYPE_SPECIALIZATION( float         , int           , int            );
BLAZE_CREATE_BUILTIN_LOWTYPE_SPECIALIZATION( float         , unsigned long , unsigned long  );
BLAZE_CREATE_BUILTIN_LOWTYPE_SPECIALIZATION( float         , long          , long           );
#if defined(_WIN64)
BLAZE_CREATE_BUILTIN_LOWTYPE_SPECIALIZATION( float         , std::size_t   , std::size_t    );
BLAZE_CREATE_BUILTIN_LOWTYPE_SPECIALIZATION( float         , std::ptrdiff_t, std::ptrdiff_t );
#endif
BLAZE_CREATE_BUILTIN_LOWTYPE_SPECIALIZATION( float         , float         , float          );
BLAZE_CREATE_BUILTIN_LOWTYPE_SPECIALIZATION( float         , double        , float          );
BLAZE_CREATE_BUILTIN_LOWTYPE_SPECIALIZATION( float         , long double   , float          );
/*! \endcond */
//*************************************************************************************************




//=================================================================================================
//
//  DOUBLE SPECIALIZATIONS
//
//=================================================================================================

//*************************************************************************************************
/*! \cond BLAZE_INTERNAL */
//                                           Type 1          Type 2          Low type
BLAZE_CREATE_BUILTIN_LOWTYPE_SPECIALIZATION( double        , unsigned char , unsigned char  );
BLAZE_CREATE_BUILTIN_LOWTYPE_SPECIALIZATION( double        , char          , char           );
BLAZE_CREATE_BUILTIN_LOWTYPE_SPECIALIZATION( double        , signed char   , signed char    );
BLAZE_CREATE_BUILTIN_LOWTYPE_SPECIALIZATION( double        , wchar_t       , wchar_t        );
BLAZE_CREATE_BUILTIN_LOWTYPE_SPECIALIZATION( double        , unsigned short, unsigned short );
BLAZE_CREATE_BUILTIN_LOWTYPE_SPECIALIZATION( double        , short         , short          );
BLAZE_CREATE_BUILTIN_LOWTYPE_SPECIALIZATION( double        , unsigned int  , unsigned int   );
BLAZE_CREATE_BUILTIN_LOWTYPE_SPECIALIZATION( double        , int           , int            );
BLAZE_CREATE_BUILTIN_LOWTYPE_SPECIALIZATION( double        , unsigned long , unsigned long  );
BLAZE_CREATE_BUILTIN_LOWTYPE_SPECIALIZATION( double        , long          , long           );
#if defined(_WIN64)
BLAZE_CREATE_BUILTIN_LOWTYPE_SPECIALIZATION( double        , std::size_t   , std::size_t    );
BLAZE_CREATE_BUILTIN_LOWTYPE_SPECIALIZATION( double        , std::ptrdiff_t, std::ptrdiff_t );
#endif
BLAZE_CREATE_BUILTIN_LOWTYPE_SPECIALIZATION( double        , float         , float          );
BLAZE_CREATE_BUILTIN_LOWTYPE_SPECIALIZATION( double        , double        , double         );
BLAZE_CREATE_BUILTIN_LOWTYPE_SPECIALIZATION( double        , long double   , double         );
/*! \endcond */
//*************************************************************************************************




//=================================================================================================
//
//  LONG DOUBLE SPECIALIZATIONS
//
//=================================================================================================

//*************************************************************************************************
/*! \cond BLAZE_INTERNAL */
//                                           Type 1          Type 2          Low type
BLAZE_CREATE_BUILTIN_LOWTYPE_SPECIALIZATION( long double   , unsigned char , unsigned char  );
BLAZE_CREATE_BUILTIN_LOWTYPE_SPECIALIZATION( long double   , char          , char           );
BLAZE_CREATE_BUILTIN_LOWTYPE_SPECIALIZATION( long double   , signed char   , signed char    );
BLAZE_CREATE_BUILTIN_LOWTYPE_SPECIALIZATION( long double   , wchar_t       , wchar_t        );
BLAZE_CREATE_BUILTIN_LOWTYPE_SPECIALIZATION( long double   , unsigned short, unsigned short );
BLAZE_CREATE_BUILTIN_LOWTYPE_SPECIALIZATION( long double   , short         , short          );
BLAZE_CREATE_BUILTIN_LOWTYPE_SPECIALIZATION( long double   , unsigned int  , unsigned int   );
BLAZE_CREATE_BUILTIN_LOWTYPE_SPECIALIZATION( long double   , int           , int            );
BLAZE_CREATE_BUILTIN_LOWTYPE_SPECIALIZATION( long double   , unsigned long , unsigned long  );
BLAZE_CREATE_BUILTIN_LOWTYPE_SPECIALIZATION( long double   , long          , long           );
#if defined(_WIN64)
BLAZE_CREATE_BUILTIN_LOWTYPE_SPECIALIZATION( long double   , std::size_t   , std::size_t    );
BLAZE_CREATE_BUILTIN_LOWTYPE_SPECIALIZATION( long double   , std::ptrdiff_t, std::ptrdiff_t );
#endif
BLAZE_CREATE_BUILTIN_LOWTYPE_SPECIALIZATION( long double   , float         , float          );
BLAZE_CREATE_BUILTIN_LOWTYPE_SPECIALIZATION( long double   , double        , double         );
BLAZE_CREATE_BUILTIN_LOWTYPE_SPECIALIZATION( long double   , long double   , long double    );
/*! \endcond */
//*************************************************************************************************




//=================================================================================================
//
//  COMPLEX SPECIALIZATIONS
//
//=================================================================================================

//*************************************************************************************************
/*! \cond BLAZE_INTERNAL */
BLAZE_CREATE_COMPLEX_LOWTYPE_SPECIALIZATION( unsigned char  );
BLAZE_CREATE_COMPLEX_LOWTYPE_SPECIALIZATION( char           );
BLAZE_CREATE_COMPLEX_LOWTYPE_SPECIALIZATION( signed char    );
BLAZE_CREATE_COMPLEX_LOWTYPE_SPECIALIZATION( wchar_t        );
BLAZE_CREATE_COMPLEX_LOWTYPE_SPECIALIZATION( unsigned short );
BLAZE_CREATE_COMPLEX_LOWTYPE_SPECIALIZATION( short          );
BLAZE_CREATE_COMPLEX_LOWTYPE_SPECIALIZATION( unsigned int   );
BLAZE_CREATE_COMPLEX_LOWTYPE_SPECIALIZATION( int            );
BLAZE_CREATE_COMPLEX_LOWTYPE_SPECIALIZATION( unsigned long  );
BLAZE_CREATE_COMPLEX_LOWTYPE_SPECIALIZATION( long           );
#if defined(_WIN64)
BLAZE_CREATE_COMPLEX_LOWTYPE_SPECIALIZATION( std::size_t    );
BLAZE_CREATE_COMPLEX_LOWTYPE_SPECIALIZATION( std::ptrdiff_t );
#endif
BLAZE_CREATE_COMPLEX_LOWTYPE_SPECIALIZATION( float          );
BLAZE_CREATE_COMPLEX_LOWTYPE_SPECIALIZATION( double         );
BLAZE_CREATE_COMPLEX_LOWTYPE_SPECIALIZATION( long double    );
/*! \endcond */
//*************************************************************************************************


//*************************************************************************************************
/*! \cond BLAZE_INTERNAL */
template< typename T >
struct LowType< complex<T>, complex<T> >
{
   using Type = complex< Decay_<T> >;
};
/*! \endcond */
//*************************************************************************************************


//*************************************************************************************************
/*! \cond BLAZE_INTERNAL */
template< typename T1, typename T2 >
struct LowType< complex<T1>, complex<T2> >
{
   using Type = complex< typename LowType<T1,T2>::Type >;
};
/*! \endcond */
//*************************************************************************************************

} // namespace blaze

#endif
