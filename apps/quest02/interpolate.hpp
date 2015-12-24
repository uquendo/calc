#pragma once
#ifndef _INTERPOLATE_HPP
#define _INTERPOLATE_HPP
#include "config.h"

#include "numeric/real.hpp"
#include "numeric/blas.hpp"
#include "numeric/expand_traits.hpp"

#include "calcapp/exception.hpp"

#include "quest.hpp"

namespace Calc {

  namespace interpolate {

    //dispatcher
    void perform(const AlgoParameters& parameters, Logger& log);

    //c++ version for lagrange polynomial interpolation on uniform or Chebyshev grid(first kind)
    struct numeric_cpp;
#ifdef HAVE_GSL
    //GSL version for lagrange polynomial interpolation on Chebyshev grid(first kind)
    struct contrib_gsl_che;
    //GSL version for lagrange polynomial interpolation on uniform grid
    struct contrib_gsl_uni;
#endif
  }

}

#endif /* _INTERPOLATE_HPP */
