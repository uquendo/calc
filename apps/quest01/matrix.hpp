#pragma once
#ifndef _MATRIX_HPP
#define _MATRIX_HPP
#include "config.h"

#include <memory>
#include <type_traits>
#include <cstdlib>
#include <cstring>

#include "numeric/blas_impl.hpp"
#include "numeric/cache.hpp"
#include "calcapp/exception.hpp"

using std::size_t;

namespace Calc
{

class MatrixBase
{
public:
  static constexpr size_t ALIGNMENT = 32;
  const numeric::TMatrixStorage m_storage;
  const size_t m_nrows;
  const size_t m_ncolumns;

  MatrixBase(const size_t nrows, const size_t ncolumns, const numeric::TMatrixStorage storage=numeric::TMatrixStorage::RowMajor );
  virtual ~MatrixBase() {}

  inline size_t getSize() const { return m_nrows*m_ncolumns; }
  inline size_t index(size_t i, size_t j) const
    { return (m_storage == numeric::TMatrixStorage::RowMajor ? i*m_ncolumns+j : j*m_nrows+i ); }

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
};

template<typename T> class Matrix : MatrixBase
{
private:
  T* m_data;
  char* m_buf;
  size_t m_bytesize;
public:

  static constexpr size_t getAlignment()
    { return ( std::alignment_of<T>::value < ALIGNMENT ? ALIGNMENT : std::alignment_of<T>::value ) ; }
  const numeric::TMatrixStorage m_storage;
  const size_t m_stride;

  Matrix(const size_t nrows, const size_t ncolumns, const bool reset=true,
    const numeric::TMatrixStorage storage=numeric::TMatrixStorage::RowMajor );
  ~Matrix();

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

private:
  // no copying and copy assignment allowed
  Matrix(const Matrix&);
  Matrix(const Matrix&&);
  Matrix& operator= (const Matrix&);
};

template<typename T> Matrix<T>::Matrix(const size_t nrows, const size_t ncolumns, const bool reset,
  const numeric::TMatrixStorage storage ) :
    MatrixBase(nrows, ncolumns, storage)
  , m_stride((storage == numeric::TMatrixStorage::RowMajor ? ncolumns : nrows ))
  , m_bytesize(sizeof(T)*m_ncolumns*m_nrows+getAlignment())
  , m_buf(nullptr)
  , m_data(nullptr)
{
  m_buf = (char*) malloc(m_bytesize);
  if(reset)
    memset(m_buf,0,m_bytesize);
  m_data = reinterpret_cast<T*>(numeric::align(getAlignment(), sizeof(T)*m_ncolumns*m_nrows, m_buf, m_bytesize));
  if(m_data == nullptr){
    free(m_buf);
    throw InternalError(FERR_GENERAL_ERROR, "Internal error: Can't align data storage for matrix");
  }
}

template<typename T> Matrix<T>::~Matrix()
{
  free(m_buf);
  m_buf = nullptr;
  m_data = nullptr;
}

// data access
template<typename T> inline T* MatrixBase::getDataPtr() const
{
  return dynamic_cast<const Matrix<T>&>(*this).getDataPtr();
}
template<typename T> inline T MatrixBase::get(size_t i, size_t j) const
{
  return dynamic_cast<const Matrix<T>&>(*this).get(i,j);
}
template<typename T> inline T& MatrixBase::get(size_t i,size_t j)
{
  return dynamic_cast<Matrix<T>&>(*this).get(i,j);
}
template<typename T> inline T MatrixBase::get(size_t idx) const
{
  return dynamic_cast<const Matrix<T>&>(*this).get(idx);
}
template<typename T> inline T& MatrixBase::operator()(size_t i, size_t j)
{
  return dynamic_cast<Matrix<T>&>(*this)(i,j);
}
template<typename T> inline T MatrixBase::operator()(size_t i, size_t j) const
{
  return dynamic_cast<const Matrix<T>&>(*this)(i,j);
}
template<typename T> inline T& MatrixBase::operator()(size_t idx)
{
  return dynamic_cast<Matrix<T>&>(*this)(idx);
}
template<typename T> inline T MatrixBase::operator()(size_t idx) const
{
  return dynamic_cast<const Matrix<T>&>(*this)(idx);
}
template<typename T> inline T& MatrixBase::operator[](size_t idx)
{
  return dynamic_cast<Matrix<T>&>(*this)[idx];
}
template<typename T> inline const T MatrixBase::operator[](size_t idx) const
{
  return dynamic_cast<const Matrix<T>&>(*this)[idx];
}

}

#endif /* _MATRIX_HPP */
