#include "calcapp/math/matrix.hpp"

#include "numeric/expand_traits.hpp"

namespace Calc
{

template<typename T>  Matrix<T>::~Matrix()
{
  //explicitly call destructor for types that require it
  if(std::is_class<T>::value)
  {
    size_t i;
    for (i = 0; i < getSize(); ++i) {
      m_data[i].~T();
    }
  }
  //no need to free allocated memory, m_buf will take care of it
}

template<typename T> Matrix<T>::Matrix(const size_t rows, const size_t columns, const bool reset,
  const numeric::TMatrixStorage storage )
  : MatrixBase(rows, columns, storage)
  , m_buf(nullptr)
  , m_data(nullptr)
  , m_data_packs(nullptr)
  , m_stride( isRowMajor() ? columns : rows )
{
//  static_assert(std::is_arithmetic<T>::value, "Arithmetical type expected"); //TODO: check boost::multiprecision type traits
  void* buf = nullptr;
  size_t buf_size = sizeof(pack_t)*getPackCount();
  m_data_packs = reinterpret_cast<pack_t*>(numeric::aligned_malloc(buf_size, getAlignment(), &buf));
  m_data = reinterpret_cast<T*>(m_data_packs);
  m_buf.reset(buf);

  if(m_data == nullptr) {
    throw OOMError("Can't allocate aligned data buffer for the matrix");
  }

  if(reset) {
    memset(m_buf.get(),0,buf_size);
  }

  //placement new to call constructor for types that require it
  constexpr bool initObjects = std::is_class<T>::value;
  if(initObjects)
  {
    size_t i;
    for (i = 0; i < getSize(); ++i) {
      new(m_data+i) T();
    }
  }

}

template<typename T> void Matrix<T>::ParseHeaderDat(InFileText& f, size_t& rows, size_t& columns)
{
  if(f.readNextLine_scan(1,"# %zu %zu",&rows,&columns) == 1)
  {
    //it's a square matrix
    columns = rows;
  }
}

template<typename T> Matrix<T>::Matrix(InFileText& f, const bool readData,
  const bool transpose, const numeric::TMatrixStorage storage)
  : MatrixBase(0,0,storage)
  , m_buf(nullptr)
  , m_data(nullptr)
  , m_data_packs(nullptr)
{
//  static_assert(std::is_arithmetic<T>::value, "Arithmetical type expected"); //TODO: check boost::multiprecision type traits
  if(f.fileType() != FT_MatrixText)
    throw FileFormatUnsupportedError("File format unsupported",f.fileType(),f.fileName().c_str(),f.lineNum());

  size_t f_rows=0,
         f_columns=0;
  ParseHeaderDat(f,f_rows,f_columns);
  m_rows = f_rows;
  m_columns = f_columns;
  if(transpose)
    std::swap(m_rows,m_columns);
  m_stride = ( isRowMajor() ? m_columns : m_rows );

  if(readData) {
    void* buf=nullptr;
    size_t buf_size = sizeof(pack_t)*getPackCount();
    m_data_packs = reinterpret_cast<pack_t*>(numeric::aligned_malloc(buf_size, getAlignment(), &buf));
    m_data = reinterpret_cast<T*>(m_data_packs);
    m_buf.reset(buf);
    if(m_data == nullptr) {
      throw OOMError("Can't allocate aligned data buffer for the matrix");
    }

    //no placement new call is required, data should be initialized when read in

    const size_t input_stride = ( ((isRowMajor() && transpose) || (!isRowMajor() && !transpose)) ? f_rows  : 1 );
    const size_t input_inc = ( ((isRowMajor() && transpose) || (!isRowMajor() && !transpose)) ? 1 : f_columns );
    for (size_t i = 0; i < f_rows; ++i) {
      f.readNextLine_scanNumArray<T>(f_columns, f_columns, m_data+input_inc*i, input_stride);
    }
  }
}

//data IO
template<typename T> void Matrix<T>::readFromFile(InFileText& f, const bool transpose)
{
  if(f.fileType() != FT_MatrixText)
    throw FileFormatUnsupportedError("File format unsupported",f.fileType(),f.fileName().c_str(),f.lineNum());

  size_t f_rows = m_rows,
         f_columns = m_columns;
  if(transpose)
     std::swap(f_rows,f_columns);

  if(f.lineNum()==0) {
    //just opened file, check header
    size_t tmp_rows=0,
           tmp_columns=0;
    ParseHeaderDat(f,tmp_rows,tmp_columns);
    if( ( f_rows != tmp_rows ) || ( f_columns != tmp_columns ) ) {
      throw FileFormatValueBoundsError("Mismatched size of matrix in file",f.fileType(),f.fileName().c_str(),f.lineNum());
    }
  }

  //allocate memory if necessary
  if( m_buf==nullptr || m_data==nullptr ) {
    void* buf=nullptr;
    size_t buf_size = sizeof(pack_t)*getPackCount();
    m_data_packs = reinterpret_cast<pack_t*>(numeric::aligned_malloc(buf_size, getAlignment(), &buf));
    m_data = reinterpret_cast<T*>(m_data_packs);
    m_buf.reset(buf);
    if(m_data == nullptr) {
      throw OOMError("Can't allocate aligned data buffer for the matrix");
    }
  }

  //no placement new call is required, data should be initialized when read in

  const size_t input_stride = ( ((isRowMajor() && transpose) || (!isRowMajor() && !transpose)) ? f_rows  : 1 );
  const size_t input_inc = ( ((isRowMajor() && transpose) || (!isRowMajor() && !transpose)) ? 1 : f_columns );
  for (size_t i = 0; i < f_rows; ++i) {
    f.readNextLine_scanNumArray<T>(f_columns, f_columns, m_data+input_inc*i, input_stride);
  }
}

template<typename T> void Matrix<T>::WriteHeaderDat(OutFileText& f, const size_t rows, const size_t columns)
{
  if(columns == rows)
  {
    //it's a square matrix
    f.printf("# %zu", rows);
  } else {
    f.printf("# %zu %zu", rows, columns);
  }
  f.println();
}

template<typename T> void Matrix<T>::writeToFile(OutFileText& f, const bool transpose, const int print_precision)
{
  if(f.fileType() != FT_MatrixText)
    throw FileFormatUnsupportedError("File format unsupported",f.fileType(),f.fileName().c_str(),f.lineNum());
  size_t f_rows=m_rows,
         f_columns=m_columns;
  if(transpose)
  {
    std::swap(f_rows,f_columns);
  }

  WriteHeaderDat(f, f_rows, f_columns);

  const size_t output_stride = ( ((isRowMajor() && transpose) || (!isRowMajor() && !transpose)) ? f_rows : 1 );
  const size_t output_inc = ( ((isRowMajor() && transpose) || (!isRowMajor() && !transpose)) ? 1 : f_columns );

  for (size_t i = 0; i < f_rows; ++i) {
    f.println_printNumArray(f_columns, m_data+output_inc*i, output_stride, print_precision);
  }
  f.flush();
}

// data access
template<typename T> inline T* MatrixBase::getDataPtr() const
{
  return dynamic_cast<const Matrix<T>&>(*this).getDataPtr();
}
template<typename T> inline numeric::aligned::raw_pack<T>* MatrixBase::getDataPacksPtr() const
{
  return dynamic_cast<const Matrix<T>&>(*this).getDataPacksPtr();
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

struct CreateMatrixHelperArgs
{
  enum InitType { FixedSize, FromFile, FromFileFixedSize } m_InitType;
  size_t m_rows, m_columns;
  bool m_reset, m_transpose, m_readData;
  InFileText* m_pf;
  numeric::TMatrixStorage m_storage;
};

template<CreateMatrixHelperArgs::InitType type> struct CreateMatrixHelperFunc :
  numeric::MPFuncBase<CreateMatrixHelperFunc<type>,CreateMatrixHelperArgs,MatrixBase*>
{
  template<typename T> MatrixBase* perform(const CreateMatrixHelperArgs& a)
  {
    switch(type)
    {
      case CreateMatrixHelperArgs::FixedSize:
        return dynamic_cast<MatrixBase*>(new Matrix<T>(a.m_rows,a.m_columns,a.m_reset,a.m_storage));
      case CreateMatrixHelperArgs::FromFile:
        return dynamic_cast<MatrixBase*>(new Matrix<T>(*a.m_pf,a.m_readData,a.m_transpose,a.m_storage));
      case CreateMatrixHelperArgs::FromFileFixedSize:
        return dynamic_cast<MatrixBase*>(new Matrix<T>(a.m_rows,a.m_columns,*a.m_pf,a.m_transpose,a.m_storage));
    }
    return nullptr;
  }
};

//helper functions to create matrices with corresponting type
inline MatrixBase* NewMatrix(const numeric::TPrecision p, const size_t rows, const size_t columns,
  const bool reset, const numeric::TMatrixStorage storage )
{
  CreateMatrixHelperArgs args;
  args.m_InitType = CreateMatrixHelperArgs::FixedSize;
  args.m_rows = rows; args.m_columns = columns; args.m_reset = reset; args.m_storage = storage;
  return CreateMatrixHelperFunc<CreateMatrixHelperArgs::FixedSize>()(p,args);
}

inline MatrixBase* NewMatrix(const numeric::TPrecision p, InFileText* pf, const bool readData,
  const bool transpose, const numeric::TMatrixStorage storage )
{
  if(pf==nullptr)
    return nullptr;
  CreateMatrixHelperArgs args;
  args.m_InitType = CreateMatrixHelperArgs::FromFile;
  args.m_pf = pf; args.m_readData = readData; args.m_transpose = transpose; args.m_storage = storage;
  return CreateMatrixHelperFunc<CreateMatrixHelperArgs::FromFile>()(p,args);
}

inline MatrixBase* NewMatrix(const numeric::TPrecision p, const size_t rows, const size_t columns, InFileText* pf,
  const bool transpose, const numeric::TMatrixStorage storage)
{
  if(pf==nullptr)
    return nullptr;
  CreateMatrixHelperArgs args;
  args.m_InitType = CreateMatrixHelperArgs::FromFileFixedSize;
  args.m_rows = rows; args.m_columns = columns; args.m_pf = pf; args.m_transpose = transpose; args.m_storage = storage;
  return CreateMatrixHelperFunc<CreateMatrixHelperArgs::FromFileFixedSize>()(p,args);
}

}

