#include "approximate.hpp"
#ifdef HAVE_GSL
# include <gsl/gsl_errno.h>
# include <gsl/gsl_bspline.h>
#endif

namespace Calc
{
  namespace approximate
  {

    //c++ version for cubic spline approximateting
    struct numeric_cpp : numeric::MPFuncBase<numeric_cpp,AlgoParameters>
    {
      template<typename T> inline void perform(const AlgoParameters& p)
      {
        p.progress_ptr->log().debug("Computing approximating spline...");
        p.progress_ptr->log().debug("Assembling linear system...");
        p.f->assemble_equations(p.Topt.type);
        p.progress_ptr->log().debug("Solving linear system...");
        p.f->solve_equations(p.Topt.type);
//        p.f->dumpApproximant();
        p.progress_ptr->log().debug("Evaluating approximant values on uniform grid...");
        p.f->evaluateOnUniformGrid(p.output_fineness_factor * (p.f->getPointsCount() - 1) + 1, p.Topt.type);
      }
    };

#ifdef HAVE_GSL
    //GSL version for spline approximateting
    struct contrib_gsl : numeric::MPFuncBase<contrib_gsl,AlgoParameters>
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
      ExecTimeMeter __etm(log, "approximate::perform");
//      PERF_METER(log, "approximate::perform");
      switch(parameters.Aopt.type)
      {
        case A_NumCpp:
          return numeric_cpp()(parameters.Popt.type, parameters);
#ifdef HAVE_GSL
        case A_ExtGSL:
//          return contrib_gsl()(parameters.Popt.type, parameters);
#endif
        case A_Undefined:
        default:
          throw Calc::ParameterError("Algorithm is not implemented");
      }
    }
  }
}

