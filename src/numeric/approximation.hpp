#pragma once
#ifndef _APPROXIMATION_HPP
#define _APPROXIMATION_HPP
#include "config.h"

#include "numeric/parallel.hpp"

#include <cstddef>

using std::size_t;

namespace numeric
{
  //WARNING! all approximation functions assume that the array of points is sorted

  //cubic_spline_* functions operate on spline expressed via values at knots of spline itself(S0) and its second derivative(S2)
  //see Carl de Boor, "A practical guide to splines", ch. iv, problem 6, p. 49 (2001)

/*
  //compute smoothing cubic spline expressed via values of spline and its second derivative at knots
  //see Carl de Boor, "A practical guide to splines", ch. xiv, pp. 207-210 (2001)
  template<typename T> T cubic_spline_natural_compute(const T* const __RESTRICT weights,
      const T* const __RESTRICT points, const T* const __RESTRICT values, const size_t points_sz,
      T* const __RESTRICT S0, T* const __RESTRICT S2, const size_t knots_sz,
      const T smoothing_p = T(0.5),
      const TThreading threading_model = T_Serial);
*/

  //evaluate cubic spline for single given argument
  template<typename T> T cubic_spline_evaluate_value(const T* const __RESTRICT knots,
      const T* const __RESTRICT S0, const T* const __RESTRICT S2, const size_t knots_sz,
      const T& arg,
      const TThreading threading_model = T_Serial);

  //evaluate cubic spline for the array of arguments
  //parallel versions assume that table_sz >> points_sz
  template<typename T> void cubic_spline_evaluate_table(const T* const __RESTRICT knots,
      const T* const __RESTRICT S0, const T* const __RESTRICT S2, const size_t knots_sz,
      const T* const __RESTRICT args, T* const __RESTRICT table, const size_t table_sz,
      const TThreading threading_model = T_Serial);

}

#include "numeric/approximation_spline_impl.hpp"

#endif /* _APPROXIMATION_HPP */
