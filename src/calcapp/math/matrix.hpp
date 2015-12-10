#pragma once
#ifndef _MATRIX_HPP
#define _MATRIX_HPP
#include "config.h"

#include <memory>
#include <type_traits>
#include <cstdlib>
#include <cstring>
#include <algorithm>
#include <valarray>

#include "numeric/blas.hpp"
#include "numeric/cache.hpp"
#include "numeric/real.hpp"

#include "calcapp/infile.hpp"
#include "calcapp/outfile.hpp"
#include "calcapp/exception.hpp"
#include "calcapp/log.hpp"

#ifdef HAVE_BOOST_UBLAS
# include <boost/numeric/ublas/matrix.hpp>
#endif
#ifdef HAVE_EIGEN
# include <Eigen/Dense>
#endif
#ifdef HAVE_MTL
# include <boost/numeric/mtl/matrix/dense2D.hpp>
#endif
#ifdef HAVE_ARMADILLO
# include <armadillo>
#endif


using std::size_t;

namespace Calc
{

enum class TMatrixType {
  Array,
  ValArray,
  BoostUblas,
  Eigen,
  MTL,
  Armadillo
};

class MatrixBase
{
protected:
  MatrixBase(const numeric::TMatrixStorage storage=numeric::TMatrixStorage::RowMajor )
    : m_nrows(0), m_ncolumns(0), m_storage(storage)
  {}
  MatrixBase(const size_t nrows, const size_t ncolumns,
    const numeric::TMatrixStorage storage=numeric::TMatrixStorage::RowMajor )
      : m_nrows(nrows), m_ncolumns(ncolumns), m_storage(storage)
  {}

protected:
  size_t m_nrows;
  size_t m_ncolumns;
  const numeric::TMatrixStorage m_storage;

public:
  virtual ~MatrixBase() {}

  inline bool isRowMajor() const { return (m_storage == numeric::TMatrixStorage::RowMajor); }
  inline size_t getSize() const { return m_nrows*m_ncolumns; }
  inline size_t getRowsNum() const { return m_nrows; }
  inline size_t getColumnsNum() const { return m_ncolumns; }
  inline size_t index(size_t i, size_t j) const
    { return ( isRowMajor() ? i*m_ncolumns+j : j*m_nrows+i ); }

  // data access
  template<typename T> inline T* getDataPtr() const;
  template<typename T> inline T get(size_t i, size_t j) const;
  template<typename T> inline T& get(size_t i,size_t j);
  template<typename T> inline T get(size_t idx) const;
  template<typename T> inline T& operator()(size_t i, size_t j);
  template<typename T> inline T operator()(size_t i, size_t j) const;
  template<typename T> inline T& operator()(size_t idx);
  template<typename T> inline T operator()(size_t idx) const;
  template<typename T> inline T& operator[](size_t idx);
  template<typename T> inline const T operator[](size_t idx) const ;

  //data IO
  virtual void readFromFile(InFileText& f, const bool transpose = false) = 0 ;
  virtual void writeToFile(OutFileText& f, const bool transpose = false, const int print_precision = 6) = 0 ;
protected:
  virtual void ensureAllocated() = 0;
};

template<typename T> class ArrayBasedMatrix : public MatrixBase
{
protected:
  ArrayBasedMatrix(const numeric::TMatrixStorage storage=numeric::TMatrixStorage::RowMajor )
    : MatrixBase(storage)
    , m_data(nullptr)
    , m_stride(0)
  {}
  ArrayBasedMatrix(const size_t nrows, const size_t ncolumns,
    const numeric::TMatrixStorage storage=numeric::TMatrixStorage::RowMajor )
      : MatrixBase(nrows, ncolumns, storage)
      , m_data(nullptr)
      , m_stride(( isRowMajor() ? ncolumns : nrows ))
  {}

protected:
  T* m_data;

public:
  size_t m_stride;

public:
  static constexpr size_t getDefaultAlignment()
  {
    return ( numeric::default_cache_line_size() > std::alignment_of<T>::value ? numeric::default_cache_line_size() : std::alignment_of<T>::value );
  }
  static size_t getAlignment()
  {
    const size_t line_size = numeric::cache_line_size();
    return ( line_size > std::alignment_of<T>::value ? line_size : std::alignment_of<T>::value );
  }

  void init(const bool reset = true, const bool initObjects = true);
  void init(InFileText& f, const bool readData = true, const bool transpose = false);
  virtual ~ArrayBasedMatrix() {}

  // data access
  inline T* getDataPtr() const { return m_data; }
  inline T get(size_t i, size_t j) const  { return m_data[index(i,j)];  }
  inline T& get(size_t i,size_t j)  { return m_data[index(i,j)];  }
  inline T get(size_t idx) const  { return m_data[idx];  }
  inline T& operator()(size_t i, size_t j)  { return m_data[index(i, j)];  }
  inline T operator()(size_t i, size_t j) const { return m_data[index(i, j)];  }
  inline T& operator()(size_t idx)  { return m_data[idx];  }
  inline T operator()(size_t idx) const { return m_data[idx];  }
  inline T& operator[](size_t idx)  { return m_data[idx];  }
  inline const T operator[](size_t idx) const { return m_data[idx];  }

  //data IO
  virtual void readFromFile(InFileText& f, const bool transpose = false) override;
  virtual void writeToFile(OutFileText& f, const bool transpose = false, const int print_precision = 6) override;

protected:
  static void ParseHeaderDat(InFileText& f, size_t& rows, size_t& columns);
  void WriteHeaderDat(OutFileText& f, const size_t rows, const size_t columns);
};

template<typename T> class Matrix final : public ArrayBasedMatrix<T>
{
private:
  numeric::unique_aligned_buf_ptr m_buf; // smart pointer is used to correctly deallocate memory obtained from aligned_alloc

public:
  Matrix(const numeric::TMatrixStorage storage=numeric::TMatrixStorage::RowMajor )
    : ArrayBasedMatrix<T>(storage)
  {}
  Matrix(const size_t nrows, const size_t ncolumns,
    const numeric::TMatrixStorage storage=numeric::TMatrixStorage::RowMajor )
      : ArrayBasedMatrix<T>(nrows, ncolumns, storage)
  {}
  ~Matrix();

protected:
  virtual void ensureAllocated() override;

private:
  // no copying and copy assignment allowed
  Matrix(const Matrix&);
  Matrix(const Matrix&&);
  Matrix& operator= (const Matrix&);
};

template<typename T> class ValArrayMatrix final : public ArrayBasedMatrix<T>
{
private:
  std::unique_ptr<std::valarray<T>> m_valarray;

public:
  ValArrayMatrix(const numeric::TMatrixStorage storage=numeric::TMatrixStorage::RowMajor )
    : ArrayBasedMatrix<T>(storage)
  {}
  ValArrayMatrix(const size_t nrows, const size_t ncolumns,
    const numeric::TMatrixStorage storage=numeric::TMatrixStorage::RowMajor )
      : ArrayBasedMatrix<T>(nrows, ncolumns, storage)
  {}
  ~ValArrayMatrix() {};

  inline const std::valarray<T>& getValArray() const { return *(m_valarray.get()); }
  inline std::valarray<T>& getValArray() { return *(m_valarray.get()); }
  inline std::valarray<T>* getValArrayPtr() const { return m_valarray.get(); }

protected:
  virtual void ensureAllocated() override;

private:
  // no copying and copy assignment allowed
  ValArrayMatrix(const ValArrayMatrix&);
  ValArrayMatrix(const ValArrayMatrix&&);
  ValArrayMatrix& operator= (const ValArrayMatrix&);
};

template<numeric::TMatrixStorage storage> struct TraitStorage {};
template<> struct TraitStorage<numeric::TMatrixStorage::RowMajor>
{
#ifdef HAVE_BOOST_UBLAS
  typedef boost::numeric::ublas::row_major boost_type;
#endif
#ifdef HAVE_EIGEN
  static constexpr int eigen_type = Eigen::RowMajor;
#endif
#ifdef HAVE_MTL
  typedef mtl::tag::row_major mtl_type;
#endif
};
template<> struct TraitStorage<numeric::TMatrixStorage::ColumnMajor> {
#ifdef HAVE_BOOST_UBLAS
  typedef boost::numeric::ublas::column_major boost_type;
#endif
#ifdef HAVE_EIGEN
  static constexpr int eigen_type = Eigen::ColMajor;
#endif
#ifdef HAVE_MTL
  typedef mtl::tag::col_major mtl_type;
#endif
};

#ifdef HAVE_BOOST_UBLAS
template<typename T, numeric::TMatrixStorage storage = numeric::TMatrixStorage::RowMajor> class BoostUblasMatrix final : public ArrayBasedMatrix<T>
{
private:
  std::unique_ptr<boost::numeric::ublas::matrix<T,typename TraitStorage<storage>::boost_type> > m_boost_matrix;

public:
  BoostUblasMatrix()
    : ArrayBasedMatrix<T>(storage)
  {}
  BoostUblasMatrix(const size_t nrows, const size_t ncolumns)
      : ArrayBasedMatrix<T>(nrows, ncolumns, storage)
  {}
  ~BoostUblasMatrix() {};

  inline const boost::numeric::ublas::matrix<T,typename TraitStorage<storage>::boost_type>& getBoostMatrix() const { return *(m_boost_matrix.get()); }
  inline boost::numeric::ublas::matrix<T,typename TraitStorage<storage>::boost_type>& getBoostMatrix() { return *(m_boost_matrix.get()); }
  inline boost::numeric::ublas::matrix<T,typename TraitStorage<storage>::boost_type>* getBoostMatrixPtr() const { return m_boost_matrix.get(); }

protected:
  virtual void ensureAllocated() override;

private:
  // no copying and copy assignment allowed
  BoostUblasMatrix(const BoostUblasMatrix&);
  BoostUblasMatrix(const BoostUblasMatrix&&);
  BoostUblasMatrix& operator= (const BoostUblasMatrix&);
};
#endif

#ifdef HAVE_EIGEN
template<typename T, numeric::TMatrixStorage storage = numeric::TMatrixStorage::ColumnMajor> class EigenMatrix final : public ArrayBasedMatrix<T>
{
private:
  std::unique_ptr<Eigen::Matrix<T,Eigen::Dynamic,Eigen::Dynamic,TraitStorage<storage>::eigen_type> > m_eigen_matrix;

public:
  EigenMatrix()
    : ArrayBasedMatrix<T>(storage)
  {}
  EigenMatrix(const size_t nrows, const size_t ncolumns)
      : ArrayBasedMatrix<T>(nrows, ncolumns, storage)
  {}
  ~EigenMatrix() {};

  inline const Eigen::Matrix<T,Eigen::Dynamic,Eigen::Dynamic,TraitStorage<storage>::eigen_type>& getEigenMatrix() const { return *(m_eigen_matrix.get()); }
  inline Eigen::Matrix<T,Eigen::Dynamic,Eigen::Dynamic,TraitStorage<storage>::eigen_type>& getEigenMatrix() { return *(m_eigen_matrix.get()); }
  inline Eigen::Matrix<T,Eigen::Dynamic,Eigen::Dynamic,TraitStorage<storage>::eigen_type>* getEigenMatrixPtr() const { return m_eigen_matrix.get(); }

protected:
  virtual void ensureAllocated() override;

private:
  // no copying and copy assignment allowed
  EigenMatrix(const EigenMatrix&);
  EigenMatrix(const EigenMatrix&&);
  EigenMatrix& operator= (const EigenMatrix&);
};
#endif
#ifdef HAVE_MTL
template<typename T, numeric::TMatrixStorage storage = numeric::TMatrixStorage::RowMajor> class MTLMatrix final : public ArrayBasedMatrix<T>
{
private:
  std::unique_ptr<mtl::mat::dense2D<T,mtl::mat::parameters<typename TraitStorage<storage>::mtl_type> > > m_mtl_matrix;

public:
  MTLMatrix()
    : ArrayBasedMatrix<T>(storage)
  {}
  MTLMatrix(const size_t nrows, const size_t ncolumns)
      : ArrayBasedMatrix<T>(nrows, ncolumns, storage)
  {}
  ~MTLMatrix() {};

  inline const mtl::mat::dense2D<T,mtl::mat::parameters<typename TraitStorage<storage>::mtl_type> >& getMTLMatrix() const { return *(m_mtl_matrix.get()); }
  inline mtl::mat::dense2D<T,mtl::mat::parameters<typename TraitStorage<storage>::mtl_type> >& getMTLMatrix() { return *(m_mtl_matrix.get()); }
  inline mtl::mat::dense2D<T,mtl::mat::parameters<typename TraitStorage<storage>::mtl_type> >* getMTLMatrixPtr() const { return m_mtl_matrix.get(); }

protected:
  virtual void ensureAllocated() override;

private:
  // no copying and copy assignment allowed
  MTLMatrix(const MTLMatrix&);
  MTLMatrix(const MTLMatrix&&);
  MTLMatrix& operator= (const MTLMatrix&);
};
#endif
#ifdef HAVE_ARMADILLO
template<typename T> class ArmadilloMatrix final : public ArrayBasedMatrix<T>
{
private:
  numeric::unique_aligned_buf_ptr m_buf; // smart pointer is used to correctly deallocate memory obtained from aligned_alloc
  std::unique_ptr<arma::Mat<T> > m_armadillo_matrix;

public:
  ArmadilloMatrix()
    : ArrayBasedMatrix<T>(numeric::TMatrixStorage::ColumnMajor)
  {}
  ArmadilloMatrix(const size_t nrows, const size_t ncolumns)
      : ArrayBasedMatrix<T>(nrows, ncolumns, numeric::TMatrixStorage::ColumnMajor)
  {}
  ~ArmadilloMatrix();

  inline const arma::Mat<T>& getArmadilloMatrix() const { return *(m_armadillo_matrix.get()); }
  inline arma::Mat<T>& getArmadilloMatrix() { return *(m_armadillo_matrix.get()); }
  inline arma::Mat<T>* getArmadilloMatrixPtr() const { return m_armadillo_matrix.get(); }

protected:
  virtual void ensureAllocated() override;

private:
  // no copying and copy assignment allowed
  ArmadilloMatrix(const ArmadilloMatrix&);
  ArmadilloMatrix(const ArmadilloMatrix&&);
  ArmadilloMatrix& operator= (const ArmadilloMatrix&);
};

template<> class ArmadilloMatrix<long double> final : public ArrayBasedMatrix<long double>
{
  typedef long double T;
public:
  ArmadilloMatrix()
    : ArrayBasedMatrix<T>(numeric::TMatrixStorage::ColumnMajor)
  { throw ParameterError("80-bit long double is not supported by Armadillo"); }
  ArmadilloMatrix(const size_t nrows, const size_t ncolumns)
      : ArrayBasedMatrix<T>(nrows, ncolumns, numeric::TMatrixStorage::ColumnMajor)
  { throw ParameterError("80-bit long double is not supported by Armadillo"); }
  ~ArmadilloMatrix() {}
protected:
  virtual void ensureAllocated() override {}
};
# ifdef HAVE_QUADMATH
template<> class ArmadilloMatrix<numeric::quad> final : public ArrayBasedMatrix<numeric::quad>
{
  typedef numeric::quad T;
public:
  ArmadilloMatrix()
    : ArrayBasedMatrix<T>(numeric::TMatrixStorage::ColumnMajor)
  { throw ParameterError("128-bit quad is not supported by Armadillo"); }
  ArmadilloMatrix(const size_t nrows, const size_t ncolumns)
      : ArrayBasedMatrix<T>(nrows, ncolumns, numeric::TMatrixStorage::ColumnMajor)
  { throw ParameterError("128-bit quad is not supported by Armadillo"); }
  ~ArmadilloMatrix() {}
protected:
  virtual void ensureAllocated() override {}
};
# endif
# ifdef HAVE_MPREAL
template<> class ArmadilloMatrix<numeric::mpreal> final : public ArrayBasedMatrix<numeric::mpreal>
{
  typedef numeric::mpreal T;
public:
  ArmadilloMatrix()
    : ArrayBasedMatrix<T>(numeric::TMatrixStorage::ColumnMajor)
  { throw ParameterError("MPFR is not supported by Armadillo"); }
  ArmadilloMatrix(const size_t nrows, const size_t ncolumns)
      : ArrayBasedMatrix<T>(nrows, ncolumns, numeric::TMatrixStorage::ColumnMajor)
  { throw ParameterError("MPFR is not supported by Armadillo"); }
  ~ArmadilloMatrix() {}
protected:
  virtual void ensureAllocated() override {}
};
# endif
#endif

//helper functions to create matrices with corresponting type
inline MatrixBase* NewMatrix(const numeric::TPrecision p, const size_t nrows, const size_t ncolumns,
  const bool reset = true, const numeric::TMatrixStorage storage = numeric::TMatrixStorage::RowMajor,
  const TMatrixType type = TMatrixType::Array);
inline MatrixBase* NewMatrix(const numeric::TPrecision p, InFileText& f, const bool readData = true,
  const bool transpose = false, const numeric::TMatrixStorage storage = numeric::TMatrixStorage::RowMajor,
  const TMatrixType type = TMatrixType::Array);
inline MatrixBase* NewMatrix(const numeric::TPrecision p, const size_t nrows, const size_t ncolumns, InFileText& f,
  const bool transpose = true, const numeric::TMatrixStorage storage = numeric::TMatrixStorage::RowMajor,
  const TMatrixType type = TMatrixType::Array);


}

//implementation
#include "calcapp/math/matrix_impl.hpp"

#endif /* _MATRIX_HPP */
