#include "calcapp/cli.hpp"


#include <boost/program_options/options_description.hpp>
#include <boost/program_options/variables_map.hpp>
#include <boost/program_options/parsers.hpp>
namespace bpo=boost::program_options;

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
	assert(threading[0].opt && threading[0].name && threading[0].type != -1 );
	string threadingHelp("Threading model to use: ");
	for ( int i = 0; threading[i].name; ++i ) {
		threadingHelp += threading[i].opt;
		threadingHelp += '=';
		threadingHelp += threading[i].name;
		threadingHelp += ", ";
	}
	threadingHelp.resize(threadingHelp.size() - 2);
    //preparePrecisionOptions()
	assert(precision[0].opt && precision[0].name && precision[0].type != -1 );
	string precisionHelp("Floating point number precision to use: ");
	for ( int i = 0; precision[i].name; ++i ) {
		precisionHelp += precision[i].opt;
		precisionHelp += '=';
		precisionHelp += precision[i].name;
		precisionHelp += ", ";
	}
	precisionHelp.resize(precisionHelp.size() - 2);
	bpo::options_description allOpt("Allowed options");
	allOpt.add_options()
		(HELP_OPT ",h", "print help message")
		(VERBOSE_OPT ",v", "verbose messages")
		(THREADS_OPT ",t", bpo::value<unsigned>()->default_value(0), "number of threads to use. 0=auto")
//		(ALGO_OPT ",s", bpo::value<string>()->default_value(solvers[0].opt), solversHelp.c_str())
		(PRECISION_OPT ",p", bpo::value<string>()->default_value(precision[0].opt), precisionHelp.c_str())
		(THREADING_OPT ",l", bpo::value<string>()->default_value(threading[0].opt), threadingHelp.c_str())
	;
	if ( argc <= 1 ) {
		cout << allOpt << "\n";
		return false;
	}
	bpo::store(bpo::parse_command_line(argc, argv, allOpt), argMap);
	bpo::notify(argMap);
	if ( pArgMap->count(HELP_OPT) ) {
		cout << allOpt << "\n";
		bpo::notify(argMap);
		return false;
	}
    //parseOptions()
    //parseAlgoOptions()
/*
     TAlgo algo = A_Undefined;
		if ( arg_map.count(SOLVER_OPT) > 0 ) {
			const string& s = arg_map[SOLVER_OPT].as<string>();
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
		if ( arg_map.count(PRECISION_OPT) > 0 ) {
			const string& s = arg_map[PRECISION_OPT].as<string>();
			for ( int i = 0; precision[i].name; ++i ) {
				if ( precision[i].opt == s ) {
					prec = precision[i].type;
					break;
				}
			}
		}
		
    //parseThreadingOptions()
		TThreading thr = T_Undefined;
		if ( arg_map.count(THREADING_OPT) > 0 ) {
			const string& s = arg_map[THREADING_OPT].as<string>();
			for ( int i = 0; threading[i].name; ++i ) {
				if ( threading[i].opt == s ) {
					thr = threading[i].type;
					break;
				}
			}
		}
    unsigned thread_count=arg_map[THREADS_OPT].as<unsigned>();
    //parseLoggingOptions
    const bool v=arg_map.count(VERBOSE_OPT)>0;

	return true;
};

}

