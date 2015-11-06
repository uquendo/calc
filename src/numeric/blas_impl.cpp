#include "numeric/blas_impl.hpp"
#include "numeric/real.hpp"


namespace numeric {
    //explicit instantiation
#   define INSTANTIATE_matmul_square_helper(X) template <> inline void matmul_square_helper\
        (const X* const __RESTRICT a, const X* const __RESTRICT b, X* const __RESTRICT c, const size_t sz)
    INSTANTIATE_matmul_square_helper(float);
    INSTANTIATE_matmul_square_helper(double);
    INSTANTIATE_matmul_square_helper(long double);
#   ifdef BUILD_QUAD
        INSTANTIATE_matmul_square_helper(quad);
#   endif
#   ifdef HAVE_MPREAL
        INSTANTIATE_matmul_square_helper(mpreal);
#   endif

}
