#pragma once
#ifndef _LAPACK_HPP
#define _LAPACK_HPP
#include "config.h"

#include <cstddef>

namespace numeric
{
    //banded matrices are assumed to be in CDS format
    template<typename T>
      inline void thomas_solve(
        T* const __RESTRICT lhs, T* const __RESTRICT x, T* const __RESTRICT rhs,
        const size_t sz, const size_t band, const bool reuse_storage);

    template<typename T>
      inline void thomas_diagonal_solve(
        const T* const __RESTRICT lhs, T* const __RESTRICT x, const T* const __RESTRICT rhs,
        const size_t sz, const bool reuse_storage);

    template<typename T>
      inline void thomas_tridiagonal_solve(
        T* const __RESTRICT lhs, T* const __RESTRICT x, T* const __RESTRICT rhs,
        const size_t sz, const bool reuse_storage);

    template<typename T>
      inline void thomas_fivediagonal_solve(
        T* const __RESTRICT lhs, T* const __RESTRICT x, T* const __RESTRICT rhs,
        const size_t sz, const bool reuse_storage);

    template<typename T> T residual_l2_norm(const size_t sz, const size_t stride,
        const T* const __RESTRICT lhs, const T* const __RESTRICT rhs, const T* const __RESTRICT x);

    template<typename T> T residual_l1_norm(const size_t sz, const size_t stride,
        const T* const __RESTRICT lhs, const T* const __RESTRICT rhs, const T* const __RESTRICT x);

}

#include "numeric/lapack_impl.hpp"

#endif /* _LAPACK_HPP */
