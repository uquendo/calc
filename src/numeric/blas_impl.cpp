#include "numeric/blas_impl.hpp"
#include "numeric/real.hpp"

namespace numeric {
    //explicit instantiation
   template <> inline void matmul_square_helper(const float* const __RESTRICT a, const float* const __RESTRICT b, float* const __RESTRICT c, const size_t sz);
   template <> inline void matmul_square_helper(const double* const __RESTRICT a, const double* const __RESTRICT b, double* const __RESTRICT c, const size_t sz);
   template <> inline void matmul_square_helper(const long double* const __RESTRICT a, const long double* const __RESTRICT b, long double* const __RESTRICT c, const size_t sz);

#ifdef BUILD_QUAD
   template <> inline void matmul_square_helper(const quad* const __RESTRICT a, const quad* const __RESTRICT b, quad* const __RESTRICT c, const size_t sz);
#endif
#ifdef HAVE_MPREAL
   template <> inline void matmul_square_helper(const mpreal* const __RESTRICT a, const mpreal* const __RESTRICT b, mpreal* const __RESTRICT c, const size_t sz);
#endif

}
