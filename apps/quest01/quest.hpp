#pragma once
#ifndef _QUEST_HPP
#define _QUEST_HPP

#include <string>
#include <unordered_set>

#include "calcapp/cli.hpp"
#include "calcapp/io.hpp"
#include "matrix.hpp"

#include "appconfig.h"

namespace Calc {

enum TAlgo {
  A_NumCpp=0,
  A_NumC,
  A_NumFortran,
  A_NumCppSimple,
  A_NumCSimple,
  A_NumFortranSimple,
  A_NumCppSimpleTranspose,
  A_NumCSimpleTranspose,
  A_NumFortranSimpleTranspose,
  A_NumCppStrassen,
  A_NumCStrassen,
  A_NumFortranStrassen,
  A_NumFortranInternal,
  A_ExtCppBoost,
  A_ExtCppEigen,
  A_ExtCppMTL,
  A_ExtCppArmadillo,
  A_ExtCBLAS,
  A_Undefined
};

static const OptName<TAlgo> _algo_opt_names[] = {
  { "numeric-cpp", "num-cpp", A_NumCpp },
  { "numeric-c", "num-c", A_NumC },
  { "numeric-fortran", "num-f", A_NumFortran },
  { "numeric-cpp-simple", "num-cpp-s", A_NumCppSimple },
  { "numeric-c-simple", "num-c-s", A_NumCSimple },
  { "numeric-fortran-simple", "num-f-s", A_NumFortranSimple },
  { "numeric-cpp-simple-transpose", "num-cpp-st", A_NumCppSimpleTranspose },
  { "numeric-c-simple-transpose", "num-c-st", A_NumCSimpleTranspose },
  { "numeric-fortran-simple-transpose", "num-f-st", A_NumFortranSimpleTranspose },
  { "numeric-cpp-strassen", "num-cpp-sn", A_NumCppStrassen },
  { "numeric-c-strassen", "num-c-sn", A_NumCStrassen },
  { "numeric-fortran-strassen", "num-f-sn", A_NumFortranStrassen },
  { "numeric-fortran-internal", "num-f-int", A_NumFortranInternal },
  //TODO: clean up macro hell 
#ifdef HAVE_BOOST_UBLAS
  { "contrib-cpp-boost.ublas", "ext-cpp-boost", A_ExtCppBoost },
#endif
#ifdef HAVE_EIGEN
  { "contrib-cpp-eigen", "ext-cpp-eigen", A_ExtCppEigen },
#endif
#ifdef HAVE_MTL
  { "contrib-cpp-mtl", "ext-cpp-mtl", A_ExtCppMTL },
#endif
#ifdef HAVE_ARMADILLO
  { "contrib-cpp-armadillo", "ext-cpp-arm", A_ExtCppArmadillo },
#endif
#ifdef HAVE_BLAS
  { "contrib-c-blas", "ext-c-blas", A_ExtCBLAS },
#endif
  { NULL, 0, A_Undefined }
};

static const OptName<TFileType> _input_opt_names[] = {
  { "Read matrices in .dat format", "dat", FT_MatrixText },
  { "Read matrices in .csv format", "csv", FT_Csv },
  { NULL, 0, FT_None }
};

static const OptName<TFileType> _output_opt_names[] = {
  { "Write matrices in .dat format", "dat", FT_MatrixText },
  { "Write matrices in .csv format", "csv", FT_Csv },
  { NULL, 0, FT_None }
};

struct InputOptions {
  TFileType filetype;
  std::string filename_A;
  std::string filename_B;

  InputOptions():
    filetype(FT_Undefined)
    ,filename_A("")
    ,filename_B("")
  {}
};

struct OutputOptions {
  TFileType filetype;
  std::string filename;

  OutputOptions():
    filetype(FT_Undefined)
    ,filename("")
  {}
};

struct AlgoOptions {
  TAlgo type;

  AlgoOptions():
    type(A_Undefined)
  {}
};

namespace matmul{
  struct AlgoParameters {
    const Calc::ThreadingOptions Topt;
    const Calc::PrecisionOptions Popt;
    const Calc::AlgoOptions Aopt;
    std::unique_ptr<MatrixBase> a;
    std::unique_ptr<MatrixBase> b;
    std::unique_ptr<MatrixBase> c;
    bool transposeA;
    bool transposeB;
    numeric::TMatrixStorage storage;
    size_t nrows_C;
    size_t ncolumns_C;
  };
}

class QuestAppOptions : public CliAppOptions {
public:
    QuestAppOptions();
    virtual ~QuestAppOptions() {};
    virtual bool processOptions(int argc, char* argv[]) override;
    virtual const std::string About() const override;
    inline const InputOptions& getInOpts() const { return m_input; };
    inline const OutputOptions& getOutOpts() const { return m_output; };
    inline const AlgoOptions& getAlgoOpts() const { return m_algo; };
protected:
    //prepare cli options
    virtual void prepareOptions() override;
    virtual void prepareInputOptions() override;
    virtual void prepareOutputOptions() override;
    virtual void prepareAlgoOptions() override;
    //parse cli options
    virtual bool parseOptions(int argc, char* argv[]) override;
    virtual bool parseInputOptions() override;
    virtual bool parseOutputOptions() override;
    virtual bool parseAlgoOptions() override;
protected:
    std::string inputHelp;
    std::string outputHelp;
    std::string algoHelp;
    InputOptions m_input;
    OutputOptions m_output;
    AlgoOptions m_algo; 
};

class QuestApp : public CliApp {
private:
    QuestApp();
public:
    QuestApp(const QuestAppOptions&);
    QuestApp(const QuestAppOptions&, ProgressCtrl* pc);
    QuestApp(ProgressCtrl* pc);
    virtual ~QuestApp(){};
    virtual void setDefaultOptions() override;
    virtual void readInput() override;
    virtual void run() override;
private:
    InputOptions m_input;
    OutputOptions m_output;
    AlgoOptions m_algo;
    std::unique_ptr<matmul::AlgoParameters> m_pAlgoParameters;
    std::unique_ptr<InFileText> m_pfA;
    std::unique_ptr<InFileText> m_pfB;
    std::unique_ptr<OutFileText> m_pfC;
};

}

#endif /* _QUEST_HPP */
