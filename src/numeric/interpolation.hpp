#pragma once
#ifndef _INTERPOLATION_HPP
#define _INTERPOLATION_HPP
#include "config.h"

#include "numeric/parallel.hpp"

#include <cstddef>

using std::size_t;

namespace numeric
{
  //WARNING! all interpolation functions assume that array of points is sorted
  //lagrange interpolation based on barycentric formula
  //see f.e. Jean-Paul Berrut, Lloyd Trefethen, Barycentric Lagrange Interpolation, SIAM Review 46(3), 501-517 (2004)

  //interpolate function for single given argument
  template<typename T> T lagrange_interpolate_value(const T* const __RESTRICT weights,
      const T* const __RESTRICT points, const T* const __RESTRICT values, const size_t sz,
      const T& arg,
      const bool collision_check = true,
      const TThreading threading_model = T_Serial);

  //interpolate function for array of arguments
  //parallel versions assume that table_sz >> weights_sz
  template<typename T> void lagrange_interpolate_table(const T* const __RESTRICT weights,
      const T* const __RESTRICT points, const T* const __RESTRICT values, const size_t weights_sz,
      const T* const __RESTRICT args, T* const __RESTRICT table, const size_t table_sz,
      const bool collision_check = true,
      const TThreading threading_model = T_Serial);

  //TODO: generic weights computation for arbitrary set of points
  //template<typename T> void lagrange_interpolation_compute_weights(T* const __RESTRICT weights,
  //    const T* const __RESTRICT points, const size_t weights_sz,
  //    const TThreading threading_model = T_Serial);
}

#include "numeric/interpolation_lagrange_impl.hpp"

#endif /* _INTERPOLATION_HPP */
