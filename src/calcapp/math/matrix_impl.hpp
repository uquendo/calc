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

template<typename T> Matrix<T>::Matrix(const size_t nrows, const size_t ncolumns, const bool reset,
  const numeric::TMatrixStorage storage )
  : MatrixBase(nrows, ncolumns, storage)
  , m_buf(nullptr)
  , m_data(nullptr)
  , m_stride(( isRowMajor() ? ncolumns : nrows ))
{
//  static_assert(std::is_arithmetic<T>::value, "Arithmetical type expected"); //TODO: check boost::multiprecision type traits
  void* buf = nullptr;
  m_data = reinterpret_cast<T*>(numeric::aligned_malloc(sizeof(T)*getSize(), getAlignment(), &buf));
  m_buf.reset(buf);

  if(m_data == nullptr){
    throw OOMError("Can't allocate aligned data buffer for the matrix");
  }

  if(reset) {
    memset(m_buf.get(),0,sizeof(T)*getSize());
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
    //it's square matrix
    columns = rows;
  }
}

template<typename T> Matrix<T>::Matrix(InFileText& f, const bool readData,
  const bool transpose, const numeric::TMatrixStorage storage)
  : m_buf(nullptr)
  , m_data(nullptr)
{
//  static_assert(std::is_arithmetic<T>::value, "Arithmetical type expected"); //TODO: check boost::multiprecision type traits
  if(f.fileType() != FT_MatrixText)
    throw FileFormatUnsupportedError("File format unsupported",f.fileType(),f.fileName().c_str(),f.lineNum());

  size_t f_nrows=0,
         f_ncolumns=0;
  ParseHeaderDat(f,f_nrows,f_ncolumns);
  m_nrows = f_nrows;
  m_ncolumns = f_ncolumns;
  if(transpose)
    std::swap(m_nrows,m_ncolumns);
  m_stride = ( isRowMajor() ? m_ncolumns : m_nrows );

  if(readData) {
    void* buf=nullptr;
    m_data = reinterpret_cast<T*>(numeric::aligned_malloc(sizeof(T)*getSize(), getAlignment(), &buf));
    m_buf.reset(buf);
    if(m_data == nullptr) {
      throw OOMError("Can't allocate aligned data buffer for the matrix");
    }

    //no placement new call is required, data should be initialized when read in

    const size_t input_stride = ( ((isRowMajor() && transpose) || (!isRowMajor() && !transpose)) ? f_nrows  : 1 );
    const size_t input_inc = ( ((isRowMajor() && transpose) || (!isRowMajor() && !transpose)) ? 1 : f_ncolumns );
    for (size_t i = 0; i < f_nrows; ++i) {
      f.readNextLine_scanNumArray<T>(f_ncolumns, f_ncolumns, m_data+input_inc*i, input_stride);
    }
  }
}

template<typename T> void Matrix<T>::readFromFile(InFileText& f, const bool transpose)
{
  if(f.fileType() != FT_MatrixText)
    throw FileFormatUnsupportedError("File format unsupported",f.fileType(),f.fileName().c_str(),f.lineNum());

  size_t f_nrows = m_nrows,
         f_ncolumns = m_ncolumns;
  if(transpose)
     std::swap(f_nrows,f_ncolumns);

  if(f.lineNum()==0){
    //just opened file, check header
    size_t tmp_nrows=0,
           tmp_ncolumns=0;
    ParseHeaderDat(f,tmp_nrows,tmp_ncolumns);
    if( ( f_nrows != tmp_nrows ) || ( f_ncolumns != tmp_ncolumns ) ){
      throw FileFormatValueBoundsError("Mismatched size of matrix in file",f.fileType(),f.fileName().c_str(),f.lineNum());
    }
  }

  //allocate memory if necessary
  if( m_buf==nullptr || m_data==nullptr ){
    void* buf=nullptr;
    m_data = reinterpret_cast<T*>(numeric::aligned_malloc(sizeof(T)*getSize(), getAlignment(), &buf));
    m_buf.reset(buf);
    if(m_data == nullptr) {
      throw OOMError("Can't allocate aligned data buffer for the matrix");
    }
  }

  //no placement new call is required, data should be initialized when read in

  const size_t input_stride = ( ((isRowMajor() && transpose) || (!isRowMajor() && !transpose)) ? f_nrows  : 1 );
  const size_t input_inc = ( ((isRowMajor() && transpose) || (!isRowMajor() && !transpose)) ? 1 : f_ncolumns );
  for (size_t i = 0; i < f_nrows; ++i) {
    f.readNextLine_scanNumArray<T>(f_ncolumns, f_ncolumns, m_data+input_inc*i, input_stride);
  }
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
//data IO
void readFromFile(InFileText& f, const bool transpose);

struct CreateMatrixHelperArgs
{
  enum InitType { FixedSize, FromFile, FromFileFixedSize } m_InitType;
  size_t m_nrows, m_ncolumns;
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
        return dynamic_cast<MatrixBase*>(new Matrix<T>(a.m_nrows,a.m_ncolumns,a.m_reset,a.m_storage));
      case CreateMatrixHelperArgs::FromFile:
        return dynamic_cast<MatrixBase*>(new Matrix<T>(*a.m_pf,a.m_readData,a.m_transpose,a.m_storage));
      case CreateMatrixHelperArgs::FromFileFixedSize:
        return dynamic_cast<MatrixBase*>(new Matrix<T>(a.m_nrows,a.m_ncolumns,*a.m_pf,a.m_transpose,a.m_storage));
    }
    return nullptr;
  }
};

//helper functions to create matrices with corresponting type
inline MatrixBase* NewMatrix(const numeric::TPrecision p, const size_t nrows, const size_t ncolumns,
  const bool reset, const numeric::TMatrixStorage storage )
{
  CreateMatrixHelperArgs args;
  args.m_InitType = CreateMatrixHelperArgs::FixedSize;
  args.m_nrows = nrows; args.m_ncolumns = ncolumns; args.m_reset = reset; args.m_storage = storage;
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

inline MatrixBase* NewMatrix(const numeric::TPrecision p, const size_t nrows, const size_t ncolumns, InFileText* pf,
  const bool transpose, const numeric::TMatrixStorage storage)
{
  if(pf==nullptr)
    return nullptr;
  CreateMatrixHelperArgs args;
  args.m_InitType = CreateMatrixHelperArgs::FromFileFixedSize;
  args.m_nrows = nrows; args.m_ncolumns = ncolumns; args.m_pf = pf; args.m_transpose = transpose; args.m_storage = storage;
  return CreateMatrixHelperFunc<CreateMatrixHelperArgs::FromFileFixedSize>()(p,args);
}

}

