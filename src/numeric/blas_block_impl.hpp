#pragma once
#ifndef _BLAS_BLOCK_IMPL_HPP
#define _BLAS_BLOCK_IMPL_HPP
#include "config.h"

#include "numeric/blas.hpp"
#include "numeric/complex.hpp"
#include "numeric/parallel.hpp"
#include "numeric/cache.hpp"

#ifdef HAVE_CILK
#include <cilk/cilk.h>
#endif

#ifdef HAVE_TBB
#include "numeric/parallel_tbb.hpp"
#endif

#include <limits>
#include <cstddef>
#include <cstring>

using std::size_t;

namespace numeric {


//helper for block matrix multiplication
//see paper "Anatomy of High-Performance Matrix Multiplication" by Kazushige Goto for detailed algorithm description
//NB: Goto in the paper used column major ordering, we use row major here

template<typename T, size_t nr, size_t mr>
  __FORCEINLINE inline void __mm_block_op(const T * const  __RESTRICT a, const size_t stride_a,
                                          const T * const  __RESTRICT b, const size_t stride_b,
                                                T * const  __RESTRICT c, const size_t stride_c)
{
  alignas(getDefaultAlignment<T>()) T a_pack[nr*mr];
  for(size_t j = 0; j < mr; j++)
    for(size_t i = 0; i < nr; i++)
      a_pack[i*mr+j] = a[i*stride_a+j];

  for(size_t kk = 0; kk < stride_c/mr; kk++)
  {
    for(size_t jjj = 0; jjj < mr; jjj++)
    {
      for(size_t kkk = 0; kkk < mr; kkk++)
      {
        for(size_t i = 0; i < nr; i++)
        {
          c[i*stride_c+kk*mr+kkk] += a_pack[i*mr+jjj] * b[kk*mr+kkk+jjj*stride_b];
        }
      }
    }
  }
}

#ifdef NDEBUG
template<>
  __FORCEINLINE inline void __mm_block_op<double, 4, 4>(const double * const  __RESTRICT a, const size_t stride_a,
                                                        const double * const  __RESTRICT b, const size_t stride_b,
                                                              double * const  __RESTRICT c, const size_t stride_c)
{
  constexpr size_t pack_size = 4;
  const size_t b_sz = stride_b/pack_size;
  const size_t c_sz = stride_c/pack_size;
  __m256d c_pack0, c_pack1, c_pack2, c_pack3;
  __m256d c_tmp0, c_tmp1, c_tmp2, c_tmp3;

  for(size_t kk = 0; kk < c_sz; kk++)
  {
      __m256d b_pack0 = _mm256_loadu_pd(&b[(kk+0*b_sz)*pack_size]);
      __m256d b_pack1 = _mm256_loadu_pd(&b[(kk+1*b_sz)*pack_size]);
      __m256d b_pack2 = _mm256_loadu_pd(&b[(kk+2*b_sz)*pack_size]);
      __m256d b_pack3 = _mm256_loadu_pd(&b[(kk+3*b_sz)*pack_size]);

      __m256d a_broad0 = _mm256_set1_pd(a[0+0*stride_a]);
      __m256d a_broad1 = _mm256_set1_pd(a[1+0*stride_a]);
      __m256d a_broad2 = _mm256_set1_pd(a[2+0*stride_a]);
      __m256d a_broad3 = _mm256_set1_pd(a[3+0*stride_a]);
      c_tmp0 = _mm256_add_pd(_mm256_add_pd(_mm256_mul_pd(a_broad0, b_pack0),_mm256_mul_pd(a_broad1, b_pack1)),
                              _mm256_add_pd(_mm256_mul_pd(a_broad2, b_pack2),_mm256_mul_pd(a_broad3, b_pack3)));

      __m256d a_broad4 = _mm256_set1_pd(a[0+1*stride_a]);
      __m256d a_broad5 = _mm256_set1_pd(a[1+1*stride_a]);
      __m256d a_broad6 = _mm256_set1_pd(a[2+1*stride_a]);
      __m256d a_broad7 = _mm256_set1_pd(a[3+1*stride_a]);
      c_tmp1 = _mm256_add_pd(_mm256_add_pd(_mm256_mul_pd(a_broad4, b_pack0),_mm256_mul_pd(a_broad5, b_pack1)),
                              _mm256_add_pd(_mm256_mul_pd(a_broad6, b_pack2),_mm256_mul_pd(a_broad7, b_pack3)));

      __m256d a_broad8 = _mm256_set1_pd(a[0+2*stride_a]);
      __m256d a_broad9 = _mm256_set1_pd(a[1+2*stride_a]);
      __m256d a_broad10 = _mm256_set1_pd(a[2+2*stride_a]);
      __m256d a_broad11 = _mm256_set1_pd(a[3+2*stride_a]);
      c_tmp2 = _mm256_add_pd(_mm256_add_pd(_mm256_mul_pd(a_broad8, b_pack0),_mm256_mul_pd(a_broad9, b_pack1)),
                              _mm256_add_pd(_mm256_mul_pd(a_broad10, b_pack2),_mm256_mul_pd(a_broad11, b_pack3)));

      __m256d a_broad12 = _mm256_set1_pd(a[0+3*stride_a]);
      __m256d a_broad13 = _mm256_set1_pd(a[1+3*stride_a]);
      __m256d a_broad14 = _mm256_set1_pd(a[2+3*stride_a]);
      __m256d a_broad15 = _mm256_set1_pd(a[3+3*stride_a]);
      c_tmp3 = _mm256_add_pd(_mm256_add_pd(_mm256_mul_pd(a_broad12, b_pack0),_mm256_mul_pd(a_broad13, b_pack1)),
                              _mm256_add_pd(_mm256_mul_pd(a_broad14, b_pack2),_mm256_mul_pd(a_broad15, b_pack3)));

      c_pack0 = _mm256_loadu_pd(&c[(kk+0*c_sz)*pack_size]);
      c_pack1 = _mm256_loadu_pd(&c[(kk+1*c_sz)*pack_size]);
      c_pack2 = _mm256_loadu_pd(&c[(kk+2*c_sz)*pack_size]);
      c_pack3 = _mm256_loadu_pd(&c[(kk+3*c_sz)*pack_size]);

      c_pack0 = _mm256_add_pd(c_pack0, c_tmp0);
      c_pack1 = _mm256_add_pd(c_pack1, c_tmp1);
      c_pack2 = _mm256_add_pd(c_pack2, c_tmp2);
      c_pack3 = _mm256_add_pd(c_pack3, c_tmp3);

      _mm256_storeu_pd(&c[(kk+0*c_sz)*pack_size], c_pack0);
      _mm256_storeu_pd(&c[(kk+1*c_sz)*pack_size], c_pack1);
      _mm256_storeu_pd(&c[(kk+2*c_sz)*pack_size], c_pack2);
      _mm256_storeu_pd(&c[(kk+3*c_sz)*pack_size], c_pack3);
  }
}
#endif

/*
template<>
  __FORCEINLINE inline void __mm_block_op(const float * const  __RESTRICT a, const float * const  __RESTRICT b, float * const  __RESTRICT c,
    const size_t n)
{
  const int pack_size = 8;
  __m256 c_pack0, c_pack1, c_pack2, c_pack3, c_pack4, c_pack5, c_pack6, c_pack7;
  c_pack0 = _mm256_loadu_ps(&c[0*pack_size]);
  c_pack1 = _mm256_loadu_ps(&c[1*pack_size]);
  c_pack2 = _mm256_loadu_ps(&c[2*pack_size]);
  c_pack3 = _mm256_loadu_ps(&c[3*pack_size]);
  c_pack4 = _mm256_loadu_ps(&c[4*pack_size]);
  c_pack5 = _mm256_loadu_ps(&c[5*pack_size]);
  c_pack6 = _mm256_loadu_ps(&c[6*pack_size]);
  c_pack7 = _mm256_loadu_ps(&c[7*pack_size]);
  for(int i = 0; i < n; i++)
  {
      __m256 areg0 = _mm256_set1_ps(a[i]);
      __m256 b_pack0 = _mm256_loadu_ps(&b[pack_size*(8*i + 0)]);
      c_pack0 = _mm256_add_ps(_mm256_mul_ps(areg0,b_pack0), c_pack0);
      __m256 b_pack1 = _mm256_loadu_ps(&b[pack_size*(8*i + 1)]);
      c_pack1 = _mm256_add_ps(_mm256_mul_ps(areg0,b_pack1), c_pack1);
      __m256 b_pack2 = _mm256_loadu_ps(&b[pack_size*(8*i + 2)]);
      c_pack2 = _mm256_add_ps(_mm256_mul_ps(areg0,b_pack2), c_pack2);
      __m256 b_pack3 = _mm256_loadu_ps(&b[pack_size*(8*i + 3)]);
      c_pack3 = _mm256_add_ps(_mm256_mul_ps(areg0,b_pack3), c_pack3);
      __m256 b_pack4 = _mm256_loadu_ps(&b[pack_size*(8*i + 4)]);
      c_pack4 = _mm256_add_ps(_mm256_mul_ps(areg0,b_pack4), c_pack4);
      __m256 b_pack5 = _mm256_loadu_ps(&b[pack_size*(8*i + 5)]);
      c_pack5 = _mm256_add_ps(_mm256_mul_ps(areg0,b_pack5), c_pack5);
      __m256 b_pack6 = _mm256_loadu_ps(&b[pack_size*(8*i + 6)]);
      c_pack6 = _mm256_add_ps(_mm256_mul_ps(areg0,b_pack6), c_pack6);
      __m256 b_pack7 = _mm256_loadu_ps(&b[pack_size*(8*i + 7)]);
      c_pack7 = _mm256_add_ps(_mm256_mul_ps(areg0,b_pack7), c_pack7);
  }
  _mm256_storeu_ps(&c[0*pack_size], c_pack0);
  _mm256_storeu_ps(&c[1*pack_size], c_pack1);
  _mm256_storeu_ps(&c[2*pack_size], c_pack2);
  _mm256_storeu_ps(&c[3*pack_size], c_pack3);
  _mm256_storeu_ps(&c[4*pack_size], c_pack4);
  _mm256_storeu_ps(&c[5*pack_size], c_pack5);
  _mm256_storeu_ps(&c[6*pack_size], c_pack6);
  _mm256_storeu_ps(&c[7*pack_size], c_pack7);
}
*/

template<typename T>
  __FORCEINLINE inline void __mm_block_pack(const T * const  __RESTRICT src, const size_t src_stride, const size_t src_rows,
                                                  T * const  __RESTRICT dst, const size_t dst_stride, const size_t dst_rows)
{
  for(size_t i = 0; i < src_rows; i++)
  {
    for(size_t j = 0; j < dst_stride; j++)
    {
      dst[i*dst_stride+j] = src[i*src_stride+j];
    }
  }
  if(dst_rows > src_rows)
  {
    for(size_t n = src_rows*dst_stride; n < dst_rows*dst_stride; n++)
      dst[n] = 0;
  }
}


template<typename T, size_t mr>
  __FORCEINLINE inline void __mm_block_pack_zorder_down(const T * const  __RESTRICT src, const size_t src_stride, const size_t src_rows,
                                                              T * const  __RESTRICT dst, const size_t dst_stride, const size_t dst_rows)
{
  for(size_t i = 0; i < src_rows; i++)
  {
    for(size_t j = 0; j < dst_stride; j++)
    {
      dst[(i/mr)*mr*dst_stride+i%mr+j*mr] = src[i*src_stride+j];
    }
  }
  if(dst_rows > src_rows)
  {
    for(size_t i = src_rows; i < dst_rows; i++)
      for(size_t j = 0; j < dst_stride; j++)
        dst[(i/mr)*mr*dst_stride+i%mr+j*mr] = 0;
  }
}

template<typename T>
  __FORCEINLINE inline void __mm_block_unpack(const T * const  __RESTRICT src, const size_t src_stride, const size_t src_rows,
                                                    T * const  __RESTRICT dst, const size_t dst_stride, const size_t dst_rows)
{
  for(size_t i = 0; i < dst_rows; i++)
  {
    for(size_t j = 0; j < src_stride; j++)
    {
      dst[i*dst_stride+j] = src[i*src_stride+j];
    }
  }
}


template<typename T>
  __FORCEINLINE inline void __mm_block_unpack_add(const T * const  __RESTRICT src, const size_t src_stride, const size_t src_rows,
                                                        T * const  __RESTRICT dst, const size_t dst_stride, const size_t dst_rows)
{
  for(size_t i = 0; i < dst_rows; i++)
  {
    for(size_t j = 0; j < src_stride; j++)
    {
      dst[i*dst_stride+j] += src[i*src_stride+j];
    }
  }
}

constexpr inline size_t __round_up(const size_t val, const size_t inc)
{
  return ( val % inc == 0 ? val : val - (val % inc) + inc);
}

template<typename T>
  inline void block_matmul_serial(const T* const  __RESTRICT a, const T* const  __RESTRICT b, T* const __RESTRICT c,
    const size_t nrows_op_a, const size_t ncolumns_op_a, const size_t ncolumns_op_b)
{
  //calculate block sizes and index limits
  constexpr size_t mr = default_max_hw_vector_size() / sizeof(T);
  constexpr size_t nr = default_max_hw_vector_size() / sizeof(T);
//  const size_t mc = __round_up((default_tlb_page_capacity() - (nr+4)) * default_page_size() / ( sizeof(T) *__round_up(nrows_op_a, nr) ), mr);
//TODO: block size tuning
  const size_t mc = 128;
  const size_t kc = mc; // wild guess
  const size_t nrows_op_a_rounded = __round_up(nrows_op_a, nr);
  const size_t sz_i = nrows_op_a / nr;
  const size_t sz_j = ncolumns_op_a / mc;
  const size_t sz_k = ncolumns_op_b / kc;
  const size_t remainder_sz_i = nrows_op_a % nr;
  const size_t remainder_sz_j = ncolumns_op_a % mc;
  const size_t remainder_sz_k = ncolumns_op_b % kc;
  //allocate memory for packed blocks
//  void * buf = nullptr;
//  T * const Aj_p = reinterpret_cast<T * const>(aligned_malloc(sizeof(T) * mc * __round_up(nrows_op_a, nr), getDefaultAlignment<T>(), buf));
//  T * const Bjk_p = reinterpret_cast<T * const>(aligned_malloc(sizeof(T) * mc * kc, getDefaultAlignment<T>(), buf));
//  T * const Cjki_p = reinterpret_cast<T * const>(aligned_malloc(sizeof(T) * nr * kc, getDefaultAlignment<T>(), buf));
  for(size_t j = 0; j < sz_j; j++)
  {
    alignas(getDefaultAlignment<T>()) T Aj_p[nrows_op_a_rounded * mc];
    alignas(getDefaultAlignment<T>()) T Bjk_p[mc * kc];
    alignas(getDefaultAlignment<T>()) T Cjki_p[nr * kc];
    //pack Aj -> Aj_p to minimize TLB misses
    __mm_block_pack<T>(&a[j*mc], ncolumns_op_a, nrows_op_a, Aj_p, mc, nrows_op_a_rounded);
    for(size_t k = 0; k < sz_k; k++)
    {
      //pack Bjk -> Bjk_p to minimize L2 cache misses and assist vectorization
      __mm_block_pack<T>(&b[j*mc*ncolumns_op_b+k*kc], ncolumns_op_b, mc, Bjk_p, kc, mc);
      for(size_t i = 0; i < sz_i; i++)
      {
        memset(reinterpret_cast<void*>(Cjki_p), 0, nr * kc * sizeof(T));
        for(size_t jj = 0; jj < mc/mr; jj++)
        {
          __mm_block_op<T,nr,mr>(&Aj_p[i*nr*mc+jj*mr], mc, &Bjk_p[jj*mr*kc], kc, Cjki_p, kc);
        }
        //unpack Cjki_p
        __mm_block_unpack_add<T>(Cjki_p, kc, nr, &c[i*nr*ncolumns_op_b+j*kc], ncolumns_op_b, nr);
      }
      //remainder
      if(remainder_sz_i != 0)
      {
        memset(reinterpret_cast<void*>(Cjki_p), 0, nr * kc * sizeof(T));
        for(size_t jj = 0; jj < mc/mr; jj++)
        {
          __mm_block_op<T,nr,mr>(&Aj_p[sz_i*nr*mc+jj*mr], mc, &Bjk_p[jj*mr*kc], kc, Cjki_p, kc);
        }
        __mm_block_unpack_add<T>(Cjki_p, kc, nr, &c[sz_i*nr*ncolumns_op_b+j*kc], ncolumns_op_b, remainder_sz_i);
      }
    }
    //remainder loop
    for(size_t k = 0; k < remainder_sz_k; k++)
    {
    }
  }
  //remainder loop
  for(size_t j = 0; j < remainder_sz_j; j++)
  {
  }
}

template<typename T>
  inline void block_matmul_stdthreads(const T* const  __RESTRICT a, const T* const  __RESTRICT b, T* const __RESTRICT c,
    const size_t nrows_op_a, const size_t ncolumns_op_a, const size_t ncolumns_op_b)
{
}

#ifdef HAVE_PTHREADS
template<typename T>
  inline void block_matmul_pthreads(const T* const  __RESTRICT a, const T* const  __RESTRICT b, T* const __RESTRICT c,
    const size_t nrows_op_a, const size_t ncolumns_op_a, const size_t ncolumns_op_b)
{
}
#endif

#ifdef HAVE_OPENMP
template<typename T>
  inline void block_matmul_openmp(const T* const  __RESTRICT a, const T* const  __RESTRICT b, T* const __RESTRICT c,
    const size_t nrows_op_a, const size_t ncolumns_op_a, const size_t ncolumns_op_b)
{
}
#endif

#ifdef HAVE_CILK
template<typename T>
  inline void block_matmul_cilk(const T* const  __RESTRICT a, const T* const  __RESTRICT b, T* const __RESTRICT c,
    const size_t nrows_op_a, const size_t ncolumns_op_a, const size_t ncolumns_op_b)
{
}
#endif

#ifdef HAVE_TBB
template<typename T>
  inline void block_matmul_tbb(const T* const  __RESTRICT a, const T* const  __RESTRICT b, T* const __RESTRICT c,
    const size_t nrows_op_a, const size_t ncolumns_op_a, const size_t ncolumns_op_b)
{
}
#endif

template<typename T>
  inline void dgemm_block_helper(const T* const  __RESTRICT a, const T* const  __RESTRICT b, T* const __RESTRICT c,
    const size_t nrows_op_a, const size_t ncolumns_op_a, const size_t ncolumns_op_b,
    const TThreading threading_model)
{
  switch(threading_model)
  {
    case T_Serial:
      return block_matmul_serial<T>(a,b,c,nrows_op_a,ncolumns_op_a,ncolumns_op_b);
    case T_Std:
      return block_matmul_stdthreads<T>(a,b,c,nrows_op_a,ncolumns_op_a,ncolumns_op_b);
#ifdef HAVE_PTHREADS
    case T_Posix:
      return block_matmul_pthreads<T>(a,b,c,nrows_op_a,ncolumns_op_a,ncolumns_op_b);
#endif
#ifdef HAVE_OPENMP
    case T_OpenMP:
      return block_matmul_serial<T>(a,b,c,nrows_op_a,ncolumns_op_a,ncolumns_op_b);
#endif
#ifdef HAVE_CILK
    case T_Cilk:
      return block_matmul_cilk<T>(a,b,c,nrows_op_a,ncolumns_op_a,ncolumns_op_b);
#endif
#ifdef HAVE_TBB
    case T_TBB:
      return block_matmul_tbb<T>(a,b,c,nrows_op_a,ncolumns_op_a,ncolumns_op_b);
#endif
    case T_Undefined:
    default:
      return block_matmul_serial<T>(a,b,c,nrows_op_a,ncolumns_op_a,ncolumns_op_b);
  }
}

//generic version of dgemm, C=op(A)*op(B)
template<typename T>
  void dgemm_block(const TMatrixStorage stor,
      const T* const __RESTRICT a, const T* const __RESTRICT b, T* const __RESTRICT c,
      const size_t nrows_a, const size_t ncolumns_a,
      const size_t nrows_b, const size_t ncolumns_b,
      const TThreading threading_model)
{
  switch(stor)
  {
    case TMatrixStorage::RowMajor:
      {
        return dgemm_block_helper<T>(a,b,c,nrows_a,ncolumns_a,ncolumns_b,threading_model);
      }
    case TMatrixStorage::ColumnMajor:
      {
        //calculate C'=(op(B))'*(op(A))' instead of C=op(A)*op(B)
        //column major C is row major C', so corresponing mappings are: A<->B, tA<->tB, cA<->cB
        return dgemm_block_helper<T>(b,a,c,ncolumns_b,nrows_b,nrows_a,threading_model);
      }
  }
}

//generic version of block_dgemm for square matrices
template<typename T>
  void dgemm_block(const TMatrixStorage stor,
    const T* const __RESTRICT a, const T* const __RESTRICT b, T* const __RESTRICT c,
    const size_t sz,
    const TThreading threading_model)
{
    return dgemm_block<T>(stor,a,b,c,sz,sz,sz,sz,threading_model);
}


}

#endif
