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

//reduced dgbmm for banded matrices, C=A*B
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

}

#endif /* _BLAS_BANDED_IMPL_HPP */
