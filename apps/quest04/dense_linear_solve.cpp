#include "dense_linear_solve.hpp"

#include <cmath>
#include <algorithm>

namespace Calc
{
  namespace dense_linear_solve
 {

    //dispatcher
    bool perform(const AlgoParameters& parameters, Logger& log);

    template<typename T> bool isDiagonallyDominant(const size_t sz, const size_t stride, const T* const __RESTRICT A)
    {
      for(size_t i = 0; i < sz; i++)
      {
        T row_sum = T(0.0);
        //no subnormals, please
        if(std::abs(A[i*stride+i]) < std::numeric_limits<T>::min())
          return false;
        for(size_t j = 0; j < i; j++)
        {
          row_sum += std::abs(A[i*stride+j]);
        }
        for(size_t j = i+1; j < sz; j++)
        {
          row_sum += std::abs(A[i*stride+j]);
        }
        if(!(row_sum < std::abs(A[i*stride+i])))
          return false;
      }
      return true;
    }

    //residual norm calculation
    template<typename T> T vectorDifferenceL2Norm(const size_t sz, const T* const __RESTRICT x, const T* const __RESTRICT y)
    {
      T norm = T(0.0);
      const T zero = T(0.0);
      const T one = T(1.0);
      if(sz == 1)
      {
        norm = std::abs(x[0] - y[0]);
      } else {
        T component = T(0.0);
        T scale = T(0.0);
        T sum   = T(1.0);
        for(size_t i = 0; i < sz; i++)
        {
          //compute component i of vector difference
          component = std::abs(x[i] - y[i]);
          //update norm blas-ish way
          if(!numeric::isEqualReal(component, zero))
          {
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

    template<typename T> T vectorDifferenceL1Norm(const size_t sz, const T* const __RESTRICT x, const T* const __RESTRICT y)
    {
      T norm = T(0.0);
      const T zero = T(0.0);
      const T one = T(1.0);
      if(sz == 1)
      {
        norm = std::abs(x[0] - y[0]);
      } else {
        T component = T(0.0);
        T scale = T(0.0);
        T sum   = T(1.0);
        for(size_t i = 0; i < sz; i++)
        {
          //compute component i of vector difference
          component = std::abs(x[i] - y[i]);
          //update norm blas-ish way
          if(!numeric::isEqualReal(component, zero))
          {
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

    template<typename T> bool numeric_cpp_jacobi_impl(const size_t sz,
        const T* const __RESTRICT Ab, T* __RESTRICT x, T* __RESTRICT x_next,
        Logger& log,
        const int max_iter_count = default_max_iter_count, const T eps = default_eps<T>())
    {
      const size_t stride = sz + 1;
      log.debug("note that iterative solvers from Gauss-Seidel family work only for diagonally dominant matrices");
      log.debug("checking that given matrix is diagonally dominant...");
      if(!isDiagonallyDominant(sz,stride,Ab))
      {
        log.error("the matrix of the system is NOT diagonally dominant, nothing to do here, exiting");
        return false;
      }
      for(int iter = 0; iter < max_iter_count; iter++)
      {
        for(size_t i = 0; i < sz; i++)
        {
          x_next[i] = Ab[i*stride + sz];
          for(size_t j = 0; j < i; j++)
            x_next[i] -= Ab[i*stride+j]*x[j];
          for(size_t j = i+1; j < sz; j++)
            x_next[i] -= Ab[i*stride+j]*x[j];
          x_next[i] /= Ab[i*stride+i];
        }
        if(vectorDifferenceL2Norm(sz,x,x_next) < eps)
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
        const T* const __RESTRICT Ab, T* __RESTRICT x, T* __RESTRICT x_next,
        Logger& log,
        const int max_iter_count = default_max_iter_count, const T eps = default_eps<T>())
    {
      const size_t stride = sz + 1;
      log.debug("note that iterative solvers from Gauss-Seidel family work only for diagonally dominant matrices");
      log.debug("checking that given matrix is diagonally dominant...");
      if(!isDiagonallyDominant(sz,stride,Ab))
      {
        log.error("the matrix of the system is NOT diagonally dominant, nothing to do here, exiting");
        return false;
      }
      for(int iter = 0; iter < max_iter_count; iter++)
      {
        for(size_t i = 0; i < sz; i++)
        {
          x_next[i] = Ab[i*stride + sz];
          for(size_t j = 0; j < i; j++)
            x_next[i] -= Ab[i*stride+j]*x_next[j];
          for(size_t j = i+1; j < sz; j++)
            x_next[i] -= Ab[i*stride+j]*x[j];
          x_next[i] /= Ab[i*stride+i];
        }
        if(vectorDifferenceL2Norm(sz,x,x_next) < eps)
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
        const T* const __RESTRICT Ab, T* const __RESTRICT x,
        T* __RESTRICT residual, T* __RESTRICT residual_next,
        Logger& log,
        const int max_iter_count = default_max_iter_count, const T eps = default_eps<T>() )
    {
      const size_t stride = sz + 1;
      log.debug("note that iterative solvers from Gauss-Seidel family work only for diagonally dominant matrices");
      log.debug("checking that given matrix is diagonally dominant...");
      if(!isDiagonallyDominant(sz,stride,Ab))
      {
        log.error("the matrix of the system is NOT diagonally dominant, nothing to do here, exiting");
        return false;
      }
      //force initial guess to be zero, and residual = b
      for(size_t i = 0; i < sz; i++)
      {
        x[i] = 0;
        residual[i] = Ab[i*stride+sz]/Ab[i*stride+i];
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
            residual_next[i] -= Ab[i*stride+j]*residual[j];
          residual_next[i] /= Ab[i*stride+i];
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
        T* const _buf = reinterpret_cast<T*>(p.Ab_buf);
        T* const _x = reinterpret_cast<T* const>(p.x);
        T* const _x_tmp = reinterpret_cast<T* const>(p.x_tmp);
        return numeric_cpp_jacobi_impl<T>(_sz,_buf,_x,_x_tmp,p.progress_ptr->log());
      }
    };

    //dumb c++ version of Seidel iterative solver
    struct numeric_cpp_seidel : numeric::MPFuncBase<numeric_cpp_seidel,AlgoParameters,bool>
    {
      template<typename T> inline bool perform(const AlgoParameters& p)
      {
        //warmup
        const size_t _sz = p.system_size;
        T* const _buf = reinterpret_cast<T*>(p.Ab_buf);
        T* const _x = reinterpret_cast<T* const>(p.x);
        T* const _x_tmp = reinterpret_cast<T* const>(p.x_tmp);
        return numeric_cpp_seidel_impl<T>(_sz,_buf,_x,_x_tmp,p.progress_ptr->log());
      }
    };

    //dumb c++ version of "relaxation" iterative solver
    struct numeric_cpp_relaxation : numeric::MPFuncBase<numeric_cpp_relaxation,AlgoParameters,bool>
    {
      template<typename T> inline bool perform(const AlgoParameters& p)
      {
        //warmup
        const size_t _sz = p.system_size;
        T* const _buf = reinterpret_cast<T*>(p.Ab_buf);
        T* const _x = reinterpret_cast<T* const>(p.x);
        T* const _x_tmp = reinterpret_cast<T* const>(p.x_tmp);
        return numeric_cpp_relaxation_impl<T>(_sz,_buf,_x,_x_tmp,p.progress_ptr->log());
      }
    };

    //dispatcher
    bool perform(const AlgoParameters& p, Logger& log)
    {
      numeric::ParallelScheduler __ps(p.Topt.type,p.Topt.num);
      ExecTimeMeter __etm(log, "dense_linear_solve::perform");
//      PERF_METER(log, "dense_linear_solve::perform");
      const size_t _sz = p.system_size;
      double* const _buf = p.Ab_buf.get();
      double* const _x = p.x;
      double* const _x_next = p.x_tmp;
      double* const _residual_next = p.residual_tmp;
      switch(p.Aopt.type)
      {
        case A_NumCppJacobi:
          return numeric_cpp_jacobi_impl<double>(_sz,_buf,_x,_x_next,log);
        case A_NumCppSeidel:
          return numeric_cpp_seidel_impl<double>(_sz,_buf,_x,_x_next,log);
        case A_NumCppRelaxation:
          return numeric_cpp_relaxation_impl<double>(_sz,_buf,_x,_x_next,_residual_next,log);
        case A_Undefined:
        default:
          throw Calc::ParameterError("Algorithm is not implemented");
      }
      return false;
    }
  }
}

