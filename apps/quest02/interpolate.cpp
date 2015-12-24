#include "interpolate.hpp"
#ifdef HAVE_GSL
# include <gsl/gsl_errno.h>
# include <gsl/gsl_spline.h>
#endif

namespace Calc
{
  namespace interpolate
  {

    //c++ version for lagrange polynomial interpolation on uniform or Chebyshev grid(first kind)
    struct numeric_cpp : numeric::MPFuncBase<numeric_cpp,AlgoParameters>
    {
      template<typename T> inline void perform(const AlgoParameters& p)
      {
        p.progress_ptr->log().debug("Computing interpolation polynomial...");
        p.f->computePoints(p.Topt.type);
        p.f->computeWeights(p.Topt.type);
        p.progress_ptr->log().debug("Computing interpolated values on uniform grid...");
        p.f->interpolateOnUniformGrid(p.output_fineness_factor * (p.f->getPointsCount() - 1) + 1, p.Topt.type);
      }
    };

#ifdef HAVE_GSL
    //GSL version for lagrange polynomial interpolation on Chebyshev grid(first kind)
    struct contrib_gsl_che : numeric::MPFuncBase<contrib_gsl_che,AlgoParameters>
    {
      template<typename T> inline void perform(const AlgoParameters& p)
      {
        //TODO
      }
    };

    //GSL version for lagrange polynomial interpolation on uniform grid
    struct contrib_gsl_uni : numeric::MPFuncBase<contrib_gsl_uni,AlgoParameters>
    {
      template<typename T> inline void perform(const AlgoParameters& p)
      {
        //TODO
      }
    };
#endif

    //dispatcher
    void perform(const AlgoParameters& parameters, Logger& log)
    {
      numeric::ParallelScheduler __ps(parameters.Topt.type,parameters.Topt.num);
      ExecTimeMeter __etm(log, "interpolate::perform");
//      PERF_METER(log, "interpolate::perform");
      switch(parameters.Aopt.type)
      {
        case A_NumCppChebyshev:
        case A_NumCppUniform:
          return numeric_cpp()(parameters.Popt.type, parameters);
#ifdef HAVE_GSL
        case A_ExtGSLChebyshev:
//          return contrib_gsl_che()(parameters.Popt.type, parameters);
        case A_ExtGSLUniform:
//          return contrib_gsl_uni()(parameters.Popt.type, parameters);
#endif
        case A_Undefined:
        default:
          throw Calc::ParameterError("Algorithm is not implemented");
      }
    }
  }
}

