#include <cassert>

#include "quest.hpp"

namespace Calc {

  QuestAppOptions::QuestAppOptions():CliAppOptions(std::string(APP_NAME),std::string(APP_VERSION)),
    inputHelp(""),
    outputHelp(""),
    algoHelp(""),
    m_input(FT_Matrix),
    m_output(FT_Matrix),
    m_algo(A_Undefined)
  {
  }

  const std::string QuestAppOptions::About() const
  {
    std::string about=CliAppOptions::About();
#ifdef BUILD_THREADING
    about.append("\nSupported threaded variants per algo:\n");
    //TODO: print'em all!
#endif
    return about;
  }

  void QuestAppOptions::prepareOptions()
  {
    CliAppOptions::prepareOptions();
  }

  void QuestAppOptions::prepareInputOptions(){
    assert(_input_opt_names[0].opt && _input_opt_names[0].name && _input_opt_names[0].type != -1 );
    inputHelp+="Input file format to use: \n";
    for ( int i = 0; _input_opt_names[i].name; ++i ) {
      inputHelp += _input_opt_names[i].opt;
      inputHelp += "= ";
      inputHelp += _input_opt_names[i].name;
      inputHelp += ",\n";
    }
    inputHelp.resize(inputHelp.size() - 2);
#ifdef HAVE_BOOST
    inputOpt.add_options()
      (INPUT_OPT ",i",  bpo::value<string>()->default_value(_input_opt_names[0].opt), inputHelp.c_str())
      ("in-A,A",        bpo::value<string>()->default_value("data1"), "name of matrix A file(without extension)")
      ("in-B,B",        bpo::value<string>()->default_value("data2"), "name of matrix B file(without extension)")
      ;
#endif
  }

  void QuestAppOptions::prepareOutputOptions(){
    assert(_output_opt_names[0].opt && _output_opt_names[0].name && _output_opt_names[0].type != -1 );
    outputHelp+="Output file format to use: \n";
    for ( int i = 0; _output_opt_names[i].name; ++i ) {
      outputHelp += _output_opt_names[i].opt;
      outputHelp += "= ";
      outputHelp += _output_opt_names[i].name;
      outputHelp += ",\n";
    }
    outputHelp.resize(outputHelp.size() - 2);
#ifdef HAVE_BOOST
    outputOpt.add_options()
      (OUTPUT_OPT ",o", bpo::value<string>()->default_value(_output_opt_names[0].opt), outputHelp.c_str())
      ("out-C,C",       bpo::value<string>()->default_value("result"), "name of matrix C(results) file(without extension)")
      ;
#endif
  }

  void QuestAppOptions::prepareAlgoOptions()
  {
    assert(_algo_opt_names[0].opt && _algo_opt_names[0].name && _algo_opt_names[0].type != -1 );
    algoHelp+="Computation algorithm to use: \n";
    for ( int i = 0; _algo_opt_names[i].name; ++i ) {
      algoHelp += _algo_opt_names[i].opt;
      algoHelp += "= ";
      algoHelp += _algo_opt_names[i].name;
#ifdef HAVE_BLAS
      if(_algo_opt_names[i].type == A_ExtCBLAS){
        algoHelp += "(BLAS variant: " BLAS_VENDOR ")";
      }
#endif
      algoHelp += ",\n";
    }
    algoHelp.resize(algoHelp.size() - 2);
#ifdef BUILD_THREADING
    algoHelp += "\nNote:\n";
    algoHelp += "* check --version for available threading options per algorithm\n";
#endif
#ifdef HAVE_BOOST
    algoOpt.add_options()
      (ALGO_OPT ",s",   bpo::value<string>()->default_value(_algo_opt_names[0].opt), algoHelp.c_str())
      ;
#endif
  }

  bool QuestAppOptions::parseOptions(int argc, char* argv[]){
    return CliAppOptions::parseOptions(argc,argv);
  }

  bool QuestAppOptions::parseInputOptions()
  {
    TFileType input = _input_opt_names[0].type;
#ifdef HAVE_BOOST
    if ( argMap.count(INPUT_OPT) > 0 ) {
      const string& s = argMap[INPUT_OPT].as<string>();
      for ( int i = 0; _input_opt_names[i].name; ++i ) {
        if ( _input_opt_names[i].opt == s ) {
          input = _input_opt_names[i].type;
          break;
        }
      }
    }
#endif
    m_input = input;
    //parse filenames for A and B
    //
    return true;   
  }

  bool QuestAppOptions::parseOutputOptions()
  {
    TFileType output = _output_opt_names[0].type;
#ifdef HAVE_BOOST
    if ( argMap.count(OUTPUT_OPT) > 0 ) {
      const string& s = argMap[OUTPUT_OPT].as<string>();
      for ( int i = 0; _output_opt_names[i].name; ++i ) {
        if ( _output_opt_names[i].opt == s ) {
          output = _output_opt_names[i].type;
          break;
        }
      }
    }
#endif
    m_output = output;
    //parse filename for C
    //
    return true;   
  }


  bool QuestAppOptions::parseAlgoOptions()
  {
    TAlgo algo = _algo_opt_names[0].type;
#ifdef HAVE_BOOST
    if ( argMap.count(ALGO_OPT) > 0 ) {
      const string& s = argMap[ALGO_OPT].as<string>();
      for ( int i = 0; _algo_opt_names[i].name; ++i ) {
        if ( _algo_opt_names[i].opt == s ) {
          algo = _algo_opt_names[i].type;
          break;
        }
      }
    }
#endif
    m_algo = algo;
    return true;   
  }
}
