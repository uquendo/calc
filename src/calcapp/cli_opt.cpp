#include "calcapp/cli.hpp"
#include "calcapp/system.hpp"

#include <iostream>
#include <cassert>
#include <string>
#include <sstream>

using std::string;

namespace Calc {

//TODO: STUBS!
CliApp::CliApp():CliApp(nullptr){
}

CliApp::CliApp(ProgressCtrl* p):App(p){
}

CliAppOptions::CliAppOptions(string AppName, string AppVersion):
  AppOptions(AppName,AppVersion),
#ifdef HAVE_BOOST
  allOpt("Allowed options"),
  commonOpt("Common options"),
  inputOpt("Input options"),
  outputOpt("Output options"),
  algoOpt("Algorithm options"),
#endif
  threadingHelp(""),
  precisionHelp("")
{
}

//prepare cli options
void CliAppOptions::prepareOptions(){
  prepareLoggingOptions();
  prepareThreadingOptions();
  preparePrecisionOptions();
  prepareInputOptions();
  prepareOutputOptions();
  prepareAlgoOptions();
#ifdef HAVE_BOOST
  commonOpt.add_options()
    (HELP_OPT ",h", "print help message")
    (ABOUT_OPT ",V", "print build and version information")
    (VERBOSE_OPT ",v", bpo::value<int>()->default_value(4), "log verbosity level, 0..6")
    (LOGFILE_OPT ",L", bpo::value<string>()->default_value(""), "specify name of the log file")
#ifdef BUILD_THREADING
    (THREADING_OPT ",l", bpo::value<string>()->default_value(_threading_opt_names[0].opt), threadingHelp.c_str())
    (THREADS_OPT ",t", bpo::value<unsigned>()->default_value(0), "number of threads to use. 0=auto")
#endif
    (PRECISION_OPT ",p", bpo::value<string>()->default_value(_precision_opt_names[0].opt), precisionHelp.c_str())
#ifdef HAVE_MPREAL
    (DIGITS_OPT ",d", bpo::value<unsigned>()->default_value(10), "MPFR number of decimal digits to use")
#endif
  ;
  allOpt.add(commonOpt).add(inputOpt).add(outputOpt).add(algoOpt);
#endif
}

void CliAppOptions::prepareLoggingOptions(){
}

void CliAppOptions::prepareThreadingOptions(){
  assert(_threading_opt_names[0].opt && _threading_opt_names[0].name && _threading_opt_names[0].type != -1 );
  threadingHelp += "Threading model to use: ";
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
  precisionHelp += "Floating point number precision to use: ";
  for ( int i = 0; _precision_opt_names[i].name; ++i ) {
    precisionHelp += _precision_opt_names[i].opt;
    precisionHelp += '=';
    precisionHelp += _precision_opt_names[i].name;
    precisionHelp += ", ";
  }
  precisionHelp.resize(precisionHelp.size() - 2);
}

void CliAppOptions::prepareInputOptions(){
  //TODO: write generic i/o options handling
}

void CliAppOptions::prepareOutputOptions(){
  //TODO: write generic i/o options handling
}

void CliAppOptions::prepareAlgoOptions(){
/*  
  assert(_algo_opt_names[0].opt && _algo_opt_names[0].name && _algo_opt_names[0].type != -1 );
  string _algo_opt_namesHelp("Computation algorithm to use: ");
  for ( int i = 0; _algo_opt_names[i].name; ++i ) {
    _algo_opt_namesHelp += _algo_opt_names[i].opt;
    _algo_opt_namesHelp += '=';
    _algo_opt_namesHelp += _algo_opt_names[i].name;
    _algo_opt_namesHelp += ", ";
  }
  _algo_opt_namesHelp.resize(_algo_opt_namesHelp.size() - 2);
*/
}

//parse cli options
bool CliAppOptions::parseOptions(int argc, char* argv[]){
  if ( argc <= 1 ) {
#ifdef CLIAPP_OPT_DEFAULT_HELP
    printHelp();
    return false;
#elseif CLIAPP_OPT_DEFAULT_ABOUT
    printAbout();
    return false;
#endif
  }
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
#else
  //TODO: invent some cli options parsing wheel there to make boost haters happy enough
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
  TThreading thr = _threading_opt_names[0].type;
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
  TPrecision prec = _precision_opt_names[0].type;
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
  //TODO: write generic i/o options handling
  return true;
}

bool CliAppOptions::parseOutputOptions(){
  //TODO: write generic i/o options handling
  return true;
}

bool CliAppOptions::parseAlgoOptions(){
  //implement in derived classes
  return true; 
}

const string CliAppOptions::About() const {
  string about = m_AppName;
  about.append(" version ").append(m_AppVersion).append("\n");
  about.append("\nSystem information:\n");
  about.append("running on ").append(SysUtil::getOSVersion()).append("\n");
  about.append("detected cpu : ").append(SysUtil::getCpuSpec()).append("\n");
#ifdef BUILD_THREADING
  about.append("detected ").append(std::to_string(SysUtil::getCpuCoresCount())).append(" cpu cores\n");
#endif
  about.append("\nBuild information:\n");
  about.append(SysUtil::getBuildOptions()).append("\n");
  about.append("run with -h or --help to see available options\n");
  return about;
}

const string CliAppOptions::Help() const {
  std::ostringstream oss;
#ifdef HAVE_BOOST
  oss << allOpt;
#else
  //TODO: invent some cli options parsing wheel there to make boost haters happy enough
  oss << "App was compiled without boost.program_options, so no fancy cli options yet. sorry.";
#endif
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

