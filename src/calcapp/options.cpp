#include "calcapp/options.hpp"

using namespace numeric;

namespace Calc {

const OptName<TThreading> _threading_opt_names[] = {
    { "Serial", "s", T_Serial },
    { "C++11 standard library threads", "std", T_Std },
#ifdef HAVE_POSIX
    { "POSIX threads", "px", T_Posix },
#endif
#ifdef HAVE_OPENMP
    { "OpenMP", "omp", T_OpenMP },
#endif
#ifdef HAVE_TBB
    { "Intel TBB", "tbb", T_TBB },
#endif
    { NULL, 0, T_Undefined }
};  

const OptName<TPrecision> _precision_opt_names[] = {
    { "32-bit float", "32", P_Float },
    { "64-bit double", "64", P_Double },
    { "80-bit long double", "80", P_LongDouble },
#ifdef BUILD_QUAD
    { "128-bit quad", "128", P_Quad },
#endif
#ifdef HAVE_MPREAL
    { "fixed decimal digits MPFR", "mpfr", P_MPFR },
#endif
    { NULL, 0, P_Undefined }
};

//TODO: STUBS!
AppOptions::AppOptions(std::string AppName,std::string AppVersion):m_AppName(AppName),
  m_AppVersion(AppVersion),
  m_logging({false, Logger::L_NONE, false, false, ""}),
  m_threading({T_Undefined,0}),
  m_precision({P_Undefined,0,0})
{
}

}

