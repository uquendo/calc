#pragma once
#ifndef _MATRIX_HPP
#define _MATRIX_HPP
#include "config.h"

#include <memory>
#include <type_traits>
#include <cstdlib>
#include <cstring>
#include <algorithm>

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
  MatrixBase(const numeric::TMatrixStorage storage=numeric::TMatrixStorage::RowMajor ):
      m_rows(0), m_columns(0), m_storage(storage)
  {}
  MatrixBase(const size_t rows, const size_t columns,
    const numeric::TMatrixStorage storage=numeric::TMatrixStorage::RowMajor ):
      m_rows(rows), m_columns(columns), m_storage(storage)
  {}

  size_t m_rows;
  size_t m_columns;
  const numeric::TMatrixStorage m_storage;

public:

  virtual ~MatrixBase() {}
  inline numeric::TMatrixStorage getMatrixStorageType() const { return m_storage; }
  inline bool isRowMajor() const { return (m_storage == numeric::TMatrixStorage::RowMajor); }
  inline size_t getSize() const { return m_rows*m_columns; }
  inline size_t getRowsNum() const { return m_rows; }
  inline size_t getColumnsNum() const { return m_columns; }
  inline size_t index(size_t i, size_t j) const { return ( isRowMajor() ? i*m_columns+j : j*m_rows+i ); }

  // data access
  template<typename T> inline T* getDataPtr() const;
  template<typename T> inline numeric::aligned::raw_pack<T>* getDataPacksPtr() const;
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
};

template<typename T> class Matrix : public MatrixBase
{
private:
  typedef numeric::aligned::raw_pack<T> pack_t;
  numeric::unique_aligned_buf_ptr m_buf; // smart pointer is used to correctly deallocate memory obtained from aligned_alloc
  T* m_data;
  pack_t* m_data_packs;

protected:
  size_t m_stride;

public:
  Matrix(const size_t rows, const size_t columns, const bool reset = true,
    const numeric::TMatrixStorage storage = numeric::TMatrixStorage::RowMajor );
  Matrix(InFileText& f, const bool readData = true, const bool transpose = false,
    const numeric::TMatrixStorage storage = numeric::TMatrixStorage::RowMajor );
  Matrix(const size_t rows, const size_t columns, InFileText& f, const bool transpose = true,
    const numeric::TMatrixStorage storage = numeric::TMatrixStorage::RowMajor );
  ~Matrix();

  //utility functions
  static constexpr size_t getPackSize() { return numeric::aligned::pack_size<T>(); }
  size_t getPackCount() const { return getSize() / getPackSize() + ( getSize() % getPackSize() == 0 ? 0 : 1 ); }
  static constexpr size_t getPackAlignment() { return numeric::getDefaultAlignment<T>(); }
  static size_t getAlignment() { return numeric::getCacheLineAlignment<T>(); }

  // data access
  inline T* getDataPtr() const { return m_data; }
  inline pack_t* getDataPacksPtr() const { return m_data_packs; }
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

private:
  void ParseHeaderDat(InFileText& f, size_t& rows, size_t& columns);
  void WriteHeaderDat(OutFileText& f, const size_t rows, const size_t columns);

private:
  // no copying and copy assignment allowed
  Matrix(const Matrix&);
  Matrix(const Matrix&&);
  Matrix& operator= (const Matrix&);
};

//helper functions to create matrices with corresponting type
inline MatrixBase* NewMatrix(const numeric::TPrecision p, const size_t rows, const size_t columns,
  const bool reset = true, const numeric::TMatrixStorage storage = numeric::TMatrixStorage::RowMajor );
inline MatrixBase* NewMatrix(const numeric::TPrecision p, InFileText& f, const bool readData = true,
  const bool transpose = false, const numeric::TMatrixStorage storage = numeric::TMatrixStorage::RowMajor);
inline MatrixBase* NewMatrix(const numeric::TPrecision p, const size_t rows, const size_t columns, InFileText& f,
  const bool transpose = true, const numeric::TMatrixStorage storage = numeric::TMatrixStorage::RowMajor );

}

//implementation
#include "calcapp/math/matrix_impl.hpp"

#endif /* _MATRIX_HPP */
