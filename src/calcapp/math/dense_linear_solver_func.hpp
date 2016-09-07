#pragma once
#ifndef __DENSE_LINEAR_SOLVER_FUNC_HPP
#define __DENSE_LINEAR_SOLVER_FUNC_HPP
#include "config.h"

#include "calcapp/math/dense_linear_solver.hpp"

namespace Calc
{

  //TODO: classification, class hierarhy and dispatcher

    //TODO: solver func with prealocated buffers
    template<typename T> class DenseSolverFunc
    {
private:
      ProgressCtrl * m_progress_ptr;
      const int m_max_iter_count;
      const T m_eps;

public:
      //!!!TODO: check that progress_ptr!=nullptr
      DenseSolverFunc(ProgressCtrl * progress_ptr,
         int max_iter_count = default_max_iter_count,
         T eps = default_eps<T>())
        : m_progress_ptr(progress_ptr)
        , m_max_iter_count(max_iter_count)
        , m_eps(eps)
      {}

      void operator()(const size_t sz, T* const __RESTRICT A, T* const __RESTRICT b, T* const __RESTRICT x)
      {
        Logger& log = m_progress_ptr->log();
        if(numeric::is_diagonally_dominant(sz,sz,A))
        {
          std::unique_ptr<T[]> x_next(new T[sz]);
          seidel_impl<T>(sz,A,b,x,x_next.get(),log,false,m_max_iter_count,m_eps);
        } else {
          std::unique_ptr<size_t[]> perm(new size_t[sz]);
          std::unique_ptr<T*[]> A_rows(new T*[sz]);
          gauss_full_pivoting_impl(sz,A,b,A_rows.get(),x,perm.get(),log);
        }
      }
    };

}

#endif // __DENSE_LINEAR_SOLVER_FUNC_HPP
