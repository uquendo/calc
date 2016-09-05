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

  namespace newton {

    //dispatcher
    bool perform(const AlgoParameters& parameters, Logger& log);

    //find zero of rosenbrock function's gradient
    struct rosenbrock_gradient;

  }
}

#endif /* _DENSE_LINEAR_SOLVE_HPP */
