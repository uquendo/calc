#include "newton.hpp"

#include "calcapp/math/dense_linear_solver.hpp"
#include "numeric/newton_solver.hpp"

#include <cmath>
#include <algorithm>

namespace Calc
{
 namespace newton
 {

    //
    struct rosenbrock_gradient : numeric::MPFuncBase<rosenbrock_gradient,AlgoParameters,bool>
    {
      template<typename T> inline bool perform(const AlgoParameters& p)
      {
        //warmup
        constexpr size_t _sz = 2;
        const T zero(0.0);
        const T one(1.0);
        T _f_norm = zero;
        int _iter = 0;
        std::unique_ptr<T[]> _mat_buf(new T[_sz*_sz]);
        std::unique_ptr<T[]> _vec_buf(new T[_sz]);
        std::unique_ptr<T[]> _guess(new T[_sz]);
        std::unique_ptr<T[]> _root(new T[_sz]);
        for(size_t i = 0; i < _sz; i++)
        {
          for(size_t j = 0; j < _sz; j++)
          {
            _mat_buf.get()[i] = zero;
          }
         _root.get()[i] = _vec_buf.get()[i] = zero;
         _guess.get()[i] = one + one ;
        }

        struct RosenbrockGradientFunc {
          void operator()(size_t sz, const T* const __RESTRICT arg, T* const __RESTRICT val)
          {
            const T one(1.0);
            val[0] = arg[0] - one - 200*arg[0]*(arg[1] - arg[0]*arg[0]);
            val[sz-1] = 100*(arg[sz-1] - arg[sz-2]*arg[sz-2]);
            for(size_t val_n = 1; val_n < sz - 1; val_n++)
              val[val_n] = arg[val_n] - one - 200*arg[val_n]*(arg[val_n+1] - arg[val_n]*arg[val_n]) + 100*(arg[val_n] - arg[val_n-1]*arg[val_n-1]);
          }
        } _f;

        Calc::DenseSolverFunc<T> _solver(p.progress_ptr);
//        p.progress_ptr->log().debug("");
        ExecTimeMeter __etm(p.progress_ptr->log(), "newton::perform");
        numeric::find_root_newton(_sz, _f, _solver, _guess.get(), _root.get(), _mat_buf.get(), _vec_buf.get(),
            Calc::default_eps<T>(), Calc::default_max_iter_count, _f_norm, _iter);
        if(_iter < Calc::default_max_iter_count)
          p.progress_ptr->log().fdebug("found root at iter %zu with ||f(root)||_2 = %g" , _iter, numeric::toDouble(_f_norm));
        else
          p.progress_ptr->log().fdebug("maximum iter count %zu reached. ||f(guess)||_2 = %g" , _iter, numeric::toDouble(_f_norm));
        return true;
      }
    };

    //dispatcher
    bool perform(const AlgoParameters& p, Logger& log)
    {
      numeric::ParallelScheduler __ps(p.Topt.type,p.Topt.num);
//      PERF_METER(log, "dense_linear_solve::perform");
      return rosenbrock_gradient()(p.Popt.type,p);
    }

  }
}

