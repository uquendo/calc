#include "calcapp/math/interpolant.hpp"

#include "numeric/expand_traits.hpp"

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

template<typename T> void InterpolantFixedGrid1d<T>::init(const bool reset /* = true */, const bool initObjects /* = true */ )
{
//  static_assert(std::is_arithmetic<T>::value, "Arithmetical type expected"); //TODO: check boost::multiprecision type traits
  preallocateFunctionTable();
  preallocateWeights();
  preallocateInterpolatedTable(m_cached_points_count);

  if(reset)
  {
    memset(reinterpret_cast<void*>(m_weights),0,sizeof(T)*m_points_count);
    memset(reinterpret_cast<void*>(m_points),0,sizeof(T)*m_points_count);
    memset(reinterpret_cast<void*>(m_values),0,sizeof(T)*m_points_count);
    memset(reinterpret_cast<void*>(m_cached_points),0,sizeof(T)*m_cached_points_count);
    memset(reinterpret_cast<void*>(m_cached_values),0,sizeof(T)*m_cached_points_count);
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
  }
}

template<typename T> void InterpolantFixedGrid1d<T>::ParseHeaderDat(InFileText& f, size_t& points_count, T& lower_border, T& upper_border)
{
  f.readNextLine_scan(1,"# %zu",&points_count);
  ++points_count;
  f.readNextLine_scanNums(2, &lower_border, &upper_border);
}

template<typename T> void InterpolantFixedGrid1d<T>::init(InFileText& f, const bool readData /* = true */)
{
//  static_assert(std::is_arithmetic<T>::value, "Arithmetical type expected"); //TODO: check boost::multiprecision type traits
  if(f.fileType() != FT_InterpolationTableText)
    throw FileFormatUnsupportedError("File format unsupported",f.fileType(),f.fileName().c_str(),f.lineNum());

  ParseHeaderDat(f, m_points_count, m_lower_border, m_upper_border);
  if(m_lower_border >= m_upper_border)
    throw FileFormatValueBoundsError("Lower border should be less than upper one",f.fileType(),f.fileName().c_str(),f.lineNum());

  preallocateFunctionTable();
  preallocateWeights();

  if(readData)
  {
    //no placement new call is required for interpolant value array, data should be initialized when read in(or computed)
    for (size_t i = 0; i < m_points_count; ++i)
    {
      f.readNextLine_scanNums(1, m_values+i);
    }
  }
}

template<typename T> InterpolantFixedGrid1d<T>::~InterpolantFixedGrid1d()
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
  }
  //no need to free allocated memory, m_*_buf will take care of it
}

//data IO
template<typename T> void InterpolantFixedGrid1d<T>::readFromFile(InFileText& f)
{
//  static_assert(std::is_arithmetic<T>::value, "Arithmetical type expected"); //TODO: check boost::multiprecision type traits
  if(f.fileType() != FT_InterpolationTableText)
    throw FileFormatUnsupportedError("File format unsupported",f.fileType(),f.fileName().c_str(),f.lineNum());

  if(f.lineNum()==0)
  {
    //just opened file, check header
    ParseHeaderDat(f, m_points_count, m_lower_border, m_upper_border);
    if(m_lower_border >= m_upper_border)
      throw FileFormatValueBoundsError("Lower border should be less than upper one",f.fileType(),f.fileName().c_str(),f.lineNum());
  }

  if(m_values == nullptr)
    preallocateFunctionTable();

  //no placement new call is required for interpolant arrays, data should be initialized when read in
  for (size_t i = 0; i < m_points_count; ++i)
  {
    f.readNextLine_scanNums(1, m_values+i);
  }
}

template<typename T> void InterpolantFixedGrid1d<T>::writeToFile(OutFileText& f, const int print_precision /* = 6 */)
{
  if(f.fileType() != FT_FunctionTableText)
    throw FileFormatUnsupportedError("File format unsupported",f.fileType(),f.fileName().c_str(),f.lineNum());

  for(size_t i = 0; i < m_cached_points_count; i++)
    f.println_printNumsDefault(m_cached_points[i],m_cached_values[i]);

  f.flush();
}

template<typename T> void InterpolantFixedGrid1d<T>::dumpInterpolant()
{
  for(size_t i = 0; i < m_points_count; i++)
  {
    std::cerr << m_points[i] << " " << m_weights[i] << std::endl;
  }
}

//memory management
template<typename T> void InterpolantFixedGrid1d<T>::preallocateFunctionTable()
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
  if(m_points == nullptr || m_values == nullptr) {
    throw OOMError("Can't allocate aligned data buffer for the interpolant");
  }
}

template<typename T> void InterpolantFixedGrid1d<T>::preallocateWeights()
{
  void* buf = nullptr;
  m_weights = reinterpret_cast<T*>(
      numeric::aligned_malloc(sizeof(T)*m_points_count, numeric::getCacheLineAlignment<T>(), &buf)
      );
  m_weights_buf.reset(buf);
  if(m_weights == nullptr) {
    throw OOMError("Can't allocate aligned data buffer for the interpolant");
  }
}

template<typename T> void InterpolantFixedGrid1d<T>::preallocateInterpolatedTable(size_t table_size)
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
    throw OOMError("Can't allocate aligned data buffer for the interpolant");
  }
}

//precompute table of interpolated values on uniform grid
template<typename T> void InterpolantFixedGrid1d<T>::evaluateOnUniformGrid(size_t points_count, numeric::TThreading threading_model)
{
  //prepare points for interpolation
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
  //interpolate
  numeric::lagrange_interpolate_table(m_weights,m_points,m_values,m_points_count,
      m_cached_points,m_cached_values,m_cached_points_count,
      true,threading_model);
}

//compute interpolant points: x_k = a + (b-a)*k/N
template<typename T> void InterpolantUniform1d<T>::computePoints(numeric::TThreading threading_model)
{
  const T delta = (m_upper_border - m_lower_border) / T(m_points_count - 1);
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
      for(size_t i = 0; i < m_points_count; i++)
        m_points[i] = m_lower_border + delta*T(i);
#endif
#ifdef HAVE_CILK
    case numeric::T_Cilk:
      cilk_for(size_t i = 0; i < m_points_count; i++)
        m_points[i] = m_lower_border + delta*T(i);
#endif
#ifdef HAVE_TBB
    case numeric::T_TBB:
      numeric::parallelForElem( size_t(0), m_points_count,
        [this,&delta](size_t i)
        {
          this->m_points[i] = this->m_lower_border + delta*T(i);
        }
      );
#endif
    case numeric::T_Serial:
    case numeric::T_Undefined:
    default:
      for(size_t i = 0; i < m_points_count; i++)
        m_points[i] = m_lower_border + delta*T(i);
  }
}

//compute interpolant weights: w_k = (-1)^k * C_N^k
template<typename T> void InterpolantUniform1d<T>::computeWeights(numeric::TThreading threading_model)
{
  m_weights[0] = T(1);
  for(size_t i = 1; i < m_points_count / 2 + 1; i++)
  {
    m_weights[i] = - m_weights[i-1]*T(m_points_count - i)/T(i);
  }
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
      if(m_points_count % 2 == 0)
      {
#pragma omp parallel for
        for(size_t i = m_points_count / 2 + 1; i < m_points_count ; i++)
          m_weights[i] = - m_weights[m_points_count - 1 - i];
      } else {
#pragma omp parallel for
        for(size_t i = m_points_count / 2 + 1; i < m_points_count ; i++)
          m_weights[i] = m_weights[m_points_count - 1 - i];
      }
#endif
#ifdef HAVE_CILK
    case numeric::T_Cilk:
      if(m_points_count % 2 == 0)
      {
        cilk_for(size_t i = m_points_count / 2 + 1; i < m_points_count ; i++)
          m_weights[i] = - m_weights[m_points_count - 1 - i];
      } else {
        cilk_for(size_t i = m_points_count / 2 + 1; i < m_points_count ; i++)
          m_weights[i] = m_weights[m_points_count - 1 - i];
      }
#endif
#ifdef HAVE_TBB
    case numeric::T_TBB:
      if(m_points_count % 2 == 0)
      {
        numeric::parallelForElem( size_t(m_points_count / 2 + 1), m_points_count,
          [this](size_t i)
            {
              this->m_weights[i] = - this->m_weights[this->m_points_count - 1 - i];
            }
          );
      } else {
        numeric::parallelForElem( size_t(m_points_count / 2 + 1), m_points_count,
          [this](size_t i)
            {
              this->m_weights[i] = this->m_weights[this->m_points_count - 1 - i];
            }
          );
      }
#endif
    case numeric::T_Serial:
    case numeric::T_Undefined:
    default:
      if(m_points_count % 2 == 0)
      {
        for(size_t i = m_points_count / 2 + 1; i < m_points_count ; i++)
          m_weights[i] = - m_weights[m_points_count - 1 - i];
      } else {
        for(size_t i = m_points_count / 2 + 1; i < m_points_count ; i++)
          m_weights[i] = m_weights[m_points_count - 1 - i];
      }
  }
}

//compute interpolant points: x_k = (b+a)/2 - (b-a)/2*cos(\pi(2k+1)/(2N+2))
template<typename T> void InterpolantChebyshevFirstKind1d<T>::computePoints(numeric::TThreading threading_model)
{
  using std::cos;
  const T delta = - (m_upper_border - m_lower_border)/2.0;
  const T shift = (m_upper_border + m_lower_border)/2.0;
  const T factor = numeric::half_pi_const<T>() / T(m_points_count);
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
      for(size_t i = 0; i < m_points_count; i++)
        m_points[i] = shift + delta*cos(T(2*i+1)*factor);
#endif
#ifdef HAVE_CILK
    case numeric::T_Cilk:
      cilk_for(size_t i = 0; i < m_points_count; i++)
        m_points[i] = shift + delta*cos(T(2*i+1)*factor);
#endif
#ifdef HAVE_TBB
    case numeric::T_TBB:
      numeric::parallelForElem( size_t(0), m_points_count,
        [this,&delta,&shift,&factor](size_t i)
        {
          this->m_points[i] = shift + delta*cos(T(2*i+1)*factor);
        }
      );
#endif
    case numeric::T_Serial:
    case numeric::T_Undefined:
    default:
      for(size_t i = 0; i < m_points_count; i++)
        m_points[i] = shift + delta*cos(T(2*i+1)*factor);
  }
}

//compute interpolant weights: (-1)^k * sin(\pi*(2k+1)/(2N+2))
template<typename T> void InterpolantChebyshevFirstKind1d<T>::computeWeights(numeric::TThreading threading_model)
{
  using std::sin;
  const T factor = numeric::half_pi_const<T>() / m_points_count;
  int sign = 1;
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
      for(size_t i = 0; i < m_points_count; i++)
      {
        m_weights[i] = sign * sin(factor*T(2*i+1));
        sign = -sign;
      }
#endif
#ifdef HAVE_CILK
    case numeric::T_Cilk:
      cilk_for(size_t i = 0; i < m_points_count; i++)
      {
        m_weights[i] = sign * sin(factor*T(2*i+1));
        sign = -sign;
      }
#endif
#ifdef HAVE_TBB
    case numeric::T_TBB:
      numeric::parallelForElem( size_t(0), m_points_count,
        [this,&factor,&sign](size_t i)
        {
          this->m_weights[i] = sign * sin(factor*T(2*i+1)) ;
          sign = -sign;
        }
      );
#endif
    case numeric::T_Serial:
    case numeric::T_Undefined:
    default:
      for(size_t i = 0; i < m_points_count; i++)
      {
        m_weights[i] = sign * sin(factor*T(2*i+1));
        sign = -sign;
      }
  }
}

struct CreateInterpolantHelperArgs
{
  enum InitType { FromFile /* , FixedSize,  FromFileFixedSize */ } m_InitType;
  size_t m_points_count, m_cached_points_count;
  bool m_reset, m_readData;
  InFileText* m_pf;
  TInterpolantType m_type;
  TInterpolantFlavour m_flavour;
};

template<CreateInterpolantHelperArgs::InitType type> struct CreateInterpolantHelperFunc :
  numeric::MPFuncBase<CreateInterpolantHelperFunc<type>,CreateInterpolantHelperArgs,InterpolantBase1d*>
{
  template<typename T> InterpolantBase1d* perform(const CreateInterpolantHelperArgs& a)
  {
    InterpolantFixedGrid1d<T>* p = nullptr;
    switch(a.m_flavour)
    {
      case TInterpolantFlavour::ChebyshevFirstKind:
        switch(a.m_type)
        {
#ifdef HAVE_GSL
//              case TInterpolantType::GSL:
//                break;
#endif
          case TInterpolantType::Array:
            p = dynamic_cast<InterpolantFixedGrid1d<T>*>(new InterpolantChebyshevFirstKind1d<T>()) ;
            break;
          default:
            throw ParameterError("interpolant type unsupported");
        }
        break;
      case TInterpolantFlavour::Uniform:
        switch(a.m_type)
        {
#ifdef HAVE_GSL
//              case TInterpolantType::GSL:
//                break;
#endif
          case TInterpolantType::Array:
            p = dynamic_cast<InterpolantFixedGrid1d<T>*>(new InterpolantUniform1d<T>()) ;
            break;
          default:
            throw ParameterError("interpolant type unsupported");
        }
        break;
      default:
        throw ParameterError("interpolant type unsupported");
    }
    switch(type)
    {
//      case CreateInterpolantHelperArgs::FixedSize:
//        p -> init(a.m_reset, true);
//        break;
      case CreateInterpolantHelperArgs::FromFile:
        p -> init(*a.m_pf,a.m_readData);
    }
    return dynamic_cast<InterpolantBase1d*>(p);
  }
};

//helper functions to create interpolants with given type
inline InterpolantBase1d* NewInterpolant1d(const numeric::TPrecision p, InFileText* pf, const bool read_data /* = true */,
  const TInterpolantType type /* = TInterpolantType::Array */, const TInterpolantFlavour flavour /* = TInterpolantFlavour::ChebyshevFirstKind */)
{
  if(pf==nullptr)
    return nullptr;
  CreateInterpolantHelperArgs args;
  args.m_InitType = CreateInterpolantHelperArgs::FromFile;
  args.m_pf = pf; args.m_readData = read_data;
  args.m_type = type; args.m_flavour = flavour;
  return CreateInterpolantHelperFunc<CreateInterpolantHelperArgs::FromFile>()(p,args);
}

//inline InterpolantBase1d* NewInterpolant1d(const numeric::TPrecision p, const size_t points_count, const size_t cached_points_count,
//  const TInterpolantType type /* = TInterpolantType::Array */ , const TInterpolantFlavour flavour /* = TInterpolantFlavour::ChebyshevFirstKind */)
//{}

//inline InterpolantBase1d* NewInterpolant1d(const numeric::TPrecision p, const size_t points_count, const size_t cached_points_count,
//  InFileText& f,
//  const TInterpolantType type /* = TInterpolantType::Array */ , const TInterpolantFlavour flavour /* = TInterpolantFlavour::ChebyshevFirstKind */)
//{}


}

