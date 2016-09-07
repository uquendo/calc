#pragma once
#ifndef _LAPACK_IMPL_HPP
#define _LAPACK_IMPL_HPP
#include "config.h"

#include <iostream>

#include "numeric/blas.hpp"

namespace numeric
{

    //banded matrices are assumed to be in CDS format
    template<typename T>
      inline void thomas_diagonal_solve(
        const T* const __RESTRICT lhs, T* const __RESTRICT x, const T* const __RESTRICT rhs,
        const size_t sz, const bool reuse_storage)
    {
      for(size_t i = 0; i < sz; i++ )
      {
        x[i] = rhs[i] / lhs[i];
      }
    }

    template<typename T>
      inline void thomas_tridiagonal_solve(
        T* const __RESTRICT lhs, T* const __RESTRICT x, T* const __RESTRICT rhs,
        const size_t sz, const bool reuse_storage)
    {
      if(reuse_storage)
      {
        T fac;
        //forward elimination
        for(size_t i = 1; i < sz; i++ )
        {
          fac = lhs[3*i] / lhs[3*i - 2];
          lhs[3*i + 1] -= fac*lhs[3*i - 1];
          rhs[i] -= fac*rhs[i - 1];
        }
        //backward substitution
        x[sz - 1] = rhs[sz - 1] / lhs[3*sz - 2];
        for(size_t i = 2; i <= sz; i++ )
        {
          x[sz - i] = (rhs[sz - i] - lhs[3*(sz - i) + 2]*x[sz - i + 1]) / lhs[3*(sz - i) + 1];
        }
      } else {
        //TODO: STUB!
      }
    }

    template<typename T>
      inline void thomas_fivediagonal_solve(
        T* const __RESTRICT lhs, T* const __RESTRICT x, T* const __RESTRICT rhs,
        const size_t sz, const bool reuse_storage)
    {
      if(reuse_storage)
      {
        T fac1, fac2, diag;
        //forward elimination
        diag = lhs[2];
        lhs[3] /= diag;
        lhs[4] /= diag;
        rhs[0] /= diag;
        fac1 = lhs[6] / lhs[2];
        diag = lhs[7] - fac1*lhs[3];
        lhs[8] -= fac1*lhs[4];
        lhs[8] /= diag;
        lhs[9] /= diag;
        rhs[1] -= fac1*rhs[0];
        rhs[1] /= diag;
        for(size_t i = 2; i < sz; i++ )
        {
          fac2 = lhs[5*i] ;// / lhs[5*i - 8];
          fac1 = lhs[5*i + 1] - fac2*lhs[5*i - 7];// / lhs[5*i - 3];
          diag = lhs[5*i + 2] - fac1*lhs[5*i - 2] - fac2*lhs[5*i - 6];
          lhs[5*i + 3] -= fac1*lhs[5*i - 1];
          lhs[5*i + 3] /= diag;
          lhs[5*i + 4] /= diag;
          rhs[i] -= fac1*rhs[i - 1] + fac2*rhs[i - 2];
          rhs[i] /= diag;
        }
        //backward substitution
        x[sz - 1] = rhs[sz - 1];
        x[sz - 2] = rhs[sz - 2] - lhs[5*sz - 7]*x[sz - 1];
        for(size_t i = 3; i <= sz; i++ )
        {
          x[sz - i] = rhs[sz - i] - lhs[5*(sz - i) + 3]*x[sz - i + 1] - lhs[5*(sz - i) + 4]*x[sz - i + 2];
        }
      } else {
        //TODO: STUB!
      }
    }

/*    {
      if(reuse_storage)
      {
        T fac1, fac2;
        //forward elimination
        fac1 = lhs[6] / lhs[2];
        lhs[7] -= fac1*lhs[3];
        lhs[8] -= fac1*lhs[4];
        rhs[1] -= fac1*rhs[0];
        for(size_t i = 2; i < sz; i++ )
        {
          fac2 = lhs[5*i] / lhs[5*i - 8];
          fac1 = lhs[5*i + 1] / lhs[5*i - 3];
          lhs[5*i + 2] -= fac1*lhs[5*i - 2] + fac2*lhs[5*i - 6];
          lhs[5*i + 3] -= fac1*lhs[5*i - 1];
          rhs[i] -= fac1*rhs[i - 1] + fac2*rhs[i - 2];
        }
        //backward substitution
        x[sz - 1] = rhs[sz - 1] / lhs[5*sz - 3];
        x[sz - 2] = (rhs[sz - 2] - lhs[5*sz - 7]*x[sz - 1]) / lhs[5*sz - 8];
        for(size_t i = 3; i <= sz; i++ )
        {
          x[sz - i] = (rhs[sz - i] - lhs[5*(sz - i) + 3]*x[sz - i + 1] - lhs[5*(sz - i) + 4]*x[sz - i + 2]) / lhs[5*(sz - i) + 2];
        }
      } else {
        //TODO: STUB!
      }
    }
*/

    template<typename T>
      inline void thomas_solve(
        T* const __RESTRICT lhs, T* const __RESTRICT x, T* const __RESTRICT rhs,
        const size_t sz, const size_t band, const bool reuse_storage)
    {
//      const size_t stride = 2*band + 1;
//      if(!numeric::is_banded_diagonally_dominant(sz,band,band,stride,lhs))
//      {
//        log.warning("the matrix of the system is NOT diagonally dominant, solution obtained using this routine could be very far from exact one");
//      }
      //special cases
      switch(band)
      {
        case 0:
          return thomas_diagonal_solve(lhs,x,rhs,sz,reuse_storage);
        case 1:
          return thomas_tridiagonal_solve(lhs,x,rhs,sz,reuse_storage);
        case 2:
          return thomas_fivediagonal_solve(lhs,x,rhs,sz,reuse_storage);
        default:
          break;
      }
      //TODO: generic case
    }

    //debug residual norm calculation
    template<typename T> T residual_l2_norm(const size_t sz, const size_t stride,
        const T* const __RESTRICT lhs, const T* const __RESTRICT rhs, const T* const __RESTRICT x)
    {
      T norm = T(0.0);
      const T zero = T(0.0);
      const T one = T(1.0);
      if(sz == 1)
      {
        norm = std::abs(lhs[0] * x[0] - rhs[0]);
      } else {
        T component = T(0.0);
        T scale = T(0.0);
        T sum   = T(1.0);
        for(size_t i = 0; i < sz; i++)
        {
          //compute component i of the residual
          component = - rhs[i];
          for(size_t j = 0; j < sz; j++)
          {
            component += lhs[i*stride + j] * x[j];
          }
          //update norm reference blas way
          if(!numeric::isEqualReal(component, zero))
          {
            component = std::abs(component);
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

    template<typename T> T residual_l1_norm(const size_t sz, const size_t stride,
        const T* const __RESTRICT lhs, const T* const __RESTRICT rhs, const T* const __RESTRICT x)
    {
      T norm = T(0.0);
      const T zero = T(0.0);
      const T one = T(1.0);
      if(sz == 1)
      {
        norm = std::abs(lhs[0] * x[0] - rhs[0]);
      } else {
        T component = T(0.0);
        T scale = T(0.0);
        T sum   = T(1.0);
        for(size_t i = 0; i < sz; i++)
        {
          //compute component i of the residual
          component = - rhs[i];
          for(size_t j = 0; j < sz; j++)
          {
            component += lhs[i*stride + j] * x[j];
          }
          //update norm reference blas way
          if(!numeric::isEqualReal(component, zero))
          {
            component = std::abs(component);
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

}

#endif /* _LAPACK_IMPL_HPP */
