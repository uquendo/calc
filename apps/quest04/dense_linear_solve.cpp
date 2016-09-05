#include "dense_linear_solve.hpp"

#include "calcapp/math/dense_linear_solver.hpp"

#include <cmath>
#include <algorithm>

namespace Calc
{
  namespace dense_linear_solve
 {

    //dumb c++ version of Jacobi iterative solver
    struct numeric_cpp_jacobi : numeric::MPFuncBase<numeric_cpp_jacobi,AlgoParameters,bool>
    {
      template<typename T> inline bool perform(const AlgoParameters& p)
      {
        //warmup
        const size_t _sz = p.system_size;
        T* const _A_buf = reinterpret_cast<T*>(p.A_buf);
        T* const _b_buf = reinterpret_cast<T*>(p.b_buf);
        T* const _x = reinterpret_cast<T* const>(p.x);
        T* const _x_tmp = reinterpret_cast<T* const>(p.x_tmp);
        return Calc::jacobi_impl<T>(_sz,_A_buf,_b_buf,_x,_x_tmp,p.progress_ptr->log());
      }
    };

    //dumb c++ version of Seidel iterative solver
    struct numeric_cpp_seidel : numeric::MPFuncBase<numeric_cpp_seidel,AlgoParameters,bool>
    {
      template<typename T> inline bool perform(const AlgoParameters& p)
      {
        //warmup
        const size_t _sz = p.system_size;
        T* const _A_buf = reinterpret_cast<T*>(p.A_buf);
        T* const _b_buf = reinterpret_cast<T*>(p.b_buf);
        T* const _x = reinterpret_cast<T* const>(p.x);
        T* const _x_tmp = reinterpret_cast<T* const>(p.x_tmp);
        return Calc::seidel_impl<T>(_sz,_A_buf,_b_buf,_x,_x_tmp,p.progress_ptr->log());
      }
    };

    //dumb c++ version of "relaxation" iterative solver
    struct numeric_cpp_relaxation : numeric::MPFuncBase<numeric_cpp_relaxation,AlgoParameters,bool>
    {
      template<typename T> inline bool perform(const AlgoParameters& p)
      {
        //warmup
        const size_t _sz = p.system_size;
        T* const _A_buf = reinterpret_cast<T*>(p.A_buf);
        T* const _b_buf = reinterpret_cast<T*>(p.b_buf);
        T* const _x = reinterpret_cast<T* const>(p.x);
        T* const _x_tmp = reinterpret_cast<T* const>(p.x_tmp);
        T* const _residual_tmp = reinterpret_cast<T* const>(p.residual_tmp);
        return Calc::relaxation_impl<T>(_sz,_A_buf,_b_buf,_x,_x_tmp,_residual_tmp,p.progress_ptr->log());
      }
    };

    //dispatcher
    bool perform(const AlgoParameters& p, Logger& log)
    {
      numeric::ParallelScheduler __ps(p.Topt.type,p.Topt.num);
      ExecTimeMeter __etm(log, "dense_linear_solve::perform");
//      PERF_METER(log, "dense_linear_solve::perform");
      const size_t _sz = p.system_size;
      double* const _A_buf = p.A_buf.get();
      double* const _b_buf = p.b_buf.get();
      double* const _x = p.x;
      double* const _x_next = p.x_tmp;
      double* const _residual_next = p.residual_tmp;
      switch(p.Aopt.type)
      {
        case A_NumCppJacobi:
//          return numeric_cpp_jacobi()(p.Popt.type, p);
          return Calc::jacobi_impl<double>(_sz,_A_buf,_b_buf,_x,_x_next,log);
        case A_NumCppSeidel:
//          return numeric_cpp_seidel()(p.Popt.type, p);
          return Calc::seidel_impl<double>(_sz,_A_buf,_b_buf,_x,_x_next,log);
        case A_NumCppRelaxation:
//          return numeric_cpp_relaxation()(p.Popt.type, p);
          return Calc::relaxation_impl<double>(_sz,_A_buf,_b_buf,_x,_x_next,_residual_next,log);
        case A_Undefined:
        default:
          throw Calc::ParameterError("Algorithm is not implemented");
      }
      return false;
    }
  }
}

