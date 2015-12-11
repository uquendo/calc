#include "numeric/blas.hpp"
#include "numeric/real.hpp"


namespace numeric {
  //explicit instantiation
# define INSTANTIATE_dgemm( X ) void dgemm\
    (const TMatrixStorage stor, const TMatrixTranspose transA, const TMatrixTranspose transB,\
    const X * const __RESTRICT a, const X * const __RESTRICT b, X * const __RESTRICT c,\
    const size_t nrows_a, const size_t ncolumns_a,\
    const size_t nrows_b, const size_t ncolumns_b,\
    const TThreading threading_model)

# define INSTANTIATE_dgemm_square( X ) void dgemm(\
    const TMatrixStorage stor, const TMatrixTranspose transA, const TMatrixTranspose transB,\
    const X * const __RESTRICT a, const X * const __RESTRICT b, X * const __RESTRICT c,\
    const size_t sz,\
    const TThreading threading_model)

# define INSTANTIATE_dgemm_block( X ) void dgemm_block\
    (const TMatrixStorage stor,\
    const X * const __RESTRICT a, const X * const __RESTRICT b, X * const __RESTRICT c,\
    const size_t nrows_a, const size_t ncolumns_a,\
    const size_t nrows_b, const size_t ncolumns_b,\
    const TThreading threading_model)

# define INSTANTIATE_dgemm_block_square( X ) void dgemm_block(\
    const TMatrixStorage stor,\
    const X * const __RESTRICT a, const X * const __RESTRICT b, X * const __RESTRICT c,\
    const size_t sz,\
    const TThreading threading_model)

  INSTANTIATE_dgemm(float);
  INSTANTIATE_dgemm_square(float);
  INSTANTIATE_dgemm(double);
  INSTANTIATE_dgemm_square(double);
  INSTANTIATE_dgemm(long double);
  INSTANTIATE_dgemm_square(long double);
# ifdef HAVE_QUADMATH
    INSTANTIATE_dgemm(quad);
    INSTANTIATE_dgemm_square(quad);
# endif
# ifdef HAVE_MPREAL
    INSTANTIATE_dgemm(mpreal);
    INSTANTIATE_dgemm_square(mpreal);
# endif

  INSTANTIATE_dgemm_block(float);
  INSTANTIATE_dgemm_block_square(float);
  INSTANTIATE_dgemm_block(double);
  INSTANTIATE_dgemm_block_square(double);
  INSTANTIATE_dgemm_block(long double);
  INSTANTIATE_dgemm_block_square(long double);
# ifdef HAVE_QUADMATH
    INSTANTIATE_dgemm_block(quad);
    INSTANTIATE_dgemm_block_square(quad);
# endif
# ifdef HAVE_MPREAL
    INSTANTIATE_dgemm_block(mpreal);
    INSTANTIATE_dgemm_block_square(mpreal);
# endif

}
