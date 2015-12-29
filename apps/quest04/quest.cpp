#include "quest.hpp"
#include "dense_linear_solve.hpp"

#include "calcapp/system.hpp"

#include <cassert>
#include <cstring>
#include <iostream>

using std::string;

namespace Calc {

  QuestAppOptions::QuestAppOptions():
    CliAppOptions(string(APP_NAME),string(APP_VERSION))
  {
    m_input.filetype = FT_Undefined; m_input.filename = "data";
    m_output.filetype = FT_Undefined; m_output.filename = "result";
    m_algo.type = A_Undefined;
  }

  const string QuestAppOptions::About() const
  {
    string about = CliAppOptions::About();
#ifdef BUILD_THREADING
    about.append("\nSupported threaded variants per algo:\n");
    //TODO: print'em all!
#endif
    return about;
  }

  const string QuestAppOptions::Help() const
  {
    string help = CliAppOptions::Help();
#ifndef HAVE_BOOST
    help.append("\nAlgorithm options:\n");
    help.append("  -a [ --algorithm ] arg (=");
    help.append(_algo_opt_names[0].opt);
    help.append(")\n");
    help.append(algoHelp);
    help.append("\n");
#endif
    return help;
  }

  bool QuestAppOptions::processOptions(int argc, char* argv[])
  {
    return CliAppOptions::processOptions(argc,argv);
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
      ("in-name,I",     bpo::value<string>()->default_value("data"), "name of input file(without an extension)")
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
      ("out-name,O",    bpo::value<string>()->default_value("result"), "name of results file(without an extension)")
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
      algoHelp += ",\n";
    }
    algoHelp.resize(algoHelp.size() - 2);
#ifdef BUILD_THREADING
    algoHelp += "\nNote:\n";
    algoHelp += "* check --version for available threading options per algorithm\n";
#endif
#ifdef HAVE_BOOST
    algoOpt.add_options()
      (ALGO_OPT ",a",   bpo::value<string>(), algoHelp.c_str())
      ;
#endif
  }

  bool QuestAppOptions::parseOptions(int argc, char* argv[]){
    bool result = CliAppOptions::parseOptions(argc,argv);
#ifndef HAVE_BOOST
    //fallback limited cli parsing for vanilla build
    string algo_opt;
    bool check_algo_opt = false;
    for(int i = 1; i < argc; i++)
    {
      if(std::strncmp(argv[i],"--algorithm",11) == 0)
      {
        if(!algo_opt.empty())
          throw OptionsParsingError("option '--algorithm' cannot be specified more than once");
        if( argv[i][11] != '=' )
          throw OptionsParsingError("option '--algorithm' should be followed by '=' and algorithm name without any spaces");
        algo_opt = string(argv[i]+12);
        check_algo_opt = true;
      }
      if(std::strncmp(argv[i],"-a",2) == 0)
      {
        if(!algo_opt.empty())
          throw OptionsParsingError("option '-a' cannot be specified more than once");
        if(i == argc - 1 )
          throw OptionsParsingError("option '-a' should be followed by algorithm name");
        algo_opt = string(argv[i+1]);
        check_algo_opt = true;
      }
      if(check_algo_opt)
      {
        TAlgo algo = A_Undefined;
        for ( int j = 0; _algo_opt_names[j].name; ++j ) {
          if ( _algo_opt_names[j].opt == algo_opt ) {
            algo = _algo_opt_names[j].type;
            break;
          }
        }
        if(algo == A_Undefined)
        {
          string err = "Unknown algorithm type option given: ";
          err.append(algo_opt);
          throw OptionsParsingError(err.c_str());
        }
        check_algo_opt = false;
        m_algo.type = algo;
      }
    }
#endif
    if(m_precision.type != numeric::P_Double)
    {
      std::cerr << "ignoring specified precision, only double one supported at the moment" << std::endl;
      m_precision.type = numeric::P_Double;
    }
    return result;
  }

  bool QuestAppOptions::parseInputOptions()
  {
    TFileType input = FT_Undefined;
#ifdef HAVE_BOOST
    if ( argMap.count(INPUT_OPT) > 0 ) {
      const string& s = argMap[INPUT_OPT].as<string>();
      for ( int i = 0; _input_opt_names[i].name; ++i ) {
        if ( _input_opt_names[i].opt == s ) {
          input = _input_opt_names[i].type;
          break;
        }
      }
      if(input == FT_Undefined)
      {
        std::string err = "Unknown input file type option given: ";
        err.append(s);
        throw OptionsParsingError(err.c_str());
      }
    } else
#endif
    {
      input = _input_opt_names[0].type;
    }
    m_input.filetype = input;
    //parse filename
#ifdef HAVE_BOOST
    if( argMap.count("in-name") > 0 )
      m_input.filename = argMap["in-name"].as<string>();
#endif
    return true;
  }

  bool QuestAppOptions::parseOutputOptions()
  {
    TFileType output = FT_Undefined;
#ifdef HAVE_BOOST
    if ( argMap.count(OUTPUT_OPT) > 0 ) {
      const string& s = argMap[OUTPUT_OPT].as<string>();
      for ( int i = 0; _output_opt_names[i].name; ++i ) {
        if ( _output_opt_names[i].opt == s ) {
          output = _output_opt_names[i].type;
          break;
        }
      }
      if(output == FT_Undefined)
      {
        std::string err = "Unknown output file type option given: ";
        err.append(s);
        throw OptionsParsingError(err.c_str());
      }
    } else
#endif
    {
      output = _output_opt_names[0].type;
    }
    m_output.filetype = output;
    //parse filename
#ifdef HAVE_BOOST
    if(argMap.count("out-name") > 0)
      m_output.filename = argMap["out-name"].as<string>();
#endif
    return true;
  }


  bool QuestAppOptions::parseAlgoOptions()
  {
    TAlgo algo = A_Undefined;
#ifdef HAVE_BOOST
    if ( argMap.count(ALGO_OPT) > 0 ) {
      const string& s = argMap[ALGO_OPT].as<string>();
      for ( int i = 0; _algo_opt_names[i].name; ++i ) {
        if ( _algo_opt_names[i].opt == s ) {
          algo = _algo_opt_names[i].type;
          break;
        }
      }
      if(algo == A_Undefined)
      {
        std::string err = "Unknown algorithm type option given: ";
        err.append(s);
        throw OptionsParsingError(err.c_str());
      }
    } else
#endif
    {
#ifdef QUESTAPP_OPT_DEFAULT_ALGO
      algo = QUESTAPP_OPT_DEFAULT_ALGO;
#else
      algo = _algo_opt_names[0].type;
#endif
    }
    m_algo.type = algo;
    return true;
  }

  QuestApp::QuestApp(const QuestAppOptions& opt)
    : CliApp(dynamic_cast<const CliAppOptions&>(opt))
    , m_input(opt.getInOpts()),m_output(opt.getOutOpts()),m_algo(opt.getAlgoOpts())
    , m_pAlgoParameters(new dense_linear_solve::AlgoParameters({m_threading,m_precision,m_algo,
          nullptr,nullptr,nullptr,nullptr,0,nullptr}))
    , m_pfIn(new InFileText(m_input.filename,m_input.filetype,true))
    , m_pfOut(new OutFileText(m_output.filename,m_output.filetype,false))
  {
  }

  QuestApp::QuestApp(const QuestAppOptions& opt, ProgressCtrl* pc)
    : CliApp(dynamic_cast<const CliAppOptions&>(opt), pc)
    , m_input(opt.getInOpts()),m_output(opt.getOutOpts()),m_algo(opt.getAlgoOpts())
    , m_pAlgoParameters(new dense_linear_solve::AlgoParameters({m_threading,m_precision,m_algo,
          nullptr,nullptr,nullptr,nullptr,0,pc}))
    , m_pfIn(new InFileText(m_input.filename,m_input.filetype,true))
    , m_pfOut(new OutFileText(m_output.filename,m_output.filetype,false))
  {
  }

  QuestApp::QuestApp(ProgressCtrl* pc)
    : CliApp(pc)
  {
    setDefaultOptions();
  }

  QuestApp::QuestApp():QuestApp(nullptr)
  {
  }

  void QuestApp::setDefaultOptions()
  {
    //init m_input,m_output,m_algo,m_AlgoParameters with safe defaults
    m_input.filetype = _input_opt_names[0].type; m_input.filename = "data";
    m_output.filetype = _output_opt_names[0].type; m_output.filename = "result";
#ifdef QUESTAPP_OPT_DEFAULT_ALGO
    m_algo.type = QUESTAPP_OPT_DEFAULT_ALGO;
#else
    m_algo.type = A_NumCppGauss;
#endif
    m_pAlgoParameters.reset(new dense_linear_solve::AlgoParameters({m_threading,m_precision,m_algo,
          nullptr,nullptr,nullptr,nullptr,0,ctrl()}));
    m_pfIn.reset(new InFileText(m_input.filename,m_input.filetype,true));
    m_pfOut.reset(new OutFileText(m_output.filename,m_output.filetype,false));
  }

  const std::string QuestApp::Summary() const
  {
    std::string s;
    //TODO: rewrite using getNameByType(...) helpers
    for ( int i = 0; _algo_opt_names[i].name; ++i )
    {
      if( _algo_opt_names[i].type == m_pAlgoParameters->Aopt.type )
      {
        s.append("Running dense linear system iterative solver ").append(_algo_opt_names[i].name);
        break;
      }
    }
    for ( int i = 0; _threading_opt_names[i].name; ++i )
    {
      if( _threading_opt_names[i].type == m_pAlgoParameters->Topt.type )
      {
        s.append(" (").append(_threading_opt_names[i].name).append(" variant");
        if( m_pAlgoParameters->Topt.type != numeric::T_Serial )
        {
          s.append(" with ");
          s.append(std::to_string(m_pAlgoParameters->Topt.num > 0 ? m_pAlgoParameters->Topt.num : numeric::hardware_concurrency()));
          s.append(" threads");
        }
        s.append(")");
        break;
      }
    }
    for ( int i = 0; _precision_opt_names[i].name; ++i )
    {
      if( _precision_opt_names[i].type == m_pAlgoParameters->Popt.type )
      {
        s.append(" using ").append(_precision_opt_names[i].name).append(" precision");
#ifdef HAVE_MPREAL
        if( m_pAlgoParameters->Popt.type == numeric::P_MPFR )
          s.append("(").append(std::to_string(m_pAlgoParameters->Popt.decimal_digits)).append("decimal digits)");
#endif
        break;
      }
    }
    return s;
  }

  void QuestApp::readInput()
  {
    //read input table from file
    const size_t sz = m_pAlgoParameters->system_size;
    const size_t stride = sz + 1;
    double* const buf_ptr = m_pAlgoParameters->Ab_buf.get();
    if(m_pfIn->lineNum() == 0)
      m_pfIn->readNextLine();
    for(size_t i = 0; i < sz; i++)
    {
      m_pfIn->readNextLine_scanNumArray<double>(sz, sz, buf_ptr+i*stride);
    }
    for(size_t i = 0; i < sz; i++)
    {
      m_pfIn->readNextLine_scanNums(1,buf_ptr+i*stride+sz);
    }
  }

  void QuestApp::writeOutput()
  {
    //write ouput table to file
    const size_t sz = m_pAlgoParameters->system_size;
    m_pfOut->printf("# %zu \n", m_pAlgoParameters->system_size);
    for(size_t i = 0; i < sz; i++)
    {
      m_pfOut->println_printNumsDefault(m_pAlgoParameters->x[i]);
    }
    m_pfOut->flush();
  }

  void QuestApp::run()
  {
    //PRERUN:
    log().debug(Summary());
    //init system
    log().debug("reading augumented matrix...");
    m_pfIn->readNextLine_scan(1,"# %zu",&m_pAlgoParameters->system_size);
    log().fdebug("found system of %zu linear equations", m_pAlgoParameters->system_size);
    //sanity check(input can be used by selected algorithm)
    //initialize output structure[s] and possibly file[s]
    m_pAlgoParameters->Ab_buf.reset(new double[m_pAlgoParameters->system_size*(m_pAlgoParameters->system_size+1)]);
    m_pAlgoParameters->x = new double[m_pAlgoParameters->system_size];
    m_pAlgoParameters->x_tmp = new double[m_pAlgoParameters->system_size];
    m_pAlgoParameters->residual_tmp = new double[m_pAlgoParameters->system_size];
    //read input data
    log().debug("reading input matrix...");
    readInput();
    //TODO: estimate output file size, check available disk space
    //log stats
    log().debug(SysUtil::getMemStats());
    //RUN_IO_ITER:
    //run selected algorithm
    log().debug("running main task...");
    if(dense_linear_solve::perform(*m_pAlgoParameters, log()))
    {
      //output results
      log().debug("writing solution vector...");
      writeOutput();
      //POSTRUN:
      //finalize output file
      //log stats
      if(dense_linear_solve::print_residual) //TODO: add cli option
      {
        m_pfIn->reset();
        readInput();
        log().fdebug("found solution with ||Ax-b||_1 = %g, ||Ax-b||_2 = %g"
            , dense_linear_solve::residualL1Norm(m_pAlgoParameters->system_size,m_pAlgoParameters->Ab_buf.get(),m_pAlgoParameters->x)
            , dense_linear_solve::residualL2Norm(m_pAlgoParameters->system_size,m_pAlgoParameters->Ab_buf.get(),m_pAlgoParameters->x)
            );
      }
    } else {
      log().error("failed to apply iterative solver to given system");
    }
    delete[] m_pAlgoParameters->x;
    delete[] m_pAlgoParameters->x_tmp;
    delete[] m_pAlgoParameters->residual_tmp;
    log().debug("have a nice day.");
  }

}
