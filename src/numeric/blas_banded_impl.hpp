#pragma once
#ifndef _BLAS_BANDED_IMPL_HPP
#define _BLAS_BANDED_IMPL_HPP

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

#include <cstddef>

using std::size_t;

namespace numeric {

//helper for simple serial banded matrix multiplication
template<typename T, bool tA, bool tB, bool cA, bool cB>
  __FORCEINLINE inline void _gbmm_op(const T* const  __RESTRICT a, const T* const  __RESTRICT b, T* const __RESTRICT c,
    const size_t stride_a, const size_t stride_b, const size_t stride_c,
    const size_t i, const size_t j, const size_t k)
{
  //if nothing's transposed, do c(i,k) += a(i,j) * b(j,k)
  //transpose option switches corresponding indices. tA: a(i,j) -> a(j,i), tB: b(j,k) -> b(k,j)
  //TODO: STUB!
}

template<typename T, TMM_Algo tAlgo, bool tA, bool tB, bool cA, bool cB>
  inline void tridiagonal_matmul_serial(const T* const  __RESTRICT a, const T* const  __RESTRICT b, T* const __RESTRICT c,
    const size_t rows)
{
  //TODO: transposed variants
  //preunrolled tridiagonal algo
  //i=0
  {
    //j=0
    //c[1] += a[1] * b[0]; // == 0
    c[2] += a[1] * b[1];
    c[3] += a[1] * b[2];
    //j=1
    c[2] += a[2] * b[3];
    c[3] += a[2] * b[4];
    c[4] += a[2] * b[5];
  }
  for(size_t i = 1; i < rows - 1; i++)
    for(int j = 0; j < 3; j++)
      for(int k = 0; k < 3; k++)
        c[5*i+j+k] += a[3*i+j] * b[3*(i+j-1)+k];
  //i=rows-1
  if(rows > 1)
  {
    //j=0
    c[5*(rows-1)+0] += a[3*(rows-1)] * b[3*(rows-2)+0];
    c[5*(rows-1)+1] += a[3*(rows-1)] * b[3*(rows-2)+1];
    c[5*(rows-1)+2] += a[3*(rows-1)] * b[3*(rows-2)+2];
    //j=1
    c[5*(rows-1)+1] += a[3*(rows-1)+1] * b[3*(rows-2)+3];
    c[5*(rows-1)+2] += a[3*(rows-1)+1] * b[3*(rows-2)+4];
    //c[5*(rows-1)+3] += a[3*(rows-1)+1] * b[3*(rows-1)+2] // == 0
  }
}

#ifdef HAVE_OPENMP
template<typename T, TMM_Algo tAlgo, bool tA, bool tB, bool cA, bool cB>
  inline void tridiagonal_matmul_openmp(const T* const  __RESTRICT a, const T* const  __RESTRICT b, T* const __RESTRICT c,
    const size_t rows)
{
  //TODO: transposed variants
  //preunrolled tridiagonal algo
  //i=0
  {
    //j=0
    //c[1] += a[1] * b[0]; // == 0
    c[2] += a[1] * b[1];
    c[3] += a[1] * b[2];
    //j=1
    c[2] += a[2] * b[3];
    c[3] += a[2] * b[4];
    c[4] += a[2] * b[5];
  }
#pragma omp parallel for
  for(size_t i = 1; i < rows - 1; i++)
  {
    for(int j = 0; j < 3; j++)
      for(int k = 0; k < 3; k++)
        c[5*i+j+k] += a[3*i+j] * b[3*(i+j-1)+k];
  }
  //i=rows-1
  if(rows > 1)
  {
    //j=0
    c[5*(rows-1)+0] += a[3*(rows-1)] * b[3*(rows-2)+0];
    c[5*(rows-1)+1] += a[3*(rows-1)] * b[3*(rows-2)+1];
    c[5*(rows-1)+2] += a[3*(rows-1)] * b[3*(rows-2)+2];
    //j=1
    c[5*(rows-1)+1] += a[3*(rows-1)+1] * b[3*(rows-2)+3];
    c[5*(rows-1)+2] += a[3*(rows-1)+1] * b[3*(rows-2)+4];
    //c[5*(rows-1)+3] += a[3*(rows-1)+1] * b[3*(rows-1)+2] // == 0
  }
}

template<typename T, TMM_Algo tAlgo, bool tA, bool tB, bool cA, bool cB>
  inline void banded_matmul_openmp(const T* const  __RESTRICT a, const T* const  __RESTRICT b, T* const __RESTRICT c,
    const size_t nrows_op_a, const size_t ncolumns_op_a, const size_t ncolumns_op_b,
    const size_t upper_band_op_a, const size_t lower_band_op_a, const size_t upper_band_op_b, const size_t lower_band_op_b)
{
  if( (upper_band_op_a == upper_band_op_b) &&
      (upper_band_op_a == lower_band_op_a) &&
      (upper_band_op_b == lower_band_op_b) &&
      (upper_band_op_a == 1) &&
      (nrows_op_a == ncolumns_op_a) &&
      (ncolumns_op_a == ncolumns_op_b)
    )
  {
    //product of square tridiagonal matrices
    return tridiagonal_matmul_openmp<T,tAlgo,tA,tB,cA,cB>(a,b,c,nrows_op_a);
  } else {
    //TODO: STUB!
  }
/*
  switch(tAlgo)
  {
    case TMM_Algo::IJK:
          _gbmm_op<T,tA,tB,ccA,ccB>(a,b,c,ncolumns_op_a,ncolumns_op_b,ncolumns_op_b,i,j,k);
      return;
    case TMM_Algo::JKI:
      return;
    case TMM_Algo::KIJ:
      return;
    case TMM_Algo::IKJ:
      return;
    case TMM_Algo::KJI:
      return;
    case TMM_Algo::JIK:
      return;
  }
*/
}
#endif

#ifdef HAVE_CILK
template<typename T, TMM_Algo tAlgo, bool tA, bool tB, bool cA, bool cB>
  inline void tridiagonal_matmul_cilk(const T* const  __RESTRICT a, const T* const  __RESTRICT b, T* const __RESTRICT c,
    const size_t rows)
{
  //TODO: transposed variants
  //preunrolled tridiagonal algo
  //i=0
  {
    //j=0
    //c[1] += a[1] * b[0]; // == 0
    c[2] += a[1] * b[1];
    c[3] += a[1] * b[2];
    //j=1
    c[2] += a[2] * b[3];
    c[3] += a[2] * b[4];
    c[4] += a[2] * b[5];
  }
  cilk_for(size_t i = 1; i < rows - 1; i++)
    for(int j = 0; j < 3; j++)
      for(int k = 0; k < 3; k++)
        c[5*i+j+k] += a[3*i+j] * b[3*(i+j-1)+k];
  //i=rows-1
  if(rows > 1)
  {
    //j=0
    c[5*(rows-1)+0] += a[3*(rows-1)] * b[3*(rows-2)+0];
    c[5*(rows-1)+1] += a[3*(rows-1)] * b[3*(rows-2)+1];
    c[5*(rows-1)+2] += a[3*(rows-1)] * b[3*(rows-2)+2];
    //j=1
    c[5*(rows-1)+1] += a[3*(rows-1)+1] * b[3*(rows-2)+3];
    c[5*(rows-1)+2] += a[3*(rows-1)+1] * b[3*(rows-2)+4];
    //c[5*(rows-1)+3] += a[3*(rows-1)+1] * b[3*(rows-1)+2] // == 0
  }
}

template<typename T, TMM_Algo tAlgo, bool tA, bool tB, bool cA, bool cB>
  inline void banded_matmul_cilk(const T* const  __RESTRICT a, const T* const  __RESTRICT b, T* const __RESTRICT c,
    const size_t nrows_op_a, const size_t ncolumns_op_a, const size_t ncolumns_op_b,
    const size_t upper_band_op_a, const size_t lower_band_op_a, const size_t upper_band_op_b, const size_t lower_band_op_b)
{
  if( (upper_band_op_a == upper_band_op_b) &&
      (upper_band_op_a == lower_band_op_a) &&
      (upper_band_op_b == lower_band_op_b) &&
      (upper_band_op_a == 1) &&
      (nrows_op_a == ncolumns_op_a) &&
      (ncolumns_op_a == ncolumns_op_b)
    )
  {
    //product of square tridiagonal matrices
    return tridiagonal_matmul_cilk<T,tAlgo,tA,tB,cA,cB>(a,b,c,nrows_op_a);
  } else {
    //TODO: STUB!
  }
/*
  switch(tAlgo)
  {
    case TMM_Algo::IJK:
          _gbmm_op<T,tA,tB,ccA,ccB>(a,b,c,ncolumns_op_a,ncolumns_op_b,ncolumns_op_b,i,j,k);
      return;
    case TMM_Algo::JKI:
      return;
    case TMM_Algo::KIJ:
      return;
    case TMM_Algo::IKJ:
      return;
    case TMM_Algo::KJI:
      return;
    case TMM_Algo::JIK:
      return;
  }
*/
}
#endif

#ifdef HAVE_TBB
template<typename T, TMM_Algo tAlgo, bool tA, bool tB, bool cA, bool cB>
  inline void tridiagonal_matmul_tbb(const T* const  __RESTRICT a, const T* const  __RESTRICT b, T* const __RESTRICT c,
    const size_t rows)
{
  //TODO: transposed variants
  //preunrolled tridiagonal algo
  //i=0
  {
    //j=0
    //c[1] += a[1] * b[0]; // == 0
    c[2] += a[1] * b[1];
    c[3] += a[1] * b[2];
    //j=1
    c[2] += a[2] * b[3];
    c[3] += a[2] * b[4];
    c[4] += a[2] * b[5];
  }
  parallelForElem( size_t(1), rows - 1,
      [&a,&b,&c](size_t i)
      {
        for(int j = 0; j < 3; j++)
          for(int k = 0; k < 3; k++)
            c[5*i+j+k] += a[3*i+j] * b[3*(i+j-1)+k];
      });
  //i=rows-1
  if(rows > 1)
  {
    //j=0
    c[5*(rows-1)+0] += a[3*(rows-1)] * b[3*(rows-2)+0];
    c[5*(rows-1)+1] += a[3*(rows-1)] * b[3*(rows-2)+1];
    c[5*(rows-1)+2] += a[3*(rows-1)] * b[3*(rows-2)+2];
    //j=1
    c[5*(rows-1)+1] += a[3*(rows-1)+1] * b[3*(rows-2)+3];
    c[5*(rows-1)+2] += a[3*(rows-1)+1] * b[3*(rows-2)+4];
    //c[5*(rows-1)+3] += a[3*(rows-1)+1] * b[3*(rows-1)+2] // == 0
  }
}

template<typename T, TMM_Algo tAlgo, bool tA, bool tB, bool cA, bool cB>
  inline void banded_matmul_tbb(const T* const  __RESTRICT a, const T* const  __RESTRICT b, T* const __RESTRICT c,
    const size_t nrows_op_a, const size_t ncolumns_op_a, const size_t ncolumns_op_b,
    const size_t upper_band_op_a, const size_t lower_band_op_a, const size_t upper_band_op_b, const size_t lower_band_op_b)
{
  if( (upper_band_op_a == upper_band_op_b) &&
      (upper_band_op_a == lower_band_op_a) &&
      (upper_band_op_b == lower_band_op_b) &&
      (upper_band_op_a == 1) &&
      (nrows_op_a == ncolumns_op_a) &&
      (ncolumns_op_a == ncolumns_op_b)
    )
  {
    //product of square tridiagonal matrices
    return tridiagonal_matmul_tbb<T,tAlgo,tA,tB,cA,cB>(a,b,c,nrows_op_a);
  } else {
    //TODO: STUB!
  }
/*
  switch(tAlgo)
  {
    case TMM_Algo::IJK:
          _gbmm_op<T,tA,tB,ccA,ccB>(a,b,c,ncolumns_op_a,ncolumns_op_b,ncolumns_op_b,i,j,k);
      return;
    case TMM_Algo::JKI:
      return;
    case TMM_Algo::KIJ:
      return;
    case TMM_Algo::IKJ:
      return;
    case TMM_Algo::KJI:
      return;
    case TMM_Algo::JIK:
      return;
  }
*/
}
#endif

template<typename T, TMM_Algo tAlgo, bool tA, bool tB, bool cA, bool cB>
  inline void banded_matmul_serial(const T* const  __RESTRICT a, const T* const  __RESTRICT b, T* const __RESTRICT c,
    const size_t nrows_op_a, const size_t ncolumns_op_a, const size_t ncolumns_op_b,
    const size_t upper_band_op_a, const size_t lower_band_op_a, const size_t upper_band_op_b, const size_t lower_band_op_b)
{
  if( (upper_band_op_a == upper_band_op_b) &&
      (upper_band_op_a == lower_band_op_a) &&
      (upper_band_op_b == lower_band_op_b) &&
      (upper_band_op_a == 1) &&
      (nrows_op_a == ncolumns_op_a) &&
      (ncolumns_op_a == ncolumns_op_b)
    )
  {
    //product of square tridiagonal matrices
    return tridiagonal_matmul_serial<T,tAlgo,tA,tB,cA,cB>(a,b,c,nrows_op_a);
  } else {
    //TODO: STUB!
  }
/*
  switch(tAlgo)
  {
    case TMM_Algo::IJK:
          _gbmm_op<T,tA,tB,ccA,ccB>(a,b,c,ncolumns_op_a,ncolumns_op_b,ncolumns_op_b,i,j,k);
      return;
    case TMM_Algo::JKI:
      return;
    case TMM_Algo::KIJ:
      return;
    case TMM_Algo::IKJ:
      return;
    case TMM_Algo::KJI:
      return;
    case TMM_Algo::JIK:
      return;
  }
*/
}

template<typename T, TMM_Algo tAlgo, bool tA, bool tB, bool cA, bool cB>
  inline void dgbmm_helper(const T* const  __RESTRICT a, const T* const  __RESTRICT b, T* const __RESTRICT c,
    const size_t nrows_op_a, const size_t ncolumns_op_a, const size_t ncolumns_op_b,
    const size_t upper_band_op_a, const size_t lower_band_op_a, const size_t upper_band_op_b, const size_t lower_band_op_b,
    const TThreading threading_model)
{
  switch(threading_model)
  {
    case T_Serial:
      return banded_matmul_serial<T,tAlgo,tA,tB,cA,cB>(a,b,c,nrows_op_a,ncolumns_op_a,ncolumns_op_b,
                                                        upper_band_op_a,lower_band_op_a,upper_band_op_b,lower_band_op_b);
    case T_Std:
//      return banded_matmul_stdthreads<T,tAlgo,tA,tB,cA,cB>(a,b,c,nrows_op_a,ncolumns_op_a,ncolumns_op_b);
#ifdef HAVE_PTHREADS
    case T_Posix:
//      return banded_matmul_pthreads<T,tAlgo,tA,tB,cA,cB>(a,b,c,nrows_op_a,ncolumns_op_a,ncolumns_op_b);
#endif
#ifdef HAVE_OPENMP
    case T_OpenMP:
      return banded_matmul_openmp<T,tAlgo,tA,tB,cA,cB>(a,b,c,nrows_op_a,ncolumns_op_a,ncolumns_op_b,
                                                        upper_band_op_a,lower_band_op_a,upper_band_op_b,lower_band_op_b);
#endif
#ifdef HAVE_CILK
    case T_Cilk:
      return banded_matmul_cilk<T,tAlgo,tA,tB,cA,cB>(a,b,c,nrows_op_a,ncolumns_op_a,ncolumns_op_b,
                                                        upper_band_op_a,lower_band_op_a,upper_band_op_b,lower_band_op_b);
#endif
#ifdef HAVE_TBB
    case T_TBB:
      return banded_matmul_tbb<T,tAlgo,tA,tB,cA,cB>(a,b,c,nrows_op_a,ncolumns_op_a,ncolumns_op_b,
                                                        upper_band_op_a,lower_band_op_a,upper_band_op_b,lower_band_op_b);
#endif
    case T_Undefined:
    default:
      return banded_matmul_serial<T,tAlgo,tA,tB,cA,cB>(a,b,c,nrows_op_a,ncolumns_op_a,ncolumns_op_b,
                                                        upper_band_op_a,lower_band_op_a,upper_band_op_b,lower_band_op_b);
  }
}

//reduced dgbmm for banded matrices, C+=A*B
template<typename T>
  void dgbmm(const TMatrixStorage stor, const TMatrixTranspose transA, const TMatrixTranspose transB,
      const T* const __RESTRICT a, const T* const __RESTRICT b, T* const __RESTRICT c,
      const size_t nrows_a, const size_t ncolumns_a, const size_t upper_band_a, const size_t lower_band_a,
      const size_t nrows_b, const size_t ncolumns_b, const size_t upper_band_b, const size_t lower_band_b,
      const TThreading threading_model)
{
  switch(stor)
  {
    case TMatrixStorage::RowMajor:
      {
        //TODO: STUB!
        return dgbmm_helper<T,TMM_Algo::IJK,false,false,false,false>(a,b,c,nrows_a,ncolumns_a,ncolumns_b,
            upper_band_a,lower_band_a,upper_band_b,lower_band_b,threading_model);
      }
    case TMatrixStorage::ColumnMajor:
      {
        //TODO: STUB!
      }
  }
}

//reduced dgbmm for square symmetrically banded matrices
template<typename T>
  void dgbmm(const TMatrixStorage stor, const TMatrixTranspose transA, const TMatrixTranspose transB,
    const T* const __RESTRICT a, const T* const __RESTRICT b, T* const __RESTRICT c,
    const size_t sz, const size_t band,
    const TThreading threading_model)
{
  return dgbmm<T>(stor,transA,transB,a,b,c,sz,sz,band,band,sz,sz,band,band,threading_model);
}

template<typename T, bool transA>
  void dgbmv_helper(const T* const __RESTRICT a, const T* const __RESTRICT x, T* const __RESTRICT y,
      const size_t nrows_a, const size_t ncolumns_a, const size_t upper_band_a, const size_t lower_band_a,
      const T alpha,
      const TThreading threading_model)
{
  const size_t sz = ( transA ? nrows_a : ncolumns_a );
  const size_t stride_a = upper_band_a + lower_band_a + 1;
  if(!transA)
  {
    //too few operations to benefit from threading(< stride_a^2)
    for(size_t j = 0; j < upper_band_a; j++)
    {
      for(size_t i = 0; i <= j + lower_band_a; i++)
      {
        y[i] += alpha*a[lower_band_a + j + i*(stride_a - 1)]*x[j];
      }
    }
    switch(threading_model)
    {
      case T_Std:
  //      break;
#ifdef HAVE_PTHREADS
      case T_Posix:
  //      break;
#endif
#ifdef HAVE_OPENMP
      case T_OpenMP:
#pragma omp parallel for
        for(size_t j = 0; j <= sz - stride_a; j++)
        {
          for(size_t i = 0; i < stride_a; i++)
          {
            y[i + j] += alpha*a[stride_a - 1 + j*stride_a + i*(stride_a - 1)]*x[upper_band_a + j];
          }
        }
        break;
#endif
#ifdef HAVE_CILK
      case T_Cilk:
        cilk_for(size_t j = 0; j <= sz - stride_a; j++)
        {
          for(size_t i = 0; i < stride_a; i++)
          {
            y[i + j] += alpha*a[stride_a - 1 + j*stride_a + i*(stride_a - 1)]*x[upper_band_a + j];
          }
        }
        break;
#endif
#ifdef HAVE_TBB
      case T_TBB:
        parallelForElem(size_t(0), sz - stride_a + 1,
        [&a,&x,&y,&stride_a,&alpha,&upper_band_a](size_t j){
          for(size_t i = 0; i < stride_a; i++)
          {
            y[i + j] += alpha*a[stride_a - 1 + j*stride_a + i*(stride_a - 1)]*x[upper_band_a + j];
          }
        });
        break;
#endif
      case T_Serial:
      case T_Undefined:
      default:
        for(size_t j = 0; j <= sz - stride_a; j++)
        {
          for(size_t i = 0; i < stride_a; i++)
          {
            y[i + j] += alpha*a[stride_a - 1 + j*stride_a + i*(stride_a - 1)]*x[upper_band_a + j];
          }
        }
        break;
    }
    //too few operations to benefit from threading(< stride_a^2)
    for(size_t j = sz - stride_a + 1; j < sz - upper_band_a; j++)
    {
      for(size_t i = 0 ; i < sz - j; i++)
      {
        y[i + j] += alpha*a[stride_a - 1 + j*stride_a + i*(stride_a - 1)]*x[upper_band_a + j];
      }
    }
  } else {
    //TODO: STUB!
  }
}

//generic dgbmv for banded matrices, y+=\alpha*op(A)*x
template<typename T>
  void dgbmv(const TMatrixStorage stor, const TMatrixTranspose transA,
      const T* const __RESTRICT a, const T* const __RESTRICT x, T* const __RESTRICT y,
      const size_t nrows_a, const size_t ncolumns_a, const size_t upper_band_a, const size_t lower_band_a,
      const T alpha /* = T(1.0) */,
      const TThreading threading_model /* = T_Serial */ )
{
  const bool tA = transA != TMatrixTranspose::No;
//  const bool cA = transA == TMatrixTranspose::Conjugate; // TODO: STUB!
  switch(stor)
  {
    case TMatrixStorage::RowMajor:
      {
        if(tA)
          return dgbmv_helper<T,true>(a,x,y,nrows_a,ncolumns_a,upper_band_a,lower_band_a,alpha,threading_model);
        else
          return dgbmv_helper<T,false>(a,x,y,nrows_a,ncolumns_a,upper_band_a,lower_band_a,alpha,threading_model);
      }
    case TMatrixStorage::ColumnMajor:
      {
        if(tA)
          return dgbmv_helper<T,false>(a,x,y,ncolumns_a,nrows_a,lower_band_a,upper_band_a,alpha,threading_model);
        else
          return dgbmv_helper<T,true>(a,x,y,ncolumns_a,nrows_a,lower_band_a,upper_band_a,alpha,threading_model);
      }
  }
}

//generic dgbmv for square symmetrically banded matrices
template<typename T>
  void dgbmv(const TMatrixStorage stor, const TMatrixTranspose transA,
    const T* const __RESTRICT a, const T* const __RESTRICT x, T* const __RESTRICT y,
    const size_t sz, const size_t band,
    const T alpha /* = T(1.0) */,
    const TThreading threading_model /* = T_Serial */)
{
  return dgbmv<T>(stor,transA,a,x,y,sz,sz,band,band,alpha,threading_model);
}


//add for banded matrices, a += b, assuming that band width of b is less or equal than that of a
template<typename T>
  void banded_add_helper(
    T* const __RESTRICT a, const T* const __RESTRICT b,
    const size_t nrows, const size_t ncolumns,
    const size_t upper_band_a, const size_t lower_band_a,
    const size_t upper_band_b, const size_t lower_band_b,
    const TThreading threading_model)
{
  const size_t lower_delta = lower_band_a - lower_band_b;
  const size_t sz = std::min(nrows,ncolumns);
  const size_t stride_a = upper_band_a + lower_band_a + 1;
  const size_t stride_b = upper_band_b + lower_band_b + 1;
  switch(threading_model)
  {
    case T_Serial:
      for(size_t i = 0; i < sz; i++)
      {
        for(size_t j = 0; j < stride_b; j++)
        {
          a[i*stride_a + lower_delta + j] += b[i*stride_b + j];
        }
      }
      break;
    case T_Std:
//      break;
#ifdef HAVE_PTHREADS
    case T_Posix:
//      break;
#endif
#ifdef HAVE_OPENMP
    case T_OpenMP:
#pragma omp parallel for
      for(size_t i = 0; i < sz; i++)
      {
        for(size_t j = 0; j < stride_b; j++)
        {
          a[i*stride_a + lower_delta + j] += b[i*stride_b + j];
        }
      }
      break;
#endif
#ifdef HAVE_CILK
    case T_Cilk:
    cilk_for(size_t i = 0; i < sz; i++)
      {
        for(size_t j = 0; j < stride_b; j++)
        {
          a[i*stride_a + lower_delta + j] += b[i*stride_b + j];
        }
      }
      break;
#endif
#ifdef HAVE_TBB
    case T_TBB:
      parallelForElem(size_t(0), sz,
      [&a,&b,&stride_a,&stride_b,&lower_delta](size_t i){
        for(size_t j = 0; j < stride_b; j++)
        {
          a[i*stride_a + lower_delta + j] += b[i*stride_b + j];
        }
      });
      break;
#endif
    case T_Undefined:
    default:
      for(size_t i = 0; i < sz; i++)
      {
        for(size_t j = 0; j < stride_b; j++)
        {
          a[i*stride_a + lower_delta + j] += b[i*stride_b + j];
        }
      }
      break;
  }
}

//add for banded matrices, a += b, assuming that band width of b is equal or less than that of a
template<typename T>
  void banded_add(const TMatrixStorage stor,
    T* const __RESTRICT a, const T* const __RESTRICT b,
    const size_t nrows, const size_t ncolumns,
    const size_t upper_band_a, const size_t lower_band_a,
    const size_t upper_band_b, const size_t lower_band_b,
    const TThreading threading_model)
{
  switch(stor)
  {
    case TMatrixStorage::RowMajor:
      {
        return banded_add_helper<T>(a,b,nrows,ncolumns,
            upper_band_a,lower_band_a,upper_band_b,lower_band_b,
            threading_model);
      }
    case TMatrixStorage::ColumnMajor:
      {
        return banded_add_helper<T>(a,b,ncolumns,nrows,
            lower_band_a,upper_band_a,lower_band_b,upper_band_b,
            threading_model);
      }
  }
}

//add for square banded matrices, a += b, assuming that band width of b is equal or less than that of a
template<typename T>
  void banded_add(const TMatrixStorage stor,
    T* const __RESTRICT a, const T* const __RESTRICT b,
    const size_t sz,
    const size_t upper_band_a, const size_t lower_band_a,
    const size_t upper_band_b, const size_t lower_band_b,
    const TThreading threading_model)
{
  return banded_add<T>(stor,a,b,sz,sz,upper_band_a,lower_band_a,upper_band_b,lower_band_b,threading_model);
}

//add for square symmetrically banded matrices, a += b, assuming that band width of b is equal or less than that of a
template<typename T>
  void banded_add(const TMatrixStorage stor,
    T* const __RESTRICT a, const T* const __RESTRICT b,
    const size_t sz,
    const size_t band_a,
    const size_t band_b,
    const TThreading threading_model)
{
  return banded_add<T>(stor,a,b,sz,sz,band_a,band_a,band_b,band_b,threading_model);
}


template<typename T> bool is_banded_diagonally_dominant(const size_t sz,
    const size_t lower_band, const size_t upper_band,
    const size_t stride, const T* const __RESTRICT A)
{
  for(size_t i = 0; i < sz; i++)
  {
    T row_sum = T(0.0);
    //no subnormals, please
    if(std::abs(A[i*stride+lower_band]) < std::numeric_limits<T>::min())
      return false;
    for(size_t j = 0; j < lower_band; j++)
    {
      row_sum += std::abs(A[i*stride+j]);
    }
    for(size_t j = lower_band + 1; j < lower_band + upper_band + 1; j++)
    {
      row_sum += std::abs(A[i*stride+j]);
    }
    if(!(row_sum < std::abs(A[i*stride+lower_band])))
      return false;
  }
  return true;
}

}
#endif /* _BLAS_BANDED_IMPL_HPP */
