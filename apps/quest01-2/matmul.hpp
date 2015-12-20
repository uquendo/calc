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

    //default c version for square tridiagonal matrices in row major order
    struct numeric_c;
    //simple c version for square tridiagonal matrices in row major order
    struct numeric_c_simple;
    //simple c version for square tridiagonal matrices in row major order with second matrix transposed
    struct numeric_c_simple_transpose;
    //diagonal(MRK) c version for square tridiagonal matrices in row major order
    struct numeric_c_mrk;

    //default c++ version for square tridiagonal matrices in row major order
    struct numeric_cpp;
    //simple c++ version for square tridiagonal matrices in row major order
    struct numeric_cpp_simple;
    //simple c++ version for square tridiagonal matrices in row major order with second matrix transposed
    struct numeric_cpp_simple_transpose;
    //diagonal(MRK) c++ version for square tridiagonal matrices in row major order
    struct numeric_cpp_mrk;
    //simple c++ version for square tridiagonal matrices in row major order using std::valarray
    struct numeric_cpp_valarray;
    //simple c++ version for square tridiagonal matrices in row major order with second matrix transposed using std::valarray
    struct numeric_cpp_valarray_transpose;
    //diagonal(MRK) c++ version for square tridiagonal matrices in row major order using std::valarray
    struct numeric_cpp_valarray_mrk;

    //default fortran version for square tridiagonal matrices in column major order
    struct numeric_fortran;
    //simple fortran version for square tridiagonal matrices in column major order
    struct numeric_fortran_simple;
    //simple fortran version for square tridiagonal matrices in column major order with second matrix transposed
    struct numeric_fortran_simple_transpose;
    //diagonal(MRK) fortran version for square tridiagonal matrices in column major order
    struct numeric_fortran_mrk;

  }

}

#endif /* _MATMUL_HPP */
