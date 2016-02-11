#pragma once
#ifndef _APPROXIMANT_HPP
#define _APPROXIMANT_HPP
#include "config.h"

#include <memory>

#include "numeric/cache.hpp"
#include "numeric/real.hpp"
#include "numeric/approximation.hpp"

#include "calcapp/infile.hpp"
#include "calcapp/outfile.hpp"
#include "calcapp/exception.hpp"
#include "calcapp/log.hpp"

#ifdef HAVE_GSL
# include <gsl/gsl_errno.h>
# include <gsl/gsl_bspline.h>
#endif

using std::size_t;

namespace Calc
{

enum class TApproximantType {
  Array,
  GSL
};

enum class TApproximantFlavour {
  CubicSmoothingSpline
};

class ApproximantBase1d
{
protected:
  size_t m_points_count;
  size_t m_cached_points_count;

protected:
  ApproximantBase1d(size_t points_count = 0, size_t cached_points_count = 0)
    : m_points_count(points_count)
    , m_cached_points_count(cached_points_count)
  {}

public:
  virtual void init(const bool reset = true, const bool initObjects = true) = 0;
  virtual void init(InFileText& f, const bool readData = true) = 0;
  virtual ~ApproximantBase1d() {}

  //generic parameters
  size_t getPointsCount() const { return m_points_count;  }
  size_t getCachedPointsCount() const { return m_cached_points_count;  }
  virtual double getRoundedUpperBorder() const = 0;
  virtual double getRoundedLowerBorder() const = 0;

  //some introspection
  virtual inline TApproximantType getBackendType() const = 0;
  virtual inline TApproximantFlavour getFlavour() const = 0;

  //data IO
  virtual void readFromFile(InFileText& f) = 0;
  virtual void writeToFile(OutFileText& f, const int print_precision = 6) = 0;
  //debug dumping
  virtual void dumpApproximant() = 0;

  //memory management
  virtual void preallocateParametersTable() = 0;
  virtual void preallocateTemporaries() = 0;
  virtual void preallocateInputDataTable() = 0;
  virtual void preallocateApproximatedDataTable(size_t table_size) = 0;

  //compute approximant
  virtual void assemble_equations(numeric::TThreading threading_model = numeric::T_Serial) = 0;
  virtual void solve_equations(numeric::TThreading threading_model = numeric::T_Serial) = 0;
  virtual void compute(numeric::TThreading threading_model = numeric::T_Serial) = 0; // should be called from init() normally
  //precompute table of approximated values on uniform grid
  virtual void evaluateOnUniformGrid(size_t points_count, numeric::TThreading threading_model) = 0;
};

template<typename T> class ApproximantFixedGrid1d : public ApproximantBase1d
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
  ApproximantFixedGrid1d(size_t points_count = 0, size_t cached_points_count = 0)
    : ApproximantBase1d(points_count,cached_points_count)
  {}
  ApproximantFixedGrid1d(T lower_border, T upper_border,
      size_t points_count = 0, size_t cached_points_count = 0)
    : ApproximantBase1d(points_count,cached_points_count)
    , m_lower_border(lower_border)
    , m_upper_border(upper_border)
  {}

public:
  void init(const bool reset = true, const bool initObjects = true) override;
  void init(InFileText& f, const bool readData = true) override;
  ~ApproximantFixedGrid1d();

  //generic parameters
  double getRoundedUpperBorder() const override { return numeric::toDouble(m_upper_border); }
  double getRoundedLowerBorder() const override { return numeric::toDouble(m_lower_border); }
  T getUpperBorder() const { return m_upper_border; }
  T getLowerBorder() const { return m_lower_border; }

  //some introspection
  inline TApproximantType getBackendType() const override { return TApproximantType::Array; }

  //data access
  //virtual inline T at(T arg, numeric::TThreading threading_model = numeric::T_Serial) const = 0;
  inline T operator()(T arg) const { return at(arg); }

  //data IO
  void readFromFile(InFileText& f) override;
  void writeToFile(OutFileText& f, const int print_precision = 6) override;

  //memory management
  void preallocateInputDataTable() override;
  void preallocateApproximatedDataTable(size_t table_size) override;

protected:
  static void ParseHeaderDat(InFileText& f, size_t& points_count);
};

template<typename T> class ApproximantCubicSmoothingSpline1d final : public ApproximantFixedGrid1d<T>
{
protected:
  using ApproximantBase1d::m_points_count;
  using ApproximantBase1d::m_cached_points_count;
  using ApproximantFixedGrid1d<T>::m_lower_border;
  using ApproximantFixedGrid1d<T>::m_upper_border;
  using ApproximantFixedGrid1d<T>::m_points;
  using ApproximantFixedGrid1d<T>::m_values;
  using ApproximantFixedGrid1d<T>::m_weights;
  using ApproximantFixedGrid1d<T>::m_cached_points;
  using ApproximantFixedGrid1d<T>::m_cached_values;
  //approximant parameters
  T m_smoothing_p;
  numeric::unique_aligned_buf_ptr m_S0_buf;
  numeric::unique_aligned_buf_ptr m_S2_buf;
  T* m_S0;
  T* m_S2;
  //temporaries
  numeric::unique_aligned_buf_ptr m_lhs_buf, m_rhs_buf, m_R_buf, m_Q_buf, m_WQ_buf;
  T *m_lhs, *m_rhs, *m_R, *m_Q, *m_WQ;

public:
  ApproximantCubicSmoothingSpline1d(size_t points_count = 4, size_t cached_points_count = 2)
    : ApproximantFixedGrid1d<T>(points_count,cached_points_count)
    , m_smoothing_p(0.5)
  { if(points_count < 4) throw ParameterError("At least 4 points should be specified for cubic spline approximation"); }
  ApproximantCubicSmoothingSpline1d(T lower_border, T upper_border,
      size_t points_count = 4, size_t cached_points_count = 2)
    : ApproximantFixedGrid1d<T>(lower_border,upper_border,points_count,cached_points_count)
    , m_smoothing_p(0.5)
  { if(points_count < 4) throw ParameterError("At least 4 points should be specified for cubic spline approximation"); }
  ApproximantCubicSmoothingSpline1d(T lower_border, T upper_border, T smoothing_p,
      size_t points_count = 4, size_t cached_points_count = 2)
    : ApproximantFixedGrid1d<T>(lower_border,upper_border,points_count,cached_points_count)
    , m_smoothing_p(smoothing_p)
  { if(points_count < 4) throw ParameterError("At least 4 points should be specified for cubic spline approximation"); }

  //some introspection
  inline TApproximantFlavour getFlavour() const override { return TApproximantFlavour::CubicSmoothingSpline; }

  //data access
  inline const T getSmoothingP() const { return m_smoothing_p; }
  inline void setSmoothingP(T p) { m_smoothing_p = p; }
  inline const T at(T arg, numeric::TThreading threading_model = numeric::T_Serial) const
    { return numeric::cubic_spline_evaluate_value(m_points,m_S0,m_S2,m_points_count,arg,threading_model); }

  //memory management
  void preallocateParametersTable() override;
  void preallocateTemporaries() override;

  //compute approximant
  static __FORCEINLINE inline void prepare_matrices_op(const size_t i,
    const T* const __RESTRICT points, const T* const __RESTRICT weights,
    T* const __RESTRICT R, T* const __RESTRICT Q, T* const __RESTRICT WQ,
    const T smoothing_k = 1.0);
  void prepare_matrices(numeric::TThreading threading_model = numeric::T_Serial);
  void assemble_equations(numeric::TThreading threading_model = numeric::T_Serial) override;
  void solve_equations(numeric::TThreading threading_model = numeric::T_Serial) override;
  void compute(numeric::TThreading threading_model = numeric::T_Serial) override
  {
    preallocateTemporaries();
    assemble_equations(threading_model);
    solve_equations(threading_model);
  }

  //debug dumping
  void dumpApproximant() override;

  //precompute table of approximated values on uniform grid
  void evaluateOnUniformGrid(size_t points_count, numeric::TThreading threading_model) override;

private:
  // no copying and copy assignment allowed
  ApproximantCubicSmoothingSpline1d(const ApproximantCubicSmoothingSpline1d&) = delete;
  ApproximantCubicSmoothingSpline1d(const ApproximantCubicSmoothingSpline1d&&) = delete;
  ApproximantCubicSmoothingSpline1d& operator= (const ApproximantCubicSmoothingSpline1d&) = delete;
};


#ifdef HAVE_GSL
//template<typename T> class GSLSplineApproximant1d : public ApproximantBase1d
//{
  //TODO
//};
#endif

//helper functions to create approximants with given type
inline ApproximantBase1d* NewApproximant1d(const numeric::TPrecision p, InFileText* f, const bool readData = true,
  const TApproximantType type = TApproximantType::Array, const TApproximantFlavour flavour = TApproximantFlavour::CubicSmoothingSpline);
/*
inline ApproximantBase1d* NewApproximant1d(const numeric::TPrecision p, const size_t points_count, const size_t cached_points_count,
  const TApproximantType type = TApproximantType::Array, const TApproximantFlavour flavour = TApproximantFlavour::CubicSmoothingSpline);
inline ApproximantBase1d* NewApproximant1d(const numeric::TPrecision p, const size_t points_count, const size_t cached_points_count,
  InFileText& f,
  const TApproximantType type = TApproximantType::Array, const TApproximantFlavour flavour = TApproximantFlavour::CubicSmoothingSpline);
*/

}

//implementation
#include "calcapp/math/approximant_impl.hpp"

#endif /* _APPROXIMANT_HPP */
