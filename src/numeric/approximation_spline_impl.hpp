#pragma once
#ifndef _APPROXIMATION_LAGRANGE_IMPL_HPP
#define _APPROXIMATION_LAGRANGE_IMPL_HPP
#include <config.h>

#include "numeric/approximation.hpp"
#include "numeric/parallel.hpp"
#include "numeric/cache.hpp"

#ifdef HAVE_CILK
#include <cilk/cilk.h>
#include <cilk/reducer_opadd.h>
#endif

#ifdef HAVE_TBB
#include "numeric/parallel_tbb.hpp"
#endif

#include <algorithm>
#include <cstddef>
#include <iostream>

using std::size_t;

namespace numeric {

/*
  //compute smoothing cubic spline expressed via values of spline and its second derivative at knots
  //see Carl de Boor, "A practical guide to splines", ch. xiv, pp. 207-210 (2001)
  template<typename T> T cubic_spline_natural_compute(const T* const __RESTRICT weights,
      const T* const __RESTRICT points, const T* const __RESTRICT values, const size_t points_sz,
      T* const __RESTRICT S0, T* const __RESTRICT S2, const size_t knots_sz,
      const T smoothing_p,
      const TThreading threading_model)
  {
  }
*/

  template<typename T> T cubic_spline_evaluate_value_left_helper(const T* const __RESTRICT knots,
      const T* const __RESTRICT S0, const T* const __RESTRICT S2,
      const T& arg)
  {
    const T h = knots[1] - knots[0];
    const T t = (arg - knots[0]) / h;
    return ( S0[0]*(T(1) - t) + S0[1]*t - t*h*h*(2*S2[0] + S2[1])/T(6) );
  }

  template<typename T> T cubic_spline_evaluate_value_right_helper(const T* const __RESTRICT knots,
      const T* const __RESTRICT S0, const T* const __RESTRICT S2, const size_t sz,
      const T& arg)
  {
    const T h = knots[sz-1] - knots[sz-2];
    const T t = (arg - knots[sz-1]) / h;
    return ( S0[sz-2]*(T(1) - t) + S0[sz-1]*t - t*h*h*(2*S2[sz-2] + S2[sz-1])/T(6) );
  }

  template<typename T> T cubic_spline_evaluate_value_internal_helper(const T* const __RESTRICT knots,
      const T* const __RESTRICT S0, const T* const __RESTRICT S2,
      const T& arg, const size_t interval_id)
  {
    const T h = knots[interval_id] - knots[interval_id-1];
    const T t = (arg - knots[interval_id-1]) / h;
    return ( S0[interval_id-1]*(T(1) - t) + S0[interval_id]*t - t*(T(1) - t)*h*h*((T(2) - t)*S2[interval_id-1] + (T(1) + t)*S2[interval_id])/T(6) );
  }

  template<typename T> T cubic_spline_evaluate_value(const T* const __RESTRICT knots,
      const T* const __RESTRICT S0, const T* const __RESTRICT S2, const size_t sz,
      const T& arg)
  {
    //binary search for arg in knots array to detect the interval for evaluation
    auto interval_ptr = std::lower_bound(&knots[0], &knots[sz], arg);
    const size_t interval_id = interval_ptr - knots;
    if(interval_id == 0)
    {
      return cubic_spline_evaluate_value_left_helper(knots,S0,S2,arg);
    }
    else if(interval_id == sz)
    {
      return cubic_spline_evaluate_value_right_helper(knots,S0,S2,sz,arg);
    }
    return cubic_spline_evaluate_value_internal_helper(knots,S0,S2,arg,interval_id);
  }

  template<typename T> void cubic_spline_evaluate_table_serial(const T* const __RESTRICT knots,
      const T* const __RESTRICT S0, const T* const __RESTRICT S2, const size_t sz,
      const T* const __RESTRICT args, T* const __RESTRICT table, const size_t table_sz)
  {
    //binary search for first and last knots in args array to detect the interval for evaluation
    auto left_interval_ptr = &args[0];
    auto right_interval_ptr = &args[table_sz-1];
    if( (*left_interval_ptr) < knots[0] )
      left_interval_ptr = std::lower_bound(&args[0], &args[table_sz], knots[0]);
    if( (*right_interval_ptr) > knots[sz-1] )
      right_interval_ptr = std::lower_bound(&args[0], &args[table_sz], knots[sz-1]);
    const size_t from_id = left_interval_ptr - args;
    const size_t to_id = right_interval_ptr - args;
    for(size_t i = 0; i < from_id; i++)
    {
      table[i] = cubic_spline_evaluate_value_left_helper(knots,S0,S2,args[i]);
    }
    for(size_t i = from_id, interval_id = 1; i <= to_id; i++)
    {
      while(args[i] > knots[interval_id])
        ++interval_id;
      table[i] = cubic_spline_evaluate_value_internal_helper(knots,S0,S2,args[i],interval_id);
    }
    for(size_t i = to_id + 1; i < table_sz; i++)
    {
      table[i] = cubic_spline_evaluate_value_right_helper(knots,S0,S2,sz,args[i]);
    }
  }

#ifdef HAVE_OPENMP
  template<typename T> void cubic_spline_evaluate_table_openmp(const T* const __RESTRICT knots,
      const T* const __RESTRICT S0, const T* const __RESTRICT S2, const size_t sz,
      const T* const __RESTRICT args, T* const __RESTRICT table, const size_t table_sz)
  {
    //binary search for first and last knots in args array to detect the interval for evaluation
    auto left_interval_ptr = &args[0];
    auto right_interval_ptr = &args[table_sz-1];
    if( (*left_interval_ptr) < knots[0] )
      left_interval_ptr = std::lower_bound(&args[0], &args[table_sz], knots[0]);
    if( (*right_interval_ptr) > knots[sz-1] )
      right_interval_ptr = std::lower_bound(&args[0], &args[table_sz], knots[sz-1]);
    const size_t from_id = left_interval_ptr - args;
    const size_t to_id = right_interval_ptr - args;
#pragma omp parallel for
    for(size_t i = 0; i < from_id; i++)
    {
      table[i] = cubic_spline_evaluate_value_left_helper(knots,S0,S2,args[i]);
    }
    size_t interval_id = 1;
#pragma omp parallel for private(interval_id)
    for(size_t i = from_id; i < to_id; i++)
    {
      while(args[i] > knots[interval_id])
        ++interval_id;
      table[i] = cubic_spline_evaluate_value_internal_helper(knots,S0,S2,args[i],interval_id);
    }
#pragma omp parallel for
    for(size_t i = to_id; i < table_sz; i++)
    {
      table[i] = cubic_spline_evaluate_value_right_helper(knots,S0,S2,sz,args[i]);
    }
  }
#endif

#ifdef HAVE_CILK
  template<typename T> void cubic_spline_evaluate_table_cilk(const T* const __RESTRICT knots,
      const T* const __RESTRICT S0, const T* const __RESTRICT S2, const size_t sz,
      const T* const __RESTRICT args, T* const __RESTRICT table, const size_t table_sz)
  {
    //binary search for first and last knots in args array to detect the interval for evaluation
    auto left_interval_ptr = &args[0];
    auto right_interval_ptr = &args[table_sz-1];
    if( (*left_interval_ptr) < knots[0] )
      left_interval_ptr = std::lower_bound(&args[0], &args[table_sz], knots[0]);
    if( (*right_interval_ptr) > knots[sz-1] )
      right_interval_ptr = std::lower_bound(&args[0], &args[table_sz], knots[sz-1]);
    const size_t from_id = left_interval_ptr - args;
    const size_t to_id = right_interval_ptr - args;
    cilk_for(size_t i = 0; i < from_id; i++)
    {
      table[i] = cubic_spline_evaluate_value_left_helper(knots,S0,S2,args[i]);
    }
    cilk_for(size_t i = from_id, interval_id = 1; i < to_id; i++)
    {
      while(args[i] > knots[interval_id])
        ++interval_id;
      table[i] = cubic_spline_evaluate_value_internal_helper(knots,S0,S2,args[i],interval_id);
    }
    cilk_for(size_t i = to_id; i < table_sz; i++)
    {
      table[i] = cubic_spline_evaluate_value_right_helper(knots,S0,S2,sz,args[i]);
    }
  }
#endif

#ifdef HAVE_TBB
  template<typename T> void cubic_spline_evaluate_table_tbb(const T* const __RESTRICT knots,
      const T* const __RESTRICT S0, const T* const __RESTRICT S2, const size_t sz,
      const T* const __RESTRICT args, T* const __RESTRICT table, const size_t table_sz)
  {
    //binary search for first and last knots in args array to detect the interval for evaluation
    auto left_interval_ptr = &args[0];
    auto right_interval_ptr = &args[table_sz-1];
    if( (*left_interval_ptr) < knots[0] )
      left_interval_ptr = std::lower_bound(&args[0], &args[table_sz], knots[0]);
    if( (*right_interval_ptr) > knots[sz-1] )
      right_interval_ptr = std::lower_bound(&args[0], &args[table_sz], knots[sz-1]);
    const size_t from_id = left_interval_ptr - args;
    const size_t to_id = right_interval_ptr - args;
    parallelForElem(size_t(0), from_id,
      [&](size_t i)
      {
        table[i] = cubic_spline_evaluate_value_left_helper(knots,S0,S2,args[i]);
      }
    );

    struct {
      size_t m_interval_id;
      const T* const m_knots;
      const T* const m_S0;
      const T* const m_S2;
      const T* const m_args;
      T* const m_table;

      void operator()(size_t i)
      {
        while(m_args[i] > m_knots[m_interval_id])
          ++m_interval_id;
        m_table[i] = cubic_spline_evaluate_value_internal_helper(m_knots,m_S0,m_S2,m_args[i],m_interval_id);
      }
    } EvalFunc(1, knots, S0, S2, args, table);

    parallelForElem(from_id, to_id, EvalFunc );

    parallelForElem(to_id, table_sz,
      [&](size_t i)
      {
        table[i] = cubic_spline_evaluate_value_right_helper(knots,S0,S2,sz,args[i]);
      }
    );
  }
#endif

  //evaluate cubic spline for the array of arguments
  //parallel versions assume that table_sz >> points_sz
  template<typename T> void cubic_spline_evaluate_table(const T* const __RESTRICT knots,
      const T* const __RESTRICT S0, const T* const __RESTRICT S2, const size_t knots_sz,
      const T* const __RESTRICT args, T* const __RESTRICT table, const size_t table_sz,
      const TThreading threading_model)
  {
    switch(threading_model)
    {
      case T_Serial:
        return cubic_spline_evaluate_table_serial<T>(knots,S0,S2,knots_sz,args,table,table_sz);
      case T_Std:
  //      return cubic_spline_evaluate_table_stdthreads<T>(knots,S0,S2,knots_sz,args,table,table_sz);
#ifdef HAVE_PTHREADS
      case T_Posix:
  //      return cubic_spline_evaluate_table_pthreads<T>(knots,S0,S2,knots_sz,args,table,table_sz);
#endif
#ifdef HAVE_OPENMP
      case T_OpenMP:
        return cubic_spline_evaluate_table_openmp<T>(knots,S0,S2,knots_sz,args,table,table_sz);
#endif
#ifdef HAVE_CILK
      case T_Cilk:
        return cubic_spline_evaluate_table_cilk<T>(knots,S0,S2,knots_sz,args,table,table_sz);
#endif
#ifdef HAVE_TBB
      case T_TBB:
        return cubic_spline_evaluate_table_tbb<T>(knots,S0,S2,knots_sz,args,table,table_sz);
#endif
      case T_Undefined:
      default:
        return cubic_spline_evaluate_table_serial<T>(knots,S0,S2,knots_sz,args,table,table_sz);
    }
  }

}

#endif /* _APPROXIMATION_LAGRANGE_IMPL_HPP */
