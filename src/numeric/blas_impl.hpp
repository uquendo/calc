#pragma once
#ifndef _BLAS_IMPL_HPP
#define _BLAS_IMPL_HPP
#include "config.h"

#include "numeric/blas.hpp"
#include "numeric/real.hpp"
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

//elemental operation for simple matrix multiplication
template<typename T, bool tA, bool tB, bool cA, bool cB>
  __FORCEINLINE inline void _gemm_op(const T* const  __RESTRICT a, const T* const  __RESTRICT b, T* const __RESTRICT c,
    const size_t stride_a, const size_t stride_b, const size_t stride_c,
    const size_t i, const size_t j, const size_t k)
{
  //if nothing's transposed, do c(i,k) += a(i,j) * b(j,k)
  //transpose option switches corresponding indices. tA: a(i,j) -> a(j,i), tB: b(j,k) -> b(k,j)
  c[i*stride_c+k] +=  ( cA ? conj<T>(a[tA ? j*stride_a+i : i*stride_a+j]) : a[tA ? j*stride_a+i : i*stride_a+j] ) *
                      ( cB ? conj<T>(b[tB ? k*stride_b+j : j*stride_b+k]) : b[tB ? k*stride_b+j : j*stride_b+k] );
}

template<typename T, TMM_Algo tAlgo, bool tA, bool tB, bool cA, bool cB>
  inline void matmul_serial(const T* const  __RESTRICT a, const T* const  __RESTRICT b, T* const __RESTRICT c,
    const size_t nrows_op_a, const size_t ncolumns_op_a, const size_t ncolumns_op_b)
{
  constexpr bool ccA = (cA && is_complex<T>::value) ;
  constexpr bool ccB = (cB && is_complex<T>::value) ;
  switch(tAlgo)
  {
//it's a bit of a shame, but these macros actually enchance readability IMO
#define _FOR_I for(size_t i=0; i < nrows_op_a; i++)
#define _FOR_J for(size_t j=0; j < ncolumns_op_a; j++)
#define _FOR_K for(size_t k=0; k < ncolumns_op_b; k++)
    case TMM_Algo::IJK:
        _FOR_I _FOR_J _FOR_K
            _gemm_op<T,tA,tB,ccA,ccB>(a,b,c,ncolumns_op_a,ncolumns_op_b,ncolumns_op_b,i,j,k);
      return;
    case TMM_Algo::JKI:
        _FOR_J _FOR_K _FOR_I
          _gemm_op<T,tA,tB,ccA,ccB>(a,b,c,ncolumns_op_a,ncolumns_op_b,ncolumns_op_b,i,j,k);
      return;
    case TMM_Algo::KIJ:
        _FOR_K _FOR_I _FOR_J
          _gemm_op<T,tA,tB,ccA,ccB>(a,b,c,ncolumns_op_a,ncolumns_op_b,ncolumns_op_b,i,j,k);
      return;
    case TMM_Algo::IKJ:
        _FOR_I _FOR_K _FOR_J
              _gemm_op<T,tA,tB,ccA,ccB>(a,b,c,ncolumns_op_a,ncolumns_op_b,ncolumns_op_b,i,j,k);
      return;
    case TMM_Algo::KJI:
        _FOR_K _FOR_J _FOR_I
          _gemm_op<T,tA,tB,ccA,ccB>(a,b,c,ncolumns_op_a,ncolumns_op_b,ncolumns_op_b,i,j,k);
      return;
    case TMM_Algo::JIK:
        _FOR_J _FOR_I _FOR_K
          _gemm_op<T,tA,tB,ccA,ccB>(a,b,c,ncolumns_op_a,ncolumns_op_b,ncolumns_op_b,i,j,k);
      return;
#undef _FOR_I
#undef _FOR_J
#undef _FOR_K
  }
}

template<typename T, TMM_Algo tAlgo, bool tA, bool tB, bool cA, bool cB>
  inline void matmul_stdthreads(const T* const  __RESTRICT a, const T* const  __RESTRICT b, T* const __RESTRICT c,
    const size_t nrows_op_a, const size_t ncolumns_op_a, const size_t ncolumns_op_b)
{
  constexpr bool ccA = (cA && is_complex<T>::value) ;
  constexpr bool ccB = (cB && is_complex<T>::value) ;
  switch(tAlgo)
  {
//it's a bit of a shame, but these macros actually enchance readability IMO
#define _FOR_I for(size_t i=0; i < nrows_op_a; i++)
#define _FOR_J for(size_t j=0; j < ncolumns_op_a; j++)
#define _FOR_K for(size_t k=0; k < ncolumns_op_b; k++)
    case TMM_Algo::IJK:
        _FOR_I _FOR_J _FOR_K
          _gemm_op<T,tA,tB,ccA,ccB>(a,b,c,ncolumns_op_a,ncolumns_op_b,ncolumns_op_b,i,j,k);
      return;
    case TMM_Algo::JKI:
        _FOR_J _FOR_K _FOR_I
          _gemm_op<T,tA,tB,ccA,ccB>(a,b,c,ncolumns_op_a,ncolumns_op_b,ncolumns_op_b,i,j,k);
      return;
    case TMM_Algo::KIJ:
        _FOR_K _FOR_I _FOR_J
          _gemm_op<T,tA,tB,ccA,ccB>(a,b,c,ncolumns_op_a,ncolumns_op_b,ncolumns_op_b,i,j,k);
      return;
    case TMM_Algo::IKJ:
        _FOR_I _FOR_K _FOR_J
          _gemm_op<T,tA,tB,ccA,ccB>(a,b,c,ncolumns_op_a,ncolumns_op_b,ncolumns_op_b,i,j,k);
      return;
    case TMM_Algo::KJI:
        _FOR_K _FOR_J _FOR_I
          _gemm_op<T,tA,tB,ccA,ccB>(a,b,c,ncolumns_op_a,ncolumns_op_b,ncolumns_op_b,i,j,k);
      return;
    case TMM_Algo::JIK:
        _FOR_J _FOR_I _FOR_K
          _gemm_op<T,tA,tB,ccA,ccB>(a,b,c,ncolumns_op_a,ncolumns_op_b,ncolumns_op_b,i,j,k);
      return;
#undef _FOR_I
#undef _FOR_J
#undef _FOR_K
  }
}

#ifdef HAVE_PTHREADS
template<typename T, TMM_Algo tAlgo, bool tA, bool tB, bool cA, bool cB>
  inline void matmul_pthreads(const T* const  __RESTRICT a, const T* const  __RESTRICT b, T* const __RESTRICT c,
    const size_t nrows_op_a, const size_t ncolumns_op_a, const size_t ncolumns_op_b)
{
  constexpr bool ccA = (cA && is_complex<T>::value) ;
  constexpr bool ccB = (cB && is_complex<T>::value) ;
  switch(tAlgo)
  {
//it's a bit of a shame, but these macros actually enchance readability IMO
#define _FOR_I for(size_t i=0; i < nrows_op_a; i++)
#define _FOR_J for(size_t j=0; j < ncolumns_op_a; j++)
#define _FOR_K for(size_t k=0; k < ncolumns_op_b; k++)
    case TMM_Algo::IJK:
        _FOR_I _FOR_J _FOR_K
          _gemm_op<T,tA,tB,ccA,ccB>(a,b,c,ncolumns_op_a,ncolumns_op_b,ncolumns_op_b,i,j,k);
      return;
    case TMM_Algo::JKI:
        _FOR_J _FOR_K _FOR_I
          _gemm_op<T,tA,tB,ccA,ccB>(a,b,c,ncolumns_op_a,ncolumns_op_b,ncolumns_op_b,i,j,k);
      return;
    case TMM_Algo::KIJ:
        _FOR_K _FOR_I _FOR_J
          _gemm_op<T,tA,tB,ccA,ccB>(a,b,c,ncolumns_op_a,ncolumns_op_b,ncolumns_op_b,i,j,k);
      return;
    case TMM_Algo::IKJ:
        _FOR_I _FOR_K _FOR_J
          _gemm_op<T,tA,tB,ccA,ccB>(a,b,c,ncolumns_op_a,ncolumns_op_b,ncolumns_op_b,i,j,k);
      return;
    case TMM_Algo::KJI:
        _FOR_K _FOR_J _FOR_I
          _gemm_op<T,tA,tB,ccA,ccB>(a,b,c,ncolumns_op_a,ncolumns_op_b,ncolumns_op_b,i,j,k);
      return;
    case TMM_Algo::JIK:
        _FOR_J _FOR_I _FOR_K
          _gemm_op<T,tA,tB,ccA,ccB>(a,b,c,ncolumns_op_a,ncolumns_op_b,ncolumns_op_b,i,j,k);
      return;
#undef _FOR_I
#undef _FOR_J
#undef _FOR_K
  }
}
#endif

#ifdef HAVE_OPENMP
template<typename T, TMM_Algo tAlgo, bool tA, bool tB, bool cA, bool cB>
  inline void matmul_openmp(const T* const  __RESTRICT a, const T* const  __RESTRICT b, T* const __RESTRICT c,
    const size_t nrows_op_a, const size_t ncolumns_op_a, const size_t ncolumns_op_b)
{
  constexpr bool ccA = (cA && is_complex<T>::value) ;
  constexpr bool ccB = (cB && is_complex<T>::value) ;
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
            _gemm_op<T,tA,tB,ccA,ccB>(a,b,c,ncolumns_op_a,ncolumns_op_b,ncolumns_op_b,i,j,k);
        }
      return;
    case TMM_Algo::JKI:
#pragma omp parallel for
        _FOR_J {
          _FOR_K _FOR_I
            _gemm_op<T,tA,tB,ccA,ccB>(a,b,c,ncolumns_op_a,ncolumns_op_b,ncolumns_op_b,i,j,k);
        }
    case TMM_Algo::KIJ:
#pragma omp parallel for
        _FOR_K {
          _FOR_I _FOR_J
            _gemm_op<T,tA,tB,ccA,ccB>(a,b,c,ncolumns_op_a,ncolumns_op_b,ncolumns_op_b,i,j,k);
        }
    case TMM_Algo::IKJ:
#pragma omp parallel for
        _FOR_I {
          _FOR_K _FOR_J
            _gemm_op<T,tA,tB,ccA,ccB>(a,b,c,ncolumns_op_a,ncolumns_op_b,ncolumns_op_b,i,j,k);
        }
      return;
    case TMM_Algo::KJI:
#pragma omp parallel for
        _FOR_K {
          _FOR_J _FOR_I
            _gemm_op<T,tA,tB,ccA,ccB>(a,b,c,ncolumns_op_a,ncolumns_op_b,ncolumns_op_b,i,j,k);
        }
      return;
    case TMM_Algo::JIK:
#pragma omp parallel for
        _FOR_J {
          _FOR_I _FOR_K
            _gemm_op<T,tA,tB,ccA,ccB>(a,b,c,ncolumns_op_a,ncolumns_op_b,ncolumns_op_b,i,j,k);
        }
      return;
#undef _FOR_I
#undef _FOR_J
#undef _FOR_K
  }
}
#endif

#ifdef HAVE_CILK
template<typename T, TMM_Algo tAlgo, bool tA, bool tB, bool cA, bool cB>
  inline void matmul_cilk(const T* const  __RESTRICT a, const T* const  __RESTRICT b, T* const __RESTRICT c,
    const size_t nrows_op_a, const size_t ncolumns_op_a, const size_t ncolumns_op_b)
{
  constexpr bool ccA = (cA && is_complex<T>::value) ;
  constexpr bool ccB = (cB && is_complex<T>::value) ;
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
          _gemm_op<T,tA,tB,ccA,ccB>(a,b,c,ncolumns_op_a,ncolumns_op_b,ncolumns_op_b,i,j,k);
      return;
    case TMM_Algo::JKI:
        _CILK_FOR_J _FOR_K _FOR_I
          _gemm_op<T,tA,tB,ccA,ccB>(a,b,c,ncolumns_op_a,ncolumns_op_b,ncolumns_op_b,i,j,k);
      return;
    case TMM_Algo::KIJ:
        _CILK_FOR_K _FOR_I _FOR_J
          _gemm_op<T,tA,tB,ccA,ccB>(a,b,c,ncolumns_op_a,ncolumns_op_b,ncolumns_op_b,i,j,k);
      return;
    case TMM_Algo::IKJ:
        _CILK_FOR_I _FOR_K _FOR_J
          _gemm_op<T,tA,tB,ccA,ccB>(a,b,c,ncolumns_op_a,ncolumns_op_b,ncolumns_op_b,i,j,k);
      return;
    case TMM_Algo::KJI:
        _CILK_FOR_K _FOR_J _FOR_I
          _gemm_op<T,tA,tB,ccA,ccB>(a,b,c,ncolumns_op_a,ncolumns_op_b,ncolumns_op_b,i,j,k);
      return;
    case TMM_Algo::JIK:
        _CILK_FOR_J _FOR_I _FOR_K
          _gemm_op<T,tA,tB,ccA,ccB>(a,b,c,ncolumns_op_a,ncolumns_op_b,ncolumns_op_b,i,j,k);
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
template<typename T, TMM_Algo tAlgo, bool tA, bool tB, bool cA, bool cB>
  inline void matmul_tbb(const T* const  __RESTRICT a, const T* const  __RESTRICT b, T* const __RESTRICT c,
    const size_t nrows_op_a, const size_t ncolumns_op_a, const size_t ncolumns_op_b)
{
  constexpr bool ccA = (cA && is_complex<T>::value) ;
  constexpr bool ccB = (cB && is_complex<T>::value) ;
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
            _gemm_op<T,tA,tB,ccA,ccB>(a,b,c,ncolumns_op_a,ncolumns_op_b,ncolumns_op_b,i,j,k);
        } );
      return;
    case TMM_Algo::JKI:
        parallelForElem( _TBB_FOR_J {
          _FOR_K _FOR_I
            _gemm_op<T,tA,tB,ccA,ccB>(a,b,c,ncolumns_op_a,ncolumns_op_b,ncolumns_op_b,i,j,k);
        } );
    case TMM_Algo::KIJ:
        parallelForElem( _TBB_FOR_K {
          _FOR_I _FOR_J
            _gemm_op<T,tA,tB,ccA,ccB>(a,b,c,ncolumns_op_a,ncolumns_op_b,ncolumns_op_b,i,j,k);
        } );
    case TMM_Algo::IKJ:
        parallelForElem( _TBB_FOR_I {
          _FOR_K _FOR_J
            _gemm_op<T,tA,tB,ccA,ccB>(a,b,c,ncolumns_op_a,ncolumns_op_b,ncolumns_op_b,i,j,k);
        } );
      return;
    case TMM_Algo::KJI:
        parallelForElem( _TBB_FOR_K {
          _FOR_J _FOR_I
            _gemm_op<T,tA,tB,ccA,ccB>(a,b,c,ncolumns_op_a,ncolumns_op_b,ncolumns_op_b,i,j,k);
        } );
      return;
    case TMM_Algo::JIK:
        parallelForElem( _TBB_FOR_J {
          _FOR_I _FOR_K
            _gemm_op<T,tA,tB,ccA,ccB>(a,b,c,ncolumns_op_a,ncolumns_op_b,ncolumns_op_b,i,j,k);
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

template<typename T, TMM_Algo tAlgo, bool tA, bool tB, bool cA, bool cB>
  inline void dgemm_helper(const T* const  __RESTRICT a, const T* const  __RESTRICT b, T* const __RESTRICT c,
    const size_t nrows_op_a, const size_t ncolumns_op_a, const size_t ncolumns_op_b,
    const TThreading threading_model)
{
  switch(threading_model)
  {
    case T_Serial:
      return matmul_serial<T,tAlgo,tA,tB,cA,cB>(a,b,c,nrows_op_a,ncolumns_op_a,ncolumns_op_b);
    case T_Std:
      return matmul_stdthreads<T,tAlgo,tA,tB,cA,cB>(a,b,c,nrows_op_a,ncolumns_op_a,ncolumns_op_b);
#ifdef HAVE_PTHREADS
    case T_Posix:
      return matmul_pthreads<T,tAlgo,tA,tB,cA,cB>(a,b,c,nrows_op_a,ncolumns_op_a,ncolumns_op_b);
#endif
#ifdef HAVE_OPENMP
    case T_OpenMP:
      return matmul_openmp<T,tAlgo,tA,tB,cA,cB>(a,b,c,nrows_op_a,ncolumns_op_a,ncolumns_op_b);
#endif
#ifdef HAVE_CILK
    case T_Cilk:
      return matmul_cilk<T,tAlgo,tA,tB,cA,cB>(a,b,c,nrows_op_a,ncolumns_op_a,ncolumns_op_b);
#endif
#ifdef HAVE_TBB
    case T_TBB:
      return matmul_tbb<T,tAlgo,tA,tB,cA,cB>(a,b,c,nrows_op_a,ncolumns_op_a,ncolumns_op_b);
#endif
    case T_Undefined:
    default:
      return matmul_serial<T,tAlgo,tA,tB,cA,cB>(a,b,c,nrows_op_a,ncolumns_op_a,ncolumns_op_b);
  }
}

//generic version of dgemm, C=op(A)*op(B)
template<typename T>
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
            return dgemm_helper<T,TMM_Algo::KIJ,true,true,true,true>(a,b,c,ncolumns_a,nrows_a,nrows_b,threading_model);
          else if(cA && !cB)
            return dgemm_helper<T,TMM_Algo::KIJ,true,true,true,false>(a,b,c,ncolumns_a,nrows_a,nrows_b,threading_model);
          else if(!cA && cB)
            return dgemm_helper<T,TMM_Algo::KIJ,true,true,false,true>(a,b,c,ncolumns_a,nrows_a,nrows_b,threading_model);
          else //if((!cA) && (!cB))
            return dgemm_helper<T,TMM_Algo::KIJ,true,true,false,false>(a,b,c,ncolumns_a,nrows_a,nrows_b,threading_model);
        }
        else if (tA && !tB) // JIK
        {
          if(cA)
            return dgemm_helper<T,TMM_Algo::JIK,true,false,true,false>(a,b,c,ncolumns_a,nrows_a,ncolumns_b,threading_model);
          else
            return dgemm_helper<T,TMM_Algo::JIK,true,false,false,false>(a,b,c,ncolumns_a,nrows_a,ncolumns_b,threading_model);
        }
        else if(!tA && tB) // IKJ
        {
          if(cB)
            return dgemm_helper<T,TMM_Algo::IKJ,false,true,false,true>(a,b,c,nrows_a,ncolumns_a,nrows_b,threading_model);
          else
            return dgemm_helper<T,TMM_Algo::IKJ,false,true,false,false>(a,b,c,nrows_a,ncolumns_a,nrows_b,threading_model);
        }
        else //if((!tA) && (!tB)) // IJK
        {
          return dgemm_helper<T,TMM_Algo::IJK,false,false,false,false>(a,b,c,nrows_a,ncolumns_a,ncolumns_b,threading_model);
        }
      }
    case TMatrixStorage::ColumnMajor:
      {
        //calculate C'=(op(B))'*(op(A))' instead of C=op(A)*op(B)
        //column major C is row major C', so corresponing mappings are: A<->B, tA<->tB, cA<->cB
        if(!tA && !tB) // IJK
        {
          return dgemm_helper<T,TMM_Algo::IJK,false,false,false,false>(b,a,c,ncolumns_b,nrows_b,nrows_a,threading_model);
        }
        else if (!tB && tA) // JIK
        {
          if(cA)
            return dgemm_helper<T,TMM_Algo::JIK,false,true,false,true>(b,a,c,ncolumns_b,nrows_b,ncolumns_a,threading_model);
          else
            return dgemm_helper<T,TMM_Algo::JIK,false,true,false,false>(b,a,c,ncolumns_b,nrows_b,ncolumns_a,threading_model);
        }
        else if(tB && !tA) // IKJ
        {
          if(cB)
            return dgemm_helper<T,TMM_Algo::IKJ,true,false,true,false>(b,a,c,nrows_b,ncolumns_b,nrows_a,threading_model);
          else
            return dgemm_helper<T,TMM_Algo::IKJ,true,false,false,false>(b,a,c,nrows_b,ncolumns_b,nrows_a,threading_model);
        }
        else //if(tA && tB) // KIJ
        {
          if(cA && cB)
            return dgemm_helper<T,TMM_Algo::KIJ,true,true,true,true>(b,a,c,nrows_b,ncolumns_b,ncolumns_a,threading_model);
          else if(cA && !cB)
            return dgemm_helper<T,TMM_Algo::KIJ,true,true,false,true>(b,a,c,nrows_b,ncolumns_b,ncolumns_a,threading_model);
          else if(!cA && cB)
            return dgemm_helper<T,TMM_Algo::KIJ,true,true,true,false>(b,a,c,nrows_b,ncolumns_b,ncolumns_a,threading_model);
          else //if((!cA) && (!cB))
            return dgemm_helper<T,TMM_Algo::KIJ,true,true,false,false>(b,a,c,nrows_b,ncolumns_b,ncolumns_a,threading_model);
        }
      }
  }
}

//generic version of dgemm for square matrices
template<typename T>
  void dgemm(const TMatrixStorage stor, const TMatrixTranspose transA, const TMatrixTranspose transB,
    const T* const __RESTRICT a, const T* const __RESTRICT b, T* const __RESTRICT c,
    const size_t sz,
    const TThreading threading_model)
{
    return dgemm<T>(stor,transA,transB,a,b,c,sz,sz,sz,sz,threading_model);
}

//elemental operation for simple matrix-vector multiplication
template<typename T, bool tA, bool cA>
  __FORCEINLINE inline void _gemv_op(const T* const  __RESTRICT a, const T* const  __RESTRICT x, T* const __RESTRICT y,
    const size_t stride_a, const T alpha,
    const size_t i, const size_t j)
{
  //if nothing's transposed, do  y(i) += a(i,j) * x(j)
  //transpose option switches corresponding indices. tA: a(i,j) -> a(j,i)
  y[i] +=  ( cA ? conj<T>(a[tA ? j*stride_a+i : i*stride_a+j]) : a[tA ? j*stride_a+i : i*stride_a+j] ) * x[j] * alpha;
}

template<typename T, bool tA, bool cA>
  inline void gemv_serial(const T* const  __RESTRICT a, const T* const  __RESTRICT x, T* const __RESTRICT y,
    const size_t nrows_op_a, const size_t ncolumns_op_a,
    const T alpha, const T beta)
{
  if(tA)
  {
    for(size_t i=0; i < ncolumns_op_a; i++)
      y[i]*=beta;
    for(size_t j=0; j < nrows_op_a; j++)
      for(size_t i=0; i < ncolumns_op_a; i++)
        _gemv_op<T,tA,cA>(a,x,y,ncolumns_op_a,alpha,i,j);
  } else {
    for(size_t i=0; i < nrows_op_a; i++)
    {
      y[i]*=beta;
      for(size_t j=0; j < ncolumns_op_a; j++)
        _gemv_op<T,tA,cA>(a,x,y,ncolumns_op_a,alpha,i,j);
    }
  }
}

template<typename T, bool tA, bool cA>
  inline void dgemv_helper(const T* const __RESTRICT a, const T* const __RESTRICT x, T* const __RESTRICT y,
      const size_t nrows_op_a, const size_t ncolumns_op_a,
      const T alpha, const T beta,
      const TThreading threading_model)
{
  switch(threading_model)
  {
    case T_Serial:
      return gemv_serial<T,tA,cA>(a,x,y,nrows_op_a,ncolumns_op_a,alpha,beta);
    case T_Std:
//      return gemv_stdthreads<T,tA,cA>(a,x,y,nrows_op_a,ncolumns_op_a,alpha,beta);
#ifdef HAVE_PTHREADS
    case T_Posix:
//      return gemv_pthreads<T,tA,cA>(a,x,y,nrows_op_a,ncolumns_op_a,alpha,beta);
#endif
#ifdef HAVE_OPENMP
    case T_OpenMP:
//      return gemv_openmp<T,tA,cA>(a,x,y,nrows_op_a,ncolumns_op_a,alpha,beta);
#endif
#ifdef HAVE_CILK
    case T_Cilk:
//      return gemv_cilk<T,tA,cA>(a,x,y,nrows_op_a,ncolumns_op_a,alpha,beta);
#endif
#ifdef HAVE_TBB
    case T_TBB:
//      return gemv_tbb<T,tA,cA>(a,x,y,nrows_op_a,ncolumns_op_a,alpha,beta);
#endif
    case T_Undefined:
    default:
      return gemv_serial<T,tA,cA>(a,x,y,nrows_op_a,ncolumns_op_a,alpha,beta);
  }
}

//generic dgbmv, y = \beta*y + \alpha*op(A)*x
template<typename T>
  void dgemv(const TMatrixStorage stor, const TMatrixTranspose transA,
      const T* const __RESTRICT a, const T* const __RESTRICT x, T* const __RESTRICT y,
      const size_t nrows_a, const size_t ncolumns_a,
      const T alpha, const T beta,
      const TThreading threading_model)
{
  const bool tA = (transA != TMatrixTranspose::No) ;
  const bool cA = (transA == TMatrixTranspose::Conjugate && is_complex<T>::value) ;
  switch(stor)
  {
    case TMatrixStorage::RowMajor:
      if(tA) {
        if(cA)
          return dgemv_helper<T,true,true>(a, x, y, ncolumns_a, nrows_a, alpha, beta, threading_model);
        else
          return dgemv_helper<T,true,false>(a, x, y, ncolumns_a, nrows_a, alpha, beta, threading_model);
      } else {
        if(cA)
          return dgemv_helper<T,false,true>(a, x, y, nrows_a, ncolumns_a, alpha, beta, threading_model);
        else
          return dgemv_helper<T,false,false>(a, x, y, nrows_a, ncolumns_a, alpha, beta, threading_model);
      }
    case TMatrixStorage::ColumnMajor:
      if(!tA) {
        if(cA)
          return dgemv_helper<T,true,true>(a, x, y, ncolumns_a, nrows_a, alpha, beta, threading_model);
        else
          return dgemv_helper<T,true,false>(a, x, y, ncolumns_a, nrows_a, alpha, beta, threading_model);
      } else {
        if(cA)
          return dgemv_helper<T,false,true>(a, x, y, nrows_a, ncolumns_a, alpha, beta, threading_model);
        else
          return dgemv_helper<T,false,false>(a, x, y, nrows_a, ncolumns_a, alpha, beta, threading_model);
      }
  }
}

//generic dgbmv for square matrices
template<typename T>
  void dgemv(const TMatrixStorage stor, const TMatrixTranspose transA,
      const T* const __RESTRICT a, const T* const __RESTRICT x, T* const __RESTRICT y,
      const size_t sz,
      const T alpha, const T beta,
      const TThreading threading_model)
{
    return dgemv<T>(stor,transA,a,x,y,sz,sz,alpha,beta,threading_model);
}

template<typename T> bool is_diagonally_dominant(const size_t sz, const size_t stride, const T* const __RESTRICT A)
{
  for(size_t i = 0; i < sz; i++)
  {
    T row_sum = T(0.0);
    //no subnormals, please
    if(std::abs(A[i*stride+i]) < std::numeric_limits<T>::min())
      return false;
    for(size_t j = 0; j < i; j++)
    {
      row_sum += std::abs(A[i*stride+j]);
    }
    for(size_t j = i+1; j < sz; j++)
    {
      row_sum += std::abs(A[i*stride+j]);
    }
    if(!(row_sum < std::abs(A[i*stride+i])))
      return false;
  }
  return true;
}

//vector distance(induced by norm of difference) calculation
template<typename T> T vector_norm_L2(const size_t sz, const T* const __RESTRICT x)
{
  T norm = T(0.0);
  const T zero = T(0.0);
  const T one = T(1.0);
  if(sz == 1)
  {
    norm = std::abs(x[0]);
  } else {
    T component = T(0.0);
    T scale = T(0.0);
    T sum   = T(1.0);
    for(size_t i = 0; i < sz; i++)
    {
      //compute component i of vector difference
      component = std::abs(x[i]);
      //update norm blas-ish way
      if(!numeric::isEqualReal(component, zero))
      {
        if (scale < component)
        {
          sum = one + sum * pow(scale / component, 2);
          scale = component;
        } else {
          sum += pow(component / scale, 2);
        }
      }
    }
    norm = scale * sqrt(sum);
  }
  return norm;
}

//vector distance(induced by norm of difference) calculation
template<typename T> T vector_distance_L2(const size_t sz, const T* const __RESTRICT x, const T* const __RESTRICT y)
{
  T norm = T(0.0);
  const T zero = T(0.0);
  const T one = T(1.0);
  if(sz == 1)
  {
    norm = std::abs(x[0] - y[0]);
  } else {
    T component = T(0.0);
    T scale = T(0.0);
    T sum   = T(1.0);
    for(size_t i = 0; i < sz; i++)
    {
      //compute component i of vector difference
      component = std::abs(x[i] - y[i]);
      //update norm blas-ish way
      if(!numeric::isEqualReal(component, zero))
      {
        if (scale < component)
        {
          sum = one + sum * pow(scale / component, 2);
          scale = component;
        } else {
          sum += pow(component / scale, 2);
        }
      }
    }
    norm = scale * sqrt(sum);
  }
  return norm;
}

template<typename T> T vector_norm_L1(const size_t sz, const T* const __RESTRICT x)
{
  T norm = T(0.0);
  const T zero = T(0.0);
  const T one = T(1.0);
  if(sz == 1)
  {
    norm = std::abs(x[0]);
  } else {
    T component = T(0.0);
    T scale = T(0.0);
    T sum   = T(1.0);
    for(size_t i = 0; i < sz; i++)
    {
      //compute component i of vector difference
      component = std::abs(x[i]);
      //update norm blas-ish way
      if(!numeric::isEqualReal(component, zero))
      {
        if (scale < component)
        {
          sum = one + sum * (scale / component);
          scale = component;
        } else {
          sum += component / scale;
        }
      }
    }
    norm = scale * sum;
  }
  return norm;
}

template<typename T> T vector_distance_L1(const size_t sz, const T* const __RESTRICT x, const T* const __RESTRICT y)
{
  T norm = T(0.0);
  const T zero = T(0.0);
  const T one = T(1.0);
  if(sz == 1)
  {
    norm = std::abs(x[0] - y[0]);
  } else {
    T component = T(0.0);
    T scale = T(0.0);
    T sum   = T(1.0);
    for(size_t i = 0; i < sz; i++)
    {
      //compute component i of vector difference
      component = std::abs(x[i] - y[i]);
      //update norm blas-ish way
      if(!numeric::isEqualReal(component, zero))
      {
        if (scale < component)
        {
          sum = one + sum * (scale / component);
          scale = component;
        } else {
          sum += component / scale;
        }
      }
    }
    norm = scale * sum;
  }
  return norm;
}

//TODO: write faster block transpose
template<typename T> void square_transpose(const size_t sz, T* const __RESTRICT a)
{
  for(size_t i = 0; i < sz - 1; i++)
    for(size_t j = i + 1; j < sz - 1; j++)
      std::swap(a[i*sz + j], a[j*sz + i]);
}

}

#endif
