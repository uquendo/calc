#include "calcapp/options.hpp"

namespace Calc {

ThreadingOptName _threading_opt_names[] = {
    { "Serial", "s", T_Serial },
    { "C++11 standard library threads", "std", T_Std },
    { "POSIX threads", "posix", T_Posix },
    { "OpenMP", "omp", T_OpenMP },
    { "Intel TBB", "tbb", T_TBB },
    { NULL, 0, T_Undefined }
};  

PrecisionOptName _precision_opt_names[] = {
    { "32-bit float", "32", P_Float },
    { "64-bit double", "64", P_Double },
    { "128-bit quad", "128", P_Quad },
    { "fixed decimal digits MPFR", "mpfr", P_MPFR },
    { NULL, 0, P_Undefined }
};

bool AppOptions::processOptions(int argc, char* argv[]){ 
    return false;
}

}

