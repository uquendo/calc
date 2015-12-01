#pragma once
#ifndef _EXPAND_TRAITS_HPP
#define _EXPAND_TRAITS_HPP
#include "config.h"
#include "numeric/real.hpp"

#include <functional>

//parameter pack expansion

namespace numeric
{

template<Precision_ID_t... Indices>
struct indices {
  typedef indices<Indices..., sizeof...(Indices)> next;
};

template<Precision_ID_t N>
struct build_indices {
  typedef typename build_indices<N - 1>::type::next type;
};

template<>
struct build_indices<0> {
  typedef indices<> type;
};

//typedef typename build_indices<Precision_ID_t(P_Undefined)>::type ForAllPrecision;

//TODO: allow using variable number of arguments in functor via variadic templates
template<class DerivedFunc, class TArg=void, class TReturnValue=void, TPrecision MaxPrecision=P_Undefined>
class MPFuncBase
{
  template<Precision_ID_t Pid> TReturnValue perform(const TArg& p)
  {
    static constexpr TPrecision P = TPrecision(Pid);
    typedef typename TraitBuiltin<P>::type T;
    return static_cast<DerivedFunc&>(*this).template perform<T>(p);
  }
  template<Precision_ID_t... Indices> TReturnValue dispatch(const TPrecision prec, const TArg& p, indices<Indices...>)
  {
    typedef TReturnValue (MPFuncBase<DerivedFunc,TArg,TReturnValue,MaxPrecision>::* MPFuncBaseMemberFun) (const TArg&);
    MPFuncBaseMemberFun lookup[] = { &MPFuncBase<DerivedFunc,TArg,TReturnValue,MaxPrecision>::perform<Indices>... };
    return ((this)->*(lookup[Precision_ID_t(prec)]))(p);
  }
public:
  virtual ~MPFuncBase(){};
  TReturnValue operator()(const TPrecision prec, const TArg& p)  {
    return dispatch(prec, p, typename build_indices<Precision_ID_t(MaxPrecision)>::type());
  };
};

/*
 *  //example usage
 *  struct SomeFunc : MPFuncBase<SomeAlgoFunc,SomeArg,SomeReturnValue>
 *  {
 *    template<typename T> SomeReturnValue perform(SomeArg);
 *  }
 *  //specialize for some TPrecision precision
 *  SomeAlgoFunc()(precision);
 */

}

//fallback macros as an alternative to functor-based expansion

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
