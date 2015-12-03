#include "matmul.hpp"

namespace Calc
{
  namespace matmul
  {
    //default c++ version for matrices in row major order
    struct numeric_cpp_simple : numeric::MPFuncBase<numeric_cpp_simple,AlgoParameters>
    {
      template<typename T> inline void perform(const AlgoParameters& p)
      {
        numeric::dgemm<T>(numeric::TMatrixStorage::RowMajor, numeric::TMatrixTranspose::No, numeric::TMatrixTranspose::No,
            p.a->getDataPtr<T>(), p.b->getDataPtr<T>(), p.c->getDataPtr<T>(), p.a->m_nrows, p.a->m_ncolumns, p.b->m_nrows, p.b->m_ncolumns, p.Topt.type);
      }
    };

    //default c++ version for square matrices in row major order with second matrix transposed
    struct numeric_cpp_simple_transpose : numeric::MPFuncBase<numeric_cpp_simple_transpose,AlgoParameters>
    {
      template<typename T> void perform(const AlgoParameters& p)
      {
        numeric::dgemm<T>(numeric::TMatrixStorage::RowMajor, numeric::TMatrixTranspose::No, numeric::TMatrixTranspose::Transpose,
          p.a->getDataPtr<T>(), p.b->getDataPtr<T>(), p.c->getDataPtr<T>(), p.a->m_nrows, p.a->m_ncolumns, p.b->m_nrows, p.b->m_ncolumns, p.Topt.type);
      }
    };

    //dispatcher
    void perform(const AlgoParameters& parameters, Logger& log)
    {
      PERF_METER(log, "matmul::perform");
//      Calc::ExecTimeMeter __etm(log, "matmul::perform");
      switch(parameters.Aopt.type)
      {
        case A_NumC:
        case A_NumCSimple:
        case A_NumCSimpleTranspose:
        case A_NumCStrassen:
          throw Calc::ParameterError("Algorithm is not implemented");

        case A_NumCpp:
        case A_NumCppSimple:
          return numeric_cpp_simple()(parameters.Popt.type, parameters);
        case A_NumCppSimpleTranspose:
          return numeric_cpp_simple_transpose()(parameters.Popt.type, parameters);
        case A_NumCppStrassen:
          throw Calc::ParameterError("Algorithm is not implemented");


        case A_NumFortran:
        case A_NumFortranSimple:
        case A_NumFortranSimpleTranspose:
        case A_NumFortranStrassen:
        case A_NumFortranInternal:
          throw Calc::ParameterError("Algorithm is not implemented");

        case A_ExtCppBoost:
        case A_ExtCppEigen:
        case A_ExtCppMTL:
        case A_ExtCppArmadillo:

        case A_Undefined:
        default:
          throw Calc::ParameterError("Algorithm is not implemented");
      }
    }

  }
}

