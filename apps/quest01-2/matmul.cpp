#include "matmul.hpp"
#include <valarray>
#include <numeric>

namespace Calc
{
  namespace matmul
  {

    //default c version for matrices in row major order
    struct numeric_c : numeric::MPFuncBase<numeric_c,AlgoParameters>
    {
      template<typename T> inline void perform(const AlgoParameters& p)
      {
        //TODO: select fastest algorithm for given problem size
      }
    };

    //simple c version for matrices in row major order
    struct numeric_c_simple : numeric::MPFuncBase<numeric_c_simple,AlgoParameters>
    {
      template<typename T> inline void perform(const AlgoParameters& p)
      {
        //TODO:
      }
    };

    //simple c version for matrices in row major order with second matrix transposed
    struct numeric_c_simple_transpose : numeric::MPFuncBase<numeric_c_simple_transpose,AlgoParameters>
    {
      template<typename T> inline void perform(const AlgoParameters& p)
      {
        //TODO:
      }
    };

    //diagonal(MRK) c version for square tridiagonal matrices in row major order
    struct numeric_c_mrk : numeric::MPFuncBase<numeric_c_mrk,AlgoParameters>
    {
      template<typename T> inline void perform(const AlgoParameters& p)
      {
        //TODO: N. Madsen, G. Rodrigue und J.Karush [1976]. Matrix multiplication by diagonals on a vector/parallel processor, Inf. Proc. Lett., vol.5
      }
    };

    //default c++ version for matrices in row major order
    struct numeric_cpp : numeric::MPFuncBase<numeric_cpp,AlgoParameters>
    {
      template<typename T> inline void perform(const AlgoParameters& p)
      {
        //TODO: select fastest algorithm for given problem size
      }
    };

    //simple c++ version for matrices in row major order
    struct numeric_cpp_simple : numeric::MPFuncBase<numeric_cpp_simple,AlgoParameters>
    {
      template<typename T> inline void perform(const AlgoParameters& p)
      {
        const CDSBandedMatrix<T>* const a = dynamic_cast<CDSBandedMatrix<T>*>(p.a.get());
        const CDSBandedMatrix<T>* const b = dynamic_cast<CDSBandedMatrix<T>*>(p.b.get());
        CDSBandedMatrix<T>* const c = dynamic_cast<CDSBandedMatrix<T>*>(p.c.get());
        if(a == nullptr || b == nullptr || c == nullptr)
          throw Calc::ParameterError("numeric_cpp_simple algo internal error");
        numeric::dgbmm<T>(numeric::TMatrixStorage::RowMajor, numeric::TMatrixTranspose::No, numeric::TMatrixTranspose::No,
          p.a->getDataPtr<T>(), p.b->getDataPtr<T>(), p.c->getDataPtr<T>(),
          p.a->getRowsNum(), p.a->getColumnsNum(), a->getUpperBand(), a->getLowerBand(),
          p.b->getRowsNum(), p.b->getColumnsNum(), b->getUpperBand(), b->getLowerBand(),
          p.Topt.type);
      }
    };

    //simple c++ version for square matrices in row major order with second matrix transposed
    struct numeric_cpp_simple_transpose : numeric::MPFuncBase<numeric_cpp_simple_transpose,AlgoParameters>
    {
      template<typename T> void perform(const AlgoParameters& p)
      {
        const CDSBandedMatrix<T>* const a = dynamic_cast<CDSBandedMatrix<T>*>(p.a.get());
        const CDSBandedMatrix<T>* const b = dynamic_cast<CDSBandedMatrix<T>*>(p.b.get());
        CDSBandedMatrix<T>* const c = dynamic_cast<CDSBandedMatrix<T>*>(p.c.get());
        if(a == nullptr || b == nullptr || c == nullptr)
          throw Calc::ParameterError("numeric_cpp_simple_transpose algo internal error");
        numeric::dgbmm<T>(numeric::TMatrixStorage::RowMajor, numeric::TMatrixTranspose::No, numeric::TMatrixTranspose::Transpose,
          p.a->getDataPtr<T>(), p.b->getDataPtr<T>(), p.c->getDataPtr<T>(),
          p.a->getRowsNum(), p.a->getColumnsNum(), a->getUpperBand(), a->getLowerBand(),
          p.b->getRowsNum(), p.b->getColumnsNum(), b->getUpperBand(), b->getLowerBand(),
          p.Topt.type);
      }
    };

    //diagonal(MRK) c++ version for square tridiagonal matrices in row major order
    struct numeric_cpp_mrk : numeric::MPFuncBase<numeric_cpp_mrk,AlgoParameters>
    {
      template<typename T> inline void perform(const AlgoParameters& p)
      {
        //TODO: N. Madsen, G. Rodrigue und J.Karush [1976]. Matrix multiplication by diagonals on a vector/parallel processor, Inf. Proc. Lett., vol.5
      }
    };

    //simple c++ version for square matrices in row major order using std::valarray
    struct numeric_cpp_valarray : numeric::MPFuncBase<numeric_cpp_valarray,AlgoParameters>
    {
      template<typename T> inline void perform(const AlgoParameters& p)
      {
        //warming up
        if(p.Topt.type != numeric::T_Serial)
          throw Calc::ParameterError("Algotithm is not implemented");
        const size_t rows = p.a->getRowsNum();
//        const size_t columns = p.b->getColumnsNum();
//        const size_t stride = p.a->getColumnsNum();
        const ValArrayCDSBandedMatrix<T>* const a = dynamic_cast<ValArrayCDSBandedMatrix<T>*>(p.a.get());
        const ValArrayCDSBandedMatrix<T>* const b = dynamic_cast<ValArrayCDSBandedMatrix<T>*>(p.b.get());
        ValArrayCDSBandedMatrix<T>* const c = dynamic_cast<ValArrayCDSBandedMatrix<T>*>(p.c.get());
        if(a == nullptr || b == nullptr || c == nullptr)
          throw Calc::ParameterError("Valarray algo internal error");
        const std::valarray<T>& A = a->getValArray();
        const std::valarray<T>& B = b->getValArray();
        std::valarray<T>& C = c->getValArray();
        if((c->getUpperBand() != (a->getUpperBand()+b->getUpperBand())) ||
           (c->getLowerBand() != (a->getLowerBand()+b->getLowerBand())))
          throw Calc::ParameterError("Matrix sizes mismatch");

        //actual run
        if( (a->getUpperBand() != b->getUpperBand()) ||
            (a->getUpperBand() != a->getLowerBand()) ||
            (b->getUpperBand() != b->getLowerBand()) ||
            (a->getUpperBand() != 1) )
        {
          //TODO: generic algo.
          //STUB!
          /*
          const size_t a_stride = a->getUpperBand()+a->getLowerBand()+1;
          const size_t b_stride = b->getUpperBand()+b->getLowerBand()+1;
          const size_t c_stride = c->getUpperBand()+c->getLowerBand()+1;
          const size_t b_upper = b->getUpperBand();
          const size_t b_lower = b->getLowerBand();
          //i<b_upper
          //
          for(size_t i = b_upper; i < std::min(rows,stride-a_stride+b_upper); i++)
            for(size_t j = 0; j < a_stride; j++)
            {
              //C[std::slice(i*c_stride+j,b_stride,1)] += A[i*a_stride+j] * B[std::slice(b_stride*(i+j-b_upper),b_stride,1)];
            }
          //i>stride-a_stride+b_upper
          */
        } else {
          //preunrolled tridiagonal algo
          //i=0
          {
            //j=0
            //C[1] += A[1] * B[0]; // == 0
            C[2] += A[1] * B[1];
            C[3] += A[1] * B[2];
            //j=1
            C[2] += A[2] * B[3];
            C[3] += A[2] * B[4];
            C[4] += A[2] * B[5];
          }
          for(size_t i = 1; i < rows - 1; i++)
            for(int j = 0; j < 3; j++)
              C[std::slice(5*i+j,3,1)] += A[3*i+j] * B[std::slice(3*(i+j-1),3,1)];
          //i=rows-1
          if(rows > 1)
          {
            //j=0
            C[5*(rows-1)+0] += A[3*(rows-1)] * B[3*(rows-2)+0];
            C[5*(rows-1)+1] += A[3*(rows-1)] * B[3*(rows-2)+1];
            C[5*(rows-1)+2] += A[3*(rows-1)] * B[3*(rows-2)+2];
            //j=1
            C[5*(rows-1)+1] += A[3*(rows-1)+1] * B[3*(rows-2)+3];
            C[5*(rows-1)+2] += A[3*(rows-1)+1] * B[3*(rows-2)+4];
            //C[5*(rows-1)+3] += A[3*(rows-1)+1] * B[3*(rows-1)+2] // == 0
          }
        }
      }
    };

    //simple c++ version for square matrices in row major order with second matrix transposed using std::valarray
    struct numeric_cpp_valarray_transpose : numeric::MPFuncBase<numeric_cpp_valarray_transpose,AlgoParameters>
    {
      template<typename T> inline void perform(const AlgoParameters& p)
      {
        //warming up
        if(p.Topt.type != numeric::T_Serial)
          throw Calc::ParameterError("Algotithm is not implemented");
//        const size_t rows = p.a->getRowsNum();
//        const size_t columns = p.b->getColumnsNum();
//        const size_t stride = p.a->getColumnsNum();
        const ValArrayCDSBandedMatrix<T>* const a = dynamic_cast<ValArrayCDSBandedMatrix<T>*>(p.a.get());
        const ValArrayCDSBandedMatrix<T>* const b = dynamic_cast<ValArrayCDSBandedMatrix<T>*>(p.b.get());
        ValArrayCDSBandedMatrix<T>* const c = dynamic_cast<ValArrayCDSBandedMatrix<T>*>(p.c.get());
        if(a == nullptr || b == nullptr || c == nullptr)
          throw Calc::ParameterError("Valarray algo internal error");
        const std::valarray<T>& A = a->getValArray();
        const std::valarray<T>& B = b->getValArray();
        std::valarray<T>& C = c->getValArray();

        //actual run
      }
    };

    //diagonal(MRK) c++ version for square tridiagonal matrices in row major order using std::valarray
    struct numeric_cpp_valarray_mrk : numeric::MPFuncBase<numeric_cpp_valarray_mrk,AlgoParameters>
    {
      template<typename T> inline void perform(const AlgoParameters& p)
      {
        //warming up
        if(p.Topt.type != numeric::T_Serial)
          throw Calc::ParameterError("Algotithm is not implemented");
//        const size_t rows = p.a->getRowsNum();
//        const size_t columns = p.b->getColumnsNum();
//        const size_t stride = p.a->getColumnsNum();
        const ValArrayCDSBandedMatrix<T>* const a = dynamic_cast<ValArrayCDSBandedMatrix<T>*>(p.a.get());
        const ValArrayCDSBandedMatrix<T>* const b = dynamic_cast<ValArrayCDSBandedMatrix<T>*>(p.b.get());
        ValArrayCDSBandedMatrix<T>* const c = dynamic_cast<ValArrayCDSBandedMatrix<T>*>(p.c.get());
        if(a == nullptr || b == nullptr || c == nullptr)
          throw Calc::ParameterError("Valarray algo internal error");
        const std::valarray<T>& A = a->getValArray();
        const std::valarray<T>& B = b->getValArray();
        std::valarray<T>& C = c->getValArray();
        if((c->getUpperBand() != (a->getUpperBand()+b->getUpperBand())) ||
           (c->getLowerBand() != (a->getLowerBand()+b->getLowerBand())))
          throw Calc::ParameterError("Matrix sizes mismatch");

        //actual run
        //TODO: N. Madsen, G. Rodrigue und J.Karush [1976]. Matrix multiplication by diagonals on a vector/parallel processor, Inf. Proc. Lett., vol.5
      }
    };

    //default fortran version for matrices in column major order
    struct numeric_fortran : numeric::MPFuncBase<numeric_fortran,AlgoParameters>
    {
      template<typename T> inline void perform(const AlgoParameters& p)
      {
        //TODO: select fastest algorithm for given problem size
      }
    };

    //simple fortran version for matrices in column major order
    struct numeric_fortran_simple : numeric::MPFuncBase<numeric_fortran_simple,AlgoParameters>
    {
      template<typename T> inline void perform(const AlgoParameters& p)
      {
        //TODO:
      }
    };

    //diagonal(MRK) fortran version for square tridiagonal matrices in column major order
    struct numeric_fortran_mrk : numeric::MPFuncBase<numeric_fortran_mrk,AlgoParameters>
    {
      template<typename T> inline void perform(const AlgoParameters& p)
      {
        //TODO: N. Madsen, G. Rodrigue und J.Karush [1976]. Matrix multiplication by diagonals on a vector/parallel processor, Inf. Proc. Lett., vol.5
      }
    };

    //simple fortran version for matrices in column major order with second matrix transposed
    struct numeric_fortran_simple_transpose : numeric::MPFuncBase<numeric_fortran_simple_transpose,AlgoParameters>
    {
      template<typename T> inline void perform(const AlgoParameters& p)
      {
        //TODO:
      }
    };

    //dispatcher
    void perform(const AlgoParameters& parameters, Logger& log)
    {
      if(parameters.Aopt.type == A_NumCppValarray || parameters.Aopt.type == A_NumCppValarrayTranspose)
      {

      }
      numeric::ParallelScheduler __ps(parameters.Topt.type,parameters.Topt.num);
      ExecTimeMeter __etm(log, "matmul::perform");
//      PERF_METER(log, "matmul::perform");
      switch(parameters.Aopt.type)
      {
        case A_NumC:
//          return numeric_c()(parameters.Popt.type, parameters);
        case A_NumCSimple:
//          return numeric_c_simple()(parameters.Popt.type, parameters);
        case A_NumCSimpleTranspose:
//          return numeric_c_simple_transpose()(parameters.Popt.type, parameters);
          throw Calc::ParameterError("Algorithm is not implemented");

        case A_NumCpp:
//          return numeric_cpp()(parameters.Popt.type, parameters);
        case A_NumCppSimple:
          return numeric_cpp_simple()(parameters.Popt.type, parameters);
        case A_NumCppValarray:
          return numeric_cpp_valarray()(parameters.Popt.type, parameters);
        case A_NumCppMRK:
//          return numeric_cpp_mrk()(parameters.Popt.type, parameters);
        case A_NumCppValarrayMRK:
//          return numeric_cpp_valarray_mrk()(parameters.Popt.type, parameters);
        case A_NumCppSimpleTranspose:
//          return numeric_cpp_simple_transpose()(parameters.Popt.type, parameters);
        case A_NumCppValarrayTranspose:
//          return numeric_cpp_valarray_transpose()(parameters.Popt.type, parameters);
          throw Calc::ParameterError("Algorithm is not implemented");

        case A_NumFortran:
//          return numeric_fortran()(parameters.Popt.type, parameters);
        case A_NumFortranSimple:
//          return numeric_fortran_simple()(parameters.Popt.type, parameters);
        case A_NumFortranSimpleTranspose:
//          return numeric_fortran_simple_transpose()(parameters.Popt.type, parameters);
          throw Calc::ParameterError("Algorithm is not implemented");

        case A_Undefined:
        default:
          throw Calc::ParameterError("Algorithm is not implemented");
      }
    }
  }
}

