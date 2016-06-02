#include "dense_linear_solve.hpp"

#include <cmath>
#include <algorithm>

namespace Calc
{
  namespace dense_linear_solve
 {

    template<typename T> bool numeric_cpp_jacobi_impl(const size_t sz,
        const T* const __RESTRICT A, const T* const __RESTRICT b,
        T* __RESTRICT x, T* __RESTRICT x_next,
        Logger& log,
        const int max_iter_count = default_max_iter_count, const T eps = default_eps<T>())
    {
      const size_t stride = sz;
      log.debug("note that iterative solvers from Gauss-Seidel family work only for diagonally dominant matrices");
      log.debug("checking that given matrix is diagonally dominant...");
      if(!numeric::is_diagonally_dominant(sz,stride,A))
      {
        log.error("the matrix of the system is NOT diagonally dominant, nothing to do here, exiting");
        return false;
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
          log.fdebug("at interation %d ||x_{n+1} - x_n||_2 < %g , stoping iterations",iter,numeric::toDouble(eps));
          return true;
        }
        std::swap(x,x_next);
      }
      log.fwarning("maximum iteration count(%d) reached. chances are, we're still very far from the solution(or requested epsilon(%g) is too small)",max_iter_count,numeric::toDouble(eps));
      return true;
    }

    template<typename T> bool numeric_cpp_seidel_impl(const size_t sz,
        const T* const __RESTRICT A, const T* const __RESTRICT b,
        T* __RESTRICT x, T* __RESTRICT x_next,
        Logger& log,
        const int max_iter_count = default_max_iter_count, const T eps = default_eps<T>())
    {
      const size_t stride = sz;
      log.debug("note that iterative solvers from Gauss-Seidel family work only for diagonally dominant matrices");
      log.debug("checking that given matrix is diagonally dominant...");
      if(!numeric::is_diagonally_dominant(sz,stride,A))
      {
        log.error("the matrix of the system is NOT diagonally dominant, nothing to do here, exiting");
        return false;
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
      log.fwarning("maximum iteration count(%d) reached. chances are, we're still very far from the solution(or requested epsilon(%g) is too small)",max_iter_count,numeric::toDouble(eps));
      return true;
    }

    template<typename T> bool numeric_cpp_relaxation_impl(const size_t sz,
        const T* const __RESTRICT A, const T* const __RESTRICT b, T* const __RESTRICT x,
        T* __RESTRICT residual, T* __RESTRICT residual_next,
        Logger& log,
        const int max_iter_count = default_max_iter_count, const T eps = default_eps<T>() )
    {
      const size_t stride = sz;
      log.debug("note that iterative solvers from Gauss-Seidel family work only for diagonally dominant matrices");
      log.debug("checking that given matrix is diagonally dominant...");
      if(!numeric::is_diagonally_dominant(sz,stride,A))
      {
        log.error("the matrix of the system is NOT diagonally dominant, nothing to do here, exiting");
        return false;
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
      log.fwarning("maximum iteration count(%d) reached. chances are, we're still very far from the solution(or requested epsilon(%g) is too small)",max_iter_count,numeric::toDouble(eps));
      return true;
    }

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
        return numeric_cpp_jacobi_impl<T>(_sz,_A_buf,_b_buf,_x,_x_tmp,p.progress_ptr->log());
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
        return numeric_cpp_seidel_impl<T>(_sz,_A_buf,_b_buf,_x,_x_tmp,p.progress_ptr->log());
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
        return numeric_cpp_relaxation_impl<T>(_sz,_A_buf,_b_buf,_x,_x_tmp,_residual_tmp,p.progress_ptr->log());
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
          return numeric_cpp_jacobi_impl<double>(_sz,_A_buf,_b_buf,_x,_x_next,log);
        case A_NumCppSeidel:
//          return numeric_cpp_seidel()(p.Popt.type, p);
          return numeric_cpp_seidel_impl<double>(_sz,_A_buf,_b_buf,_x,_x_next,log);
        case A_NumCppRelaxation:
//          return numeric_cpp_relaxation()(p.Popt.type, p);
          return numeric_cpp_relaxation_impl<double>(_sz,_A_buf,_b_buf,_x,_x_next,_residual_next,log);
        case A_Undefined:
        default:
          throw Calc::ParameterError("Algorithm is not implemented");
      }
      return false;
    }
  }
}

