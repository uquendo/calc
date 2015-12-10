#include "calcapp/math/matrix.hpp"

#include "numeric/expand_traits.hpp"

namespace Calc
{

template<typename T> void ArrayBasedMatrix<T>::init(const bool reset, const bool initObjects)
{
//  static_assert(std::is_arithmetic<T>::value, "Arithmetical type expected"); //TODO: check boost::multiprecision type traits
  ensureAllocated();

  if(reset)
    memset(reinterpret_cast<void*>(getDataPtr()),0,sizeof(T)*getSize());

  //placement new to call constructor for types that require it
  constexpr bool holdsObjects = std::is_class<T>::value;
  if(holdsObjects && initObjects)
  {
    size_t i;
    for (i = 0; i < getSize(); ++i) {
      new(getDataPtr()+i) T();
    }
  }
}

template<typename T> void ArrayBasedMatrix<T>::ParseHeaderDat(InFileText& f, size_t& rows, size_t& columns)
{
  if(f.readNextLine_scan(1,"# %zu %zu",&rows,&columns) == 1)
  {
    //it's a square matrix
    columns = rows;
  }
}

template<typename T> void ArrayBasedMatrix<T>::init(InFileText& f, const bool readData,
  const bool transpose)
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
    ensureAllocated();

    //no placement new call is required, data should be initialized when read in
    const size_t input_stride = ( ((isRowMajor() && transpose) || (!isRowMajor() && !transpose)) ? f_nrows  : 1 );
    const size_t input_inc = ( ((isRowMajor() && transpose) || (!isRowMajor() && !transpose)) ? 1 : f_ncolumns );
    for (size_t i = 0; i < f_nrows; ++i) {
      f.readNextLine_scanNumArray<T>(f_ncolumns, f_ncolumns, getDataPtr()+input_inc*i, input_stride);
    }
  }
}

//data IO
template<typename T> void ArrayBasedMatrix<T>::readFromFile(InFileText& f, const bool transpose)
{
  if(f.fileType() != FT_MatrixText)
    throw FileFormatUnsupportedError("File format unsupported",f.fileType(),f.fileName().c_str(),f.lineNum());

  size_t f_nrows = m_nrows,
         f_ncolumns = m_ncolumns;
  if(transpose)
     std::swap(f_nrows,f_ncolumns);

  if(f.lineNum()==0) {
    //just opened file, check header
    size_t tmp_nrows=0,
           tmp_ncolumns=0;
    ParseHeaderDat(f,tmp_nrows,tmp_ncolumns);
    if( ( f_nrows != tmp_nrows ) || ( f_ncolumns != tmp_ncolumns ) ) {
      throw FileFormatValueBoundsError("Mismatched size of matrix in file",f.fileType(),f.fileName().c_str(),f.lineNum());
    }
  }

  //allocate memory if necessary
  ensureAllocated();

  //no placement new call is required, data should be initialized when read in
  const size_t input_stride = ( ((isRowMajor() && transpose) || (!isRowMajor() && !transpose)) ? f_nrows  : 1 );
  const size_t input_inc = ( ((isRowMajor() && transpose) || (!isRowMajor() && !transpose)) ? 1 : f_ncolumns );
  for (size_t i = 0; i < f_nrows; ++i) {
    f.readNextLine_scanNumArray<T>(f_ncolumns, f_ncolumns, getDataPtr()+input_inc*i, input_stride);
  }
}

template<typename T> void ArrayBasedMatrix<T>::WriteHeaderDat(OutFileText& f, const size_t rows, const size_t columns)
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

template<typename T> void ArrayBasedMatrix<T>::writeToFile(OutFileText& f, const bool transpose, const int print_precision)
{
  if(f.fileType() != FT_MatrixText)
    throw FileFormatUnsupportedError("File format unsupported",f.fileType(),f.fileName().c_str(),f.lineNum());
  size_t f_nrows=m_nrows,
         f_ncolumns=m_ncolumns;
  if(transpose)
  {
    std::swap(f_nrows,f_ncolumns);
  }

  WriteHeaderDat(f, f_nrows, f_ncolumns);

  const size_t output_stride = ( ((isRowMajor() && transpose) || (!isRowMajor() && !transpose)) ? f_nrows : 1 );
  const size_t output_inc = ( ((isRowMajor() && transpose) || (!isRowMajor() && !transpose)) ? 1 : f_ncolumns );

  for (size_t i = 0; i < f_nrows; ++i) {
    f.println_printNumArray(f_ncolumns, getDataPtr()+output_inc*i, output_stride, print_precision);
  }
  f.flush();
}

template<typename T>  Matrix<T>::~Matrix()
{
  //explicitly call destructor for types that require it
  if(std::is_class<T>::value)
  {
    size_t i;
    for (i = 0; i < MatrixBase::getSize(); ++i) {
      ArrayBasedMatrix<T>::m_data[i].~T();
    }
  }
  //no need to free allocated memory, m_buf will take care of it
}

template<typename T> inline void Matrix<T>::ensureAllocated()
{
  if( m_buf==nullptr || ArrayBasedMatrix<T>::m_data==nullptr )
  {
    void* buf=nullptr;
    ArrayBasedMatrix<T>::m_data = reinterpret_cast<T*>(
        numeric::aligned_malloc(sizeof(T)*MatrixBase::getSize(), ArrayBasedMatrix<T>::getAlignment(), &buf)
        );
    m_buf.reset(buf);
    if(ArrayBasedMatrix<T>::m_data == nullptr) {
      throw OOMError("Can't allocate aligned data buffer for the matrix");
    }
  }
}

template<typename T> inline void ValArrayMatrix<T>::ensureAllocated()
{
  if( m_valarray == nullptr || ArrayBasedMatrix<T>::m_data == nullptr )
  {
    m_valarray.reset(new std::valarray<T>(MatrixBase::getSize()));
    ArrayBasedMatrix<T>::m_data = &(*(m_valarray.get()))[0];
  }
}

#ifdef HAVE_BOOST_UBLAS
template<typename T, numeric::TMatrixStorage storage> inline void BoostUblasMatrix<T,storage>::ensureAllocated()
{
  if( m_boost_matrix == nullptr || ArrayBasedMatrix<T>::m_data == nullptr)
  {
    using namespace boost::numeric::ublas;
    m_boost_matrix.reset(new matrix<T,typename TraitStorage<storage>::boost_type>(MatrixBase::getRowsNum(), MatrixBase::getColumnsNum()));
    ArrayBasedMatrix<T>::m_data = &((m_boost_matrix->data())[0]);
  }
}
#endif

#ifdef HAVE_EIGEN
template<typename T, numeric::TMatrixStorage storage> inline void EigenMatrix<T,storage>::ensureAllocated()
{
  if( m_eigen_matrix == nullptr || ArrayBasedMatrix<T>::m_data == nullptr)
  {
    m_eigen_matrix.reset(new 
      Eigen::Matrix<T,Eigen::Dynamic,Eigen::Dynamic,TraitStorage<storage>::eigen_type>(
        MatrixBase::getRowsNum(), MatrixBase::getColumnsNum()));
    ArrayBasedMatrix<T>::m_data = &((m_eigen_matrix->data())[0]);
  }
}
#endif

#ifdef HAVE_MTL
template<typename T, numeric::TMatrixStorage storage> inline void MTLMatrix<T,storage>::ensureAllocated()
{
  if( m_mtl_matrix == nullptr || ArrayBasedMatrix<T>::m_data == nullptr)
  {
    using namespace mtl;
    m_mtl_matrix.reset(new mat::dense2D<T,mat::parameters<typename TraitStorage<storage>::mtl_type> >(MatrixBase::getRowsNum(), MatrixBase::getColumnsNum()));
    ArrayBasedMatrix<T>::m_data = &((m_mtl_matrix->address_data())[0]);
  }
}
#endif

#ifdef HAVE_ARMADILLO
template<typename T> inline void ArmadilloMatrix<T>::ensureAllocated()
{
  if( m_armadillo_matrix == nullptr || m_buf == nullptr || ArrayBasedMatrix<T>::m_data == nullptr)
  {
    void* buf=nullptr;
    ArrayBasedMatrix<T>::m_data = reinterpret_cast<T*>(
        numeric::aligned_malloc(sizeof(T)*MatrixBase::getSize(), ArrayBasedMatrix<T>::getAlignment(), &buf)
        );
    m_buf.reset(buf);
    if(ArrayBasedMatrix<T>::m_data == nullptr) {
      throw OOMError("Can't allocate aligned data buffer for the matrix");
    }
    if(!MatrixBase::isRowMajor())
      m_armadillo_matrix.reset(new arma::Mat<T>(ArrayBasedMatrix<T>::m_data, MatrixBase::getRowsNum(), MatrixBase::getColumnsNum(), false, true));
    else
      throw ParameterError("Armadillo doesn't support row major stored matrices");
  }
}

template<typename T>  ArmadilloMatrix<T>::~ArmadilloMatrix()
{
  //explicitly call destructor for types that require it
  if(std::is_class<T>::value)
  {
    size_t i;
    for (i = 0; i < MatrixBase::getSize(); ++i) {
      ArrayBasedMatrix<T>::m_data[i].~T();
    }
  }
  //no need to free allocated memory, m_buf will take care of it
}
#endif

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

struct CreateMatrixHelperArgs
{
  enum InitType { FixedSize, FromFile, FromFileFixedSize } m_InitType;
  size_t m_nrows, m_ncolumns;
  bool m_reset, m_transpose, m_readData;
  InFileText* m_pf;
  numeric::TMatrixStorage m_storage;
  TMatrixType m_type;
};

template<CreateMatrixHelperArgs::InitType type> struct CreateMatrixHelperFunc :
  numeric::MPFuncBase<CreateMatrixHelperFunc<type>,CreateMatrixHelperArgs,MatrixBase*>
{
  template<typename T> MatrixBase* perform(const CreateMatrixHelperArgs& a)
  {
    ArrayBasedMatrix<T>* p = nullptr;
    switch(type)
    {
      case CreateMatrixHelperArgs::FixedSize:
      case CreateMatrixHelperArgs::FromFileFixedSize:
        switch(a.m_type)
        {
#ifdef HAVE_BOOST_UBLAS
          case TMatrixType::BoostUblas:
            if(a.m_storage == numeric::TMatrixStorage::RowMajor)
              p = dynamic_cast<ArrayBasedMatrix<T>*>(new BoostUblasMatrix<T,numeric::TMatrixStorage::RowMajor>(a.m_nrows,a.m_ncolumns)) ;
            else
              p = dynamic_cast<ArrayBasedMatrix<T>*>(new BoostUblasMatrix<T,numeric::TMatrixStorage::ColumnMajor>(a.m_nrows,a.m_ncolumns)) ;
            break;
#endif
#ifdef HAVE_EIGEN
          case TMatrixType::Eigen:
            if(a.m_storage == numeric::TMatrixStorage::RowMajor)
              p = dynamic_cast<ArrayBasedMatrix<T>*>(new EigenMatrix<T,numeric::TMatrixStorage::RowMajor>(a.m_nrows,a.m_ncolumns)) ;
            else
              p = dynamic_cast<ArrayBasedMatrix<T>*>(new EigenMatrix<T,numeric::TMatrixStorage::ColumnMajor>(a.m_nrows,a.m_ncolumns)) ;
            break;
#endif
#ifdef HAVE_MTL
          case TMatrixType::MTL:
            if(a.m_storage == numeric::TMatrixStorage::RowMajor)
              p = dynamic_cast<ArrayBasedMatrix<T>*>(new MTLMatrix<T,numeric::TMatrixStorage::RowMajor>(a.m_nrows,a.m_ncolumns)) ;
            else
              p = dynamic_cast<ArrayBasedMatrix<T>*>(new MTLMatrix<T,numeric::TMatrixStorage::ColumnMajor>(a.m_nrows,a.m_ncolumns)) ;
            break;
#endif
#ifdef HAVE_ARMADILLO
          case TMatrixType::Armadillo:
            if(a.m_storage == numeric::TMatrixStorage::RowMajor)
              throw ParameterError("Armadillo doesn't support row major stored matrices");
            else
              p = dynamic_cast<ArrayBasedMatrix<T>*>(new ArmadilloMatrix<T>(a.m_nrows,a.m_ncolumns)) ;
            break;
#endif
          case TMatrixType::ValArray:
            p = dynamic_cast<ArrayBasedMatrix<T>*>(new ValArrayMatrix<T>(a.m_nrows,a.m_ncolumns,a.m_storage)) ;
            break;
          case TMatrixType::Array:
            p = dynamic_cast<ArrayBasedMatrix<T>*>(new Matrix<T>(a.m_nrows,a.m_ncolumns,a.m_storage)) ;
            break;
          default:
            throw ParameterError("matrix type unsupported");
        }
        if(type == CreateMatrixHelperArgs::FixedSize)
          p -> init(a.m_reset, a.m_type == TMatrixType::Array || a.m_type == TMatrixType::Armadillo);
        else
          p -> init(*a.m_pf,a.m_readData,a.m_transpose);
        break;

      case CreateMatrixHelperArgs::FromFile:
        switch(a.m_type)
        {
#ifdef HAVE_BOOST_UBLAS
          case TMatrixType::BoostUblas:
            if(a.m_storage == numeric::TMatrixStorage::RowMajor)
              p = dynamic_cast<ArrayBasedMatrix<T>*>(new BoostUblasMatrix<T,numeric::TMatrixStorage::RowMajor>()) ;
            else
              p = dynamic_cast<ArrayBasedMatrix<T>*>(new BoostUblasMatrix<T,numeric::TMatrixStorage::ColumnMajor>()) ;
            break;
#endif
#ifdef HAVE_EIGEN
          case TMatrixType::Eigen:
            if(a.m_storage == numeric::TMatrixStorage::RowMajor)
              p = dynamic_cast<ArrayBasedMatrix<T>*>(new EigenMatrix<T,numeric::TMatrixStorage::RowMajor>()) ;
            else
              p = dynamic_cast<ArrayBasedMatrix<T>*>(new EigenMatrix<T,numeric::TMatrixStorage::ColumnMajor>()) ;
            break;
#endif
#ifdef HAVE_MTL
          case TMatrixType::MTL:
            if(a.m_storage == numeric::TMatrixStorage::RowMajor)
              p = dynamic_cast<ArrayBasedMatrix<T>*>(new MTLMatrix<T,numeric::TMatrixStorage::RowMajor>()) ;
            else
              p = dynamic_cast<ArrayBasedMatrix<T>*>(new MTLMatrix<T,numeric::TMatrixStorage::ColumnMajor>()) ;
            break;
#endif
#ifdef HAVE_ARMADILLO
          case TMatrixType::Armadillo:
            if(a.m_storage == numeric::TMatrixStorage::RowMajor)
              throw ParameterError("Armadillo doesn't support row major stored matrices");
            else
              p = dynamic_cast<ArrayBasedMatrix<T>*>(new ArmadilloMatrix<T>()) ;
            break;
#endif
          case TMatrixType::ValArray:
            p = dynamic_cast<ArrayBasedMatrix<T>*>(new ValArrayMatrix<T>(a.m_storage)) ;
            break;
          case TMatrixType::Array:
            p = dynamic_cast<ArrayBasedMatrix<T>*>(new Matrix<T>(a.m_storage)) ;
            break;
          default:
            throw ParameterError("matrix type unsupported");
        }
        p -> init(*a.m_pf,a.m_readData,a.m_transpose);
        break;
      }
    return p;
  }
};

//helper functions to create matrices with corresponding type
inline MatrixBase* NewMatrix(const numeric::TPrecision p, const size_t nrows, const size_t ncolumns,
  const bool reset, const numeric::TMatrixStorage storage, const TMatrixType type)
{
  CreateMatrixHelperArgs args;
  args.m_InitType = CreateMatrixHelperArgs::FixedSize;
  args.m_nrows = nrows; args.m_ncolumns = ncolumns; args.m_reset = reset; args.m_storage = storage;
  args.m_type = type;
  return CreateMatrixHelperFunc<CreateMatrixHelperArgs::FixedSize>()(p,args);
}

inline MatrixBase* NewMatrix(const numeric::TPrecision p, InFileText* pf, const bool readData,
  const bool transpose, const numeric::TMatrixStorage storage,  const TMatrixType type)
{
  if(pf==nullptr)
    return nullptr;
  CreateMatrixHelperArgs args;
  args.m_InitType = CreateMatrixHelperArgs::FromFile;
  args.m_pf = pf; args.m_readData = readData; args.m_transpose = transpose; args.m_storage = storage;
  args.m_type = type;
  return CreateMatrixHelperFunc<CreateMatrixHelperArgs::FromFile>()(p,args);
}

inline MatrixBase* NewMatrix(const numeric::TPrecision p, const size_t nrows, const size_t ncolumns, InFileText* pf,
  const bool transpose, const numeric::TMatrixStorage storage, const TMatrixType type)
{
  if(pf==nullptr)
    return nullptr;
  CreateMatrixHelperArgs args;
  args.m_InitType = CreateMatrixHelperArgs::FromFileFixedSize;
  args.m_nrows = nrows; args.m_ncolumns = ncolumns; args.m_pf = pf; args.m_transpose = transpose; args.m_storage = storage;
  args.m_type = type;
  return CreateMatrixHelperFunc<CreateMatrixHelperArgs::FromFileFixedSize>()(p,args);
}

}

