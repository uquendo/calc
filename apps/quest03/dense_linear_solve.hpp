#pragma once
#ifndef _DENSE_LINEAR_SOLVE_HPP
#define _DENSE_LINEAR_SOLVE_HPP
#include "config.h"

#include "numeric/real.hpp"
#include "numeric/blas.hpp"
#include "numeric/expand_traits.hpp"

#include "calcapp/exception.hpp"

#include "quest.hpp"

namespace Calc {

  namespace dense_linear_solve {

    //dispatcher
    void perform(const AlgoParameters& parameters, Logger& log);

    //dumb c++ version of gauss elimination without pivoting
    struct numeric_cpp_gauss;

    //dumb c++ version of jordan elimination without pivoting
    struct numeric_cpp_jordan;

    //dumb c++ version of gauss elimination with full pivoting
    struct numeric_cpp_gauss_full_pivoting;

    //debug residual norm calculation
    template<typename T> T residualL2Norm(const size_t sz, T* const __RESTRICT Ab, T* const __RESTRICT x)
    {
      const size_t stride = sz + 1;
      T norm = T(0.0);
      const T zero = T(0.0);
      const T one = T(1.0);
      if(sz == 1)
      {
        norm = std::abs(Ab[0] * x[0] - Ab[1]);
      } else {
        T component = T(0.0);
        T scale = T(0.0);
        T sum   = T(1.0);
        for(size_t i = 0; i < sz; i++)
        {
          //compute component i of the residual
          component = -Ab[i*stride+sz];
          for(size_t j = 0; j < sz; j++)
          {
            component += Ab[i*stride+j] * x[j];
          }
          //update norm reference blas way
          if(!numeric::isEqualReal(component, zero))
          {
            component = std::abs(component);
            if (scale < component)
            {
              sum = one + sum * std::pow(scale / component, 2);
              scale = component;
            } else {
              sum += std::pow(component / scale, 2);
            }
          }
        }
        norm = scale * sqrt(sum);
      }
      return norm;
    }

    template<typename T> T residualL1Norm(const size_t sz, T* const __RESTRICT Ab, T* const __RESTRICT x)
    {
      const size_t stride = sz + 1;
      T norm = T(0.0);
      const T zero = T(0.0);
      const T one = T(1.0);
      if(sz == 1)
      {
        norm = std::abs(Ab[0] * x[0] - Ab[1]);
      } else {
        T component = T(0.0);
        T scale = T(0.0);
        T sum   = T(1.0);
        for(size_t i = 0; i < sz; i++)
        {
          //compute component i of the residual
          component = -Ab[i*stride+sz];
          for(size_t j = 0; j < sz; j++)
          {
            component += Ab[i*stride+j] * x[j];
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

}

#endif /* _DENSE_LINEAR_SOLVE_HPP */
