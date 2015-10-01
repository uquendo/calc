#pragma once
#ifndef _QUEST_HPP
#define _QUEST_HPP

#include "calcapp/cli.hpp"

namespace Calc {

enum TAlgo {
    A_Cached,
    A_Undefined
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
    QuestAppOptions() {};
    virtual ~QuestAppOptions() {};
    virtual bool processOptions(int argc, char* argv[]) {return CliAppOptions::processOptions(argc,argv);};
protected:
    //prepare cli options
    virtual void prepareOptions(){};
    virtual void prepareInputOptions(){};
    virtual void prepareOutputOptions(){};
    virtual void prepareAlgoOptions(){};
    //parse cli options
    virtual bool parseOptions(){return false;};
    virtual bool parseInputOptions(){return false;};
    virtual bool parseOutputOptions(){return false;};
    virtual bool parseAlgoOptions(){return false;};
};

class QuestApp : public CliApp {
public:
    QuestApp(){};
    QuestApp(ProgressCtrl* pc):CliApp(pc){};
    virtual ~QuestApp(){};
    virtual void setOptions(const QuestAppOptions&){};
    virtual void readInput(){};
    virtual void run(){};
};

}

#endif /* _QUEST_HPP */
