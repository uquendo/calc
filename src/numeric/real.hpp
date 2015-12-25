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
#include <boost/math/constants/constants.hpp>
# else
#include <mpreal.h>
# endif
#endif

#include <limits>
#include <cmath>

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

template<typename T, int ulp = 1> inline bool isEqualReal(const T& a, const T& b)
{
  using std::abs;
  return abs(a - b) < std::numeric_limits<T>::epsilon() * abs(a + b) * ulp
    //check for subnormals and corner cases like 0 == 0
    || abs(a - b) < std::numeric_limits<T>::min();
}

template<typename T> inline T pi_const()
{
  return T(3.141592653589793238462643383279502884L);
}

template<typename T> inline T half_pi_const()
{
  return T(1.570796326794896619231321691639751442L);
}

#ifdef HAVE_MPREAL
template<> inline mpreal pi_const()
{
# ifdef HAVE_BOOST_MPREAL
  return boost::math::constants::pi<mpreal>();
# else
  return const_pi();
#endif
}

template<> inline mpreal half_pi_const()
{
# ifdef HAVE_BOOST_MPREAL
  return boost::math::constants::half_pi<mpreal>();
# else
  return const_pi() / mpreal(2);
#endif
}
#endif

template<typename T> inline double toDouble(T x)
{
  return (double)x;
}
#ifdef HAVE_QUADMATH
template<> inline double toDouble(numeric::quad x)
{
  return x.convert_to<double>();
}
#endif
#ifdef HAVE_MPREAL
template<> inline double toDouble(numeric::mpreal x)
{
# ifdef HAVE_BOOST_MPREAL
  return x.convert_to<double>();
# else
  return x.toDouble();
# endif
}
#endif

}

#endif /* _REAL_HPP */
