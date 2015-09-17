#ifndef _BLAS_IMPL_HPP
#define _BLAS_IMPL_HPP
#include "config.h"

#include <cstddef>

#ifndef HAVE_RESTRICT
#define __restrict
#endif

using std::size_t;

namespace numeric {

//helper for simple serial matrix multiplication
//a -- nrows_a x ncolumns_a, b -- ncolumns_a x ncolumns_b , c=a*b -- nrows_a x ncolumns_b
template <typename T> inline void matmul_helper(const T* const  __restrict a, const T* const  __restrict b, T* const __restrict c,
                                                    const size_t nrows_a, const size_t ncolumns_a, const size_t ncolumns_b)
{
    size_t i,j,k;
    for(i=0; i<nrows_a; i++)
        for(j=0; j<ncolumns_a; j++)
            for(k=0; k<ncolumns_b; k++)
                c[i][k] += a[i][j] * b[j][k];
    return;
}

//version for square matrices
template <typename T> inline void matmul_square_helper(const T* const __restrict a, const T* const __restrict b, T* const __restrict c,
        const size_t sz)
{
    return matmul_helper<T>(a,b,c,sz,sz,sz);
}

}

#endif
