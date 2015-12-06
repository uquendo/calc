#pragma once
#ifndef _BLAS_IMPL_HPP
#define _BLAS_IMPL_HPP
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

using std::size_t;

namespace numeric {


//elementary operation
template<typename T, bool tA, bool tB, bool cA, bool cB>
  __FORCEINLINE inline void _mm_op(const T* const  __RESTRICT a, const T* const  __RESTRICT b, T* const __RESTRICT c,
    const size_t stride_a, const size_t stride_b, const size_t stride_c,
    const size_t i, const size_t j, const size_t k)
{
  //if nothing's transposed, do c(i,k) += a(i,j) * b(j,k)
  //transpose option switches corresponding indices. tA: a(i,j) -> a(j,i), tB: b(j,k) -> b(k,j)
  c[i*stride_c+k] +=  ( cA ? conj<T>(a[tA ? j*stride_a+i : i*stride_a+j]) : a[tA ? j*stride_a+i : i*stride_a+j] ) *
                      ( cB ? conj<T>(b[tB ? k*stride_b+j : j*stride_b+k]) : b[tB ? k*stride_b+j : j*stride_b+k] );
}

//helper for simple serial matrix multiplication
template<typename T, TMM_Algo tAlgo, bool tA, bool tB, bool cA, bool cB, typename S = T>
  inline void matmul_serial(const T* const  __RESTRICT _a, const T* const  __RESTRICT _b, T* const __RESTRICT _c,
    const size_t nrows_op_a, const size_t ncolumns_op_a, const size_t ncolumns_op_b)
{
  constexpr bool ccA = (cA && is_complex<T>::value) ;
  constexpr bool ccB = (cB && is_complex<T>::value) ;
  const void*  __RESTRICT __a = reinterpret_cast<const void*>(_a);
  const void*  __RESTRICT __b = reinterpret_cast<const void*>(_b);
  void*  __RESTRICT __c = reinterpret_cast<void*>(_c);
  __ASSUME_ALIGNED(__a, DEFAULT_CACHE_LINE_SIZE);
  __ASSUME_ALIGNED(__b, DEFAULT_CACHE_LINE_SIZE);
  __ASSUME_ALIGNED(__c, DEFAULT_CACHE_LINE_SIZE);
  const S* const  __RESTRICT a = reinterpret_cast<const S* const>(__a);
  const S* const  __RESTRICT b = reinterpret_cast<const S* const>(__b);
  S* const  __RESTRICT c = reinterpret_cast<S* const>(__c);
  switch(tAlgo)
  {
//it's a bit of a shame, but these macros actually enchance readability IMO
#define _FOR_I for(size_t i=0; i < nrows_op_a; i++)
#define _FOR_J for(size_t j=0; j < ncolumns_op_a; j++)
#define _FOR_K for(size_t k=0; k < ncolumns_op_b; k++)
    case TMM_Algo::IJK:
        _FOR_I _FOR_J _FOR_K
          _mm_op<S,tA,tB,ccA,ccB>(a,b,c,ncolumns_op_a,ncolumns_op_b,ncolumns_op_b,i,j,k);
      return;
    case TMM_Algo::JKI:
        _FOR_J _FOR_K _FOR_I
          _mm_op<S,tA,tB,ccA,ccB>(a,b,c,ncolumns_op_a,ncolumns_op_b,ncolumns_op_b,i,j,k);
      return;
    case TMM_Algo::KIJ:
        _FOR_K _FOR_I _FOR_J
          _mm_op<S,tA,tB,ccA,ccB>(a,b,c,ncolumns_op_a,ncolumns_op_b,ncolumns_op_b,i,j,k);
      return;
    case TMM_Algo::IKJ:
        _FOR_I _FOR_K _FOR_J
          _mm_op<S,tA,tB,ccA,ccB>(a,b,c,ncolumns_op_a,ncolumns_op_b,ncolumns_op_b,i,j,k);
      return;
    case TMM_Algo::KJI:
        _FOR_K _FOR_J _FOR_I
          _mm_op<S,tA,tB,ccA,ccB>(a,b,c,ncolumns_op_a,ncolumns_op_b,ncolumns_op_b,i,j,k);
      return;
    case TMM_Algo::JIK:
        _FOR_J _FOR_I _FOR_K
          _mm_op<S,tA,tB,ccA,ccB>(a,b,c,ncolumns_op_a,ncolumns_op_b,ncolumns_op_b,i,j,k);
      return;
#undef _FOR_I
#undef _FOR_J
#undef _FOR_K
  }
}

template<typename T, TMM_Algo tAlgo, bool tA, bool tB, bool cA, bool cB, typename S = T>
  inline void matmul_stdthreads(const T* const  __RESTRICT _a, const T* const  __RESTRICT _b, T* const __RESTRICT _c,
    const size_t nrows_op_a, const size_t ncolumns_op_a, const size_t ncolumns_op_b)
{
  constexpr bool ccA = (cA && is_complex<T>::value) ;
  constexpr bool ccB = (cB && is_complex<T>::value) ;
  const void*  __RESTRICT __a = reinterpret_cast<const void*>(_a);
  const void*  __RESTRICT __b = reinterpret_cast<const void*>(_b);
  void*  __RESTRICT __c = reinterpret_cast<void*>(_c);
  __ASSUME_ALIGNED(__a, DEFAULT_CACHE_LINE_SIZE);
  __ASSUME_ALIGNED(__b, DEFAULT_CACHE_LINE_SIZE);
  __ASSUME_ALIGNED(__c, DEFAULT_CACHE_LINE_SIZE);
  const S* const  __RESTRICT a = reinterpret_cast<const S* const>(__a);
  const S* const  __RESTRICT b = reinterpret_cast<const S* const>(__b);
  S* const  __RESTRICT c = reinterpret_cast<S* const>(__c);
  switch(tAlgo)
  {
//it's a bit of a shame, but these macros actually enchance readability IMO
#define _FOR_I for(size_t i=0; i < nrows_op_a; i++)
#define _FOR_J for(size_t j=0; j < ncolumns_op_a; j++)
#define _FOR_K for(size_t k=0; k < ncolumns_op_b; k++)
    case TMM_Algo::IJK:
        _FOR_I _FOR_J _FOR_K
          _mm_op<S,tA,tB,ccA,ccB>(a,b,c,ncolumns_op_a,ncolumns_op_b,ncolumns_op_b,i,j,k);
      return;
    case TMM_Algo::JKI:
        _FOR_J _FOR_K _FOR_I
          _mm_op<S,tA,tB,ccA,ccB>(a,b,c,ncolumns_op_a,ncolumns_op_b,ncolumns_op_b,i,j,k);
      return;
    case TMM_Algo::KIJ:
        _FOR_K _FOR_I _FOR_J
          _mm_op<S,tA,tB,ccA,ccB>(a,b,c,ncolumns_op_a,ncolumns_op_b,ncolumns_op_b,i,j,k);
      return;
    case TMM_Algo::IKJ:
        _FOR_I _FOR_K _FOR_J
          _mm_op<S,tA,tB,ccA,ccB>(a,b,c,ncolumns_op_a,ncolumns_op_b,ncolumns_op_b,i,j,k);
      return;
    case TMM_Algo::KJI:
        _FOR_K _FOR_J _FOR_I
          _mm_op<S,tA,tB,ccA,ccB>(a,b,c,ncolumns_op_a,ncolumns_op_b,ncolumns_op_b,i,j,k);
      return;
    case TMM_Algo::JIK:
        _FOR_J _FOR_I _FOR_K
          _mm_op<S,tA,tB,ccA,ccB>(a,b,c,ncolumns_op_a,ncolumns_op_b,ncolumns_op_b,i,j,k);
      return;
#undef _FOR_I
#undef _FOR_J
#undef _FOR_K
  }
}

#ifdef HAVE_PTHREADS
template<typename T, TMM_Algo tAlgo, bool tA, bool tB, bool cA, bool cB, typename S = T>
  inline void matmul_pthreads(const T* const  __RESTRICT _a, const T* const  __RESTRICT _b, T* const __RESTRICT _c,
    const size_t nrows_op_a, const size_t ncolumns_op_a, const size_t ncolumns_op_b)
{
  constexpr bool ccA = (cA && is_complex<T>::value) ;
  constexpr bool ccB = (cB && is_complex<T>::value) ;
  const void*  __RESTRICT __a = reinterpret_cast<const void*>(_a);
  const void*  __RESTRICT __b = reinterpret_cast<const void*>(_b);
  void*  __RESTRICT __c = reinterpret_cast<void*>(_c);
  __ASSUME_ALIGNED(__a, DEFAULT_CACHE_LINE_SIZE);
  __ASSUME_ALIGNED(__b, DEFAULT_CACHE_LINE_SIZE);
  __ASSUME_ALIGNED(__c, DEFAULT_CACHE_LINE_SIZE);
  const S* const  __RESTRICT a = reinterpret_cast<const S* const>(__a);
  const S* const  __RESTRICT b = reinterpret_cast<const S* const>(__b);
  S* const  __RESTRICT c = reinterpret_cast<S* const>(__c);
  switch(tAlgo)
  {
//it's a bit of a shame, but these macros actually enchance readability IMO
#define _FOR_I for(size_t i=0; i < nrows_op_a; i++)
#define _FOR_J for(size_t j=0; j < ncolumns_op_a; j++)
#define _FOR_K for(size_t k=0; k < ncolumns_op_b; k++)
    case TMM_Algo::IJK:
        _FOR_I _FOR_J _FOR_K
          _mm_op<S,tA,tB,ccA,ccB>(a,b,c,ncolumns_op_a,ncolumns_op_b,ncolumns_op_b,i,j,k);
      return;
    case TMM_Algo::JKI:
        _FOR_J _FOR_K _FOR_I
          _mm_op<S,tA,tB,ccA,ccB>(a,b,c,ncolumns_op_a,ncolumns_op_b,ncolumns_op_b,i,j,k);
      return;
    case TMM_Algo::KIJ:
        _FOR_K _FOR_I _FOR_J
          _mm_op<S,tA,tB,ccA,ccB>(a,b,c,ncolumns_op_a,ncolumns_op_b,ncolumns_op_b,i,j,k);
      return;
    case TMM_Algo::IKJ:
        _FOR_I _FOR_K _FOR_J
          _mm_op<S,tA,tB,ccA,ccB>(a,b,c,ncolumns_op_a,ncolumns_op_b,ncolumns_op_b,i,j,k);
      return;
    case TMM_Algo::KJI:
        _FOR_K _FOR_J _FOR_I
          _mm_op<S,tA,tB,ccA,ccB>(a,b,c,ncolumns_op_a,ncolumns_op_b,ncolumns_op_b,i,j,k);
      return;
    case TMM_Algo::JIK:
        _FOR_J _FOR_I _FOR_K
          _mm_op<S,tA,tB,ccA,ccB>(a,b,c,ncolumns_op_a,ncolumns_op_b,ncolumns_op_b,i,j,k);
      return;
#undef _FOR_I
#undef _FOR_J
#undef _FOR_K
  }
}
#endif

#ifdef HAVE_OPENMP
template<typename T, TMM_Algo tAlgo, bool tA, bool tB, bool cA, bool cB, typename S = T>
  inline void matmul_openmp(const T* const  __RESTRICT _a, const T* const  __RESTRICT _b, T* const __RESTRICT _c,
    const size_t nrows_op_a, const size_t ncolumns_op_a, const size_t ncolumns_op_b)
{
  constexpr bool ccA = (cA && is_complex<T>::value) ;
  constexpr bool ccB = (cB && is_complex<T>::value) ;
  const void*  __RESTRICT __a = reinterpret_cast<const void*>(_a);
  const void*  __RESTRICT __b = reinterpret_cast<const void*>(_b);
  void*  __RESTRICT __c = reinterpret_cast<void*>(_c);
  __ASSUME_ALIGNED(__a, DEFAULT_CACHE_LINE_SIZE);
  __ASSUME_ALIGNED(__b, DEFAULT_CACHE_LINE_SIZE);
  __ASSUME_ALIGNED(__c, DEFAULT_CACHE_LINE_SIZE);
  const S* const  __RESTRICT a = reinterpret_cast<const S* const>(__a);
  const S* const  __RESTRICT b = reinterpret_cast<const S* const>(__b);
  S* const  __RESTRICT c = reinterpret_cast<S* const>(__c);
  switch(tAlgo)
  {
//it's a bit of a shame, but these macros actually enchance readability IMO
#define _FOR_I for(size_t i=0; i < nrows_op_a; i++)
#define _FOR_J for(size_t j=0; j < ncolumns_op_a; j++)
#define _FOR_K for(size_t k=0; k < ncolumns_op_b; k++)
    case TMM_Algo::IJK:
#pragma omp parallel for
        _FOR_I {
          _FOR_J _FOR_K
            _mm_op<S,tA,tB,ccA,ccB>(a,b,c,ncolumns_op_a,ncolumns_op_b,ncolumns_op_b,i,j,k);
        }
      return;
    case TMM_Algo::JKI:
#pragma omp parallel for
        _FOR_J {
          _FOR_K _FOR_I
            _mm_op<S,tA,tB,ccA,ccB>(a,b,c,ncolumns_op_a,ncolumns_op_b,ncolumns_op_b,i,j,k);
        }
    case TMM_Algo::KIJ:
#pragma omp parallel for
        _FOR_K {
          _FOR_I _FOR_J
            _mm_op<S,tA,tB,ccA,ccB>(a,b,c,ncolumns_op_a,ncolumns_op_b,ncolumns_op_b,i,j,k);
        }
    case TMM_Algo::IKJ:
#pragma omp parallel for
        _FOR_I {
          _FOR_K _FOR_J
            _mm_op<S,tA,tB,ccA,ccB>(a,b,c,ncolumns_op_a,ncolumns_op_b,ncolumns_op_b,i,j,k);
        }
      return;
    case TMM_Algo::KJI:
#pragma omp parallel for
        _FOR_K {
          _FOR_J _FOR_I
            _mm_op<S,tA,tB,ccA,ccB>(a,b,c,ncolumns_op_a,ncolumns_op_b,ncolumns_op_b,i,j,k);
        }
      return;
    case TMM_Algo::JIK:
#pragma omp parallel for
        _FOR_J {
          _FOR_I _FOR_K
            _mm_op<S,tA,tB,ccA,ccB>(a,b,c,ncolumns_op_a,ncolumns_op_b,ncolumns_op_b,i,j,k);
        }
      return;
#undef _FOR_I
#undef _FOR_J
#undef _FOR_K
  }
}
#endif

#ifdef HAVE_CILK
template<typename T, TMM_Algo tAlgo, bool tA, bool tB, bool cA, bool cB, typename S = T>
  inline void matmul_cilk(const T* const  __RESTRICT _a, const T* const  __RESTRICT _b, T* const __RESTRICT _c,
    const size_t nrows_op_a, const size_t ncolumns_op_a, const size_t ncolumns_op_b)
{
  constexpr bool ccA = (cA && is_complex<T>::value) ;
  constexpr bool ccB = (cB && is_complex<T>::value) ;
  const void*  __RESTRICT __a = reinterpret_cast<const void*>(_a);
  const void*  __RESTRICT __b = reinterpret_cast<const void*>(_b);
  void*  __RESTRICT __c = reinterpret_cast<void*>(_c);
  __ASSUME_ALIGNED(__a, DEFAULT_CACHE_LINE_SIZE);
  __ASSUME_ALIGNED(__b, DEFAULT_CACHE_LINE_SIZE);
  __ASSUME_ALIGNED(__c, DEFAULT_CACHE_LINE_SIZE);
  const S* const  __RESTRICT a = reinterpret_cast<const S* const>(__a);
  const S* const  __RESTRICT b = reinterpret_cast<const S* const>(__b);
  S* const  __RESTRICT c = reinterpret_cast<S* const>(__c);
  switch(tAlgo)
  {
//it's a bit of a shame, but these macros actually enchance readability IMO
#define _FOR_I for(size_t i=0; i < nrows_op_a; i++)
#define _FOR_J for(size_t j=0; j < ncolumns_op_a; j++)
#define _FOR_K for(size_t k=0; k < ncolumns_op_b; k++)
#define _CILK_FOR_I cilk_for(size_t i=0; i < nrows_op_a; i++)
#define _CILK_FOR_J cilk_for(size_t j=0; j < ncolumns_op_a; j++)
#define _CILK_FOR_K cilk_for(size_t k=0; k < ncolumns_op_b; k++)
    case TMM_Algo::IJK:
        _CILK_FOR_I _FOR_J _FOR_K
          _mm_op<S,tA,tB,ccA,ccB>(a,b,c,ncolumns_op_a,ncolumns_op_b,ncolumns_op_b,i,j,k);
      return;
    case TMM_Algo::JKI:
        _CILK_FOR_J _FOR_K _FOR_I
          _mm_op<S,tA,tB,ccA,ccB>(a,b,c,ncolumns_op_a,ncolumns_op_b,ncolumns_op_b,i,j,k);
      return;
    case TMM_Algo::KIJ:
        _CILK_FOR_K _FOR_I _FOR_J
          _mm_op<S,tA,tB,ccA,ccB>(a,b,c,ncolumns_op_a,ncolumns_op_b,ncolumns_op_b,i,j,k);
      return;
    case TMM_Algo::IKJ:
        _CILK_FOR_I _FOR_K _FOR_J
          _mm_op<S,tA,tB,ccA,ccB>(a,b,c,ncolumns_op_a,ncolumns_op_b,ncolumns_op_b,i,j,k);
      return;
    case TMM_Algo::KJI:
        _CILK_FOR_K _FOR_J _FOR_I
          _mm_op<S,tA,tB,ccA,ccB>(a,b,c,ncolumns_op_a,ncolumns_op_b,ncolumns_op_b,i,j,k);
      return;
    case TMM_Algo::JIK:
        _CILK_FOR_J _FOR_I _FOR_K
          _mm_op<S,tA,tB,ccA,ccB>(a,b,c,ncolumns_op_a,ncolumns_op_b,ncolumns_op_b,i,j,k);
      return;
#undef _FOR_I
#undef _FOR_J
#undef _FOR_K
#undef _CILK_FOR_I
#undef _CILK_FOR_J
#undef _CILK_FOR_K
  }
}
#endif

#ifdef HAVE_TBB
template<typename T, TMM_Algo tAlgo, bool tA, bool tB, bool cA, bool cB, typename S = T>
  inline void matmul_tbb(const T* const  __RESTRICT _a, const T* const  __RESTRICT _b, T* const __RESTRICT _c,
    const size_t nrows_op_a, const size_t ncolumns_op_a, const size_t ncolumns_op_b)
{
  constexpr bool ccA = (cA && is_complex<T>::value) ;
  constexpr bool ccB = (cB && is_complex<T>::value) ;
  const void*  __RESTRICT __a = reinterpret_cast<const void*>(_a);
  const void*  __RESTRICT __b = reinterpret_cast<const void*>(_b);
  void*  __RESTRICT __c = reinterpret_cast<void*>(_c);
  __ASSUME_ALIGNED(__a, DEFAULT_CACHE_LINE_SIZE);
  __ASSUME_ALIGNED(__b, DEFAULT_CACHE_LINE_SIZE);
  __ASSUME_ALIGNED(__c, DEFAULT_CACHE_LINE_SIZE);
  const S* const  __RESTRICT a = reinterpret_cast<const S* const>(__a);
  const S* const  __RESTRICT b = reinterpret_cast<const S* const>(__b);
  S* const  __RESTRICT c = reinterpret_cast<S* const>(__c);
  switch(tAlgo)
  {
//it's a bit of a shame, but these macros actually enchance readability IMO
#define _FOR_I for(size_t i=0; i < nrows_op_a; i++)
#define _FOR_J for(size_t j=0; j < ncolumns_op_a; j++)
#define _FOR_K for(size_t k=0; k < ncolumns_op_b; k++)
#define _TBB_FOR_I size_t(0), nrows_op_a,[&a,&b,&c,&nrows_op_a,&ncolumns_op_a,&ncolumns_op_b](size_t i)
#define _TBB_FOR_J size_t(0), ncolumns_op_a,[&a,&b,&c,&nrows_op_a,&ncolumns_op_a,&ncolumns_op_b](size_t j)
#define _TBB_FOR_K size_t(0), ncolumns_op_b,[&a,&b,&c,&nrows_op_a,&ncolumns_op_a,&ncolumns_op_b](size_t k)
    case TMM_Algo::IJK:
        parallelForElem( _TBB_FOR_I {
          _FOR_J _FOR_K
            _mm_op<S,tA,tB,ccA,ccB>(a,b,c,ncolumns_op_a,ncolumns_op_b,ncolumns_op_b,i,j,k);
        } );
      return;
    case TMM_Algo::JKI:
        parallelForElem( _TBB_FOR_J {
          _FOR_K _FOR_I
            _mm_op<S,tA,tB,ccA,ccB>(a,b,c,ncolumns_op_a,ncolumns_op_b,ncolumns_op_b,i,j,k);
        } );
    case TMM_Algo::KIJ:
        parallelForElem( _TBB_FOR_K {
          _FOR_I _FOR_J
            _mm_op<S,tA,tB,ccA,ccB>(a,b,c,ncolumns_op_a,ncolumns_op_b,ncolumns_op_b,i,j,k);
        } );
    case TMM_Algo::IKJ:
        parallelForElem( _TBB_FOR_I {
          _FOR_K _FOR_J
            _mm_op<S,tA,tB,ccA,ccB>(a,b,c,ncolumns_op_a,ncolumns_op_b,ncolumns_op_b,i,j,k);
        } );
      return;
    case TMM_Algo::KJI:
        parallelForElem( _TBB_FOR_K {
          _FOR_J _FOR_I
            _mm_op<S,tA,tB,ccA,ccB>(a,b,c,ncolumns_op_a,ncolumns_op_b,ncolumns_op_b,i,j,k);
        } );
      return;
    case TMM_Algo::JIK:
        parallelForElem( _TBB_FOR_J {
          _FOR_I _FOR_K
            _mm_op<S,tA,tB,ccA,ccB>(a,b,c,ncolumns_op_a,ncolumns_op_b,ncolumns_op_b,i,j,k);
        } );
      return;
#undef _FOR_I
#undef _FOR_J
#undef _FOR_K
#undef _TBB_FOR_I
#undef _TBB_FOR_J
#undef _TBB_FOR_K
  }
}
#endif

template<typename T, TMM_Algo tAlgo, bool tA, bool tB, bool cA, bool cB, typename S = T>
  inline void dgemm_helper(const T* const  __RESTRICT a, const T* const  __RESTRICT b, T* const __RESTRICT c,
    const size_t nrows_op_a, const size_t ncolumns_op_a, const size_t ncolumns_op_b,
    const TThreading threading_model)
{
  switch(threading_model)
  {
    case T_Serial:
      return matmul_serial<T,tAlgo,tA,tB,cA,cB,S>(a,b,c,nrows_op_a,ncolumns_op_a,ncolumns_op_b);
    case T_Std:
      return matmul_stdthreads<T,tAlgo,tA,tB,cA,cB,S>(a,b,c,nrows_op_a,ncolumns_op_a,ncolumns_op_b);
#ifdef HAVE_PTHREADS
    case T_Posix:
      return matmul_pthreads<T,tAlgo,tA,tB,cA,cB,S>(a,b,c,nrows_op_a,ncolumns_op_a,ncolumns_op_b);
#endif
#ifdef HAVE_OPENMP
    case T_OpenMP:
      return matmul_openmp<T,tAlgo,tA,tB,cA,cB,S>(a,b,c,nrows_op_a,ncolumns_op_a,ncolumns_op_b);
#endif
#ifdef HAVE_CILK
    case T_Cilk:
      return matmul_cilk<T,tAlgo,tA,tB,cA,cB,S>(a,b,c,nrows_op_a,ncolumns_op_a,ncolumns_op_b);
#endif
#ifdef HAVE_TBB
    case T_TBB:
      return matmul_tbb<T,tAlgo,tA,tB,cA,cB,S>(a,b,c,nrows_op_a,ncolumns_op_a,ncolumns_op_b);
#endif
    case T_Undefined:
    default:
      return matmul_serial<T,tAlgo,tA,tB,cA,cB,S>(a,b,c,nrows_op_a,ncolumns_op_a,ncolumns_op_b);
  }
}


//generic alignment-aware version of dgemm, C=op(A)*op(B)
template<typename T, typename S>
  void dgemm(const TMatrixStorage stor, const TMatrixTranspose transA, const TMatrixTranspose transB,
      const T* const __RESTRICT a, const T* const __RESTRICT b, T* const __RESTRICT c,
      const size_t nrows_a, const size_t ncolumns_a,
      const size_t nrows_b, const size_t ncolumns_b,
      const TThreading threading_model)
{
  const bool tA = (transA != TMatrixTranspose::No) ;
  const bool tB = (transB != TMatrixTranspose::No) ;
  const bool cA = (transA == TMatrixTranspose::Conjugate && is_complex<T>::value) ;
  const bool cB = (transB == TMatrixTranspose::Conjugate && is_complex<T>::value) ;

  switch(stor)
  {
    case TMatrixStorage::RowMajor:
      {
        if(tA && tB) // KIJ
        {
          if(cA && cB)
            return dgemm_helper<T,TMM_Algo::KIJ,true,true,true,true,S>(a,b,c,ncolumns_a,nrows_a,nrows_b,threading_model);
          else if(cA && !cB)
            return dgemm_helper<T,TMM_Algo::KIJ,true,true,true,false,S>(a,b,c,ncolumns_a,nrows_a,nrows_b,threading_model);
          else if(!cA && cB)
            return dgemm_helper<T,TMM_Algo::KIJ,true,true,false,true,S>(a,b,c,ncolumns_a,nrows_a,nrows_b,threading_model);
          else //if((!cA) && (!cB))
            return dgemm_helper<T,TMM_Algo::KIJ,true,true,false,false,S>(a,b,c,ncolumns_a,nrows_a,nrows_b,threading_model);
        }
        else if (tA && !tB) // JIK
        {
          if(cA)
            return dgemm_helper<T,TMM_Algo::JIK,true,false,true,false,S>(a,b,c,ncolumns_a,nrows_a,ncolumns_b,threading_model);
          else
            return dgemm_helper<T,TMM_Algo::JIK,true,false,false,false,S>(a,b,c,ncolumns_a,nrows_a,ncolumns_b,threading_model);
        }
        else if(!tA && tB) // IKJ
        {
          if(cB)
            return dgemm_helper<T,TMM_Algo::IKJ,false,true,false,true,S>(a,b,c,nrows_a,ncolumns_a,nrows_b,threading_model);
          else
            return dgemm_helper<T,TMM_Algo::IKJ,false,true,false,false,S>(a,b,c,nrows_a,ncolumns_a,nrows_b,threading_model);
        }
        else //if((!tA) && (!tB)) // IJK
        {
          return dgemm_helper<T,TMM_Algo::IJK,false,false,false,false,S>(a,b,c,nrows_a,ncolumns_a,ncolumns_b,threading_model);
        }
      }
    case TMatrixStorage::ColumnMajor:
      {
        //calculate C'=(op(B))'*(op(A))' instead of C=op(A)*op(B)
        //corresponing mappings: tA->!tA,tB->!tB, A<->B
        if(!tA && !tB) // KIJ
        {
          return dgemm_helper<T,TMM_Algo::IJK,true,true,false,false,S>(b,a,c,ncolumns_b,nrows_b,nrows_a,threading_model);
        }
        else if (!tB && tA) // JIK
        {
          if(cA)
            return dgemm_helper<T,TMM_Algo::JIK,true,false,false,true,S>(b,a,c,ncolumns_b,nrows_b,ncolumns_a,threading_model);
          else
            return dgemm_helper<T,TMM_Algo::JIK,true,false,false,false,S>(b,a,c,ncolumns_b,nrows_b,ncolumns_a,threading_model);
        }
        else if(tB && !tA) // IKJ
        {
          if(cB)
            return dgemm_helper<T,TMM_Algo::IKJ,false,true,true,false,S>(b,a,c,nrows_b,ncolumns_b,nrows_a,threading_model);
          else
            return dgemm_helper<T,TMM_Algo::IKJ,false,true,false,false,S>(b,a,c,nrows_b,ncolumns_b,nrows_a,threading_model);
        }
        else //if(tA && tB) // IJK
        {
          if(cA && cB)
            return dgemm_helper<T,TMM_Algo::KIJ,false,false,true,true,S>(b,a,c,nrows_b,ncolumns_b,ncolumns_a,threading_model);
          else if(cA && !cB)
            return dgemm_helper<T,TMM_Algo::KIJ,false,false,false,true,S>(b,a,c,nrows_b,ncolumns_b,ncolumns_a,threading_model);
          else if(!cA && cB)
            return dgemm_helper<T,TMM_Algo::KIJ,false,false,true,false,S>(b,a,c,nrows_b,ncolumns_b,ncolumns_a,threading_model);
          else //if((!cA) && (!cB))
            return dgemm_helper<T,TMM_Algo::KIJ,false,false,false,false,S>(b,a,c,nrows_b,ncolumns_b,ncolumns_a,threading_model);
        }
      }
  }

}
/*
//generic unaligned version of dgemm, C=op(A)*op(B)
template<typename T>
  void dgemm(const TMatrixStorage stor, const TMatrixTranspose transA, const TMatrixTranspose transB,
      const T* const __RESTRICT a, const T* const __RESTRICT b, T* const __RESTRICT c,
      const size_t nrows_a, const size_t ncolumns_a,
      const size_t nrows_b, const size_t ncolumns_b,
      const TThreading threading_model)
{
  return aligned_dgemm<T,T,0>(stor,transA,transB,a,b,c,nrows_a,ncolumns_a,nrows_b,ncolumns_b,threading_model);
}
*/
//generic alignment-aware version of dgemm for square matrices
template<typename T, typename S>
  void dgemm(const TMatrixStorage stor, const TMatrixTranspose transA, const TMatrixTranspose transB,
    const T* const __RESTRICT a, const T* const __RESTRICT b, T* const __RESTRICT c,
    const size_t sz,
    const TThreading threading_model)
{
    return dgemm<T,S>(stor,transA,transB,a,b,c,sz,sz,sz,sz,threading_model);
}
/*
//generic unaligned version of dgemm for square matrices
template<typename T>
  void dgemm(const TMatrixStorage stor, const TMatrixTranspose transA, const TMatrixTranspose transB,
    const T* const __RESTRICT a, const T* const __RESTRICT b, T* const __RESTRICT c,
    const size_t sz,
    const TThreading threading_model)
{
    return aligned_dgemm<T,0>(stor,transA,transB,a,b,c,sz,threading_model);
}
*/
}

#endif
