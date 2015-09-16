#pragma once
#ifndef _REAL_HPP
#define _REAL_HPP
#includee "config.h"

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


enum TPrecision {
	P_Float,
	P_Double,
	P_Quad,
	P_MPFR,
	P_Undefined
};

#endif /* _REAL_HPP */
