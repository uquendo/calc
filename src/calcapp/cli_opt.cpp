#include <iostream>
#include <cassert>
#include <sstream>
#include "calcapp/cli.hpp"
#include "calcapp/system.hpp"


namespace Calc {

//TODO: STUBS!
CliApp::CliApp(){
}

CliApp::CliApp(ProgressCtrl* p):App(p){
}

CliApp::~CliApp(){
}

CliAppOptions::CliAppOptions(std::string AppName, std::string AppVersion):
  AppOptions(AppName,AppVersion),
#ifdef HAVE_BOOST
  allOpt("Allowed options"),
#endif
  threadingHelp("Threading model to use: "),
  precisionHelp("Floating point number precision to use: ")
{
}

CliAppOptions::~CliAppOptions(){
}

//prepare cli options
void CliAppOptions::prepareOptions(){
  prepareLoggingOptions();
  prepareThreadingOptions();
  preparePrecisionOptions();
  prepareAlgoOptions();
#ifdef HAVE_BOOST
	allOpt.add_options()
		(HELP_OPT ",h", "print help message")
		(ABOUT_OPT ",V", "print build and version information")
    (VERBOSE_OPT ",v", bpo::value<int>()->default_value(4), "log verbosity level, 0..6")
    (LOGFILE_OPT ",L", bpo::value<string>()->default_value(""), "specify name of the log file")
		(THREADING_OPT ",l", bpo::value<string>()->default_value(_threading_opt_names[0].opt), threadingHelp.c_str())
		(THREADS_OPT ",t", bpo::value<unsigned>()->default_value(0), "number of threads to use. 0=auto")
		(PRECISION_OPT ",p", bpo::value<string>()->default_value(_precision_opt_names[0].opt), precisionHelp.c_str())
#ifdef HAVE_MPREAL
		(DIGITS_OPT ",d", bpo::value<unsigned>()->default_value(10), "MPFR number of decimal digits to use")
#endif
//    (ALGO_OPT ",s", bpo::value<string>()->default_value(solvers[0].opt), solversHelp.c_str())
	;
#endif
}

void CliAppOptions::prepareLoggingOptions(){
}

void CliAppOptions::prepareThreadingOptions(){
  assert(_threading_opt_names[0].opt && _threading_opt_names[0].name && _threading_opt_names[0].type != -1 );
	for ( int i = 0; _threading_opt_names[i].name; ++i ) {
		threadingHelp += _threading_opt_names[i].opt;
		threadingHelp += '=';
		threadingHelp += _threading_opt_names[i].name;
		threadingHelp += ", ";
	}
	threadingHelp.resize(threadingHelp.size() - 2);
}

void CliAppOptions::preparePrecisionOptions(){
  assert(_precision_opt_names[0].opt && _precision_opt_names[0].name && _precision_opt_names[0].type != -1 );
	for ( int i = 0; _precision_opt_names[i].name; ++i ) {
		precisionHelp += _precision_opt_names[i].opt;
		precisionHelp += '=';
		precisionHelp += _precision_opt_names[i].name;
		precisionHelp += ", ";
	}
	precisionHelp.resize(precisionHelp.size() - 2);
}

void CliAppOptions::prepareInputOptions(){
}

void CliAppOptions::prepareOutputOptions(){
}

void CliAppOptions::prepareAlgoOptions(){
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
}

//parse cli options
bool CliAppOptions::parseOptions(int argc, char* argv[]){
#ifdef CLIAPP_OPT_DEFAULT_HELP
	if ( argc <= 1 ) {
    printHelp();
		return false;
	}
#endif
#ifdef HAVE_BOOST
	bpo::store(bpo::parse_command_line(argc, argv, allOpt), argMap);
	bpo::notify(argMap);
	if ( argMap.count(HELP_OPT) ) {
    printHelp();
    bpo::notify(argMap);
    return false;
	} else if ( argMap.count(ABOUT_OPT) ) {
    printAbout();
    bpo::notify(argMap);
    return false;
  }
#endif
  bool parsing_succeded = (
    parseLoggingOptions() ||
    parseThreadingOptions() ||
    parsePrecisionOptions() ||
    parseInputOptions() ||
    parseOutputOptions() ||
    parseAlgoOptions() 
  );
  return parsing_succeded;
}

bool CliAppOptions::parseLoggingOptions(){
  bool v=false;
#ifdef HAVE_BOOST
  v=argMap.count(VERBOSE_OPT)>0;
#endif
  m_logging.verbose=v;
  return true;
}

bool CliAppOptions::parseThreadingOptions(){
  TThreading thr = T_Undefined;
  unsigned thread_count = 0;
#ifdef HAVE_BOOST
  if ( argMap.count(THREADING_OPT) > 0 ) {
      const string& s = argMap[THREADING_OPT].as<string>();
      for ( int i = 0; _threading_opt_names[i].name; ++i ) {
          if ( _threading_opt_names[i].opt == s ) {
              thr = _threading_opt_names[i].type;
              break;
          }
      }
  }
  thread_count=argMap[THREADS_OPT].as<unsigned>();
#endif
  m_threading.type = thr;
  m_threading.num = thread_count;
  return true;
}

bool CliAppOptions::parsePrecisionOptions(){
  TPrecision prec = P_Undefined;
  unsigned digits = 10;
#ifdef HAVE_BOOST
  if ( argMap.count(PRECISION_OPT) > 0 ) {
      const string& s = argMap[PRECISION_OPT].as<string>();
      for ( int i = 0; _precision_opt_names[i].name; ++i ) {
          if ( _precision_opt_names[i].opt == s ) {
              prec = _precision_opt_names[i].type;
              break;
          }
      }
  }
#ifdef HAVE_MPREAL
  digits=argMap[DIGITS_OPT].as<unsigned>();
#endif
#endif
  m_precision.type=prec;
  m_precision.decimal_digits=digits;
  return true;
}

bool CliAppOptions::parseInputOptions(){
  return true;
}

bool CliAppOptions::parseOutputOptions(){
  return true;
}

bool CliAppOptions::parseAlgoOptions(){
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
  return true; 
}

const std::string CliAppOptions::About() const {
  std::string about = m_AppName;
  about.append(" version ").append(m_AppVersion).append("\n");
  about.append("\nSystem information:\n");
  about.append("running on ").append(SysUtil::getOSVersion()).append("\n");
  about.append("detected cpu : ").append(SysUtil::getCpuSpec()).append("\n");
#ifdef BUILD_THREADING
  about.append("detected ").append(std::to_string(SysUtil::getCpuCoresCount())).append(" cpu cores\n");
#endif
  about.append("\nBuild information:\n");
  about.append(SysUtil::getBuildOptions()).append("\n");
  return about;
}

const std::string CliAppOptions::Help() const {
  std::ostringstream oss;
  oss << allOpt;
  return oss.str();
}

void CliAppOptions::printAbout(){
  std::cout << About() << std::endl;
}

void CliAppOptions::printHelp(){
  std::cout << Help() << std::endl;
}

bool CliAppOptions::processOptions(int argc, char* argv[]){ 
  prepareOptions();
 	return parseOptions(argc, argv);
}

}

