#pragma once
#ifndef _DENSE_LINEAR_SOLVE_HPP
#define _DENSE_LINEAR_SOLVE_HPP
#include "config.h"

#include "numeric/real.hpp"
#include "numeric/blas.hpp"
#include "numeric/lapack.hpp"
#include "numeric/expand_traits.hpp"

#include "calcapp/exception.hpp"

#include "quest.hpp"

namespace Calc {

  namespace dense_linear_solve {

    static constexpr int default_max_iter_count = 10000;
    template<typename T> constexpr T default_eps() { return T(1.0e-12); }

    //dispatcher
    bool perform(const AlgoParameters& parameters, Logger& log);

    //dumb c++ version of Jacobi iterative solver
    struct numeric_cpp_jacobi;

    //dumb c++ version of Seidel iterative solver
    struct numeric_cpp_seidel;

    //dumb c++ version of relaxation iterative solver
    struct numeric_cpp_relaxation;

  }
}

#endif /* _DENSE_LINEAR_SOLVE_HPP */
