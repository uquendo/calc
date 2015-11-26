#pragma once
#ifndef _MATMUL_HPP
#define _MATMUL_HPP
#include "config.h"

#include "numeric/real.hpp"
#include "numeric/blas_impl.hpp"
#include "numeric/expand_traits.hpp"

#include "calcapp/exception.hpp"

#include "quest.hpp"

namespace Calc {

  namespace matmul {

    //dispatcher
    void perform(AlgoParameters& parameters);

    //default c++ version for square matrices in row major order
    void numeric_cpp_simple(AlgoParameters& p);
    //default c++ version for square matrices in row major order with second matrix transposed
    void numeric_cpp_simple_transpose(AlgoParameters& p);

  }

}

#endif /* _MATMUL_HPP */

