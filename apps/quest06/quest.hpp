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

namespace newton {
  struct AlgoParameters {
    const Calc::ThreadingOptions Topt;
    const Calc::PrecisionOptions Popt;
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
protected:
    //prepare cli options
    void prepareOptions() override;
    void prepareInputOptions() override {};
    void prepareOutputOptions() override {};
    void prepareAlgoOptions() override {};
    //parse cli options
    bool parseOptions(int argc, char* argv[]) override;
    bool parseInputOptions() override { return true; };
    bool parseOutputOptions() override { return true; };
    bool parseAlgoOptions() override { return true; };
};

class QuestApp : public CliApp {
private:
    QuestApp();
public:
    QuestApp(const QuestAppOptions&);
    QuestApp(const QuestAppOptions&, ProgressCtrl* pc);
    QuestApp(ProgressCtrl* pc);
    virtual ~QuestApp(){};
    void setDefaultOptions() override {};
    void readInput() override {};
    void writeOutput() override {};
    void run() override;
    const std::string Summary() const;
private:
    std::unique_ptr<newton::AlgoParameters> m_pAlgoParameters;
};

}

#endif /* _QUEST_HPP */
