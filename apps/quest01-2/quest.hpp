#pragma once
#ifndef _QUEST_HPP
#define _QUEST_HPP
#include "config.h"
#include "appconfig.h"

#include <string>
#include <unordered_set>

#include "calcapp/cli.hpp"
#include "calcapp/io.hpp"
#include "calcapp/math/matrix.hpp"

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
  A_NumCppMRK,
  A_NumCMRK,
  A_NumFortranMRK,
  A_NumCppValarray,
  A_NumCppValarrayTranspose,
  A_NumCppValarrayMRK,
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
  { "numeric-cpp-valarray", "num-cpp-v", A_NumCppValarray },
  { "numeric-cpp-valarray-transpose", "num-cpp-vt", A_NumCppValarrayTranspose },
  { "numeric-cpp-mrk", "num-cpp-mrk", A_NumCppMRK },
  { "numeric-c-mrk", "num-c-mrk", A_NumCMRK },
  { "numeric-fortran-mrk", "num-f-mrk", A_NumFortranMRK },
  { "numeric-cpp-valarray-mrk", "num-cpp-v-mrk", A_NumCppValarrayMRK },
  { nullptr, nullptr, A_Undefined }
};

static const OptName<TFileType> _input_opt_names[] = {
  { "Read matrices in .dat format", "dat", FT_MatrixText },
  { "Read matrices in .csv format", "csv", FT_Csv },
  { nullptr, nullptr, FT_None }
};

static const OptName<TFileType> _output_opt_names[] = {
  { "Write matrices in .dat format", "dat", FT_MatrixText },
  { "Write matrices in .csv format", "csv", FT_Csv },
  { nullptr, nullptr, FT_None }
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
    size_t nrows_C;
    size_t ncolumns_C;
    bool transposeA;
    bool transposeB;
    numeric::TMatrixStorage storage;
    bool initCsize();
  };
}

class QuestAppOptions : public CliAppOptions {
public:
    QuestAppOptions();
    virtual ~QuestAppOptions() {};
    bool processOptions(int argc, char* argv[]) override;
    const std::string About() const override;
    const std::string Help() const override;
    inline const InputOptions& getInOpts() const { return m_input; };
    inline const OutputOptions& getOutOpts() const { return m_output; };
    inline const AlgoOptions& getAlgoOpts() const { return m_algo; };
protected:
    //prepare cli options
    void prepareOptions() override;
    void prepareInputOptions() override;
    void prepareOutputOptions() override;
    void prepareAlgoOptions() override;
    //parse cli options
    bool parseOptions(int argc, char* argv[]) override;
    bool parseInputOptions() override;
    bool parseOutputOptions() override;
    bool parseAlgoOptions() override;
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
    void setDefaultOptions() override;
    void readInput() override;
    void writeOutput() override;
    void run() override;
    const std::string Summary() const;
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
