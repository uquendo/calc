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

#ifdef BUILD_QUAD
#include <boost/multiprecision/float128.hpp>
#endif
#ifdef HAVE_MPREAL
#include <mpreal.h>
#endif

namespace numeric {

enum TPrecision {
	P_Float,
	P_Double,
	P_Quad,
	P_MPFR,
	P_Undefined
};

#ifdef BUILD_QUAD
#ifdef HAVE_BOOST
typedef boost::multiprecision::float128 quad;
#else
#error "manual detection of quad type is not available yet.\n enable boost to autodetect and correctly wrap compiler-specific quad precision floats and corresponding math functions"
#endif
#endif
#ifdef HAVE_MPREAL
typedef mpfr::mpreal mpreal;
#endif

}

#endif /* _REAL_HPP */
