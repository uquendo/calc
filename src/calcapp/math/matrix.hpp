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

using std::size_t;

namespace Calc
{

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

//helper functions to create matrices with corresponting type
inline MatrixBase* NewMatrix(const numeric::TPrecision p, const size_t nrows, const size_t ncolumns,
  const bool reset = true, const numeric::TMatrixStorage storage = numeric::TMatrixStorage::RowMajor,
  const bool useValArray = false);
inline MatrixBase* NewMatrix(const numeric::TPrecision p, InFileText& f, const bool readData = true,
  const bool transpose = false, const numeric::TMatrixStorage storage = numeric::TMatrixStorage::RowMajor,
  const bool useValArray = false);
inline MatrixBase* NewMatrix(const numeric::TPrecision p, const size_t nrows, const size_t ncolumns, InFileText& f,
  const bool transpose = true, const numeric::TMatrixStorage storage = numeric::TMatrixStorage::RowMajor,
  const bool useValArray = false);


}

//implementation
#include "calcapp/math/matrix_impl.hpp"

#endif /* _MATRIX_HPP */
