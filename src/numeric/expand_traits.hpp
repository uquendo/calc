#pragma once
#ifndef _EXPAND_TRAITS_HPP
#define _EXPAND_TRAITS_HPP

//TODO: replace macro with functor-based expansion

#ifdef HAVE_QUADMATH
# ifdef HAVE_BOOST
#   define _EXPAND_TRAITS_PRECISION_QUAD(_PRECISION_VAR_,_FUN_NAME_, ... ) _FUN_NAME_< numeric::TraitBuiltin< numeric::P_Quad >::type > ( __VA_ARGS__ )
# else
#   define _EXPAND_TRAITS_PRECISION_QUAD(_PRECISION_VAR_,_FUN_NAME_, ... )
# endif
#else
# define _EXPAND_TRAITS_PRECISION_QUAD(_PRECISION_VAR_,_FUN_NAME_, ... )
#endif

#ifdef HAVE_MPREAL
# define _EXPAND_TRAITS_PRECISION_MPFR(_PRECISION_VAR_,_FUN_NAME_, ... ) _FUN_NAME_< numeric::TraitBuiltin< numeric::P_MPFR >::type > ( __VA_ARGS__ )
#else
# define _EXPAND_TRAITS_PRECISION_MPFR(_PRECISION_VAR_,_FUN_NAME_, ... )
#endif

#define EXPAND_TRAITS_PRECISION(_PRECISION_VAR_,_FUN_NAME_, ... ) switch(_PRECISION_VAR_)\
{\
  case numeric::P_Float :\
    _FUN_NAME_< numeric::TraitBuiltin< numeric::P_Float >::type > ( __VA_ARGS__ );\
    break;\
  case numeric::P_Double :\
    _FUN_NAME_< numeric::TraitBuiltin< numeric::P_Double >::type > ( __VA_ARGS__ );\
    break;\
  case numeric::P_LongDouble :\
    _FUN_NAME_< numeric::TraitBuiltin< numeric::P_LongDouble >::type > ( __VA_ARGS__ );\
    break;\
  case numeric::P_Quad :\
    _EXPAND_TRAITS_PRECISION_QUAD( _PRECISION_VAR_ , _FUN_NAME_ , __VA_ARGS__ ) ;\
    break;\
  case numeric::P_MPFR :\
  _EXPAND_TRAITS_PRECISION_MPFR( _PRECISION_VAR_ , _FUN_NAME_ , __VA_ARGS__ ) ;\
    break;\
  case numeric::P_Undefined :\
    break;\
}\



#endif /* _EXPAND_TRAITS_HPP */
