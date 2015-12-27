#pragma once
#ifndef _DENSE_LINEAR_SOLVE_HPP
#define _DENSE_LINEAR_SOLVE_HPP
#include "config.h"

#include "numeric/real.hpp"
#include "numeric/blas.hpp"
#include "numeric/expand_traits.hpp"

#include "calcapp/exception.hpp"

#include "quest.hpp"

namespace Calc {

  namespace dense_linear_solve {

    //dispatcher
    void perform(const AlgoParameters& parameters, Logger& log);

    //dumb c++ version of gauss elimination without pivoting
    struct numeric_cpp_gauss;

    //dumb c++ version of jordan elimination without pivoting
    struct numeric_cpp_jordan;

    //dumb c++ version of gauss elimination with full pivoting
    struct numeric_cpp_gauss_full_pivoting;
  }

}

#endif /* _DENSE_LINEAR_SOLVE_HPP */
