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

    //default c version for square matrices in row major order
    struct numeric_c;
    //simple c version for square matrices in row major order
    struct numeric_c_simple;
    //simple c version for square matrices in row major order with second matrix transposed
    struct numeric_c_simple_transpose;
    //strassen c version for matrices in row major order
    struct numeric_c_strassen;

    //default c++ version for square matrices in row major order
    struct numeric_cpp;
    //simple c++ version for square matrices in row major order
    struct numeric_cpp_simple;
    //simple c++ version for square matrices in row major order with second matrix transposed
    struct numeric_cpp_simple_transpose;
    //strassen c++ version for matrices in row major order
    struct numeric_cpp_strassen;

    //default fortran version for square matrices in column major order
    struct numeric_fortran;
    //simple fortran version for square matrices in column major order
    struct numeric_fortran_simple;
    //simple fortran version for square matrices in column major order with second matrix transposed
    struct numeric_fortran_simple_transpose;
    //strassen fortran version for matrices in column major order
    struct numeric_fortran_strassen;
    //internal fortran version(using matmul) for matrices in column major order
    struct numeric_fortran_internal;

#ifdef HAVE_BOOST_UBLAS
    //boost.uBlas c++ version for matrices in row major order
    struct contrib_cpp_boost_ublas;
#endif
#ifdef HAVE_EIGEN
    //Eigen c++ version for matrices in row major order
    struct contrib_cpp_eigen;
#endif
#ifdef HAVE_MTL
    //MTL c++ version for matrices in row major order
    struct contrib_cpp_mtl;
#endif
#ifdef HAVE_ARMADILLO
    //armadillo c++ version for matrices in row major order
    struct contrib_cpp_armadillo;
#endif
#ifdef HAVE_BLAS
    //cblas version for matrices in row major order
    struct contrib_c_blas;
    struct contrib_fortran_blas;
#endif

  }

}

#endif /* _MATMUL_HPP */
