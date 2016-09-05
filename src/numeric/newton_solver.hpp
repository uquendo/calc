#pragma once
#ifndef _NEWTON_SOLVER_HPP
#define _NEWTON_SOLVER_HPP
#include "config.h"

#include "numeric/real.hpp"
#include "numeric/blas.hpp"

#include <limits>
#include <cstddef>
#include <cstring>

namespace numeric {

/*
template<typename T> void example_f(size_t sz, const T* const __RESTRICT arg, T* const __RESTRICT f_value)
{
  for(size_t i = 0; i < sz; i++)
    f_value = arg[i]*arg[i]*arg[i];
}
*/

//evaluates df by forward differences
//NOTE: df matrix is stored in row major order
//TODO: add optional column major version
template<typename T, typename TFunc>
  void df(size_t sz, TFunc& f, T* const __RESTRICT arg, T* const __RESTRICT f_value, T* const __RESTRICT df_value)
{
  const T eps = sqrt(std::numeric_limits<T>::epsilon())/sz;
  const T zero = T(0.0);
  const T one = T(1.0);
  for(size_t k = 0; k < sz; k++)
  {
    const T x_k = arg[k];
    //forward differences
    arg[k] = x_k + eps;
    //evaluate f(arg+eps_k)
    f(sz, arg, df_value+k*sz);
    arg[k] = x_k;
    for(size_t i = 0; i < sz; i++)
    {
      const T f_i = f_value[i];
      if(!isEqualReal(f_i, zero))
      {
        if(std::abs(df_value[k*sz + i]) > std::abs(f_i))
        {
          df_value[k*sz + i] *= (one - f_i/df_value[k*sz + i]);
        } else {
          df_value[k*sz + i] = df_value[k*sz + i]/f_i - one;
          df_value[k*sz + i] *= f_i;
        }
      }
      df_value[k*sz + i] *= (one/eps);
    }
  }
  numeric::square_transpose(sz, df_value);
}

template<typename T, typename TFunc, typename dTFunc, typename TLinearSolver>
  void find_root_newton(size_t sz, TFunc& f, dTFunc& df, TLinearSolver& solver,
                        T* const __RESTRICT guess, T* const __RESTRICT root,
                        T* const __RESTRICT matrix_storage, T* const __RESTRICT vector_storage,
                        T l2norm_eps, int max_iter,
                        T& l2norm, int& iter)
{
  for(iter = 0; iter < max_iter; iter++)
  {
//    std::cerr << "guess is: ";
//    for(size_t i = 0; i < sz; i++)
//      std::cerr << guess[i] << " ";
//    std::cerr << std::endl;
    //evaluate f(x)
    f(sz, guess, vector_storage);
//    std::cerr << "f(guess) is: ";
//    for(size_t i = 0; i < sz; i++)
//      std::cerr << vector_storage[i] << " ";
//    std::cerr << std::endl;
    l2norm = vector_norm_L2(sz, vector_storage);
    if(l2norm < l2norm_eps)
      return;
    //evaluate df
    df(sz, guess, vector_storage, matrix_storage);
//    std::cerr << "df(guess) is: ";
//    for(size_t i  = 0; i < sz; i++)
//    {
//      for(size_t j = 0; j < sz; j++)
//        std::cerr << matrix_storage[i*sz + j] << " ";
//      std::cerr << std::endl;
//    }
    //assemble rhs
    dgemv(TMatrixStorage::RowMajor, TMatrixTranspose::No,
        matrix_storage, guess, vector_storage, sz, T(1.0), T(-1.0));
//    std::cerr << "rhs is: ";
//    for(size_t i = 0; i < sz; i++)
//      std::cerr << vector_storage[i] << " ";
//    std::cerr << std::endl;
    //solve linear system
    solver(sz, matrix_storage, vector_storage, root);
    //update guess
    memcpy(guess, root, sz*sizeof(T));
  }
}

template<typename T, typename TFunc, typename TLinearSolver>
  void find_root_newton(size_t sz, TFunc& f, TLinearSolver& solver,
                        T* const __RESTRICT guess, T* const __RESTRICT root,
                        T* const __RESTRICT matrix_storage, T* const __RESTRICT vector_storage,
                        T l2norm_eps, int max_iter,
                        T& l2norm, int& iter)
{
  struct DFunc {
    TFunc m_f;
    DFunc(TFunc& f) : m_f(f) {}
    void operator()(size_t sz, T* const __RESTRICT arg, T* const __RESTRICT f_value, T* const __RESTRICT df_value)
    {
      df(sz, m_f, arg, f_value, df_value);
    }
  } forward_difference_derivative(f);
  find_root_newton(sz, f, forward_difference_derivative, solver, guess, root, matrix_storage, vector_storage, l2norm_eps, max_iter, l2norm, iter);
}

}

#endif
