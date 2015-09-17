#include "numeric/blas_impl.hpp"

namespace numeric {

   template <> inline void matmul_square_helper(const double* const __restrict a, const double* const __restrict b, double* const __restrict c,
                   const size_t sz);

}
