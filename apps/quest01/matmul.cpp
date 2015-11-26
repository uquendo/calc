#include "matmul.hpp"

namespace Calc
{
  namespace matmul
  {

    //dispatcher
    void perform(AlgoParameters& parameters)
    {
      switch(parameters.opt.type)
      {
        case A_Undefined:
          return;
        case A_NumCppSimple:
          return numeric_cpp_simple(parameters);
        case A_NumCppSimpleTranspose:
          return numeric_cpp_simple_transpose(parameters);
        default:
          throw Calc::ParameterError("Algorithm is not implemented");
      }
    }


    //default c++ version for square matrices in row major order
    template<typename T> void numeric_cpp_simple_helper(AlgoParameters& p)
    {
      return numeric::dgemm<T>(numeric::TMatrixStorage::RowMajor, numeric::TMatrixTranspose::No, numeric::TMatrixTranspose::No,
          static_cast<const T * const >(p.a), static_cast<const T * const >(p.b), static_cast<T * const >(p.c), p.sz, p.tm);
    }

    //default c++ version for square matrices in row major order
    template<typename T> void numeric_cpp_simple_transpose_helper(AlgoParameters& p)
    {
      return numeric::dgemm<T>(numeric::TMatrixStorage::RowMajor, numeric::TMatrixTranspose::No, numeric::TMatrixTranspose::Transpose,
          static_cast<const T * const >(p.a), static_cast<const T * const >(p.b), static_cast<T * const >(p.c), p.sz, p.tm);
    }

    void numeric_cpp_simple(AlgoParameters& p)
    {
      EXPAND_TRAITS_PRECISION(p.P, numeric_cpp_simple_helper , p);
    }

    //default c++ version for square matrices in row major order with second matrix transposed
    void numeric_cpp_simple_transpose(AlgoParameters& p)
    {
      EXPAND_TRAITS_PRECISION(p.P, numeric_cpp_simple_transpose_helper, p);
    }

  }
}

