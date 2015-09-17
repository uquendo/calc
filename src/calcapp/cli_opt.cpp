#include <iostream>

#include <boost/program_options/options_description.hpp>
#include <boost/program_options/variables_map.hpp>
#include <boost/program_options/parsers.hpp>
namespace bpo=boost::program_options;

#include "calcapp/cli.hpp"


namespace Calc {

bool CliAppOptions::processOptions(int argc, char* argv[]){ 
    bpo::variables_map argMap;
    //prepareOptions()
    //prepareAlgoOptions()
/*	
	assert(solvers[0].opt && solvers[0].name && solvers[0].type != -1 );
	string solversHelp("Computation algorithm to use: ");
	for ( int i = 0; solvers[i].name; ++i ) {
		solversHelp += solvers[i].opt;
		solversHelp += '=';
		solversHelp += solvers[i].name;
		solversHelp += ", ";
	}
	solversHelp.resize(solversHelp.size() - 2);
*/
    //prepareThreadingOptions()
	assert(_threading_opt_names[0].opt && _threading_opt_names[0].name && _threading_opt_names[0].type != -1 );
	string threadingHelp("Threading model to use: ");
	for ( int i = 0; _threading_opt_names[i].name; ++i ) {
		threadingHelp += _threading_opt_names[i].opt;
		threadingHelp += '=';
		threadingHelp += _threading_opt_names[i].name;
		threadingHelp += ", ";
	}
	threadingHelp.resize(threadingHelp.size() - 2);
    //preparePrecisionOptions()
	assert(_precision_opt_names[0].opt && _precision_opt_names[0].name && _precision_opt_names[0].type != -1 );
	string precisionHelp("Floating point number precision to use: ");
	for ( int i = 0; _precision_opt_names[i].name; ++i ) {
		precisionHelp += _precision_opt_names[i].opt;
		precisionHelp += '=';
		precisionHelp += _precision_opt_names[i].name;
		precisionHelp += ", ";
	}
	precisionHelp.resize(precisionHelp.size() - 2);
	bpo::options_description allOpt("Allowed options");
	allOpt.add_options()
		(HELP_OPT ",h", "print help message")
		(VERBOSE_OPT ",v", "verbose messages")
		(THREADS_OPT ",t", bpo::value<unsigned>()->default_value(0), "number of threads to use. 0=auto")
//		(ALGO_OPT ",s", bpo::value<string>()->default_value(solvers[0].opt), solversHelp.c_str())
		(PRECISION_OPT ",p", bpo::value<string>()->default_value(_precision_opt_names[0].opt), precisionHelp.c_str())
		(THREADING_OPT ",l", bpo::value<string>()->default_value(_threading_opt_names[0].opt), threadingHelp.c_str())
	;
	if ( argc <= 1 ) {
        std::cout << allOpt << "\n";
		return false;
	}
	bpo::store(bpo::parse_command_line(argc, argv, allOpt), argMap);
	bpo::notify(argMap);
	if ( argMap.count(HELP_OPT) ) {
        std::cout << allOpt << "\n";
		bpo::notify(argMap);
		return false;
	}
    //parseOptions()
    //parseAlgoOptions()
/*
     TAlgo algo = A_Undefined;
		if ( argMap.count(SOLVER_OPT) > 0 ) {
			const string& s = argMap[SOLVER_OPT].as<string>();
			for ( int i = 0; solvers[i].name; ++i ) {
				if ( solvers[i].opt == s ) {
					algo = solvers[i].type;
					break;
				}
			}
		}
*/		

    //parsePrecisionOptions()
    TPrecision prec = P_Undefined;
    if ( argMap.count(PRECISION_OPT) > 0 ) {
        const string& s = argMap[PRECISION_OPT].as<string>();
        for ( int i = 0; _precision_opt_names[i].name; ++i ) {
            if ( _precision_opt_names[i].opt == s ) {
                prec = _precision_opt_names[i].type;
                break;
            }
        }
    }
    m_precision.type=prec;

    //parseThreadingOptions()
    TThreading thr = T_Undefined;
    if ( argMap.count(THREADING_OPT) > 0 ) {
        const string& s = argMap[THREADING_OPT].as<string>();
        for ( int i = 0; _threading_opt_names[i].name; ++i ) {
            if ( _threading_opt_names[i].opt == s ) {
                thr = _threading_opt_names[i].type;
                break;
            }
        }
    }
    unsigned thread_count=argMap[THREADS_OPT].as<unsigned>();
    m_threading.type = thr;
    m_threading.num = thread_count;
    
    //parseLoggingOptions
    const bool v=argMap.count(VERBOSE_OPT)>0;
    m_logging.verbose=v;

	return true;
}

}

