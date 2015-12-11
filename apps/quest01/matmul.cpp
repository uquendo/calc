#include "matmul.hpp"
#include <valarray>
#include <numeric>

#ifdef HAVE_BOOST_UBLAS
# include <boost/numeric/ublas/matrix.hpp>
#endif
#ifdef HAVE_EIGEN
# include <Eigen/Dense>
#endif
#ifdef HAVE_MTL
# include <boost/numeric/mtl/mtl.hpp>
# include <boost/numeric/mtl/matrix/dense2D.hpp>
#endif
#ifdef HAVE_ARMADILLO
# include <armadillo>
#endif
#ifdef HAVE_BLAS
#endif

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

    //strassen c version for matrices in row major order
    struct numeric_c_strassen : numeric::MPFuncBase<numeric_c_strassen,AlgoParameters>
    {
      template<typename T> inline void perform(const AlgoParameters& p)
      {
        //TODO: strassen multiplication should be better for large matrices
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
        numeric::dgemm<T>(numeric::TMatrixStorage::RowMajor, numeric::TMatrixTranspose::No, numeric::TMatrixTranspose::No,
            p.a->getDataPtr<T>(), p.b->getDataPtr<T>(), p.c->getDataPtr<T>(), p.a->getRowsNum(), p.a->getColumnsNum(), p.b->getRowsNum(), p.b->getColumnsNum(), p.Topt.type);
      }
    };

    //simple c++ version for square matrices in row major order with second matrix transposed
    struct numeric_cpp_simple_transpose : numeric::MPFuncBase<numeric_cpp_simple_transpose,AlgoParameters>
    {
      template<typename T> void perform(const AlgoParameters& p)
      {
        numeric::dgemm<T>(numeric::TMatrixStorage::RowMajor, numeric::TMatrixTranspose::No, numeric::TMatrixTranspose::Transpose,
          p.a->getDataPtr<T>(), p.b->getDataPtr<T>(), p.c->getDataPtr<T>(), p.a->getRowsNum(), p.a->getColumnsNum(), p.b->getRowsNum(), p.b->getColumnsNum(), p.Topt.type);
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
        const size_t columns = p.b->getColumnsNum();
        const size_t stride = p.a->getColumnsNum();
        const ValArrayMatrix<T>* const a = dynamic_cast<ValArrayMatrix<T>*>(p.a.get());
        const ValArrayMatrix<T>* const b = dynamic_cast<ValArrayMatrix<T>*>(p.b.get());
        ValArrayMatrix<T>* const c = dynamic_cast<ValArrayMatrix<T>*>(p.c.get());
        if(a == nullptr || b == nullptr || c == nullptr)
          throw Calc::ParameterError("Valarray algo internal error");
        const std::valarray<T>& A = a->getValArray();
        const std::valarray<T>& B = b->getValArray();
        std::valarray<T>& C = c->getValArray();

        //actual run
        for(size_t i = 0; i < rows; i++)
          for(size_t j = 0; j < stride; j++)
          {
            C[std::slice(i*columns,columns,1)] += A[i*stride+j] * B[std::slice(j*columns,columns,1)];
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
        const size_t rows = p.a->getRowsNum();
        const size_t columns = p.b->getColumnsNum();
        const size_t stride = p.a->getColumnsNum();
        const ValArrayMatrix<T>* const a = dynamic_cast<ValArrayMatrix<T>*>(p.a.get());
        const ValArrayMatrix<T>* const b = dynamic_cast<ValArrayMatrix<T>*>(p.b.get());
        ValArrayMatrix<T>* const c = dynamic_cast<ValArrayMatrix<T>*>(p.c.get());
        if(a == nullptr || b == nullptr || c == nullptr)
          throw Calc::ParameterError("Valarray algo internal error");
        const std::valarray<T>& A = a->getValArray();
        const std::valarray<T>& B = b->getValArray();
        std::valarray<T>& C = c->getValArray();

        //actual run
        for(size_t i = 0; i < rows; i++)
          for(size_t k = 0; k < columns; k++)
          {
            C[i*columns+k]=std::inner_product(&A[i*stride],&A[(i+1)*stride],&B[k*stride],T(0.0));
            //slice_array helper lack reference semantics which makes slices almost useless here
            //see f.e. "Fixing valarray for real world use" ( http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2000/n1246.ps )
            //std::valarray<T> tmp = A[std::slice(i*stride, stride, 1)];
            //tmp *= B[std::slice(i*stride, stride, 1)];
            //C[i*columns+k] = std::valarray<T>( tmp ).sum();
          }
      }
    };

    //block c++ version for matrices in row major order
    struct numeric_cpp_block : numeric::MPFuncBase<numeric_cpp_block,AlgoParameters>
    {
      template<typename T> inline void perform(const AlgoParameters& p)
      {
        numeric::dgemm_block<T>(numeric::TMatrixStorage::RowMajor, p.a->getDataPtr<T>(), p.b->getDataPtr<T>(), p.c->getDataPtr<T>(),
            p.a->getRowsNum(), p.a->getColumnsNum(), p.b->getRowsNum(), p.b->getColumnsNum(), p.Topt.type);
      }
    };


    //strassen c++ version for matrices in row major order
    struct numeric_cpp_strassen : numeric::MPFuncBase<numeric_cpp_strassen,AlgoParameters>
    {
      template<typename T> inline void perform(const AlgoParameters& p)
      {
        //TODO: strassen multiplication should be better for large matrices
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

    //simple fortran version for matrices in column major order with second matrix transposed
    struct numeric_fortran_simple_transpose : numeric::MPFuncBase<numeric_fortran_simple_transpose,AlgoParameters>
    {
      template<typename T> inline void perform(const AlgoParameters& p)
      {
        //TODO:
      }
    };

    //strassen fortran version for matrices in column major order
    struct numeric_fortran_strassen : numeric::MPFuncBase<numeric_fortran_strassen,AlgoParameters>
    {
      template<typename T> inline void perform(const AlgoParameters& p)
      {
        //TODO: strassen multiplication should be better for large matrices
      }
    };

    //internal fortran version(using matmul) for matrices in column major order
    struct numeric_fortran_internal : numeric::MPFuncBase<numeric_fortran_internal,AlgoParameters>
    {
      template<typename T> inline void perform(const AlgoParameters& p)
      {
        //TODO:
      }
    };

#ifdef HAVE_BOOST_UBLAS
    //boost.uBlas c++ version for matrices in row major order
    struct contrib_cpp_boost_ublas : numeric::MPFuncBase<contrib_cpp_boost_ublas,AlgoParameters>
    {
      template<typename T> inline void perform(const AlgoParameters& p)
      {
        using namespace boost::numeric::ublas;
        //warming up
        if(p.Topt.type != numeric::T_Serial)
          throw Calc::ParameterError("Algotithm is not implemented");
        const BoostUblasMatrix<T>* const a = dynamic_cast<BoostUblasMatrix<T>*>(p.a.get());
        const BoostUblasMatrix<T>* const b = dynamic_cast<BoostUblasMatrix<T>*>(p.b.get());
        BoostUblasMatrix<T>* const c = dynamic_cast<BoostUblasMatrix<T>*>(p.c.get());
        if(a == nullptr || b == nullptr || c == nullptr)
          throw Calc::ParameterError("contrib_cpp_boost_ublas algo internal error");
        const matrix<T>& A = a->getBoostMatrix();
        const matrix<T>& B = b->getBoostMatrix();
        matrix<T>& C = c->getBoostMatrix();

        //actual run:
        noalias(C) = prod(A, B);
      }
    };
#endif
#ifdef HAVE_EIGEN
    //Eigen c++ version for matrices in column major order
    struct contrib_cpp_eigen : numeric::MPFuncBase<contrib_cpp_eigen,AlgoParameters>
    {
      template<typename T> inline void perform(const AlgoParameters& p)
      {
        //warming up
        if(p.Topt.type != numeric::T_Serial)
          throw Calc::ParameterError("Algotithm is not implemented");
        const EigenMatrix<T>* const a = dynamic_cast<EigenMatrix<T>*>(p.a.get());
        const EigenMatrix<T>* const b = dynamic_cast<EigenMatrix<T>*>(p.b.get());
        EigenMatrix<T>* const c = dynamic_cast<EigenMatrix<T>*>(p.c.get());
        if(a == nullptr || b == nullptr || c == nullptr)
          throw Calc::ParameterError("contrib_cpp_eigen algo internal error");
        const Eigen::Matrix<T,Eigen::Dynamic,Eigen::Dynamic>& A = a->getEigenMatrix();
        const Eigen::Matrix<T,Eigen::Dynamic,Eigen::Dynamic>& B = b->getEigenMatrix();
        Eigen::Matrix<T,Eigen::Dynamic,Eigen::Dynamic>& C = c->getEigenMatrix();

        //actual run:
        C = A * B;
      }
    };
#endif
#ifdef HAVE_MTL
    //MTL c++ version for matrices in row major order
    struct contrib_cpp_mtl : numeric::MPFuncBase<contrib_cpp_mtl,AlgoParameters>
    {
      template<typename T> inline void perform(const AlgoParameters& p)
      {
        using namespace mtl;
        //warming up
        if(p.Topt.type != numeric::T_Serial)
          throw Calc::ParameterError("Algotithm is not implemented");
        const MTLMatrix<T>* const a = dynamic_cast<MTLMatrix<T>*>(p.a.get());
        const MTLMatrix<T>* const b = dynamic_cast<MTLMatrix<T>*>(p.b.get());
        MTLMatrix<T>* const c = dynamic_cast<MTLMatrix<T>*>(p.c.get());
        if(a == nullptr || b == nullptr || c == nullptr)
          throw Calc::ParameterError("contrib_cpp_mtl algo internal error");
        const mtl::mat::dense2D<T>& A = a->getMTLMatrix();
        const mtl::mat::dense2D<T>& B = b->getMTLMatrix();
        mtl::mat::dense2D<T>& C = c->getMTLMatrix();

        //actual run:
        C = A * B;
      }
    };
#endif

#ifdef HAVE_ARMADILLO
    template<typename T> struct ArmaTraits
    {
      typedef T type;
    };
    template<> struct ArmaTraits<long double>
    {
      typedef double type;
    };
# ifdef HAVE_QUADMATH
    template<> struct ArmaTraits<numeric::quad>
    {
      typedef double type;
    };
# endif
# ifdef HAVE_MPREAL
    template<> struct ArmaTraits<numeric::mpreal>
    {
      typedef double type;
    };
# endif

    //armadillo c++ version for matrices in column major order
    struct contrib_cpp_armadillo : numeric::MPFuncBase<contrib_cpp_armadillo,AlgoParameters>
    {
      template<typename T> inline void perform(const AlgoParameters& p)
      {
        //warming up
        if(p.Topt.type != numeric::T_Serial)
          throw Calc::ParameterError("Algotithm is not implemented");
        const ArmadilloMatrix<typename ArmaTraits<T>::type>* const a = dynamic_cast<ArmadilloMatrix<typename ArmaTraits<T>::type>*>(p.a.get());
        const ArmadilloMatrix<typename ArmaTraits<T>::type>* const b = dynamic_cast<ArmadilloMatrix<typename ArmaTraits<T>::type>*>(p.b.get());
        ArmadilloMatrix<typename ArmaTraits<T>::type>* const c = dynamic_cast<ArmadilloMatrix<typename ArmaTraits<T>::type>*>(p.c.get());
        if(a == nullptr || b == nullptr || c == nullptr)
          throw Calc::ParameterError("contrib_cpp_armadillo algo internal error");
        const arma::Mat<typename ArmaTraits<T>::type>& A = a->getArmadilloMatrix();
        const arma::Mat<typename ArmaTraits<T>::type>& B = b->getArmadilloMatrix();
        arma::Mat<typename ArmaTraits<T>::type>& C = c->getArmadilloMatrix();

        //actual run:
        C = A * B;
      }
    };
#endif
#ifdef HAVE_BLAS
    //cblas version for matrices in row major order
    struct contrib_c_blas : numeric::MPFuncBase<contrib_c_blas,AlgoParameters>
    {
      template<typename T> inline void perform(const AlgoParameters& p)
      {
        //TODO:
      }
    };

    //fortran blas version for matrices in column major order
    struct contrib_fortran_blas : numeric::MPFuncBase<contrib_fortran_blas,AlgoParameters>
    {
      template<typename T> inline void perform(const AlgoParameters& p)
      {
        //TODO:
      }
    };
#endif

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
        case A_NumCStrassen:
//          return numeric_c_strassen()(parameters.Popt.type, parameters);
          throw Calc::ParameterError("Algorithm is not implemented");

        case A_NumCpp:
//          return numeric_cpp()(parameters.Popt.type, parameters);
        case A_NumCppSimple:
          return numeric_cpp_simple()(parameters.Popt.type, parameters);
        case A_NumCppSimpleTranspose:
          return numeric_cpp_simple_transpose()(parameters.Popt.type, parameters);
        case A_NumCppValarray:
          return numeric_cpp_valarray()(parameters.Popt.type, parameters);
        case A_NumCppValarrayTranspose:
          return numeric_cpp_valarray_transpose()(parameters.Popt.type, parameters);
        case A_NumCppBlock:
          return numeric_cpp_block()(parameters.Popt.type, parameters);
        case A_NumCppStrassen:
//          return numeric_cpp_strassen()(parameters.Popt.type, parameters);
          throw Calc::ParameterError("Algorithm is not implemented");

        case A_NumFortran:
//          return numeric_fortran()(parameters.Popt.type, parameters);
        case A_NumFortranSimple:
//          return numeric_fortran_simple()(parameters.Popt.type, parameters);
        case A_NumFortranSimpleTranspose:
//          return numeric_fortran_simple_transpose()(parameters.Popt.type, parameters);
        case A_NumFortranStrassen:
//          return numeric_fortran_strassen()(parameters.Popt.type, parameters);
        case A_NumFortranInternal:
//          return numeric_fortran_internal()(parameters.Popt.type, parameters);
          throw Calc::ParameterError("Algorithm is not implemented");

#ifdef HAVE_BOOST_UBLAS
        case A_ExtCppBoost:
          return contrib_cpp_boost_ublas()(parameters.Popt.type, parameters);
#endif
#ifdef HAVE_EIGEN
        case A_ExtCppEigen:
          return contrib_cpp_eigen()(parameters.Popt.type, parameters);
#endif
#ifdef HAVE_MTL
        case A_ExtCppMTL:
          return contrib_cpp_mtl()(parameters.Popt.type, parameters);
#endif
#ifdef HAVE_ARMADILLO
        case A_ExtCppArmadillo:
          return contrib_cpp_armadillo()(parameters.Popt.type, parameters);
#endif
#ifdef HAVE_BLAS
        case A_ExtCBLAS:
//          return contrib_c_blas()(parameters.Popt.type, parameters);
        case A_ExtFortranBLAS:
//          return contrib_fortran_blas()(parameters.Popt.type, parameters);
#endif
        case A_Undefined:
        default:
          throw Calc::ParameterError("Algorithm is not implemented");
      }
    }
  }
}

