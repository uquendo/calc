#pragma once
#ifndef _APPROXIMATE_HPP
#define _APPROXIMATE_HPP
#include "config.h"

#include "numeric/real.hpp"
#include "numeric/blas.hpp"
#include "numeric/expand_traits.hpp"

#include "calcapp/exception.hpp"

#include "quest.hpp"

namespace Calc {

  namespace approximate {

    //dispatcher
    void perform(const AlgoParameters& parameters, Logger& log);

    //c++ version for cubic bspline approximation
    struct numeric_cpp;
#ifdef HAVE_GSL
    //GSL version for cubic bspline approximation
    struct contrib_gsl;
#endif
  }

}

#endif /* _APPROXIMATE_HPP */
