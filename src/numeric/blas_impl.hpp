#pragma once
#ifndef _BLAS_IMPL_HPP
#define _BLAS_IMPL_HPP
#include "config.h"

#include "numeric/blas.hpp"
#include "numeric/complex.hpp"
#include "numeric/parallel.hpp"

#include <limits>
#include <cstddef>

using std::size_t;

namespace numeric {


//helper for simple serial matrix multiplication
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

template<typename T, TMM_Algo tAlgo, bool tA, bool tB, bool cA, bool cB, bool sqr>
  inline void matmul_helper(const T* const  __RESTRICT a, const T* const  __RESTRICT b, T* const __RESTRICT c,
    const size_t nrows_op_a, const size_t ncolumns_op_a, const size_t ncolumns_op_b)
{
  constexpr bool ccA = (cA && is_complex<T>::value) ;
  constexpr bool ccB = (cB && is_complex<T>::value) ;

  size_t i,j,k;
  switch(tAlgo)
  {
//it's a bit of a shame, but these macros actually enchance readability IMO
#define _FOR_I for(i=0; i < nrows_op_a; i++)
#define _FOR_J for(j=0; j < ncolumns_op_a; j++)
#define _FOR_K for(k=0; k < ncolumns_op_b; k++)
    case TMM_Algo::IJK:
        _FOR_I _FOR_J _FOR_K
          _mm_op<T,tA,tB,ccA,ccB>(a,b,c,ncolumns_op_a,ncolumns_op_b,ncolumns_op_b,i,j,k);
      return;
    case TMM_Algo::JKI:
        _FOR_J _FOR_K _FOR_I
          _mm_op<T,tA,tB,ccA,ccB>(a,b,c,ncolumns_op_a,ncolumns_op_b,ncolumns_op_b,i,j,k);
      return;
    case TMM_Algo::KIJ:
        _FOR_K _FOR_I _FOR_J
          _mm_op<T,tA,tB,ccA,ccB>(a,b,c,ncolumns_op_a,ncolumns_op_b,ncolumns_op_b,i,j,k);
      return;
    case TMM_Algo::IKJ:
        _FOR_I _FOR_K _FOR_J
          _mm_op<T,tA,tB,ccA,ccB>(a,b,c,ncolumns_op_a,ncolumns_op_b,ncolumns_op_b,i,j,k);
      return;
    case TMM_Algo::KJI:
        _FOR_K _FOR_J _FOR_I
          _mm_op<T,tA,tB,ccA,ccB>(a,b,c,ncolumns_op_a,ncolumns_op_b,ncolumns_op_b,i,j,k);
      return;
    case TMM_Algo::JIK:
        _FOR_J _FOR_I _FOR_K
          _mm_op<T,tA,tB,ccA,ccB>(a,b,c,ncolumns_op_a,ncolumns_op_b,ncolumns_op_b,i,j,k);
      return;
#undef _FOR_I
#undef _FOR_J
#undef _FOR_K
  }
}

template<typename T, bool isSquare>
  void dgemm_helper(const TMatrixStorage stor, const TMatrixTranspose transA, const TMatrixTranspose transB,
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
            return matmul_helper<T,TMM_Algo::KIJ,true,true,true,true,isSquare>(a,b,c,ncolumns_a,nrows_a,nrows_b);
          else if(cA && !cB)
            return matmul_helper<T,TMM_Algo::KIJ,true,true,true,false,isSquare>(a,b,c,ncolumns_a,nrows_a,nrows_b);
          else if(!cA && cB)
            return matmul_helper<T,TMM_Algo::KIJ,true,true,false,true,isSquare>(a,b,c,ncolumns_a,nrows_a,nrows_b);
          else //if((!cA) && (!cB))
            return matmul_helper<T,TMM_Algo::KIJ,true,true,false,false,isSquare>(a,b,c,ncolumns_a,nrows_a,nrows_b);
        }
        else if (tA && !tB) // JIK
        {
          if(cA)
            return matmul_helper<T,TMM_Algo::JIK,true,false,true,false,isSquare>(a,b,c,ncolumns_a,nrows_a,ncolumns_b);
          else
            return matmul_helper<T,TMM_Algo::JIK,true,false,false,false,isSquare>(a,b,c,ncolumns_a,nrows_a,ncolumns_b);
        }
        else if(!tA && tB) // IKJ
        {
          if(cB)
            return matmul_helper<T,TMM_Algo::IKJ,false,true,false,true,isSquare>(a,b,c,nrows_a,ncolumns_a,nrows_b);
          else
            return matmul_helper<T,TMM_Algo::IKJ,false,true,false,false,isSquare>(a,b,c,nrows_a,ncolumns_a,nrows_b);
        }
        else //if((!tA) && (!tB)) // IJK
        {
          return matmul_helper<T,TMM_Algo::IJK,false,false,false,false,isSquare>(a,b,c,nrows_a,ncolumns_a,ncolumns_b);
        }
      }
    case TMatrixStorage::ColumnMajor:
      {
        //calculate C'=(op(B))'*(op(A))' instead of C=op(A)*op(B)
        //corresponing mappings: tA->!tA,tB->!tB, A<->B
        if(!tA && !tB) // KIJ
        {
          return matmul_helper<T,TMM_Algo::IJK,true,true,false,false,isSquare>(b,a,c,ncolumns_b,nrows_b,nrows_a);
        }
        else if (!tB && tA) // JIK
        {
          if(cA)
            return matmul_helper<T,TMM_Algo::JIK,true,false,false,true,isSquare>(b,a,c,ncolumns_b,nrows_b,ncolumns_a);
          else
            return matmul_helper<T,TMM_Algo::JIK,true,false,false,false,isSquare>(b,a,c,ncolumns_b,nrows_b,ncolumns_a);
        }
        else if(tB && !tA) // IKJ
        {
          if(cB)
            return matmul_helper<T,TMM_Algo::IKJ,false,true,true,false,isSquare>(b,a,c,nrows_b,ncolumns_b,nrows_a);
          else
            return matmul_helper<T,TMM_Algo::IKJ,false,true,false,false,isSquare>(b,a,c,nrows_b,ncolumns_b,nrows_a);
        }
        else //if(tA && tB) // IJK
        {
          if(cA && cB)
            return matmul_helper<T,TMM_Algo::KIJ,false,false,true,true,isSquare>(b,a,c,nrows_b,ncolumns_b,ncolumns_a);
          else if(cA && !cB)
            return matmul_helper<T,TMM_Algo::KIJ,false,false,false,true,isSquare>(b,a,c,nrows_b,ncolumns_b,ncolumns_a);
          else if(!cA && cB)
            return matmul_helper<T,TMM_Algo::KIJ,false,false,true,false,isSquare>(b,a,c,nrows_b,ncolumns_b,ncolumns_a);
          else //if((!cA) && (!cB))
            return matmul_helper<T,TMM_Algo::KIJ,false,false,false,false,isSquare>(b,a,c,nrows_b,ncolumns_b,ncolumns_a);
        }
      }
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
  const bool square = ( nrows_a == ncolumns_a && nrows_b == ncolumns_b );

  if(square) {
    return dgemm_helper<T,true>(stor,transA,transB,a,b,c,nrows_a,ncolumns_a,nrows_b,ncolumns_b,threading_model);
  } else {
    return dgemm_helper<T,false>(stor,transA,transB,a,b,c,nrows_a,ncolumns_a,nrows_b,ncolumns_b,threading_model);
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

}

#endif
