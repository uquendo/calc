#pragma once
#ifndef _REAL_HPP
#define _REAL_HPP
#include "config.h"

// isnan(), isfinite()
#if _MSC_VER
#include <float.h>
#define isnan _isnan
#define isfinite _finite
#if  _MSC_VER <1700
typedef __int8  int8_t;
typedef __int16 int16_t;
typedef __int64 int64_t;
#endif
#else
#include <cstdint>
#endif

#ifdef HAVE_QUADMATH
#include <boost/multiprecision/float128.hpp>
#endif
#ifdef HAVE_MPREAL
# ifdef HAVE_BOOST_MPREAL
#include <boost/multiprecision/mpfr.hpp>
# else
#include <mpreal.h>
# endif
#endif

namespace numeric {

typedef int Precision_ID_t;
enum TPrecision : int {
  P_Float=0,
  P_Double=1,
  P_LongDouble=2,
#ifdef HAVE_QUADMATH
  P_Quad,
#endif
#ifdef HAVE_MPREAL
  P_MPFR,
#endif
  P_Undefined
};

// type traits

template<TPrecision T>
struct TraitBuiltin {};

template<typename T>
struct TraitEnum {
  static constexpr TPrecision type=P_Undefined;
};

template<>
struct TraitBuiltin<P_Float> {
  typedef float type;
};

template<>
struct TraitEnum<float> {
  static constexpr TPrecision type=P_Float;
};

template<>
struct TraitBuiltin<P_Double> {
  typedef double type;
};

template<>
struct TraitEnum<double> {
  static constexpr TPrecision type=P_Double;
};

template<>
struct TraitBuiltin<P_LongDouble> {
  typedef long double type;
};

template<>
struct TraitEnum<long double> {
  static constexpr TPrecision type=P_LongDouble;
};

#ifdef HAVE_QUADMATH
#ifdef HAVE_BOOST
typedef boost::multiprecision::float128 quad;
//typedef float128_type quad;

template<>
struct TraitBuiltin<P_Quad> {
  typedef quad type;
};

template<>
struct TraitEnum<quad> {
  static constexpr TPrecision type=P_Quad;
};

#else
#error "manual detection of quad type is not available yet. enable boost to autodetect and correctly wrap compiler-specific quad precision floats and corresponding math functions"
#endif
#endif

#ifdef HAVE_MPREAL
# ifdef HAVE_BOOST_MPREAL
typedef boost::multiprecision::mpfr_float mpreal;
namespace mpfloat
{
  inline void setDefaultPrecision(unsigned d10){ boost::multiprecision::mpfr_float::default_precision(d10); }
  inline unsigned getDefaultPrecision(){ return boost::multiprecision::mpfr_float::default_precision(); }
}
# else
typedef mpfr::mpreal mpreal;
namespace mpfloat
{
  inline void setDefaultPrecision(unsigned d10){ mpfr::mpreal::set_default_prec(mpfr::digits2bits(int(d10))); }
  inline unsigned getDefaultPrecision(){ return unsigned(mpfr::bits2digits(mpfr::mpreal::get_default_prec())); }
}
# endif

template<>
struct TraitBuiltin<P_MPFR> {
  typedef mpreal type;
};

template<>
struct TraitEnum<mpreal> {
  static constexpr TPrecision type=P_MPFR;
};

#endif


}

#endif /* _REAL_HPP */
