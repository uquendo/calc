#pragma once
#ifndef _INTERPOLATION_LAGRANGE_IMPL_HPP
#define _INTERPOLATION_LAGRANGE_IMPL_HPP
#include <config.h>

#include "numeric/interpolation.hpp"
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

using std::size_t;

namespace numeric {

  template<typename T> T lagrange_interpolate_value_serial(const T* const __RESTRICT weights,
      const T* const __RESTRICT points, const T* const __RESTRICT values, const size_t sz,
      const T& arg, const bool collision_check)
  {
    T numerator = 0, denominator = 0;
    if(collision_check)
    {
      //binary search for arg in points to detect if it collides with some
      auto candidate = std::lower_bound(&points[0],&points[sz],arg);
      if((candidate != &points[sz]) && isEqualReal(arg, *candidate))
        return values[candidate-points];
    }
    for(size_t i = 0; i < sz; i++)
    {
      T term = weights[i] / (arg - points[i]);
      denominator += term;
      numerator += term * values[i];
    }
    return numerator / denominator;
  }

#ifdef HAVE_OPENMP
  template<typename T> T lagrange_interpolate_value_openmp(const T* const __RESTRICT weights,
      const T* const __RESTRICT points, const T* const __RESTRICT values, const size_t sz,
      const T& arg, const bool collision_check)
  {
    T numerator = 0, denominator = 0;
    if(collision_check)
    {
      //binary search for arg in points to detect if it collides with some
      auto candidate = std::lower_bound(&points[0],&points[sz],arg);
      if((candidate != &points[sz]) && isEqualReal(arg, *candidate))
        return values[candidate-points];
    }
#pragma omp parallel for reduction(+ : numerator, denominator)
    for(size_t i = 0; i < sz; i++)
    {
      T term = weights[i] / (arg - points[i]);
      denominator += term;
      numerator += term * values[i];
    }
    return numerator / denominator;
  }
#endif

#ifdef HAVE_CILK
  template<typename T> T lagrange_interpolate_value_cilk(const T* const __RESTRICT weights,
      const T* const __RESTRICT points, const T* const __RESTRICT values, const size_t sz,
      const T& arg, const bool collision_check)
  {
    cilk::reducer< cilk::op_add<T> > numerator(0), denominator(0);
    if(collision_check)
    {
      //binary search for arg in points to detect if it collides with some
      auto candidate = std::lower_bound(&points[0],&points[sz],arg);
      if((candidate != &points[sz]) && isEqualReal(arg, *candidate))
        return values[candidate-points];
    }
    for(size_t i = 0; i < sz; i++)
    {
      T term = weights[i] / (arg - points[i]);
      *denominator += term;
      *numerator += term * values[i];
    }
    return numerator.get_value() / denominator.get_value();
  }
#endif

#ifdef HAVE_TBB
  template<typename T> T lagrange_interpolate_value_tbb(const T* const __RESTRICT weights,
      const T* const __RESTRICT points, const T* const __RESTRICT values, const size_t sz,
      const T& arg, const bool collision_check)
  {
    if(collision_check)
    {
      //binary search for arg in points to detect if it collides with some
      auto candidate = std::lower_bound(&points[0],&points[sz],arg);
      if((candidate != &points[sz]) && isEqualReal(arg, *candidate))
        return values[candidate-points];
    }

    struct _InterpolantReducer {
        T numerator;
        T denominator;
        const T* const __RESTRICT m_weights;
        const T* const __RESTRICT m_points;
        const T* const __RESTRICT m_values;
        const T& m_arg;
        _InterpolantReducer(const T* const __RESTRICT weights,
            const T* const __RESTRICT points, const T* const __RESTRICT values,
            const T& arg)
          : numerator(0)
          , denominator(0)
          , m_weights(weights)
          , m_points(points)
          , m_values(values)
          , m_arg(arg)
        {}
        _InterpolantReducer( _InterpolantReducer& f, tbb::split )
        {
          numerator = 0; denominator = 0;
          m_weights = f.m_weights; m_points = f.m_points; m_values = f.m_values; m_arg = f.m_arg;
        }
        void operator()( const tbb::blocked_range<size_t>& r ) {
            T tmp_numerator(numerator), tmp_denominator(denominator);
            for( size_t i = r.begin(); i != r.end(); i++ ) {
                T term = m_weights[i] / (m_arg - m_points[i]);
                tmp_denominator += term;
                tmp_numerator += term * m_values[i];
            }
            numerator = tmp_numerator;
            denominator = tmp_denominator;
        }
        void join( _InterpolantReducer& rhs ) { numerator += rhs.numerator; denominator += rhs.denominator; }
    } interpolant(weights,points,values,arg);
    parallelReduceBlock(size_t(0), sz, interpolant);

    return interpolant.numerator / interpolant.denominator;
  }
#endif

  template<typename T> T lagrange_interpolate_value(const T* const __RESTRICT weights,
      const T* const __RESTRICT points, const T* const __RESTRICT values, const size_t sz,
      const T& arg,
      const bool collision_check,
      const TThreading threading_model)
  {
    switch(threading_model)
    {
      case T_Serial:
        return lagrange_interpolate_value_serial<T>(weights,points,values,sz,arg,collision_check);
      case T_Std:
  //      return lagrange_interpolate_value_stdthreads<T>(weights,points,values,sz,arg,collision_check);
#ifdef HAVE_PTHREADS
      case T_Posix:
  //      return lagrange_interpolate_value_pthreads<T>(weights,points,values,sz,arg,collision_check);
#endif
#ifdef HAVE_OPENMP
      case T_OpenMP:
        return lagrange_interpolate_value_openmp<T>(weights,points,values,sz,arg,collision_check);
#endif
#ifdef HAVE_CILK
      case T_Cilk:
        return lagrange_interpolate_value_cilk<T>(weights,points,values,sz,arg,collision_check);
#endif
#ifdef HAVE_TBB
      case T_TBB:
        return lagrange_interpolate_value_tbb<T>(weights,points,values,sz,arg,collision_check);
#endif
      case T_Undefined:
      default:
        return lagrange_interpolate_value_serial<T>(weights,points,values,sz,arg,collision_check);
    }
  }

  template<typename T> void lagrange_interpolate_table_serial(const T* const __RESTRICT weights,
      const T* const __RESTRICT points, const T* const __RESTRICT values, const size_t sz,
      const T* const __RESTRICT args, T* const __RESTRICT table, const size_t table_sz,
      const bool collision_check)
  {
    for(size_t i = 0; i < table_sz; i++)
    {
      table[i] = lagrange_interpolate_value_serial(weights,points,values,sz,args[i],collision_check);
    }
  }

#ifdef HAVE_OPENMP
  template<typename T> void lagrange_interpolate_table_openmp(const T* const __RESTRICT weights,
      const T* const __RESTRICT points, const T* const __RESTRICT values, const size_t sz,
      const T* const __RESTRICT args, T* const __RESTRICT table, const size_t table_sz,
      const bool collision_check)
  {
#pragma omp parallel for
    for(size_t i = 0; i < table_sz; i++)
    {
      table[i] = lagrange_interpolate_value_serial(weights,points,values,sz,args[i],collision_check);
    }
  }
#endif

#ifdef HAVE_CILK
  template<typename T> void lagrange_interpolate_table_cilk(const T* const __RESTRICT weights,
      const T* const __RESTRICT points, const T* const __RESTRICT values, const size_t sz,
      const T* const __RESTRICT args, T* const __RESTRICT table, const size_t table_sz,
      const bool collision_check)
  {
    cilk_for(size_t i = 0; i < table_sz; i++)
    {
      table[i] = lagrange_interpolate_value_serial(weights,points,values,sz,args[i],collision_check);
    }
  }
#endif

#ifdef HAVE_TBB
  template<typename T> void lagrange_interpolate_table_tbb(const T* const __RESTRICT weights,
      const T* const __RESTRICT points, const T* const __RESTRICT values, const size_t sz,
      const T* const __RESTRICT args, T* const __RESTRICT table, const size_t table_sz,
      const bool collision_check)
  {
    parallelForElem(size_t(0), table_sz,
      [&](size_t i)
      {
        table[i] = lagrange_interpolate_value_serial(weights,points,values,sz,args[i],collision_check);
      }
    );
  }
#endif

  template<typename T> void lagrange_interpolate_table(const T* const __RESTRICT weights,
      const T* const __RESTRICT points, const T* const __RESTRICT values, const size_t sz,
      const T* const __RESTRICT args, T* const __RESTRICT table, const size_t table_sz,
      const bool collision_check,
      const TThreading threading_model)
  {
    switch(threading_model)
    {
      case T_Serial:
        return lagrange_interpolate_table_serial<T>(weights,points,values,sz,args,table,table_sz,collision_check);
      case T_Std:
  //      return lagrange_interpolate_table_stdthreads<T>(weights,points,values,sz,args,table,table_sz,collision_check);
#ifdef HAVE_PTHREADS
      case T_Posix:
  //      return lagrange_interpolate_table_pthreads<T>(weights,points,values,sz,args,table,table_sz,collision_check);
#endif
#ifdef HAVE_OPENMP
      case T_OpenMP:
        return lagrange_interpolate_table_openmp<T>(weights,points,values,sz,args,table,table_sz,collision_check);
#endif
#ifdef HAVE_CILK
      case T_Cilk:
        return lagrange_interpolate_table_cilk<T>(weights,points,values,sz,args,table,table_sz,collision_check);
#endif
#ifdef HAVE_TBB
      case T_TBB:
        return lagrange_interpolate_table_tbb<T>(weights,points,values,sz,args,table,table_sz,collision_check);
#endif
      case T_Undefined:
      default:
        return lagrange_interpolate_table_serial<T>(weights,points,values,sz,args,table,table_sz,collision_check);
    }
  }

//  template<typename T> void lagrange_interpolation_compute_weights(T* const __RESTRICT weights,
//      const T* const __RESTRICT points, const size_t sz,
//      const TThreading threading_model)
//  {
//  }

}

#endif /* _INTERPOLATION_LAGRANGE_IMPL_HPP */
