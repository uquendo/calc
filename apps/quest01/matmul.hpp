#pragma once
#ifndef _MATMUL_HPP
#define _MATMUL_HPP
#include "config.h"

#include "numeric/real.hpp"
#include "numeric/blas.hpp"
#include "numeric/expand_traits.hpp"

#include "calcapp/exception.hpp"

#include "quest.hpp"

namespace Calc {

  namespace matmul {

    //dispatcher
    void perform(const AlgoParameters& parameters, Logger& log);

    //default c++ version for square matrices in row major order
    struct numeric_cpp_simple;
    //default c++ version for square matrices in row major order with second matrix transposed
    struct numeric_cpp_simple_transpose;

  }

}

#endif /* _MATMUL_HPP */

