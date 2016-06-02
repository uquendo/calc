#include "dense_linear_solve.hpp"

#include <cmath>
#include <algorithm>

namespace Calc
{
  namespace dense_linear_solve
 {

    template<typename T> void numeric_cpp_gauss_impl(const size_t sz,
        T* const __RESTRICT A, T* const __RESTRICT b, T* const __RESTRICT x,
        Logger& log)
    {
      const T small_value = T(1.e-5); //TODO: type-independent value
      const size_t stride = sz;
      log.debug("Note that gaussian elimination without pivoting works well only on diagonally-dominant matrices");
      log.debug("And if your matrix has any zeroes on main diagonal, you're definitely absolutely totally entirely doomed.");
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

    template<typename T> void numeric_cpp_jordan_impl(const size_t sz,
        T* const __RESTRICT A, T* const __RESTRICT b, T* const __RESTRICT x,
        Logger& log)
    {
      const T small_value = T(1.e-5); //TODO: type-independent value
      const size_t stride = sz;
      log.debug("Note that gaussian elimination without pivoting works well only on diagonally-dominant matrices");
      log.debug("And if your matrix has any zeroes on main diagonal, you're definitely absolutely totally entirely doomed");
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

    template<typename T> void numeric_cpp_gauss_full_pivoting_impl(const size_t sz,
        T* const __RESTRICT A, T* const __RESTRICT b,
        T* __RESTRICT *  __RESTRICT A_rows, T* const __RESTRICT x,
        size_t * const __RESTRICT index,
        Logger& log)
    {
      const T small_value = T(1.e-5); //TODO: type-independent value
      const size_t stride = sz;
      index[0] = sz - 1;
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
        return numeric_cpp_gauss_impl<T>(_sz,_A_buf,_b_buf,_x,p.progress_ptr->log());
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
        return numeric_cpp_jordan_impl<T>(_sz,_A_buf,_b_buf,_x,p.progress_ptr->log());
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
        return numeric_cpp_gauss_full_pivoting_impl<T>(_sz,_A_buf,_b_buf,_A_rows,_x,_index,p.progress_ptr->log());
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
          return numeric_cpp_gauss_impl<double>(_sz,_A_buf,_b_buf,_x,log);
        case A_NumCppJordan:
//          return numeric_cpp_jordan()(p.Popt.type, p);
          return numeric_cpp_jordan_impl<double>(_sz,_A_buf,_b_buf,_x,log);
        case A_NumCppFullPivoting:
//          return numeric_cpp_gauss_full_pivoting()(p.Popt.type, p);
          return numeric_cpp_gauss_full_pivoting_impl<double>(_sz,_A_buf,_b_buf,_A_rows,_x,
              reinterpret_cast<size_t * const>(new size_t[_sz]),log);
        case A_Undefined:
        default:
          throw Calc::ParameterError("Algorithm is not implemented");
      }
    }
  }
}

