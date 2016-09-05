#include "dense_linear_solve.hpp"

#include "calcapp/math/dense_linear_solver.hpp"

#include <cmath>
#include <algorithm>

namespace Calc
{
  namespace dense_linear_solve
 {

    //dumb c++ version of gauss elimination without pivoting
    struct numeric_cpp_gauss : numeric::MPFuncBase<numeric_cpp_gauss,AlgoParameters>
    {
      template<typename T> inline void perform(const AlgoParameters& p)
      {
        //warmup
        const size_t _sz = p.system_size;
        T* const _A_buf = reinterpret_cast<T*>(p.A_buf);
        T* const _b_buf = reinterpret_cast<T*>(p.b_buf);
        T* const _x = reinterpret_cast<T* const>(p.x);
        T**  _A_rows = reinterpret_cast<T** const>(p.A_rows);
        return Calc::gauss_impl<T>(_sz,_A_buf,_b_buf,_x,p.progress_ptr->log());
      }
    };

    //dumb c++ version of jordan elimination without any pivoting
    struct numeric_cpp_jordan : numeric::MPFuncBase<numeric_cpp_jordan,AlgoParameters>
    {
      template<typename T> inline void perform(const AlgoParameters& p)
      {
        //warmup
        const size_t _sz = p.system_size;
        T* const _A_buf = reinterpret_cast<T*>(p.A_buf);
        T* const _b_buf = reinterpret_cast<T*>(p.b_buf);
        T* const _x = reinterpret_cast<T* const>(p.x);
        T**  _A_rows = reinterpret_cast<T** const>(p.A_rows);
        return Calc::jordan_impl<T>(_sz,_A_buf,_b_buf,_x,p.progress_ptr->log());
      }
    };

    //dumb c++ version of gauss elimination with full pivoting
    struct numeric_cpp_gauss_full_pivoting : numeric::MPFuncBase<numeric_cpp_gauss_full_pivoting,AlgoParameters>
    {
      template<typename T> inline void perform(const AlgoParameters& p)
      {
        //warmup
        const size_t _sz = p.system_size;
        T* const _A_buf = reinterpret_cast<T*>(p.A_buf);
        T* const _b_buf = reinterpret_cast<T*>(p.b_buf);
        T* const _x = reinterpret_cast<T* const>(p.x);
        T** _A_rows = reinterpret_cast<T** const>(p.A_rows);
        size_t * const _index = new size_t[_sz];
        return Calc::gauss_full_pivoting_impl<T>(_sz,_A_buf,_b_buf,_A_rows,_x,_index,p.progress_ptr->log());
      }
    };

    //dispatcher
    void perform(const AlgoParameters& p, Logger& log)
    {
      numeric::ParallelScheduler __ps(p.Topt.type,p.Topt.num);
      ExecTimeMeter __etm(log, "dense_linear_solve::perform");
//      PERF_METER(log, "dense_linear_solve::perform");
      const size_t _sz = p.system_size;
      double* const _A_buf = p.A_buf.get();
      double* const _b_buf = p.b_buf.get();
      double* const _x = p.x;
      double** _A_rows = p.A_rows;
      switch(p.Aopt.type)
      {
        case A_NumCppGauss:
//          return numeric_cpp_gauss()(p.Popt.type, p);
          return Calc::gauss_impl<double>(_sz,_A_buf,_b_buf,_x,log);
        case A_NumCppJordan:
//          return numeric_cpp_jordan()(p.Popt.type, p);
          return Calc::jordan_impl<double>(_sz,_A_buf,_b_buf,_x,log);
        case A_NumCppFullPivoting:
//          return numeric_cpp_gauss_full_pivoting()(p.Popt.type, p);
          return Calc::gauss_full_pivoting_impl<double>(_sz,_A_buf,_b_buf,_A_rows,_x,
              reinterpret_cast<size_t * const>(new size_t[_sz]),log);
        case A_Undefined:
        default:
          throw Calc::ParameterError("Algorithm is not implemented");
      }
    }
  }
}

