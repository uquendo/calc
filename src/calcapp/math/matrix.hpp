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

enum class TMatrixFlavour {
  Dense,
  Banded,
  Hessenberg,
  Triangular,
  Sparse
};

class MatrixBase
{
protected:
  MatrixBase(const numeric::TMatrixStorage storage = numeric::TMatrixStorage::RowMajor )
    : m_nrows(0), m_ncolumns(0), m_storage(storage)
  {}
  MatrixBase(const size_t nrows, const size_t ncolumns,
    const numeric::TMatrixStorage storage = numeric::TMatrixStorage::RowMajor )
      : m_nrows(nrows), m_ncolumns(ncolumns), m_storage(storage)
  {}

public:
  size_t m_nrows;
  size_t m_ncolumns;
  const numeric::TMatrixStorage m_storage;

public:
  virtual void init(const bool reset = true, const bool initObjects = true) = 0;
  virtual void init(InFileText& f, const bool readData = true, const bool transpose = false) = 0;
  virtual ~MatrixBase() {}

  inline bool isRowMajor() const { return (m_storage == numeric::TMatrixStorage::RowMajor); }
  inline size_t getSize() const { return m_nrows*m_ncolumns; }
  inline size_t getRowsNum() const { return m_nrows; }
  inline size_t getColumnsNum() const { return m_ncolumns; }
  virtual inline TMatrixType getBackendType() const = 0;
  virtual inline TMatrixFlavour getFlavour() const = 0;
  virtual inline size_t getStoredSize() const = 0;

  // data access
  template<typename T> inline T* getDataPtr() const;
  virtual inline size_t index(size_t i, size_t j) const = 0;
  template<typename T> inline const T get(size_t i, size_t j) const;
  template<typename T> inline T& get(size_t i,size_t j);
  template<typename T> inline const T operator()(size_t i, size_t j) const;
  template<typename T> inline T& operator()(size_t i, size_t j);
  template<typename T> inline const T get(size_t idx) const;
  template<typename T> inline T& get(size_t idx);
  template<typename T> inline const T operator()(size_t idx) const;
  template<typename T> inline T& operator()(size_t idx);
  template<typename T> inline const T operator[](size_t idx) const ;
  template<typename T> inline T& operator[](size_t idx);

  //data IO
  virtual void readFromFile(InFileText& f, const bool transpose = false) = 0 ;
  virtual void writeToFile(OutFileText& f, const bool transpose = false, const int print_precision = 6) = 0 ;
  virtual void ensureAllocated() = 0;
};

template<typename T> class ArrayBasedDenseMatrix : public MatrixBase
{
protected:
  ArrayBasedDenseMatrix(const numeric::TMatrixStorage storage = numeric::TMatrixStorage::RowMajor )
    : MatrixBase(storage)
    , m_data(nullptr)
    , m_stride(0)
  {}
  ArrayBasedDenseMatrix(const size_t nrows, const size_t ncolumns,
    const numeric::TMatrixStorage storage = numeric::TMatrixStorage::RowMajor )
      : MatrixBase(nrows, ncolumns, storage)
      , m_data(nullptr)
      , m_stride( isRowMajor() ? ncolumns : nrows )
  {}

protected:
  T* m_data;

public:
  size_t m_stride;

public:

  virtual void init(const bool reset = true, const bool initObjects = true) override;
  virtual void init(InFileText& f, const bool readData = true, const bool transpose = false) override;
  virtual ~ArrayBasedDenseMatrix() {}

  inline TMatrixFlavour getFlavour() const override { return TMatrixFlavour::Dense; }
  inline size_t getStoredSize() const override { return getSize(); }

  // data access
  inline size_t index(size_t i, size_t j) const override { return ( isRowMajor() ? i*m_ncolumns+j : j*m_nrows+i ); }
  inline T* getDataPtr() const { return m_data; }
  inline T& get(size_t i,size_t j)  { return m_data[index(i,j)];  }
  inline const T get(size_t i, size_t j) const  { return m_data[index(i,j)];  }
  inline T& operator()(size_t i, size_t j)  { return m_data[index(i, j)];  }
  inline const T operator()(size_t i, size_t j) const { return m_data[index(i, j)];  }
  inline T& get(size_t idx) { return m_data[idx];  }
  inline const T get(size_t idx) const  { return m_data[idx];  }
  inline T& operator()(size_t idx)  { return m_data[idx];  }
  inline const T operator()(size_t idx) const { return m_data[idx];  }
  inline T& operator[](size_t idx)  { return m_data[idx];  }
  inline const T operator[](size_t idx) const { return m_data[idx];  }

  //data IO
  void readFromFile(InFileText& f, const bool transpose = false) override;
  void writeToFile(OutFileText& f, const bool transpose = false, const int print_precision = 6) override;
  virtual void ensureAllocated() = 0;

protected:
  static void ParseHeaderDat(InFileText& f, size_t& rows, size_t& columns);
  void WriteHeaderDat(OutFileText& f, const size_t rows, const size_t columns);
};

template<typename T> class Matrix final : public ArrayBasedDenseMatrix<T>
{
private:
  // smart pointer is used to correctly deallocate memory obtained from aligned_alloc
  numeric::unique_aligned_buf_ptr m_buf;

public:
  Matrix(const numeric::TMatrixStorage storage = numeric::TMatrixStorage::RowMajor )
    : ArrayBasedDenseMatrix<T>(storage)
  {}
  Matrix(const size_t nrows, const size_t ncolumns,
    const numeric::TMatrixStorage storage = numeric::TMatrixStorage::RowMajor )
      : ArrayBasedDenseMatrix<T>(nrows, ncolumns, storage)
  {}
  ~Matrix();

  inline TMatrixType getBackendType() const override { return TMatrixType::Array; }

protected:
  void ensureAllocated() override;

private:
  // no copying and copy assignment allowed
  Matrix(const Matrix&) = delete;
  Matrix(const Matrix&&) = delete;
  Matrix& operator= (const Matrix&) = delete;
};

template<typename T> class ValArrayMatrix final : public ArrayBasedDenseMatrix<T>
{
private:
  std::unique_ptr<std::valarray<T>> m_valarray;

public:
  ValArrayMatrix(const numeric::TMatrixStorage storage = numeric::TMatrixStorage::RowMajor )
    : ArrayBasedDenseMatrix<T>(storage)
  {}
  ValArrayMatrix(const size_t nrows, const size_t ncolumns,
    const numeric::TMatrixStorage storage = numeric::TMatrixStorage::RowMajor )
      : ArrayBasedDenseMatrix<T>(nrows, ncolumns, storage)
  {}
  ~ValArrayMatrix() {};

  inline TMatrixType getBackendType() const override { return TMatrixType::ValArray; }

  inline const std::valarray<T>& getValArray() const { return *(m_valarray.get()); }
  inline std::valarray<T>& getValArray() { return *(m_valarray.get()); }
  inline std::valarray<T>* getValArrayPtr() const { return m_valarray.get(); }

protected:
  void ensureAllocated() override;

private:
  // no copying and copy assignment allowed
  ValArrayMatrix(const ValArrayMatrix&) = delete;
  ValArrayMatrix(const ValArrayMatrix&&) = delete;
  ValArrayMatrix& operator= (const ValArrayMatrix&) = delete;
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
template<typename T, numeric::TMatrixStorage storage = numeric::TMatrixStorage::RowMajor>
  class BoostUblasMatrix final : public ArrayBasedDenseMatrix<T>
{
private:
  std::unique_ptr<boost::numeric::ublas::matrix<T,typename TraitStorage<storage>::boost_type> > m_boost_matrix;

public:
  BoostUblasMatrix()
    : ArrayBasedDenseMatrix<T>(storage)
  {}
  BoostUblasMatrix(const size_t nrows, const size_t ncolumns)
      : ArrayBasedDenseMatrix<T>(nrows, ncolumns, storage)
  {}
  ~BoostUblasMatrix() {};

  inline TMatrixType getBackendType() const override { return TMatrixType::BoostUblas; }

  inline const boost::numeric::ublas::matrix<T,typename TraitStorage<storage>::boost_type>&
    getBoostMatrix() const { return *(m_boost_matrix.get()); }
  inline boost::numeric::ublas::matrix<T,typename TraitStorage<storage>::boost_type>&
    getBoostMatrix() { return *(m_boost_matrix.get()); }
  inline boost::numeric::ublas::matrix<T,typename TraitStorage<storage>::boost_type>*
    getBoostMatrixPtr() const { return m_boost_matrix.get(); }

protected:
  void ensureAllocated() override;

private:
  // no copying and copy assignment allowed
  BoostUblasMatrix(const BoostUblasMatrix&) = delete;
  BoostUblasMatrix(const BoostUblasMatrix&&) = delete;
  BoostUblasMatrix& operator= (const BoostUblasMatrix&) = delete;
};
#endif

#ifdef HAVE_EIGEN
template<typename T, numeric::TMatrixStorage storage = numeric::TMatrixStorage::ColumnMajor>
  class EigenMatrix final : public ArrayBasedDenseMatrix<T>
{
private:
  std::unique_ptr<Eigen::Matrix<T,Eigen::Dynamic,Eigen::Dynamic,TraitStorage<storage>::eigen_type> > m_eigen_matrix;

public:
  EigenMatrix()
    : ArrayBasedDenseMatrix<T>(storage)
  {}
  EigenMatrix(const size_t nrows, const size_t ncolumns)
      : ArrayBasedDenseMatrix<T>(nrows, ncolumns, storage)
  {}
  ~EigenMatrix() {};

  inline TMatrixType getBackendType() const override { return TMatrixType::Eigen; }

  inline const Eigen::Matrix<T,Eigen::Dynamic,Eigen::Dynamic,TraitStorage<storage>::eigen_type>&
    getEigenMatrix() const { return *(m_eigen_matrix.get()); }
  inline Eigen::Matrix<T,Eigen::Dynamic,Eigen::Dynamic,TraitStorage<storage>::eigen_type>&
    getEigenMatrix() { return *(m_eigen_matrix.get()); }
  inline Eigen::Matrix<T,Eigen::Dynamic,Eigen::Dynamic,TraitStorage<storage>::eigen_type>*
    getEigenMatrixPtr() const { return m_eigen_matrix.get(); }

protected:
  void ensureAllocated() override;

private:
  // no copying and copy assignment allowed
  EigenMatrix(const EigenMatrix&) = delete;
  EigenMatrix(const EigenMatrix&&) = delete;
  EigenMatrix& operator= (const EigenMatrix&) = delete;
};
#endif
#ifdef HAVE_MTL
template<typename T, numeric::TMatrixStorage storage = numeric::TMatrixStorage::RowMajor>
  class MTLMatrix final : public ArrayBasedDenseMatrix<T>
{
private:
  std::unique_ptr<mtl::mat::dense2D<T,mtl::mat::parameters<typename TraitStorage<storage>::mtl_type> > > m_mtl_matrix;

public:
  MTLMatrix()
    : ArrayBasedDenseMatrix<T>(storage)
  {}
  MTLMatrix(const size_t nrows, const size_t ncolumns)
      : ArrayBasedDenseMatrix<T>(nrows, ncolumns, storage)
  {}
  ~MTLMatrix() {};

  inline TMatrixType getBackendType() const override { return TMatrixType::MTL; }

  inline const mtl::mat::dense2D<T,mtl::mat::parameters<typename TraitStorage<storage>::mtl_type> >&
    getMTLMatrix() const { return *(m_mtl_matrix.get()); }
  inline mtl::mat::dense2D<T,mtl::mat::parameters<typename TraitStorage<storage>::mtl_type> >&
    getMTLMatrix() { return *(m_mtl_matrix.get()); }
  inline mtl::mat::dense2D<T,mtl::mat::parameters<typename TraitStorage<storage>::mtl_type> >*
    getMTLMatrixPtr() const { return m_mtl_matrix.get(); }

protected:
  void ensureAllocated() override;

private:
  // no copying and copy assignment allowed
  MTLMatrix(const MTLMatrix&) = delete;
  MTLMatrix(const MTLMatrix&&) = delete;
  MTLMatrix& operator= (const MTLMatrix&) = delete;
};
#endif
#ifdef HAVE_ARMADILLO
template<typename T> class ArmadilloMatrix final : public ArrayBasedDenseMatrix<T>
{
private:
  numeric::unique_aligned_buf_ptr m_buf; // smart pointer is used to correctly deallocate memory obtained from aligned_alloc
  std::unique_ptr<arma::Mat<T> > m_armadillo_matrix;

public:
  ArmadilloMatrix()
    : ArrayBasedDenseMatrix<T>(numeric::TMatrixStorage::ColumnMajor)
  {}
  ArmadilloMatrix(const size_t nrows, const size_t ncolumns)
      : ArrayBasedDenseMatrix<T>(nrows, ncolumns, numeric::TMatrixStorage::ColumnMajor)
  {}
  ~ArmadilloMatrix();

  inline TMatrixType getBackendType() const override { return TMatrixType::Armadillo; }

  inline const arma::Mat<T>& getArmadilloMatrix() const { return *(m_armadillo_matrix.get()); }
  inline arma::Mat<T>& getArmadilloMatrix() { return *(m_armadillo_matrix.get()); }
  inline arma::Mat<T>* getArmadilloMatrixPtr() const { return m_armadillo_matrix.get(); }

protected:
  void ensureAllocated() override;

private:
  // no copying and copy assignment allowed
  ArmadilloMatrix(const ArmadilloMatrix&) = delete;
  ArmadilloMatrix(const ArmadilloMatrix&&) = delete;
  ArmadilloMatrix& operator= (const ArmadilloMatrix&) = delete;
};

template<> class ArmadilloMatrix<long double> final : public ArrayBasedDenseMatrix<long double>
{
  typedef long double T;
public:
  ArmadilloMatrix()
    : ArrayBasedDenseMatrix<T>(numeric::TMatrixStorage::ColumnMajor)
  { throw ParameterError("80-bit long double is not supported by Armadillo"); }
  ArmadilloMatrix(const size_t nrows, const size_t ncolumns)
      : ArrayBasedDenseMatrix<T>(nrows, ncolumns, numeric::TMatrixStorage::ColumnMajor)
  { throw ParameterError("80-bit long double is not supported by Armadillo"); }
  ~ArmadilloMatrix() {}
  inline TMatrixType getBackendType() const override { return TMatrixType::Armadillo; }
protected:
  void ensureAllocated() override {}
};
# ifdef HAVE_QUADMATH
template<> class ArmadilloMatrix<numeric::quad> final : public ArrayBasedDenseMatrix<numeric::quad>
{
  typedef numeric::quad T;
public:
  ArmadilloMatrix()
    : ArrayBasedDenseMatrix<T>(numeric::TMatrixStorage::ColumnMajor)
  { throw ParameterError("128-bit quad is not supported by Armadillo"); }
  ArmadilloMatrix(const size_t nrows, const size_t ncolumns)
      : ArrayBasedDenseMatrix<T>(nrows, ncolumns, numeric::TMatrixStorage::ColumnMajor)
  { throw ParameterError("128-bit quad is not supported by Armadillo"); }
  ~ArmadilloMatrix() {}
  inline TMatrixType getBackendType() const override { return TMatrixType::Armadillo; }
protected:
  void ensureAllocated() override {}
};
# endif
# ifdef HAVE_MPREAL
template<> class ArmadilloMatrix<numeric::mpreal> final : public ArrayBasedDenseMatrix<numeric::mpreal>
{
  typedef numeric::mpreal T;
public:
  ArmadilloMatrix()
    : ArrayBasedDenseMatrix<T>(numeric::TMatrixStorage::ColumnMajor)
  { throw ParameterError("MPFR is not supported by Armadillo"); }
  ArmadilloMatrix(const size_t nrows, const size_t ncolumns)
      : ArrayBasedDenseMatrix<T>(nrows, ncolumns, numeric::TMatrixStorage::ColumnMajor)
  { throw ParameterError("MPFR is not supported by Armadillo"); }
  ~ArmadilloMatrix() {}
  inline TMatrixType getBackendType() const override { return TMatrixType::Armadillo; }
protected:
  void ensureAllocated() override {}
};
# endif
#endif

template<typename T> class ArrayBasedCDSBandedMatrix : public ArrayBasedDenseMatrix<T>
{
protected:
  ArrayBasedCDSBandedMatrix(const numeric::TMatrixStorage storage = numeric::TMatrixStorage::RowMajor )
    : ArrayBasedDenseMatrix<T>(storage)
    , m_upper_band(0)
    , m_lower_band(0)
  {}
  ArrayBasedCDSBandedMatrix(const size_t nrows, const size_t ncolumns,
    const size_t upper_band, const size_t lower_band,
    const numeric::TMatrixStorage storage = numeric::TMatrixStorage::RowMajor )
      : ArrayBasedDenseMatrix<T>(nrows, ncolumns, storage)
      , m_upper_band(upper_band)
      , m_lower_band(lower_band)
  {}

public:
  size_t m_upper_band;
  size_t m_lower_band;

public:
  void init(InFileText& f, const bool readData = true, const bool transpose = false) override;
  virtual ~ArrayBasedCDSBandedMatrix() {}

  inline TMatrixFlavour getFlavour() const override { return TMatrixFlavour::Banded; }
  inline size_t getStoredSize() const override { return (m_lower_band+m_upper_band+1)*std::min(this->m_nrows,this->m_ncolumns); }

  // data access
  inline size_t index(size_t i, size_t j) const override { return ( this->isRowMajor() ? 0 : 0 ); } //TODO: STUB!

  //data IO
  void readFromFile(InFileText& f, const bool transpose = false) override;
  void writeToFile(OutFileText& f, const bool transpose = false, const int print_precision = 6) override;
  virtual void ensureAllocated() = 0;

protected:
  static void ParseHeaderDat(InFileText& f, size_t& rows, size_t& columns, size_t& upper_band, size_t& lower_band);
  void WriteHeaderDat(OutFileText& f, const size_t rows, const size_t columns, const size_t upper_band, const size_t lower_band);
};

template<typename T> class CDSBandedMatrix final : public ArrayBasedCDSBandedMatrix<T>
{
private:
  // smart pointer is used to correctly deallocate memory obtained from aligned_alloc
  numeric::unique_aligned_buf_ptr m_buf;

public:
  CDSBandedMatrix(const numeric::TMatrixStorage storage = numeric::TMatrixStorage::RowMajor )
    : ArrayBasedCDSBandedMatrix<T>(storage)
  {}
  CDSBandedMatrix(const size_t nrows, const size_t ncolumns,
    const size_t upper_band, const size_t lower_band,
    const numeric::TMatrixStorage storage = numeric::TMatrixStorage::RowMajor )
      : ArrayBasedCDSBandedMatrix<T>(nrows, ncolumns, upper_band, lower_band, storage)
  {}
  ~CDSBandedMatrix();

  inline TMatrixType getBackendType() const override { return TMatrixType::Array; }

protected:
  void ensureAllocated() override;

private:
  // no copying and copy assignment allowed
  CDSBandedMatrix(const CDSBandedMatrix&) = delete;
  CDSBandedMatrix(const CDSBandedMatrix&&) = delete;
  CDSBandedMatrix& operator= (const CDSBandedMatrix&) = delete;
};

template<typename T> class ValArrayCDSBandedMatrix final : public ArrayBasedCDSBandedMatrix<T>
{
private:
  std::unique_ptr<std::valarray<T>> m_valarray;

public:
  ValArrayCDSBandedMatrix(const numeric::TMatrixStorage storage = numeric::TMatrixStorage::RowMajor )
    : ArrayBasedCDSBandedMatrix<T>(storage)
  {}
  ValArrayCDSBandedMatrix(const size_t nrows, const size_t ncolumns,
    const size_t upper_band, const size_t lower_band,
    const numeric::TMatrixStorage storage = numeric::TMatrixStorage::RowMajor )
      : ArrayBasedCDSBandedMatrix<T>(nrows, ncolumns, upper_band, lower_band, storage)
  {}
  ~ValArrayCDSBandedMatrix() {};

  inline TMatrixType getBackendType() const override { return TMatrixType::ValArray; }

  inline const std::valarray<T>& getValArray() const { return *(m_valarray.get()); }
  inline std::valarray<T>& getValArray() { return *(m_valarray.get()); }
  inline std::valarray<T>* getValArrayPtr() const { return m_valarray.get(); }

protected:
  void ensureAllocated() override;

private:
  // no copying and copy assignment allowed
  ValArrayCDSBandedMatrix(const ValArrayCDSBandedMatrix&) = delete;
  ValArrayCDSBandedMatrix(const ValArrayCDSBandedMatrix&&) = delete;
  ValArrayCDSBandedMatrix& operator= (const ValArrayCDSBandedMatrix&) = delete;
};


//helper functions to create matrices with corresponting type
inline MatrixBase* NewMatrix(const numeric::TPrecision p, const size_t nrows, const size_t ncolumns,
  const bool reset = true, const numeric::TMatrixStorage storage = numeric::TMatrixStorage::RowMajor,
  const TMatrixType type = TMatrixType::Array, const TMatrixFlavour flavour = TMatrixFlavour::Dense,
  const size_t upper_band = 0, const size_t lower_band = 0);
inline MatrixBase* NewMatrix(const numeric::TPrecision p, InFileText& f, const bool readData = true,
  const bool transpose = false, const numeric::TMatrixStorage storage = numeric::TMatrixStorage::RowMajor,
  const TMatrixType type = TMatrixType::Array, const TMatrixFlavour flavour = TMatrixFlavour::Dense);
inline MatrixBase* NewMatrix(const numeric::TPrecision p, const size_t nrows, const size_t ncolumns, InFileText& f,
  const bool transpose = true, const numeric::TMatrixStorage storage = numeric::TMatrixStorage::RowMajor,
  const TMatrixType type = TMatrixType::Array, const TMatrixFlavour flavour = TMatrixFlavour::Dense,
  const size_t upper_band = 0, const size_t lower_band = 0);

}

//implementation
#include "calcapp/math/matrix_impl.hpp"

#endif /* _MATRIX_HPP */
