#pragma once
#ifndef __DENSE_LINEAR_SOLVER_HPP
#define __DENSE_LINEAR_SOLVER_HPP
#include "config.h"

#include "numeric/real.hpp"
#include "numeric/blas.hpp"
#include "numeric/lapack.hpp"

#include "calcapp/log.hpp"

#include <algorithm>
#include <cmath>

namespace Calc
{

    static constexpr int default_max_iter_count = 10000;
    template<typename T> constexpr T default_eps() { return T(1.0e-12); }

  //TODO: classification, class hierarhy and dispatcher

    //handmade direct solvers
    template<typename T> void gauss_impl(const size_t sz,
        T* const __RESTRICT A, T* const __RESTRICT b, T* const __RESTRICT x,
        Logger& log,
        const bool safe_checks = true)
    {
      const T small_value = T(1.e-5); //TODO: type-independent value
      const size_t stride = sz;
      if(safe_checks)
      {
        log.debug("Note that gaussian elimination without pivoting works well only on diagonally-dominant matrices");
        log.debug("And if your matrix has any zeroes on main diagonal, you're definitely absolutely totally entirely doomed.");
      }
      for(size_t k = 0; k < sz - 1; k++)
      {
        if(std::abs(A[k*stride + k]) < small_value)
          log.fwarning("Small pivot element A(%zu,%zu) found, %g",k,k,std::abs(A[k*stride + k]));
        T fac = T(1.0)/A[k*stride + k];
        for(size_t j = k + 1; j < sz; j++)
        {
          T tmp = A[j*stride + k]*fac;
          for(size_t i = k + 1; i < sz; i++)
            A[j*stride + i] -= A[k*stride + i]*tmp;
          b[j] -= b[k]*tmp;
        }
      }
      //back substitution
      for(size_t k = 0; k < sz; k++)
      {
        const size_t idx = sz - 1 - k;
        x[idx] = b[idx];
        for(size_t i = idx + 1; i < sz; i++)
          x[idx] -= A[idx*stride + i] * x[i];
        x[idx] /= A[idx*stride + idx];
      }
    }

    template<typename T> void jordan_impl(const size_t sz,
        T* const __RESTRICT A, T* const __RESTRICT b, T* const __RESTRICT x,
        Logger& log,
        const bool safe_checks = true)
    {
      const T small_value = T(1.e-5); //TODO: type-independent value
      const size_t stride = sz;
      if(safe_checks)
      {
        log.debug("Note that gaussian elimination without pivoting works well only on diagonally-dominant matrices");
        log.debug("And if your matrix has any zeroes on main diagonal, you're definitely absolutely totally entirely doomed");
      }
      for(size_t k = 0; k < sz; k++)
      {
        if(std::abs(A[k*stride + k]) < small_value)
          log.fwarning("Small pivot element A(%zu,%zu) found, %g",k,k,numeric::toDouble(std::abs(A[k*stride + k])));
        T fac = T(1.0)/A[k*stride + k];
        for(size_t j = k + 1; j < sz; j++)
        {
          T tmp = A[j*stride + k]*fac;
          for(size_t i = k + 1; i < sz; i++)
            A[j*stride + i] -= A[k*stride + i]*tmp;
          b[j] -= b[k]*tmp;
        }
        for(size_t j = 0; j < k; j++)
        {
          T tmp = A[j*stride + k]*fac;
          for(size_t i = k + 1; i < sz; i++)
            A[j*stride + i] -= A[k*stride + i]*tmp;
          b[j] -= b[k]*tmp;
        }
      }
      //scaling rhs by diagonal matrix elements
      for(size_t k = 0; k < sz; k++)
      {
        const size_t idx = sz - 1 - k;
        x[idx] = b[idx] / A[idx*stride + idx];
      }
    }

    template<typename T> void gauss_full_pivoting_impl(const size_t sz,
        T* const __RESTRICT A, T* const __RESTRICT b,
        T* __RESTRICT *  __RESTRICT A_rows, T* const __RESTRICT x,
        size_t * const __RESTRICT index,
        Logger& log)
    {
      const T small_value = T(1.e-5); //TODO: type-independent value
      const size_t stride = sz;
      index[0] = sz - 1;
      //init A_rows
      for(size_t i = 0; i < sz; i++)
        A_rows[i] = A + i*stride;
      for(size_t k = 0; k < sz - 1; k++)
      {
        size_t pivot_row = k;
        size_t pivot_column = k;
        T pivot = A_rows[k][k];
        //find pivot element
        for(size_t j = k; j < sz; j++)
        {
          for(size_t i = k; i < sz; i++)
          {
            if( std::abs(pivot) < std::abs(A_rows[j][i]) )
            {
              pivot = A_rows[j][i];
              pivot_row = j;
              pivot_column = i;
            }
          }
        }
        if(pivot_row != k) std::swap(A_rows[k],A_rows[pivot_row]);
        const bool swap_columns = (pivot_column != k);
        if(swap_columns)
        {
          index[sz - 1 - k] = pivot_column;
          for(size_t j = 0; j < sz; j++)
            std::swap(A[j*stride + k],A[j*stride + pivot_column]);
        } else {
          index[sz - 1 - k] = k;
        }
        if(std::abs(A_rows[k][k]) < small_value)
          log.fwarning("Despite all pivot selection efforts, pivot element A(%zu,%zu) is rather small, %g",
              k,k,numeric::toDouble(std::abs(A_rows[k][k])));
        T fac = T(1.0)/A_rows[k][k];
        for(size_t j = k + 1; j < sz; j++)
        {
          T tmp = A_rows[j][k]*fac;
          for(size_t i = k + 1; i < sz; i++)
            A_rows[j][i] -= A_rows[k][i]*tmp;
          b[j] -= b[k]*tmp;
        }
      }
      //back substitution
      for(size_t k = 0; k < sz; k++)
      {
        const size_t idb = sz - 1 - k;
        x[idb] = b[idb];
        for(size_t i = idb + 1; i < sz; i++)
          x[idb] -= A_rows[idb][i] * x[i];
        x[idb] /= A_rows[idb][idb];
      }
      //backtrack permutations
      for(size_t k = 0; k < sz; k++)
      {
        std::swap(x[sz - 1 - k],x[index[k]]);
      }
    }

    //handmade iterative solvers from gauss-seidel family

    template<typename T> bool jacobi_impl(const size_t sz,
        const T* const __RESTRICT A, const T* const __RESTRICT b,
        T* __RESTRICT x, T* __RESTRICT x_next,
        Logger& log,
        const bool safe_checks = true,
        const int max_iter_count = default_max_iter_count, const T eps = default_eps<T>())
    {
      const size_t stride = sz;
      if(safe_checks)
      {
        log.debug("note that iterative solvers from Gauss-Seidel family work only for diagonally dominant matrices");
        log.debug("checking that given matrix is diagonally dominant...");
        if(!numeric::is_diagonally_dominant(sz,stride,A))
        {
          log.error("the matrix of the system is NOT diagonally dominant, nothing to do here, exiting");
          return false;
        }
      }
      for(int iter = 0; iter < max_iter_count; iter++)
      {
        for(size_t i = 0; i < sz; i++)
        {
          x_next[i] = b[i];
          for(size_t j = 0; j < i; j++)
            x_next[i] -= A[i*stride + j]*x[j];
          for(size_t j = i+1; j < sz; j++)
            x_next[i] -= A[i*stride + j]*x[j];
          x_next[i] /= A[i*stride + i];
        }
        if(numeric::vector_distance_L2(sz,x,x_next) < eps)
        {
          log.fdebug("at iteration %d ||x_{n+1} - x_n||_2 < %g , stoping iterations",iter,numeric::toDouble(eps));
          return true;
        }
        std::swap(x,x_next);
      }
      log.fwarning("maximum iteration count(%d) reached. chances are, we're still very far"
          " from the solution(or requested epsilon(%g) is too small)",max_iter_count,numeric::toDouble(eps));
      return true;
    }

    template<typename T> bool seidel_impl(const size_t sz,
        const T* const __RESTRICT A, const T* const __RESTRICT b,
        T* __RESTRICT x, T* __RESTRICT x_next,
        Logger& log,
        const bool safe_checks = true,
        const int max_iter_count = default_max_iter_count, const T eps = default_eps<T>())
    {
      const size_t stride = sz;
      if(safe_checks)
      {
        log.debug("note that iterative solvers from Gauss-Seidel family work only for diagonally dominant matrices");
        log.debug("checking that given matrix is diagonally dominant...");
        if(!numeric::is_diagonally_dominant(sz,stride,A))
        {
          log.error("the matrix of the system is NOT diagonally dominant, nothing to do here, exiting");
          return false;
        }
      }
      for(int iter = 0; iter < max_iter_count; iter++)
      {
        for(size_t i = 0; i < sz; i++)
        {
          x_next[i] = b[i];
          for(size_t j = 0; j < i; j++)
            x_next[i] -= A[i*stride + j]*x_next[j];
          for(size_t j = i+1; j < sz; j++)
            x_next[i] -= A[i*stride + j]*x[j];
          x_next[i] /= A[i*stride + i];
        }
        if(numeric::vector_distance_L2(sz,x,x_next) < eps)
        {
          log.fdebug("at interation %d ||x_{n+1} - x_n||_2 < %g , stoping iterations",iter,numeric::toDouble(eps));
          return true;
        }
        std::swap(x,x_next);
      }
      log.fwarning("maximum iteration count(%d) reached. chances are, we're still very far"
          " from the solution(or requested epsilon(%g) is too small)",max_iter_count,numeric::toDouble(eps));
      return true;
    }

    template<typename T> bool relaxation_impl(const size_t sz,
        const T* const __RESTRICT A, const T* const __RESTRICT b, T* const __RESTRICT x,
        T* __RESTRICT residual, T* __RESTRICT residual_next,
        Logger& log,
        const bool safe_checks = true,
        const int max_iter_count = default_max_iter_count, const T eps = default_eps<T>() )
    {
      const size_t stride = sz;
      if(safe_checks)
      {
        log.debug("note that iterative solvers from Gauss-Seidel family work only for diagonally dominant matrices");
        log.debug("checking that given matrix is diagonally dominant...");
        if(!numeric::is_diagonally_dominant(sz,stride,A))
        {
          log.error("the matrix of the system is NOT diagonally dominant, nothing to do here, exiting");
          return false;
        }
      }
      //force initial guess to be zero, and residual = b
      for(size_t i = 0; i < sz; i++)
      {
        x[i] = 0;
        residual[i] = b[i]/A[i*stride + i];
      }
      for(int iter = 0; iter < max_iter_count; iter++)
      {
        const size_t max_component_index =
          std::max_element(residual, residual+sz, [](const T& a, const T& b) { return (std::abs(a) < std::abs(b) ); } )
            - residual;
        x[max_component_index] += residual[max_component_index];
        if(std::abs(residual[max_component_index]) < eps)
        {
          log.fdebug("at interation %d ||x_{n+1} - x_n||_2 < %g , stoping iterations",iter,numeric::toDouble(eps));
          return true;
        }
        //update residual vector
        for(size_t i = 0; i < sz; i++)
        {
          residual_next[i] = 0.0;
          for(size_t j = 0; j < sz; j++)
            residual_next[i] -= A[i*stride + j]*residual[j];
          residual_next[i] /= A[i*stride + i];
          residual_next[i] += residual[i];
        }
        std::swap(residual,residual_next);
      }
      log.fwarning("maximum iteration count(%d) reached. chances are, we're still very far"
          " from the solution(or requested epsilon(%g) is too small)",max_iter_count,numeric::toDouble(eps));
      return true;
    }

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

#endif // __DENSE_LINEAR_SOLVER_HPP
