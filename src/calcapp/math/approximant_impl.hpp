#include "calcapp/math/approximant.hpp"

#include "numeric/expand_traits.hpp"
#include "numeric/blas.hpp"
#include "numeric/lapack.hpp"

#include <cmath>
#include <cstring>
#include <iostream>

#ifdef HAVE_CILK
#include <cilk/cilk.h>
#endif

#ifdef HAVE_TBB
#include "numeric/parallel_tbb.hpp"
#endif

namespace Calc
{

template<typename T> void ApproximantFixedGrid1d<T>::init(const bool reset /* = true */, const bool initObjects /* = true */ )
{
//  static_assert(std::is_arithmetic<T>::value, "Arithmetical type expected"); //TODO: check boost::multiprecision type traits
  preallocateInputDataTable();
  preallocateParametersTable();
  preallocateApproximatedDataTable(m_cached_points_count);

  if(reset)
  {
    memset(reinterpret_cast<void*>(m_weights),0,sizeof(T)*m_points_count);
    memset(reinterpret_cast<void*>(m_points),0,sizeof(T)*m_points_count);
    memset(reinterpret_cast<void*>(m_values),0,sizeof(T)*m_points_count);
    memset(reinterpret_cast<void*>(m_cached_points),0,sizeof(T)*m_cached_points_count);
    memset(reinterpret_cast<void*>(m_cached_values),0,sizeof(T)*m_cached_points_count);
    //TODO: reset approximant parameters table
  }

  //placement new to call constructor for types that require it
  constexpr bool holdsObjects = std::is_class<T>::value;
  if(holdsObjects && initObjects)
  {
    for (size_t i = 0; i < m_points_count; ++i)
    {
      new(m_weights+i) T();
      new(m_points+i) T();
      new(m_values+i) T();
    }
    for (size_t i = 0; i < m_cached_points_count; ++i)
    {
      new(m_cached_points+i) T();
      new(m_cached_values+i) T();
    }
    //TODO: init approximant parameters table
  }
}

template<typename T> void ApproximantFixedGrid1d<T>::ParseHeaderDat(InFileText& f, size_t& points_count)
{
  f.readNextLine_scan(1,"# %zu",&points_count);
  ++points_count;
}

template<typename T> void ApproximantFixedGrid1d<T>::init(InFileText& f, const bool readData /* = true */)
{
//  static_assert(std::is_arithmetic<T>::value, "Arithmetical type expected"); //TODO: check boost::multiprecision type traits
  if(f.fileType() != FT_ApproximationTableText)
    throw FileFormatUnsupportedError("File format unsupported",f.fileType(),f.fileName().c_str(),f.lineNum());

  ParseHeaderDat(f, m_points_count);

  preallocateInputDataTable();

  if(readData)
  {
    //no placement new call is required for approximant value array, data should be initialized when read in(or computed)
    for (size_t i = 0; i < m_points_count; ++i)
    {
      f.readNextLine_scanNums(3, m_points+i, m_values+i, m_weights+i);
    }
    m_lower_border = m_points[0];
    m_upper_border = m_points[m_points_count - 1];
  }
}

template<typename T> ApproximantFixedGrid1d<T>::~ApproximantFixedGrid1d()
{
  //explicitly call destructor for types that require it
  if(std::is_class<T>::value)
  {
    for (size_t i = 0; i < m_points_count; ++i) {
      m_weights[i].~T();
      m_points[i].~T();
      m_values[i].~T();
    }
    for (size_t i = 0; i < m_cached_points_count; ++i) {
      m_cached_points[i].~T();
      m_cached_values[i].~T();
    }
    //TODO: destroy objects in approximant parameters table
  }
  //no need to free allocated memory, m_*_buf will take care of it
}

//data IO
template<typename T> void ApproximantFixedGrid1d<T>::readFromFile(InFileText& f)
{
//  static_assert(std::is_arithmetic<T>::value, "Arithmetical type expected"); //TODO: check boost::multiprecision type traits
  if(f.fileType() != FT_ApproximationTableText)
    throw FileFormatUnsupportedError("File format unsupported",f.fileType(),f.fileName().c_str(),f.lineNum());

  if(f.lineNum()==0)
  {
    //just opened file, check header
    ParseHeaderDat(f, m_points_count);
  }

  if(m_points == nullptr || m_values == nullptr || m_weights == nullptr)
    preallocateInputDataTable();

  //no placement new call is required for approximant arrays, data should be initialized when read in
  for(size_t i = 0; i < m_points_count; ++i)
  {
    f.readNextLine_scanNums(3, m_points+i, m_values+i, m_weights+i);
  }
  m_lower_border = m_points[0];
  m_upper_border = m_points[m_points_count - 1];
}

template<typename T> void ApproximantFixedGrid1d<T>::writeToFile(OutFileText& f, const int print_precision /* = 6 */)
{
  if(f.fileType() != FT_FunctionTableText)
    throw FileFormatUnsupportedError("File format unsupported",f.fileType(),f.fileName().c_str(),f.lineNum());

  for(size_t i = 0; i < m_cached_points_count; i++)
    f.println_printNumsDefault(m_cached_points[i],m_cached_values[i]);

  f.flush();
}

//memory management
template<typename T> void ApproximantFixedGrid1d<T>::preallocateInputDataTable()
{
  void* buf = nullptr;
  m_points = reinterpret_cast<T*>(
      numeric::aligned_malloc(sizeof(T)*m_points_count, numeric::getCacheLineAlignment<T>(), &buf)
      );
  m_points_buf.reset(buf);
  buf = nullptr;
  m_values = reinterpret_cast<T*>(
      numeric::aligned_malloc(sizeof(T)*m_points_count, numeric::getCacheLineAlignment<T>(), &buf)
      );
  m_values_buf.reset(buf);
  buf = nullptr;
  m_weights = reinterpret_cast<T*>(
      numeric::aligned_malloc(sizeof(T)*m_points_count, numeric::getCacheLineAlignment<T>(), &buf)
      );
  m_weights_buf.reset(buf);
  if(m_points == nullptr || m_values == nullptr || m_weights == nullptr) {
    throw OOMError("Can't allocate aligned data buffer for the approximant input data");
  }
}

template<typename T> void ApproximantFixedGrid1d<T>::preallocateApproximatedDataTable(size_t table_size)
{
  m_cached_points_count = table_size;
  void* buf = nullptr;
  m_cached_points = reinterpret_cast<T*>(
      numeric::aligned_malloc(sizeof(T)*m_cached_points_count, numeric::getCacheLineAlignment<T>(), &buf)
      );
  m_cached_points_buf.reset(buf);
  buf = nullptr;
  m_cached_values = reinterpret_cast<T*>(
      numeric::aligned_malloc(sizeof(T)*m_cached_points_count, numeric::getCacheLineAlignment<T>(), &buf)
      );
  m_cached_values_buf.reset(buf);
  if(m_cached_points == nullptr || m_cached_values == nullptr) {
    throw OOMError("Can't allocate aligned data buffer for the approximant cached data");
  }
}

template<typename T> void ApproximantCubicSmoothingSpline1d<T>::preallocateTemporaries()
{
  void* buf = nullptr;
  m_lhs = reinterpret_cast<T*>(
      numeric::aligned_malloc(sizeof(T)*(5*m_points_count), numeric::getCacheLineAlignment<T>(), &buf)
      );
  m_lhs_buf.reset(buf);
  buf = nullptr;
  m_rhs = reinterpret_cast<T*>(
      numeric::aligned_malloc(sizeof(T)*m_points_count, numeric::getCacheLineAlignment<T>(), &buf)
      );
  m_rhs_buf.reset(buf);
  buf = nullptr;
  m_R = reinterpret_cast<T*>(
      numeric::aligned_malloc(sizeof(T)*(3*m_points_count), numeric::getCacheLineAlignment<T>(), &buf)
      );
  m_R_buf.reset(buf);
  buf = nullptr;
  m_Q = reinterpret_cast<T*>(
      numeric::aligned_malloc(sizeof(T)*(3*m_points_count), numeric::getCacheLineAlignment<T>(), &buf)
      );
  m_Q_buf.reset(buf);
  buf = nullptr;
  m_WQ = reinterpret_cast<T*>(
      numeric::aligned_malloc(sizeof(T)*(3*m_points_count), numeric::getCacheLineAlignment<T>(), &buf)
      );
  m_WQ_buf.reset(buf);
  if(m_lhs == nullptr || m_rhs == nullptr || m_R == nullptr || m_Q == nullptr || m_WQ == nullptr ) {
    throw OOMError("Can't allocate aligned data buffer for the approximant's temporaries");
  }
}

template<typename T> void ApproximantCubicSmoothingSpline1d<T>::preallocateParametersTable()
{
  void* buf = nullptr;
  m_S0 = reinterpret_cast<T*>(
      numeric::aligned_malloc(sizeof(T)*m_points_count, numeric::getCacheLineAlignment<T>(), &buf)
      );
  m_S0_buf.reset(buf);
  buf = nullptr;
  m_S2 = reinterpret_cast<T*>(
      numeric::aligned_malloc(sizeof(T)*m_points_count, numeric::getCacheLineAlignment<T>(), &buf)
      );
  m_S2_buf.reset(buf);
  if(m_S0 == nullptr || m_S2 == nullptr) {
    throw OOMError("Can't allocate aligned data buffer for the approximant");
  }
}

template<typename T> __FORCEINLINE inline
  void ApproximantCubicSmoothingSpline1d<T>::prepare_matrices_op(const size_t i,
    const T* const __RESTRICT points, const T* const __RESTRICT weights,
    T* const __RESTRICT R, T* const __RESTRICT Q, T* const __RESTRICT WQ,
    const T smoothing_k /* = 1.0 */ )
{
    T r1, r2, r3, q1, q2, q3;
    r1 = points[i] - points[i - 1];
    r2 = points[i + 1] - points[i - 1];
    r3 = points[i + 1] - points[i];
    q1 = T(1)/r1;
    q2 = T(1)/r3;
    q3 = T(1)/weights[i];
    R[3*i] = r1;
    R[3*i + 1] = r2*2;
    R[3*i + 2] = r3;
    Q[3*i] = q1*6;
    Q[3*i + 1] = -(q1 + q2)*6;
    Q[3*i + 2] = q2*6;
    WQ[3*i] = q1*q3*smoothing_k;
    WQ[3*i + 1] = -(q1 + q2)*q3*smoothing_k;
    WQ[3*i + 2] = q1*q3*smoothing_k;
}

//compute approximant
template<typename T> void ApproximantCubicSmoothingSpline1d<T>::prepare_matrices(numeric::TThreading threading_model)
{
  const T k = (T(1) - m_smoothing_p)/m_smoothing_p;
  //fill matrices R, 6Q', (1-p)/pWQ
  switch(threading_model)
  {
    case numeric::T_Std:
      //TODO
#ifdef HAVE_PTHREADS
    case numeric::T_Posix:
      //TODO
#endif
#ifdef HAVE_OPENMP
    case numeric::T_OpenMP:
    {
#pragma omp parallel for
      for(size_t i = 1; i < m_points_count - 1; i++)
        prepare_matrices_op(i,m_points,m_weights,m_R,m_Q,m_WQ,k);
      break;
    }
#endif
#ifdef HAVE_CILK
    case numeric::T_Cilk:
    {
      cilk_for(size_t i = 1; i < m_points_count - 1; i++)
        prepare_matrices_op(i,m_points,m_weights,m_R,m_Q,m_WQ,k);
      break;
    }
#endif
#ifdef HAVE_TBB
    case numeric::T_TBB:
    {
      struct {
        const T* const points;
        const T* const weights;
        T* const R;
        T* const Q;
        T* const WQ;
        const T k;
        void operator()(size_t i)
        {
          return prepare_matrices_op(i,points,weights,R,Q,WQ,k);
        }
      } EvalFunc(m_points,m_weights,m_R,m_Q,m_WQ,k);
      numeric::parallelForElem( size_t(1), m_points_count - 1, EvalFunc);
      break;
    }
#endif
    case numeric::T_Serial:
    case numeric::T_Undefined:
    default:
    {
      for(size_t i = 1; i < m_points_count - 1; i++)
        prepare_matrices_op(i,m_points,m_weights,m_R,m_Q,m_WQ,k);
      break;
    }
  }
  //fill last and first rows
  T tmp = m_points[m_points_count - 1] - m_points[m_points_count - 2];
  m_R[3*m_points_count - 4] = T(0);
  m_R[3*m_points_count - 3] = T(0);
  m_R[3*m_points_count - 2] = tmp*2;
  m_R[3*m_points_count - 1] = T(0);
//  m_Q[3*m_points_count - 4] = m_Q[3*m_points_count - 4];
  m_Q[3*m_points_count - 3] = T(0);
  m_Q[3*m_points_count - 2] = T(0);
  m_Q[3*m_points_count - 1] = T(0);
  m_WQ[3*m_points_count - 4] = T(0);
  m_WQ[3*m_points_count - 3] = k/(tmp*m_weights[m_points_count - 1]);
  m_WQ[3*m_points_count - 2] = T(0);
  m_WQ[3*m_points_count - 1] = T(0);
  tmp = m_points[1] - m_points[0];
  m_R[0] = T(0);
  m_R[1] = tmp*2;
  m_R[2] = T(0);
  m_R[3] = T(0);
  m_Q[0] = T(0);
  m_Q[1] = T(0);
  m_Q[2] = T(0);
//  m_Q[3] = m_Q[3];
  m_WQ[0] = T(0);
  m_WQ[1] = T(0);
  m_WQ[2] = k/(tmp*m_weights[0]);
  m_WQ[3] = T(0);
}

template<typename T> void ApproximantCubicSmoothingSpline1d<T>::assemble_equations(numeric::TThreading threading_model)
{
  prepare_matrices(threading_model);
  //lhs R+6Q'*(1-p)/pWQ
  numeric::dgbmm(numeric::TMatrixStorage::RowMajor, numeric::TMatrixTranspose::No, numeric::TMatrixTranspose::No,
      m_Q, m_WQ, m_lhs, m_points_count, 1, threading_model);
  numeric::banded_add(numeric::TMatrixStorage::RowMajor, m_lhs, m_R, m_points_count, 2, 1, threading_model);
  //rhs 6Q'*y
  numeric::dgbmv(numeric::TMatrixStorage::RowMajor, numeric::TMatrixTranspose::No,
      m_Q, m_values, m_rhs, m_points_count, 1, T(1), threading_model);
}

template<typename T> void ApproximantCubicSmoothingSpline1d<T>::solve_equations(numeric::TThreading threading_model)
{
  //solve for S2
  numeric::thomas_solve(m_lhs, m_S2, m_rhs, m_points_count, 2, /* reuse_storage = */ true);
  //S0 = y
  memcpy(reinterpret_cast<void*>(m_S0), reinterpret_cast<const void*>(m_values), m_points_count*sizeof(T));
  //S0 += -(1-p)/pWQ*S2
  numeric::dgbmv(numeric::TMatrixStorage::RowMajor, numeric::TMatrixTranspose::No,
      m_WQ, m_S2, m_S0, m_points_count, 1, T(-1), threading_model);
}

//precompute table of approximated values on uniform grid
template<typename T> void ApproximantCubicSmoothingSpline1d<T>::evaluateOnUniformGrid(size_t points_count, numeric::TThreading threading_model)
{
  //prepare points for approximation
  const T delta = (m_upper_border - m_lower_border) / T(points_count - 1);
  switch(threading_model)
  {
    case numeric::T_Std:
      //TODO
#ifdef HAVE_PTHREADS
    case numeric::T_Posix:
      //TODO
#endif
#ifdef HAVE_OPENMP
    case numeric::T_OpenMP:
#pragma omp parallel for
      for(size_t i = 0; i < points_count; i++)
        m_cached_points[i] = m_lower_border + delta*T(i);
#endif
#ifdef HAVE_CILK
    case numeric::T_Cilk:
      cilk_for(size_t i = 0; i < points_count; i++)
        m_cached_points[i] = m_lower_border + delta*T(i);
#endif
#ifdef HAVE_TBB
    case numeric::T_TBB:
      numeric::parallelForElem( size_t(0), points_count,
        [this,&delta](size_t i)
        {
          this->m_cached_points[i] = this->m_lower_border + delta*T(i);
        }
      );
#endif
    case numeric::T_Serial:
    case numeric::T_Undefined:
    default:
      for(size_t i = 0; i < points_count; i++)
        m_cached_points[i] = m_lower_border + delta*T(i);
  }
  //evaluate
  numeric::cubic_spline_evaluate_table(m_points,m_S0,m_S2,m_points_count,
      m_cached_points,m_cached_values,m_cached_points_count,
      threading_model);
}

template<typename T> void ApproximantCubicSmoothingSpline1d<T>::dumpApproximant()
{
  for(size_t i = 1; i < m_points_count; i++)
  {
    T h = m_points[i] - m_points[i-1];
    h *= h;
    h /= 6;
    std::cerr << "[" << m_points[i-1] << ";" << m_points[i] << "] : f_i(t) = ";
    std::cerr << m_S0[i] << "*t + " << m_S0[i-1] << "*(1-t) - t(1-t)((2-t)*" << h*m_S2[i-1] << " + (1+t)*" << h*m_S2[i] << " ) " << std::endl;
//    std::cerr << "f'_i(t) = "
  }
}

struct CreateApproximantHelperArgs
{
  enum InitType { FromFile /* , FixedSize,  FromFileFixedSize */ } m_InitType;
  size_t m_points_count, m_cached_points_count;
  bool m_reset, m_readData;
  InFileText* m_pf;
  TApproximantType m_type;
  TApproximantFlavour m_flavour;
};

template<CreateApproximantHelperArgs::InitType type> struct CreateApproximantHelperFunc :
  numeric::MPFuncBase<CreateApproximantHelperFunc<type>,CreateApproximantHelperArgs,ApproximantBase1d*>
{
  template<typename T> ApproximantBase1d* perform(const CreateApproximantHelperArgs& a)
  {
    ApproximantFixedGrid1d<T>* p = nullptr;
    switch(a.m_flavour)
    {
      case TApproximantFlavour::CubicSmoothingSpline:
        switch(a.m_type)
        {
#ifdef HAVE_GSL
//              case TApproximantType::GSL:
//                break;
#endif
          case TApproximantType::Array:
            p = dynamic_cast<ApproximantFixedGrid1d<T>*>(new ApproximantCubicSmoothingSpline1d<T>()) ;
            break;
          default:
            throw ParameterError("approximant type unsupported");
        }
        break;
      default:
        throw ParameterError("approximant type unsupported");
    }
    switch(type)
    {
//      case CreateApproximantHelperArgs::FixedSize:
//        p -> init(a.m_reset, true);
//        break;
      case CreateApproximantHelperArgs::FromFile:
        p -> init(*a.m_pf,a.m_readData);
    }
    return dynamic_cast<ApproximantBase1d*>(p);
  }
};

//helper functions to create approximants with given type
inline ApproximantBase1d* NewApproximant1d(const numeric::TPrecision p, InFileText* pf, const bool read_data /* = true */,
  const TApproximantType type /* = TApproximantType::Array */, const TApproximantFlavour flavour /* = TApproximantFlavour::CubicSmoothingSpline: */)
{
  if(pf==nullptr)
    return nullptr;
  CreateApproximantHelperArgs args;
  args.m_InitType = CreateApproximantHelperArgs::FromFile;
  args.m_pf = pf; args.m_readData = read_data;
  args.m_type = type; args.m_flavour = flavour;
  return CreateApproximantHelperFunc<CreateApproximantHelperArgs::FromFile>()(p,args);
}

//inline ApproximantBase1d* NewApproximant1d(const numeric::TPrecision p, const size_t points_count, const size_t cached_points_count,
//  const TApproximantType type /* = TApproximantType::Array */ , const TApproximantFlavour flavour /* = TApproximantFlavour::CubicSmoothingSpline */)
//{}

//inline ApproximantBase1d* NewApproximant1d(const numeric::TPrecision p, const size_t points_count, const size_t cached_points_count,
//  InFileText& f,
//  const TApproximantType type /* = TApproximantType::Array */ , const TApproximantFlavour flavour /* = TApproximantFlavour::CubicSmoothingSpline */)
//{}

}

