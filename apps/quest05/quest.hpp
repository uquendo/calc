#pragma once
#ifndef _QUEST_HPP
#define _QUEST_HPP
#include "config.h"
#include "appconfig.h"

#include <string>
#include <unordered_set>

#include "calcapp/cli.hpp"
#include "calcapp/io.hpp"
#include "calcapp/math/approximant.hpp"

namespace Calc {

enum TAlgo {
  A_NumCpp=0,
#ifdef HAVE_GSL
  A_ExtGSL,
#endif
  A_Undefined
};

static const OptName<TAlgo> _algo_opt_names[] = {
  { "libnumeric c++ variant", "num-cpp", A_NumCpp },
//  { "GSL variant", "ext-gsl", A_ExtGSL },
  { nullptr, nullptr, A_Undefined }
};

static const OptName<TFileType> _input_opt_names[] = {
  { "Read function values and weights table in .dat format", "dat", FT_ApproximationTableText },
  { nullptr, nullptr, FT_None }
};

static const OptName<TFileType> _output_opt_names[] = {
  { "Write function values table in .dat format", "dat", FT_FunctionTableText },
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

namespace approximate {
  static constexpr size_t default_output_fineness_factor = 100;
  struct AlgoParameters {
    const Calc::ThreadingOptions Topt;
    const Calc::PrecisionOptions Popt;
    const Calc::AlgoOptions Aopt;
    std::unique_ptr<ApproximantBase1d> f;
    size_t output_fineness_factor;
    size_t table_size;
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
    std::unique_ptr<approximate::AlgoParameters> m_pAlgoParameters;
    std::unique_ptr<InFileText> m_pfIn;
    std::unique_ptr<OutFileText> m_pfOut;
};

}

#endif /* _QUEST_HPP */
