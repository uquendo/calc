#pragma once
#ifndef _BLAS_HPP
#define _BLAS_HPP
#include "config.h"

#include "numeric/parallel.hpp"

#include <cstddef>

using std::size_t;

namespace numeric
{

enum class TMatrixStorage { RowMajor, ColumnMajor };
enum class TMatrixTranspose : char { No='N', Transpose='T', Conjugate='C' };
enum class TMM_Algo { IJK, JKI, KIJ, IKJ, KJI, JIK };

//generic version of dgemm, C=op(A)*op(B)
template<typename T>
  void dgemm(const TMatrixStorage stor, const TMatrixTranspose transA, const TMatrixTranspose transB,
      const T* const __RESTRICT a, const T* const __RESTRICT b, T* const __RESTRICT c,
      const size_t nrows_a, const size_t ncolumns_a,
      const size_t nrows_b, const size_t ncolumns_b,
      const TThreading threading_model=T_Serial);

//generic version of dgemm for square matrices
template<typename T>
  void dgemm(const TMatrixStorage stor, const TMatrixTranspose transA, const TMatrixTranspose transB,
    const T* const __RESTRICT a, const T* const __RESTRICT b, T* const __RESTRICT c,
    const size_t sz,
    const TThreading threading_model=T_Serial);

}

//implementation
#include "numeric/blas_impl.hpp"

#endif /* _BLAS_HPP */
