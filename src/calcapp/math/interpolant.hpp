#pragma once
#ifndef _INTERPOLANT_HPP
#define _INTERPOLANT_HPP
#include "config.h"

#include <memory>

#include "numeric/cache.hpp"
#include "numeric/real.hpp"
#include "numeric/interpolation.hpp"

#include "calcapp/infile.hpp"
#include "calcapp/outfile.hpp"
#include "calcapp/exception.hpp"
#include "calcapp/log.hpp"

#ifdef HAVE_GSL
# include <gsl/gsl_errno.h>
# include <gsl/gsl_spline.h>
#endif

using std::size_t;

namespace Calc
{

enum class TInterpolantType {
  Array,
  GSL
};

enum class TInterpolantFlavour {
  Uniform,
  ChebyshevFirstKind
};

class InterpolantBase1d
{
protected:
  size_t m_points_count;
  size_t m_cached_points_count;

protected:
  InterpolantBase1d(size_t points_count = 0, size_t cached_points_count = 0)
    : m_points_count(points_count), m_cached_points_count(cached_points_count)
  {}

public:
  virtual void init(const bool reset = true, const bool initObjects = true) = 0;
  virtual void init(InFileText& f, const bool readData = true) = 0;
  virtual ~InterpolantBase1d() {}

  //generic parameters
  size_t getPointsCount() const { return m_points_count;  }
  size_t getInterpolatedPointsCount() const { return m_cached_points_count;  }
  virtual double getRoundedUpperBorder() const = 0;
  virtual double getRoundedLowerBorder() const = 0;

  //some introspection
  virtual inline TInterpolantType getBackendType() const = 0;
  virtual inline TInterpolantFlavour getFlavour() const = 0;

  //data IO
  virtual void readFromFile(InFileText& f) = 0;
  virtual void writeToFile(OutFileText& f, const int print_precision = 6) = 0;

  //memory management
  virtual void preallocateFunctionTable() = 0;
  virtual void preallocateWeights() = 0;
  virtual void preallocateInterpolatedTable(size_t table_size) = 0;

  //compute interpolant points
  virtual void computePoints(numeric::TThreading threading_model = numeric::T_Serial) = 0; // should be called from init() normally
  //compute interpolant weights
  virtual void computeWeights(numeric::TThreading threading_model = numeric::T_Serial) = 0; // should be called from init() normally
  //precompute table of interpolated values on uniform grid
  virtual void interpolateOnUniformGrid(size_t points_count, numeric::TThreading threading_model) = 0;
};

template<typename T> class InterpolantFixedGrid1d : public InterpolantBase1d
{
protected:
  T m_lower_border;
  T m_upper_border;
  numeric::unique_aligned_buf_ptr m_weights_buf;
  numeric::unique_aligned_buf_ptr m_points_buf;
  numeric::unique_aligned_buf_ptr m_values_buf;
  numeric::unique_aligned_buf_ptr m_cached_points_buf;
  numeric::unique_aligned_buf_ptr m_cached_values_buf;
  T* m_weights;
  T* m_points;
  T* m_values;
  T* m_cached_points;
  T* m_cached_values;

protected:
  InterpolantFixedGrid1d(size_t points_count = 0, size_t cached_points_count = 0)
    : InterpolantBase1d(points_count,cached_points_count)
  {}
  InterpolantFixedGrid1d(T lower_border, T upper_border,
      size_t points_count = 0, size_t cached_points_count = 0)
    : InterpolantBase1d(points_count,cached_points_count)
    , m_lower_border(lower_border)
    , m_upper_border(upper_border)
  {}

public:
  void init(const bool reset = true, const bool initObjects = true) override;
  void init(InFileText& f, const bool readData = true) override;
  ~InterpolantFixedGrid1d();

  //generic parameters
  double getRoundedUpperBorder() const override { return numeric::toDouble(m_upper_border); }
  double getRoundedLowerBorder() const override { return numeric::toDouble(m_lower_border); }
  T getUpperBorder() const { return m_upper_border; }
  T getLowerBorder() const { return m_lower_border; }

  //some introspection
  inline TInterpolantType getBackendType() const override { return TInterpolantType::Array; }

  //interpolated data access
  inline const T at(T arg, numeric::TThreading threading_model = numeric::T_Serial) const
    { return numeric::lagrange_interpolate_value(m_weights,m_points,m_values,m_points_count,arg,true,threading_model); }
  inline const T operator()(T arg) const { return at(arg); }

  //data IO
  void readFromFile(InFileText& f) override;
  void writeToFile(OutFileText& f, const int print_precision = 6) override;

  //memory management
  void preallocateFunctionTable() override;
  void preallocateWeights() override;
  void preallocateInterpolatedTable(size_t table_size) override;

  //precompute table of interpolated values on uniform grid
  void interpolateOnUniformGrid(size_t points_count, numeric::TThreading threading_model) override;
protected:
  static void ParseHeaderDat(InFileText& f, size_t& points_count, T lower_border, T upper_border);
};

template<typename T> class InterpolantUniform1d final : public InterpolantFixedGrid1d<T>
{
protected:
  using InterpolantBase1d::m_points_count;
  using InterpolantFixedGrid1d<T>::m_lower_border;
  using InterpolantFixedGrid1d<T>::m_upper_border;
  using InterpolantFixedGrid1d<T>::m_points;
  using InterpolantFixedGrid1d<T>::m_weights;

public:
  InterpolantUniform1d(size_t points_count = 2, size_t cached_points_count = 2)
    : InterpolantFixedGrid1d<T>(points_count,cached_points_count)
  { if(points_count < 2) throw ParameterError("At least 2 points should be specified for interpolation"); }
  InterpolantUniform1d(T lower_border, T upper_border,
      size_t points_count = 2, size_t cached_points_count = 2)
    : InterpolantFixedGrid1d<T>(lower_border,upper_border,points_count,cached_points_count)
  { if(points_count < 2) throw ParameterError("At least 2 points should be specified for interpolation"); }

  //some introspection
  inline TInterpolantFlavour getFlavour() const override { return TInterpolantFlavour::Uniform; }

  //compute interpolant points
  void computePoints(numeric::TThreading threading_model = numeric::T_Serial) override; // should be called from init() normally
  //compute interpolant weights
  void computeWeights(numeric::TThreading threading_model = numeric::T_Serial) override; // should be called from init() normally

private:
  // no copying and copy assignment allowed
  InterpolantUniform1d(const InterpolantUniform1d&) = delete;
  InterpolantUniform1d(const InterpolantUniform1d&&) = delete;
  InterpolantUniform1d& operator= (const InterpolantUniform1d&) = delete;
};

template<typename T> class InterpolantChebyshevFirstKind1d final : public InterpolantFixedGrid1d<T>
{
protected:
  using InterpolantBase1d::m_points_count;
  using InterpolantFixedGrid1d<T>::m_lower_border;
  using InterpolantFixedGrid1d<T>::m_upper_border;
  using InterpolantFixedGrid1d<T>::m_points;
  using InterpolantFixedGrid1d<T>::m_weights;

public:
  InterpolantChebyshevFirstKind1d(size_t points_count = 2, size_t cached_points_count = 2)
    : InterpolantFixedGrid1d<T>(points_count,cached_points_count)
  { if(points_count < 2) throw ParameterError("At least 2 points should be specified for interpolation"); }
  InterpolantChebyshevFirstKind1d(T lower_border, T upper_border,
      size_t points_count = 2, size_t cached_points_count = 2)
    : InterpolantFixedGrid1d<T>(lower_border,upper_border,points_count,cached_points_count)
  { if(points_count < 2) throw ParameterError("At least 2 points should be specified for interpolation"); }

  //some introspection
  inline TInterpolantFlavour getFlavour() const override { return TInterpolantFlavour::ChebyshevFirstKind; }

  //compute interpolant points
  void computePoints(numeric::TThreading threading_model = numeric::T_Serial) override; // should be called from init() normally
  //compute interpolant weights
  void computeWeights(numeric::TThreading threading_model = numeric::T_Serial) override; // should be called from init() normally

private:
  // no copying and copy assignment allowed
  InterpolantChebyshevFirstKind1d(const InterpolantChebyshevFirstKind1d&) = delete;
  InterpolantChebyshevFirstKind1d(const InterpolantChebyshevFirstKind1d&&) = delete;
  InterpolantChebyshevFirstKind1d& operator= (const InterpolantChebyshevFirstKind1d&) = delete;
};


#ifdef HAVE_GSL
//template<typename T> class GSLInterpolant1d : public InterpolantBase1d
//{
  //TODO
//};
#endif

//helper functions to create interpolants with given type
inline InterpolantBase1d* NewInterpolant1d(const numeric::TPrecision p, const size_t points_count, const size_t cached_points_count,
  const TInterpolantType type = TInterpolantType::Array, const TInterpolantFlavour flavour = TInterpolantFlavour::ChebyshevFirstKind);
inline InterpolantBase1d* NewInterpolant1d(const numeric::TPrecision p, InFileText* f, const bool readData = true,
  const TInterpolantType type = TInterpolantType::Array, const TInterpolantFlavour flavour = TInterpolantFlavour::ChebyshevFirstKind);
/*
inline InterpolantBase1d* NewInterpolant1d(const numeric::TPrecision p, const size_t points_count, const size_t cached_points_count,
  InFileText& f,
  const TInterpolantType type = TInterpolantType::Array, const TInterpolantFlavour flavour = TInterpolantFlavour::ChebyshevFirstKind);
*/

}

//implementation
#include "calcapp/math/interpolant_impl.hpp"

#endif /* _INTERPOLANT_HPP */
