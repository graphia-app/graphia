//=================================================================================================
/*!
//  \file blaze/math/typetraits/HighType.h
//  \brief Header file for the HighType type trait
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

#ifndef _BLAZE_MATH_TYPETRAITS_HIGHTYPE_H_
#define _BLAZE_MATH_TYPETRAITS_HIGHTYPE_H_


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
/*!\brief Base template for the HighType type trait.
// \ingroup math_traits
//
// \section hightype_general General
//
// The HighType class template determines the more significant, dominating data type of the two
// given data types \a T1 and \a T2. For instance, in case both \a T1 and \a T2 are built-in data
// types, the nested type \a Type is set to the larger or signed data type. In case no higher
// data type can be selected, \a Type is set to \a INVALID_TYPE. Note that cv-qualifiers and
// reference modifiers are generally ignored.
//
// Per default, the HighType template provides specializations for the following built-in data
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
// arithmetic types, wherever a more significant data type can be selected:
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
// \n \section hightype_specializations Creating custom specializations
//
// It is possible to specialize the HighType template for additional user-defined data types.
// The following example shows the according specialization for two dynamic column vectors:

   \code
   template< typename T1, typename T2 >
   struct HighType< DynamicVector<T1,false>, DynamicVector<T2,false> >
   {
      typedef DynamicVector< typename HighType<T1,T2>::Type, false >  Type;
   };
   \endcode
*/
template< typename T1, typename T2 >
struct HighType
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
   using Helper = HighType< Decay_<T1>, Decay_<T2> >;
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
/*!\brief Auxiliary alias declaration for the HighType type trait.
// \ingroup type_traits
//
// The HighType_ alias declaration provides a convenient shortcut to access the nested \a Type of
// the HighType class template. For instance, given the types \a T1 and \a T2 the following two
// type definitions are identical:

   \code
   using Type1 = typename HighType<T1,T2>::Type;
   using Type2 = HighType_<T1,T2>;
   \endcode
*/
template< typename T1, typename T2 >
using HighType_ = typename HighType<T1,T2>::Type;
//*************************************************************************************************




//=================================================================================================
//
//  HIGHTYPE SPECIALIZATION FOR IDENTICAL TYPES
//
//=================================================================================================

//*************************************************************************************************
/*! \cond BLAZE_INTERNAL */
/*!\brief Specialization for two identical types.
// \ingroup math_traits
//
// This specialization of the HighType class template handles the special case that the two
// given types are identical. In this case, the nested types \a HighType and \a LowType are
// set to the given type \a T (ignoring \a const and \a volatile qualifiers and reference
// modifiers).
*/
template< typename T >
struct HighType<T,T>
{
   //**********************************************************************************************
   using Type = Decay_<T>;
   //**********************************************************************************************
};
/*! \endcond */
//*************************************************************************************************




//=================================================================================================
//
//  HIGHTYPE SPECIALIZATION MACRO
//
//=================================================================================================

//*************************************************************************************************
/*! \cond BLAZE_INTERNAL */
/*!\brief Macro for the creation of HighType specializations for the built-in data types.
// \ingroup math_traits
//
// This macro is used for the setup of the HighType specializations for the built-in data types.
*/
#define BLAZE_CREATE_BUILTIN_HIGHTYPE_SPECIALIZATION(T1,T2,HIGH) \
   template<> \
   struct HighType< T1, T2 > \
   { \
      using Type = HIGH; \
   }
/*! \endcond */
//*************************************************************************************************


//*************************************************************************************************
/*! \cond BLAZE_INTERNAL */
/*!\brief Macro for the creation of HighType specializations for the complex data type.
// \ingroup math_traits
//
// This macro is used for the setup of the HighType specializations for the complex data type.
*/
#define BLAZE_CREATE_COMPLEX_HIGHTYPE_SPECIALIZATION( T1 ) \
   template< typename T2 > \
   struct HighType< T1, complex<T2> > \
   { \
      using Type = complex<T2>; \
   }; \
   template< typename T2 > \
   struct HighType< complex<T2>, T1 > \
   { \
      using Type = complex<T2>; \
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
//                                            Type 1          Type 2          High type
BLAZE_CREATE_BUILTIN_HIGHTYPE_SPECIALIZATION( unsigned char , unsigned char , unsigned char  );
BLAZE_CREATE_BUILTIN_HIGHTYPE_SPECIALIZATION( unsigned char , char          , char           );
BLAZE_CREATE_BUILTIN_HIGHTYPE_SPECIALIZATION( unsigned char , signed char   , signed char    );
BLAZE_CREATE_BUILTIN_HIGHTYPE_SPECIALIZATION( unsigned char , wchar_t       , wchar_t        );
BLAZE_CREATE_BUILTIN_HIGHTYPE_SPECIALIZATION( unsigned char , unsigned short, unsigned short );
BLAZE_CREATE_BUILTIN_HIGHTYPE_SPECIALIZATION( unsigned char , short         , short          );
BLAZE_CREATE_BUILTIN_HIGHTYPE_SPECIALIZATION( unsigned char , unsigned int  , unsigned int   );
BLAZE_CREATE_BUILTIN_HIGHTYPE_SPECIALIZATION( unsigned char , int           , int            );
BLAZE_CREATE_BUILTIN_HIGHTYPE_SPECIALIZATION( unsigned char , unsigned long , unsigned long  );
BLAZE_CREATE_BUILTIN_HIGHTYPE_SPECIALIZATION( unsigned char , long          , long           );
#if defined(_WIN64)
BLAZE_CREATE_BUILTIN_HIGHTYPE_SPECIALIZATION( unsigned char , std::size_t   , std::size_t    );
BLAZE_CREATE_BUILTIN_HIGHTYPE_SPECIALIZATION( unsigned char , std::ptrdiff_t, std::ptrdiff_t );
#endif
BLAZE_CREATE_BUILTIN_HIGHTYPE_SPECIALIZATION( unsigned char , float         , float          );
BLAZE_CREATE_BUILTIN_HIGHTYPE_SPECIALIZATION( unsigned char , double        , double         );
BLAZE_CREATE_BUILTIN_HIGHTYPE_SPECIALIZATION( unsigned char , long double   , long double    );
/*! \endcond */
//*************************************************************************************************




//=================================================================================================
//
//  CHAR SPECIALIZATIONS
//
//=================================================================================================

//*************************************************************************************************
/*! \cond BLAZE_INTERNAL */
//                                            Type 1          Type 2          High type
BLAZE_CREATE_BUILTIN_HIGHTYPE_SPECIALIZATION( char          , unsigned char , char           );
BLAZE_CREATE_BUILTIN_HIGHTYPE_SPECIALIZATION( char          , char          , char           );
BLAZE_CREATE_BUILTIN_HIGHTYPE_SPECIALIZATION( char          , signed char   , signed char    );
BLAZE_CREATE_BUILTIN_HIGHTYPE_SPECIALIZATION( char          , wchar_t       , wchar_t        );
BLAZE_CREATE_BUILTIN_HIGHTYPE_SPECIALIZATION( char          , unsigned short, unsigned short );
BLAZE_CREATE_BUILTIN_HIGHTYPE_SPECIALIZATION( char          , short         , short          );
BLAZE_CREATE_BUILTIN_HIGHTYPE_SPECIALIZATION( char          , unsigned int  , unsigned int   );
BLAZE_CREATE_BUILTIN_HIGHTYPE_SPECIALIZATION( char          , int           , int            );
BLAZE_CREATE_BUILTIN_HIGHTYPE_SPECIALIZATION( char          , unsigned long , unsigned long  );
BLAZE_CREATE_BUILTIN_HIGHTYPE_SPECIALIZATION( char          , long          , long           );
#if defined(_WIN64)
BLAZE_CREATE_BUILTIN_HIGHTYPE_SPECIALIZATION( char          , std::size_t   , std::size_t    );
BLAZE_CREATE_BUILTIN_HIGHTYPE_SPECIALIZATION( char          , std::ptrdiff_t, std::ptrdiff_t );
#endif
BLAZE_CREATE_BUILTIN_HIGHTYPE_SPECIALIZATION( char          , float         , float          );
BLAZE_CREATE_BUILTIN_HIGHTYPE_SPECIALIZATION( char          , double        , double         );
BLAZE_CREATE_BUILTIN_HIGHTYPE_SPECIALIZATION( char          , long double   , long double    );
/*! \endcond */
//*************************************************************************************************




//=================================================================================================
//
//  SIGNED CHAR SPECIALIZATIONS
//
//=================================================================================================

//*************************************************************************************************
/*! \cond BLAZE_INTERNAL */
//                                            Type 1          Type 2          High type
BLAZE_CREATE_BUILTIN_HIGHTYPE_SPECIALIZATION( signed char   , unsigned char , signed char    );
BLAZE_CREATE_BUILTIN_HIGHTYPE_SPECIALIZATION( signed char   , char          , signed char    );
BLAZE_CREATE_BUILTIN_HIGHTYPE_SPECIALIZATION( signed char   , signed char   , signed char    );
BLAZE_CREATE_BUILTIN_HIGHTYPE_SPECIALIZATION( signed char   , wchar_t       , wchar_t        );
BLAZE_CREATE_BUILTIN_HIGHTYPE_SPECIALIZATION( signed char   , unsigned short, unsigned short );
BLAZE_CREATE_BUILTIN_HIGHTYPE_SPECIALIZATION( signed char   , short         , short          );
BLAZE_CREATE_BUILTIN_HIGHTYPE_SPECIALIZATION( signed char   , unsigned int  , unsigned int   );
BLAZE_CREATE_BUILTIN_HIGHTYPE_SPECIALIZATION( signed char   , int           , int            );
BLAZE_CREATE_BUILTIN_HIGHTYPE_SPECIALIZATION( signed char   , unsigned long , unsigned long  );
BLAZE_CREATE_BUILTIN_HIGHTYPE_SPECIALIZATION( signed char   , long          , long           );
#if defined(_WIN64)
BLAZE_CREATE_BUILTIN_HIGHTYPE_SPECIALIZATION( signed char   , std::size_t   , std::size_t    );
BLAZE_CREATE_BUILTIN_HIGHTYPE_SPECIALIZATION( signed char   , std::ptrdiff_t, std::ptrdiff_t );
#endif
BLAZE_CREATE_BUILTIN_HIGHTYPE_SPECIALIZATION( signed char   , float         , float          );
BLAZE_CREATE_BUILTIN_HIGHTYPE_SPECIALIZATION( signed char   , double        , double         );
BLAZE_CREATE_BUILTIN_HIGHTYPE_SPECIALIZATION( signed char   , long double   , long double    );
/*! \endcond */
//*************************************************************************************************




//=================================================================================================
//
//  WCHAR_T SPECIALIZATIONS
//
//=================================================================================================

//*************************************************************************************************
/*! \cond BLAZE_INTERNAL */
//                                            Type 1          Type 2          High type
BLAZE_CREATE_BUILTIN_HIGHTYPE_SPECIALIZATION( wchar_t       , unsigned char , wchar_t        );
BLAZE_CREATE_BUILTIN_HIGHTYPE_SPECIALIZATION( wchar_t       , char          , wchar_t        );
BLAZE_CREATE_BUILTIN_HIGHTYPE_SPECIALIZATION( wchar_t       , signed char   , wchar_t        );
BLAZE_CREATE_BUILTIN_HIGHTYPE_SPECIALIZATION( wchar_t       , wchar_t       , wchar_t        );
BLAZE_CREATE_BUILTIN_HIGHTYPE_SPECIALIZATION( wchar_t       , unsigned short, unsigned short );
BLAZE_CREATE_BUILTIN_HIGHTYPE_SPECIALIZATION( wchar_t       , short         , short          );
BLAZE_CREATE_BUILTIN_HIGHTYPE_SPECIALIZATION( wchar_t       , unsigned int  , unsigned int   );
BLAZE_CREATE_BUILTIN_HIGHTYPE_SPECIALIZATION( wchar_t       , int           , int            );
BLAZE_CREATE_BUILTIN_HIGHTYPE_SPECIALIZATION( wchar_t       , unsigned long , unsigned long  );
BLAZE_CREATE_BUILTIN_HIGHTYPE_SPECIALIZATION( wchar_t       , long          , long           );
#if defined(_WIN64)
BLAZE_CREATE_BUILTIN_HIGHTYPE_SPECIALIZATION( wchar_t       , std::size_t   , std::size_t    );
BLAZE_CREATE_BUILTIN_HIGHTYPE_SPECIALIZATION( wchar_t       , std::ptrdiff_t, std::ptrdiff_t );
#endif
BLAZE_CREATE_BUILTIN_HIGHTYPE_SPECIALIZATION( wchar_t       , float         , float          );
BLAZE_CREATE_BUILTIN_HIGHTYPE_SPECIALIZATION( wchar_t       , double        , double         );
BLAZE_CREATE_BUILTIN_HIGHTYPE_SPECIALIZATION( wchar_t       , long double   , long double    );
/*! \endcond */
//*************************************************************************************************




//=================================================================================================
//
//  UNSIGNED SHORT SPECIALIZATIONS
//
//=================================================================================================

//*************************************************************************************************
/*! \cond BLAZE_INTERNAL */
//                                            Type 1          Type 2          High type
BLAZE_CREATE_BUILTIN_HIGHTYPE_SPECIALIZATION( unsigned short, unsigned char , unsigned short );
BLAZE_CREATE_BUILTIN_HIGHTYPE_SPECIALIZATION( unsigned short, char          , unsigned short );
BLAZE_CREATE_BUILTIN_HIGHTYPE_SPECIALIZATION( unsigned short, signed char   , unsigned short );
BLAZE_CREATE_BUILTIN_HIGHTYPE_SPECIALIZATION( unsigned short, wchar_t       , unsigned short );
BLAZE_CREATE_BUILTIN_HIGHTYPE_SPECIALIZATION( unsigned short, unsigned short, unsigned short );
BLAZE_CREATE_BUILTIN_HIGHTYPE_SPECIALIZATION( unsigned short, short         , short          );
BLAZE_CREATE_BUILTIN_HIGHTYPE_SPECIALIZATION( unsigned short, unsigned int  , unsigned int   );
BLAZE_CREATE_BUILTIN_HIGHTYPE_SPECIALIZATION( unsigned short, int           , int            );
BLAZE_CREATE_BUILTIN_HIGHTYPE_SPECIALIZATION( unsigned short, unsigned long , unsigned long  );
BLAZE_CREATE_BUILTIN_HIGHTYPE_SPECIALIZATION( unsigned short, long          , long           );
#if defined(_WIN64)
BLAZE_CREATE_BUILTIN_HIGHTYPE_SPECIALIZATION( unsigned short, std::size_t   , std::size_t    );
BLAZE_CREATE_BUILTIN_HIGHTYPE_SPECIALIZATION( unsigned short, std::ptrdiff_t, std::ptrdiff_t );
#endif
BLAZE_CREATE_BUILTIN_HIGHTYPE_SPECIALIZATION( unsigned short, float         , float          );
BLAZE_CREATE_BUILTIN_HIGHTYPE_SPECIALIZATION( unsigned short, double        , double         );
BLAZE_CREATE_BUILTIN_HIGHTYPE_SPECIALIZATION( unsigned short, long double   , long double    );
/*! \endcond */
//*************************************************************************************************




//=================================================================================================
//
//  SHORT SPECIALIZATIONS
//
//=================================================================================================

//*************************************************************************************************
/*! \cond BLAZE_INTERNAL */
//                                            Type 1          Type 2          High type
BLAZE_CREATE_BUILTIN_HIGHTYPE_SPECIALIZATION( short         , unsigned char , short          );
BLAZE_CREATE_BUILTIN_HIGHTYPE_SPECIALIZATION( short         , char          , short          );
BLAZE_CREATE_BUILTIN_HIGHTYPE_SPECIALIZATION( short         , signed char   , short          );
BLAZE_CREATE_BUILTIN_HIGHTYPE_SPECIALIZATION( short         , wchar_t       , short          );
BLAZE_CREATE_BUILTIN_HIGHTYPE_SPECIALIZATION( short         , unsigned short, short          );
BLAZE_CREATE_BUILTIN_HIGHTYPE_SPECIALIZATION( short         , short         , short          );
BLAZE_CREATE_BUILTIN_HIGHTYPE_SPECIALIZATION( short         , unsigned int  , unsigned int   );
BLAZE_CREATE_BUILTIN_HIGHTYPE_SPECIALIZATION( short         , int           , int            );
BLAZE_CREATE_BUILTIN_HIGHTYPE_SPECIALIZATION( short         , unsigned long , unsigned long  );
BLAZE_CREATE_BUILTIN_HIGHTYPE_SPECIALIZATION( short         , long          , long           );
#if defined(_WIN64)
BLAZE_CREATE_BUILTIN_HIGHTYPE_SPECIALIZATION( short         , std::size_t   , std::size_t    );
BLAZE_CREATE_BUILTIN_HIGHTYPE_SPECIALIZATION( short         , std::ptrdiff_t, std::ptrdiff_t );
#endif
BLAZE_CREATE_BUILTIN_HIGHTYPE_SPECIALIZATION( short         , float         , float          );
BLAZE_CREATE_BUILTIN_HIGHTYPE_SPECIALIZATION( short         , double        , double         );
BLAZE_CREATE_BUILTIN_HIGHTYPE_SPECIALIZATION( short         , long double   , long double    );
/*! \endcond */
//*************************************************************************************************




//=================================================================================================
//
//  UNSIGNED INT SPECIALIZATIONS
//
//=================================================================================================

//*************************************************************************************************
/*! \cond BLAZE_INTERNAL */
//                                            Type 1          Type 2          High type
BLAZE_CREATE_BUILTIN_HIGHTYPE_SPECIALIZATION( unsigned int  , unsigned char , unsigned int   );
BLAZE_CREATE_BUILTIN_HIGHTYPE_SPECIALIZATION( unsigned int  , char          , unsigned int   );
BLAZE_CREATE_BUILTIN_HIGHTYPE_SPECIALIZATION( unsigned int  , signed char   , unsigned int   );
BLAZE_CREATE_BUILTIN_HIGHTYPE_SPECIALIZATION( unsigned int  , wchar_t       , unsigned int   );
BLAZE_CREATE_BUILTIN_HIGHTYPE_SPECIALIZATION( unsigned int  , unsigned short, unsigned int   );
BLAZE_CREATE_BUILTIN_HIGHTYPE_SPECIALIZATION( unsigned int  , short         , unsigned int   );
BLAZE_CREATE_BUILTIN_HIGHTYPE_SPECIALIZATION( unsigned int  , unsigned int  , unsigned int   );
BLAZE_CREATE_BUILTIN_HIGHTYPE_SPECIALIZATION( unsigned int  , int           , int            );
BLAZE_CREATE_BUILTIN_HIGHTYPE_SPECIALIZATION( unsigned int  , unsigned long , unsigned long  );
BLAZE_CREATE_BUILTIN_HIGHTYPE_SPECIALIZATION( unsigned int  , long          , long           );
#if defined(_WIN64)
BLAZE_CREATE_BUILTIN_HIGHTYPE_SPECIALIZATION( unsigned int  , std::size_t   , std::size_t    );
BLAZE_CREATE_BUILTIN_HIGHTYPE_SPECIALIZATION( unsigned int  , std::ptrdiff_t, std::ptrdiff_t );
#endif
BLAZE_CREATE_BUILTIN_HIGHTYPE_SPECIALIZATION( unsigned int  , float         , float          );
BLAZE_CREATE_BUILTIN_HIGHTYPE_SPECIALIZATION( unsigned int  , double        , double         );
BLAZE_CREATE_BUILTIN_HIGHTYPE_SPECIALIZATION( unsigned int  , long double   , long double    );
/*! \endcond */
//*************************************************************************************************




//=================================================================================================
//
//  INT SPECIALIZATIONS
//
//=================================================================================================

//*************************************************************************************************
/*! \cond BLAZE_INTERNAL */
//                                            Type 1          Type 2          High type
BLAZE_CREATE_BUILTIN_HIGHTYPE_SPECIALIZATION( int           , unsigned char , int            );
BLAZE_CREATE_BUILTIN_HIGHTYPE_SPECIALIZATION( int           , char          , int            );
BLAZE_CREATE_BUILTIN_HIGHTYPE_SPECIALIZATION( int           , signed char   , int            );
BLAZE_CREATE_BUILTIN_HIGHTYPE_SPECIALIZATION( int           , wchar_t       , int            );
BLAZE_CREATE_BUILTIN_HIGHTYPE_SPECIALIZATION( int           , unsigned short, int            );
BLAZE_CREATE_BUILTIN_HIGHTYPE_SPECIALIZATION( int           , short         , int            );
BLAZE_CREATE_BUILTIN_HIGHTYPE_SPECIALIZATION( int           , unsigned int  , int            );
BLAZE_CREATE_BUILTIN_HIGHTYPE_SPECIALIZATION( int           , int           , int            );
BLAZE_CREATE_BUILTIN_HIGHTYPE_SPECIALIZATION( int           , unsigned long , unsigned long  );
BLAZE_CREATE_BUILTIN_HIGHTYPE_SPECIALIZATION( int           , long          , long           );
#if defined(_WIN64)
BLAZE_CREATE_BUILTIN_HIGHTYPE_SPECIALIZATION( int           , std::size_t   , std::size_t    );
BLAZE_CREATE_BUILTIN_HIGHTYPE_SPECIALIZATION( int           , std::ptrdiff_t, std::ptrdiff_t );
#endif
BLAZE_CREATE_BUILTIN_HIGHTYPE_SPECIALIZATION( int           , float         , float          );
BLAZE_CREATE_BUILTIN_HIGHTYPE_SPECIALIZATION( int           , double        , double         );
BLAZE_CREATE_BUILTIN_HIGHTYPE_SPECIALIZATION( int           , long double   , long double    );
/*! \endcond */
//*************************************************************************************************




//=================================================================================================
//
//  UNSIGNED LONG SPECIALIZATIONS
//
//=================================================================================================

//*************************************************************************************************
/*! \cond BLAZE_INTERNAL */
//                                            Type 1          Type 2          High type
BLAZE_CREATE_BUILTIN_HIGHTYPE_SPECIALIZATION( unsigned long , unsigned char , unsigned long  );
BLAZE_CREATE_BUILTIN_HIGHTYPE_SPECIALIZATION( unsigned long , char          , unsigned long  );
BLAZE_CREATE_BUILTIN_HIGHTYPE_SPECIALIZATION( unsigned long , signed char   , unsigned long  );
BLAZE_CREATE_BUILTIN_HIGHTYPE_SPECIALIZATION( unsigned long , wchar_t       , unsigned long  );
BLAZE_CREATE_BUILTIN_HIGHTYPE_SPECIALIZATION( unsigned long , unsigned short, unsigned long  );
BLAZE_CREATE_BUILTIN_HIGHTYPE_SPECIALIZATION( unsigned long , short         , unsigned long  );
BLAZE_CREATE_BUILTIN_HIGHTYPE_SPECIALIZATION( unsigned long , unsigned int  , unsigned long  );
BLAZE_CREATE_BUILTIN_HIGHTYPE_SPECIALIZATION( unsigned long , int           , unsigned long  );
BLAZE_CREATE_BUILTIN_HIGHTYPE_SPECIALIZATION( unsigned long , unsigned long , unsigned long  );
BLAZE_CREATE_BUILTIN_HIGHTYPE_SPECIALIZATION( unsigned long , long          , long           );
#if defined(_WIN64)
BLAZE_CREATE_BUILTIN_HIGHTYPE_SPECIALIZATION( unsigned long , std::size_t   , std::size_t    );
BLAZE_CREATE_BUILTIN_HIGHTYPE_SPECIALIZATION( unsigned long , std::ptrdiff_t, std::ptrdiff_t );
#endif
BLAZE_CREATE_BUILTIN_HIGHTYPE_SPECIALIZATION( unsigned long , float         , float          );
BLAZE_CREATE_BUILTIN_HIGHTYPE_SPECIALIZATION( unsigned long , double        , double         );
BLAZE_CREATE_BUILTIN_HIGHTYPE_SPECIALIZATION( unsigned long , long double   , long double    );
/*! \endcond */
//*************************************************************************************************




//=================================================================================================
//
//  LONG SPECIALIZATIONS
//
//=================================================================================================

//*************************************************************************************************
/*! \cond BLAZE_INTERNAL */
//                                            Type 1          Type 2          High type
BLAZE_CREATE_BUILTIN_HIGHTYPE_SPECIALIZATION( long          , unsigned char , long           );
BLAZE_CREATE_BUILTIN_HIGHTYPE_SPECIALIZATION( long          , char          , long           );
BLAZE_CREATE_BUILTIN_HIGHTYPE_SPECIALIZATION( long          , signed char   , long           );
BLAZE_CREATE_BUILTIN_HIGHTYPE_SPECIALIZATION( long          , wchar_t       , long           );
BLAZE_CREATE_BUILTIN_HIGHTYPE_SPECIALIZATION( long          , unsigned short, long           );
BLAZE_CREATE_BUILTIN_HIGHTYPE_SPECIALIZATION( long          , short         , long           );
BLAZE_CREATE_BUILTIN_HIGHTYPE_SPECIALIZATION( long          , unsigned int  , long           );
BLAZE_CREATE_BUILTIN_HIGHTYPE_SPECIALIZATION( long          , int           , long           );
BLAZE_CREATE_BUILTIN_HIGHTYPE_SPECIALIZATION( long          , unsigned long , long           );
BLAZE_CREATE_BUILTIN_HIGHTYPE_SPECIALIZATION( long          , long          , long           );
#if defined(_WIN64)
BLAZE_CREATE_BUILTIN_HIGHTYPE_SPECIALIZATION( long          , std::size_t   , std::size_t    );
BLAZE_CREATE_BUILTIN_HIGHTYPE_SPECIALIZATION( long          , std::ptrdiff_t, std::ptrdiff_t );
#endif
BLAZE_CREATE_BUILTIN_HIGHTYPE_SPECIALIZATION( long          , float         , float          );
BLAZE_CREATE_BUILTIN_HIGHTYPE_SPECIALIZATION( long          , double        , double         );
BLAZE_CREATE_BUILTIN_HIGHTYPE_SPECIALIZATION( long          , long double   , long double    );
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
//                                            Type 1          Type 2          High type
BLAZE_CREATE_BUILTIN_HIGHTYPE_SPECIALIZATION( std::size_t   , unsigned char , std::size_t    );
BLAZE_CREATE_BUILTIN_HIGHTYPE_SPECIALIZATION( std::size_t   , char          , std::size_t    );
BLAZE_CREATE_BUILTIN_HIGHTYPE_SPECIALIZATION( std::size_t   , signed char   , std::size_t    );
BLAZE_CREATE_BUILTIN_HIGHTYPE_SPECIALIZATION( std::size_t   , wchar_t       , std::size_t    );
BLAZE_CREATE_BUILTIN_HIGHTYPE_SPECIALIZATION( std::size_t   , unsigned short, std::size_t    );
BLAZE_CREATE_BUILTIN_HIGHTYPE_SPECIALIZATION( std::size_t   , short         , std::size_t    );
BLAZE_CREATE_BUILTIN_HIGHTYPE_SPECIALIZATION( std::size_t   , unsigned int  , std::size_t    );
BLAZE_CREATE_BUILTIN_HIGHTYPE_SPECIALIZATION( std::size_t   , int           , std::size_t    );
BLAZE_CREATE_BUILTIN_HIGHTYPE_SPECIALIZATION( std::size_t   , unsigned long , std::size_t    );
BLAZE_CREATE_BUILTIN_HIGHTYPE_SPECIALIZATION( std::size_t   , long          , std::size_t    );
BLAZE_CREATE_BUILTIN_HIGHTYPE_SPECIALIZATION( std::size_t   , std::size_t   , std::size_t    );
BLAZE_CREATE_BUILTIN_HIGHTYPE_SPECIALIZATION( std::size_t   , std::ptrdiff_t, std::ptrdiff_t );
BLAZE_CREATE_BUILTIN_HIGHTYPE_SPECIALIZATION( std::size_t   , float         , float          );
BLAZE_CREATE_BUILTIN_HIGHTYPE_SPECIALIZATION( std::size_t   , double        , double         );
BLAZE_CREATE_BUILTIN_HIGHTYPE_SPECIALIZATION( std::size_t   , long double   , long double    );
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
//                                            Type 1          Type 2          High type
BLAZE_CREATE_BUILTIN_HIGHTYPE_SPECIALIZATION( std::ptrdiff_t, unsigned char , std::ptrdiff_t );
BLAZE_CREATE_BUILTIN_HIGHTYPE_SPECIALIZATION( std::ptrdiff_t, char          , std::ptrdiff_t );
BLAZE_CREATE_BUILTIN_HIGHTYPE_SPECIALIZATION( std::ptrdiff_t, signed char   , std::ptrdiff_t );
BLAZE_CREATE_BUILTIN_HIGHTYPE_SPECIALIZATION( std::ptrdiff_t, wchar_t       , std::ptrdiff_t );
BLAZE_CREATE_BUILTIN_HIGHTYPE_SPECIALIZATION( std::ptrdiff_t, unsigned short, std::ptrdiff_t );
BLAZE_CREATE_BUILTIN_HIGHTYPE_SPECIALIZATION( std::ptrdiff_t, short         , std::ptrdiff_t );
BLAZE_CREATE_BUILTIN_HIGHTYPE_SPECIALIZATION( std::ptrdiff_t, unsigned int  , std::ptrdiff_t );
BLAZE_CREATE_BUILTIN_HIGHTYPE_SPECIALIZATION( std::ptrdiff_t, int           , std::ptrdiff_t );
BLAZE_CREATE_BUILTIN_HIGHTYPE_SPECIALIZATION( std::ptrdiff_t, unsigned long , std::ptrdiff_t );
BLAZE_CREATE_BUILTIN_HIGHTYPE_SPECIALIZATION( std::ptrdiff_t, long          , std::ptrdiff_t );
BLAZE_CREATE_BUILTIN_HIGHTYPE_SPECIALIZATION( std::ptrdiff_t, std::size_t   , std::ptrdiff_t );
BLAZE_CREATE_BUILTIN_HIGHTYPE_SPECIALIZATION( std::ptrdiff_t, std::ptrdiff_t, std::ptrdiff_t );
BLAZE_CREATE_BUILTIN_HIGHTYPE_SPECIALIZATION( std::ptrdiff_t, float         , float          );
BLAZE_CREATE_BUILTIN_HIGHTYPE_SPECIALIZATION( std::ptrdiff_t, double        , double         );
BLAZE_CREATE_BUILTIN_HIGHTYPE_SPECIALIZATION( std::ptrdiff_t, long double   , long double    );
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
//                                            Type 1          Type 2          High type
BLAZE_CREATE_BUILTIN_HIGHTYPE_SPECIALIZATION( float         , unsigned char , float          );
BLAZE_CREATE_BUILTIN_HIGHTYPE_SPECIALIZATION( float         , char          , float          );
BLAZE_CREATE_BUILTIN_HIGHTYPE_SPECIALIZATION( float         , signed char   , float          );
BLAZE_CREATE_BUILTIN_HIGHTYPE_SPECIALIZATION( float         , wchar_t       , float          );
BLAZE_CREATE_BUILTIN_HIGHTYPE_SPECIALIZATION( float         , unsigned short, float          );
BLAZE_CREATE_BUILTIN_HIGHTYPE_SPECIALIZATION( float         , short         , float          );
BLAZE_CREATE_BUILTIN_HIGHTYPE_SPECIALIZATION( float         , unsigned int  , float          );
BLAZE_CREATE_BUILTIN_HIGHTYPE_SPECIALIZATION( float         , int           , float          );
BLAZE_CREATE_BUILTIN_HIGHTYPE_SPECIALIZATION( float         , unsigned long , float          );
BLAZE_CREATE_BUILTIN_HIGHTYPE_SPECIALIZATION( float         , long          , float          );
#if defined(_WIN64)
BLAZE_CREATE_BUILTIN_HIGHTYPE_SPECIALIZATION( float         , std::size_t   , float          );
BLAZE_CREATE_BUILTIN_HIGHTYPE_SPECIALIZATION( float         , std::ptrdiff_t, float          );
#endif
BLAZE_CREATE_BUILTIN_HIGHTYPE_SPECIALIZATION( float         , float         , float          );
BLAZE_CREATE_BUILTIN_HIGHTYPE_SPECIALIZATION( float         , double        , double         );
BLAZE_CREATE_BUILTIN_HIGHTYPE_SPECIALIZATION( float         , long double   , long double    );
/*! \endcond */
//*************************************************************************************************




//=================================================================================================
//
//  DOUBLE SPECIALIZATIONS
//
//=================================================================================================

//*************************************************************************************************
/*! \cond BLAZE_INTERNAL */
//                                            Type 1          Type 2          High type
BLAZE_CREATE_BUILTIN_HIGHTYPE_SPECIALIZATION( double        , unsigned char , double         );
BLAZE_CREATE_BUILTIN_HIGHTYPE_SPECIALIZATION( double        , char          , double         );
BLAZE_CREATE_BUILTIN_HIGHTYPE_SPECIALIZATION( double        , signed char   , double         );
BLAZE_CREATE_BUILTIN_HIGHTYPE_SPECIALIZATION( double        , wchar_t       , double         );
BLAZE_CREATE_BUILTIN_HIGHTYPE_SPECIALIZATION( double        , unsigned short, double         );
BLAZE_CREATE_BUILTIN_HIGHTYPE_SPECIALIZATION( double        , short         , double         );
BLAZE_CREATE_BUILTIN_HIGHTYPE_SPECIALIZATION( double        , unsigned int  , double         );
BLAZE_CREATE_BUILTIN_HIGHTYPE_SPECIALIZATION( double        , int           , double         );
BLAZE_CREATE_BUILTIN_HIGHTYPE_SPECIALIZATION( double        , unsigned long , double         );
BLAZE_CREATE_BUILTIN_HIGHTYPE_SPECIALIZATION( double        , long          , double         );
#if defined(_WIN64)
BLAZE_CREATE_BUILTIN_HIGHTYPE_SPECIALIZATION( double        , std::size_t   , double         );
BLAZE_CREATE_BUILTIN_HIGHTYPE_SPECIALIZATION( double        , std::ptrdiff_t, double         );
#endif
BLAZE_CREATE_BUILTIN_HIGHTYPE_SPECIALIZATION( double        , float         , double         );
BLAZE_CREATE_BUILTIN_HIGHTYPE_SPECIALIZATION( double        , double        , double         );
BLAZE_CREATE_BUILTIN_HIGHTYPE_SPECIALIZATION( double        , long double   , long double    );
/*! \endcond */
//*************************************************************************************************




//=================================================================================================
//
//  LONG DOUBLE SPECIALIZATIONS
//
//=================================================================================================

//*************************************************************************************************
/*! \cond BLAZE_INTERNAL */
//                                            Type 1          Type 2          High type
BLAZE_CREATE_BUILTIN_HIGHTYPE_SPECIALIZATION( long double   , unsigned char , long double    );
BLAZE_CREATE_BUILTIN_HIGHTYPE_SPECIALIZATION( long double   , char          , long double    );
BLAZE_CREATE_BUILTIN_HIGHTYPE_SPECIALIZATION( long double   , signed char   , long double    );
BLAZE_CREATE_BUILTIN_HIGHTYPE_SPECIALIZATION( long double   , wchar_t       , long double    );
BLAZE_CREATE_BUILTIN_HIGHTYPE_SPECIALIZATION( long double   , unsigned short, long double    );
BLAZE_CREATE_BUILTIN_HIGHTYPE_SPECIALIZATION( long double   , short         , long double    );
BLAZE_CREATE_BUILTIN_HIGHTYPE_SPECIALIZATION( long double   , unsigned int  , long double    );
BLAZE_CREATE_BUILTIN_HIGHTYPE_SPECIALIZATION( long double   , int           , long double    );
BLAZE_CREATE_BUILTIN_HIGHTYPE_SPECIALIZATION( long double   , unsigned long , long double    );
BLAZE_CREATE_BUILTIN_HIGHTYPE_SPECIALIZATION( long double   , long          , long double    );
#if defined(_WIN64)
BLAZE_CREATE_BUILTIN_HIGHTYPE_SPECIALIZATION( long double   , std::size_t   , long double    );
BLAZE_CREATE_BUILTIN_HIGHTYPE_SPECIALIZATION( long double   , std::ptrdiff_t, long double    );
#endif
BLAZE_CREATE_BUILTIN_HIGHTYPE_SPECIALIZATION( long double   , float         , long double    );
BLAZE_CREATE_BUILTIN_HIGHTYPE_SPECIALIZATION( long double   , double        , long double    );
BLAZE_CREATE_BUILTIN_HIGHTYPE_SPECIALIZATION( long double   , long double   , long double    );
/*! \endcond */
//*************************************************************************************************




//=================================================================================================
//
//  COMPLEX SPECIALIZATIONS
//
//=================================================================================================

//*************************************************************************************************
/*! \cond BLAZE_INTERNAL */
BLAZE_CREATE_COMPLEX_HIGHTYPE_SPECIALIZATION( unsigned char  );
BLAZE_CREATE_COMPLEX_HIGHTYPE_SPECIALIZATION( char           );
BLAZE_CREATE_COMPLEX_HIGHTYPE_SPECIALIZATION( signed char    );
BLAZE_CREATE_COMPLEX_HIGHTYPE_SPECIALIZATION( wchar_t        );
BLAZE_CREATE_COMPLEX_HIGHTYPE_SPECIALIZATION( unsigned short );
BLAZE_CREATE_COMPLEX_HIGHTYPE_SPECIALIZATION( short          );
BLAZE_CREATE_COMPLEX_HIGHTYPE_SPECIALIZATION( unsigned int   );
BLAZE_CREATE_COMPLEX_HIGHTYPE_SPECIALIZATION( int            );
BLAZE_CREATE_COMPLEX_HIGHTYPE_SPECIALIZATION( unsigned long  );
BLAZE_CREATE_COMPLEX_HIGHTYPE_SPECIALIZATION( long           );
#if defined(_WIN64)
BLAZE_CREATE_COMPLEX_HIGHTYPE_SPECIALIZATION( std::size_t    );
BLAZE_CREATE_COMPLEX_HIGHTYPE_SPECIALIZATION( std::ptrdiff_t );
#endif
BLAZE_CREATE_COMPLEX_HIGHTYPE_SPECIALIZATION( float          );
BLAZE_CREATE_COMPLEX_HIGHTYPE_SPECIALIZATION( double         );
BLAZE_CREATE_COMPLEX_HIGHTYPE_SPECIALIZATION( long double    );
/*! \endcond */
//*************************************************************************************************


//*************************************************************************************************
/*! \cond BLAZE_INTERNAL */
template< typename T >
struct HighType< complex<T>, complex<T> >
{
   using Type = complex< Decay_<T> >;
};
/*! \endcond */
//*************************************************************************************************


//*************************************************************************************************
/*! \cond BLAZE_INTERNAL */
template< typename T1, typename T2 >
struct HighType< complex<T1>, complex<T2> >
{
   using Type = complex< typename HighType<T1,T2>::Type >;
};
/*! \endcond */
//*************************************************************************************************

} // namespace blaze

#endif
