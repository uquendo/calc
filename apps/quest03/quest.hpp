#pragma once
#ifndef _QUEST_HPP
#define _QUEST_HPP
#include "config.h"
#include "appconfig.h"

#include <string>
#include <unordered_set>

#include "calcapp/cli.hpp"
#include "calcapp/io.hpp"
#include "calcapp/infile.hpp"
#include "calcapp/outfile.hpp"

namespace Calc {

enum TAlgo {
  A_NumCppGauss=0,
  A_NumCppJordan,
  A_NumCppFullPivoting,
  A_Undefined
};

static const OptName<TAlgo> _algo_opt_names[] = {
  { "dumb libnumeric c++ variant of Gauss elimination without pivoting", "num-cpp-gauss-s", A_NumCppGauss },
  { "dumb libnumeric c++ variant of Gauss-Jordan elimination without pivoting", "num-cpp-jordan-s", A_NumCppJordan },
  { "dumb libnumeric c++ variant of Gauss elimination with full pivoting", "num-cpp-gauss-fp-s", A_NumCppFullPivoting },
  { nullptr, nullptr, A_Undefined }
};

static const OptName<TFileType> _input_opt_names[] = {
  { "Read augumented matrix in .dat format", "dat", FT_MatrixText },
  { nullptr, nullptr, FT_None }
};

static const OptName<TFileType> _output_opt_names[] = {
  { "Write solution vector in .dat format", "dat", FT_MatrixText },
  { nullptr, nullptr, FT_None }
};

struct InputOptions {
  TFileType filetype;
  std::string filename;

  InputOptions():
    filetype(FT_Undefined)
    ,filename("")
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

namespace dense_linear_solve {
  static constexpr bool print_residual = true; //TODO: cli option
  struct AlgoParameters {
    const Calc::ThreadingOptions Topt;
    const Calc::PrecisionOptions Popt;
    const Calc::AlgoOptions Aopt;
    std::unique_ptr<double[]> Ab_buf;
    double** Ab_rows;
    double* x;
    size_t system_size;
    ProgressCtrl * progress_ptr;
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
    std::unique_ptr<dense_linear_solve::AlgoParameters> m_pAlgoParameters;
    std::unique_ptr<InFileText> m_pfIn;
    std::unique_ptr<OutFileText> m_pfOut;
};

}

#endif /* _QUEST_HPP */
