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
enum class TMM_Algo : int { IJK=0, JKI, KIJ, IKJ, KJI, JIK };

//generic version of dgemm, C=op(A)*op(B)
template<typename T>
  void dgemm(const TMatrixStorage stor, const TMatrixTranspose transA, const TMatrixTranspose transB,
      const T* const __RESTRICT a, const T* const __RESTRICT b, T* const __RESTRICT c,
      const size_t nrows_a, const size_t ncolumns_a,
      const size_t nrows_b, const size_t ncolumns_b,
      const TThreading threading_model = T_Serial);

//generic version of dgemm for square matrices
template<typename T>
  void dgemm(const TMatrixStorage stor, const TMatrixTranspose transA, const TMatrixTranspose transB,
    const T* const __RESTRICT a, const T* const __RESTRICT b, T* const __RESTRICT c,
    const size_t sz,
    const TThreading threading_model = T_Serial);

//block version of reduced dgemm, C=A*B
template<typename T>
  void dgemm_block(const TMatrixStorage stor,
      const T* const __RESTRICT a, const T* const __RESTRICT b, T* const __RESTRICT c,
      const size_t nrows_a, const size_t ncolumns_a,
      const size_t nrows_b, const size_t ncolumns_b,
      const TThreading threading_model = T_Serial);

//block version of reduced dgemm for square matrices
template<typename T>
  void dgemm_block(const TMatrixStorage stor,
    const T* const __RESTRICT a, const T* const __RESTRICT b, T* const __RESTRICT c,
    const size_t sz,
    const TThreading threading_model = T_Serial);

//generic dgbmv, y = \beta*y + \alpha*op(A)*x
template<typename T>
  void dgemv(const TMatrixStorage stor, const TMatrixTranspose transA,
      const T* const __RESTRICT a, const T* const __RESTRICT x, T* const __RESTRICT y,
      const size_t nrows_a, const size_t ncolumns_a,
      const T alpha = T(1.0), const T beta = T(1.0),
      const TThreading threading_model = T_Serial);

//generic dgbmv for square matrices
template<typename T>
  void dgemv(const TMatrixStorage stor, const TMatrixTranspose transA,
      const T* const __RESTRICT a, const T* const __RESTRICT x, T* const __RESTRICT y,
      const size_t sz,
      const T alpha = T(1.0), const T beta = T(1.0),
      const TThreading threading_model = T_Serial);

//vector norm calculation
template<typename T>
  T vector_norm_L1(const size_t sz, const T* const __RESTRICT x);
template<typename T>
  T vector_norm_L2(const size_t sz, const T* const __RESTRICT x);

//vector distance(induced by norm of difference) calculation
template<typename T>
  T vector_distance_L1(const size_t sz, const T* const __RESTRICT x, const T* const __RESTRICT y);
template<typename T>
  T vector_distance_L2(const size_t sz, const T* const __RESTRICT x, const T* const __RESTRICT y);

//test for diagonal dominance
template<typename T>
  bool is_diagonally_dominant(const size_t sz, const size_t stride,
      const T* const __RESTRICT a);

//generic dgbmm for banded matrices, C+=op(A)*op(B)
//TODO:STUB! only tridiagonal matrices at the moment
template<typename T>
  void dgbmm(const TMatrixStorage stor, const TMatrixTranspose transA, const TMatrixTranspose transB,
      const T* const __RESTRICT a, const T* const __RESTRICT b, T* const __RESTRICT c,
      const size_t nrows_a, const size_t ncolumns_a, const size_t upper_band_a, const size_t lower_band_a,
      const size_t nrows_b, const size_t ncolumns_b, const size_t upper_band_b, const size_t lower_band_b,
      const TThreading threading_model = T_Serial);

//generic dgbmm for square symmetrically banded matrices
template<typename T>
  void dgbmm(const TMatrixStorage stor, const TMatrixTranspose transA, const TMatrixTranspose transB,
    const T* const __RESTRICT a, const T* const __RESTRICT b, T* const __RESTRICT c,
    const size_t sz, const size_t band,
    const TThreading threading_model = T_Serial);

//generic dgbmv for banded matrices, y+=op(A)*x
//TODO:STUB! only tridiagonal matrices at the moment
template<typename T>
  void dgbmv(const TMatrixStorage stor, const TMatrixTranspose transA,
      const T* const __RESTRICT a, const T* const __RESTRICT x, T* const __RESTRICT y,
      const size_t nrows_a, const size_t ncolumns_a, const size_t upper_band_a, const size_t lower_band_a,
      const T alpha = T(1.0),
      const TThreading threading_model = T_Serial);

//generic dgbmv for square symmetrically banded matrices
template<typename T>
  void dgbmv(const TMatrixStorage stor, const TMatrixTranspose transA,
    const T* const __RESTRICT a, const T* const __RESTRICT x, T* const __RESTRICT y,
    const size_t sz, const size_t band,
    const T alpha = T(1.0),
    const TThreading threading_model = T_Serial);

//add for banded matrices, a += b, assuming that band width of b is less or equal than that of a
template<typename T>
  void banded_add(const TMatrixStorage stor,
    T* const __RESTRICT a, const T* const __RESTRICT b,
    const size_t nrows_a, const size_t ncolumns_a,
    const size_t upper_band_a, const size_t lower_band_a,
    const size_t upper_band_b, const size_t lower_band_b,
    const TThreading threading_model = T_Serial);

//add for square banded matrices, a += b, assuming that band width of b is equal or less than that of a
template<typename T>
  void banded_add(const TMatrixStorage stor,
    T* const __RESTRICT a, const T* const __RESTRICT b,
    const size_t sz,
    const size_t upper_band_a, const size_t lower_band_a,
    const size_t upper_band_b, const size_t lower_band_b,
    const TThreading threading_model);

//add for square symmetrically banded matrices, a += b, assuming that band width of b is equal or less than that of a
template<typename T>
  void banded_add(const TMatrixStorage stor,
    T* const __RESTRICT a, const T* const __RESTRICT b,
    const size_t sz,
    const size_t band_a,
    const size_t band_b,
    const TThreading threading_model = T_Serial);

//test for diagonal dominance for banded matrices
template<typename T> bool is_banded_diagonally_dominant(const size_t sz,
    const size_t lower_band, const size_t upper_band,
    const size_t stride, const T* const __RESTRICT a);

}

//simple ijk implementation
#include "numeric/blas_impl.hpp"
//block implementation based on paper "Anatomy of High-Performance Matrix Multiplication" by Kazushige Goto
#include "numeric/blas_block_impl.hpp"
//simple ijk implementation for banded matrices in CDS format
#include "numeric/blas_banded_impl.hpp"

#endif /* _BLAS_HPP */
