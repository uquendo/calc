#pragma once
#ifndef _QUEST_HPP
#define _QUEST_HPP

#include "calcapp/cli.hpp"

namespace Calc {

enum TAlgo {
    T_Cached,
    T_Undefined
};

struct AlgoOptName {
	const char * name;
	const char * opt;
	TAlgo type;
};

static const AlgoOptName _algo_opt_names[] = {
	{ "Cached", "c", A_Cached },
	{ NULL, 0, A_Undefined }
};

class QuestAppOptions : public CliAppOptions {
public:
    QuestAppOptions();
    virtual ~QuestAppOptions();
protected:
    virtual bool processOptions(int argc, char* argv[]);
    //prepare cli options
    virtual void prepareOptions();
    virtual void prepareInputOptions();
    virtual void prepareOutputOptions();
    virtual void prepareAlgoOptions();
    //parse cli options
    virtual bool parseOptions();
    virtual bool parseInputOptions();
    virtual bool parseOutputOptions();
    virtual bool parseAlgoOptions();
}

class QuestApp : public App {
public:
    QuestApp();
    virtual ~QuestApp();
}

}

#endif /* _QUEST_HPP */
