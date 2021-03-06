#include "quest.hpp"
#include "matmul.hpp"

#include "calcapp/system.hpp"

#include <cassert>

using std::string;

namespace Calc {

  QuestAppOptions::QuestAppOptions():
    CliAppOptions(string(APP_NAME),string(APP_VERSION))
  {
    m_input.filetype = _input_opt_names[0].type; m_input.filename_A = "data1";  m_input.filename_B = "data2";
    m_output.filetype = _output_opt_names[0].type; m_output.filename = "result";
    m_algo.type=A_Undefined;
  }

  const string QuestAppOptions::About() const
  {
    string about=CliAppOptions::About();
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
      ("in-A,A",        bpo::value<string>()->default_value("data1"), "name of matrix A file(without an extension if format is specified)")
      ("in-B,B",        bpo::value<string>()->default_value("data2"), "name of matrix B file(without an extension if format is specified)")
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
      ("out-C,C",       bpo::value<string>()->default_value("result"), "name of matrix C(results) file(without an extension if format is specified)")
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
      (ALGO_OPT ",a",   bpo::value<string>()->default_value(_algo_opt_names[0].opt), algoHelp.c_str())
      ;
#endif
  }

  bool QuestAppOptions::parseOptions(int argc, char* argv[]){
    bool result = CliAppOptions::parseOptions(argc,argv);
#ifndef HAVE_BOOST
    //fallback limited cli parsing for vanilla build
    std::string algo_opt;
    bool check_algo_opt = false;
    for(int i = 1; i < argc; i++)
    {
      if(std::strncmp(argv[i],"--algorithm",11) == 0)
      {
        if(!algo_opt.empty())
          throw OptionsParsingError("option '--algorithm' cannot be specified more than once");
        if( argv[i][11] != '=' )
          throw OptionsParsingError("option '--algorithm' should be followed by '=' and algorithm name without any spaces");
        algo_opt = std::string(argv[i]+12);
        check_algo_opt = true;
      }
      if(std::strncmp(argv[i],"-a",2) == 0)
      {
        if(!algo_opt.empty())
          throw OptionsParsingError("option '-a' cannot be specified more than once");
        if(i == argc - 1 )
          throw OptionsParsingError("option '-a' should be followed by algorithm name");
        algo_opt = std::string(argv[i+1]);
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
          std::string err = "Unknown algorithm type option given: ";
          err.append(algo_opt);
          throw OptionsParsingError(err.c_str());
        }
        check_algo_opt = false;
        m_algo.type = algo;
      }
    }
#endif
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
        return false;
    } else
#endif
    {
      input = _input_opt_names[0].type;
    }
    m_input.filetype = input;
    //parse filenames for A and B
#ifdef HAVE_BOOST
    m_input.filename_A = argMap["in-A"].as<string>();
    m_input.filename_B = argMap["in-B"].as<string>();
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
        return false;
    } else
#endif
    {
      output = _output_opt_names[0].type;
    }
    m_output.filetype = output;
    //parse filename for C
#ifdef HAVE_BOOST
    m_output.filename = argMap["out-C"].as<string>();
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
        return false;
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

  bool matmul::AlgoParameters::initCsize()
  {
    bool canMultiply = false;
    if(transposeB) {
      canMultiply = ( a->getColumnsNum() == b->getColumnsNum() );
      nrows_C = a->getRowsNum();
      ncolumns_C = b->getRowsNum();
    } else if(transposeA) {
      canMultiply = ( a->getRowsNum() == b->getRowsNum() );
      nrows_C = a->getColumnsNum();
      ncolumns_C = b->getColumnsNum();
    } else {
      canMultiply = ( a->getColumnsNum() == b->getRowsNum() );
      nrows_C = a->getRowsNum();
      ncolumns_C = b->getColumnsNum();
    }
    return canMultiply;
  }

  QuestApp::QuestApp(const QuestAppOptions& opt)
    : CliApp(dynamic_cast<const CliAppOptions&>(opt))
    , m_input(opt.getInOpts()),m_output(opt.getOutOpts()),m_algo(opt.getAlgoOpts())
    , m_pAlgoParameters(new matmul::AlgoParameters({m_threading,m_precision,m_algo,nullptr,nullptr,nullptr
          ,0,0,false,false,numeric::TMatrixStorage::RowMajor}))
    , m_pfA(new InFileText(m_input.filename_A,m_input.filetype,true))
    , m_pfB(new InFileText(m_input.filename_B,m_input.filetype,true))
    , m_pfC(new OutFileText(m_output.filename,m_output.filetype,false))
  {
  }

  QuestApp::QuestApp(const QuestAppOptions& opt, ProgressCtrl* pc)
    : CliApp(dynamic_cast<const CliAppOptions&>(opt), pc)
    , m_input(opt.getInOpts()),m_output(opt.getOutOpts()),m_algo(opt.getAlgoOpts())
    , m_pAlgoParameters(new matmul::AlgoParameters({m_threading,m_precision,m_algo,nullptr,nullptr,nullptr
          ,0,0,false,false,numeric::TMatrixStorage::RowMajor}))
    , m_pfA(new InFileText(m_input.filename_A,m_input.filetype,true))
    , m_pfB(new InFileText(m_input.filename_B,m_input.filetype,true))
    , m_pfC(new OutFileText(m_output.filename,m_output.filetype,false))
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
    m_input.filetype = _input_opt_names[0].type; m_input.filename_A = "data1";  m_input.filename_B = "data2";
    m_output.filetype = _output_opt_names[0].type; m_output.filename = "result";
    m_algo.type = A_NumCppSimple;
    m_pAlgoParameters.reset(new matmul::AlgoParameters({m_threading,m_precision,m_algo,nullptr,nullptr,nullptr
          ,0,0,false,false,numeric::TMatrixStorage::RowMajor}));
    m_pfA.reset(new InFileText(m_input.filename_A,m_input.filetype,true));
    m_pfB.reset(new InFileText(m_input.filename_B,m_input.filetype,true));
    m_pfC.reset(new OutFileText(m_output.filename,m_output.filetype,false));
  }

  const std::string QuestApp::Summary() const
  {
    std::string s;
    //TODO: rewrite using getNameByType(...) helpers
    for ( int i = 0; _algo_opt_names[i].name; ++i )
    {
      if( _algo_opt_names[i].type == m_pAlgoParameters->Aopt.type )
      {
        s.append("Running dense matrix multiplication algorithm ").append(_algo_opt_names[i].name);
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
    //read input matrices from files
    m_pAlgoParameters->a->readFromFile(*m_pfA,m_pAlgoParameters->transposeA);
    m_pAlgoParameters->b->readFromFile(*m_pfB,m_pAlgoParameters->transposeB);
  }

  void QuestApp::writeOutput()
  {
    //write ouput matrix to file
    m_pAlgoParameters->c->writeToFile(*m_pfC,false,m_pAlgoParameters->Popt.print_precision);
  }

  void QuestApp::run()
  {
    //PRERUN:
    log().debug(Summary());
    //prepare matrix reading flags per algo
    if( (m_algo.type == A_NumCSimpleTranspose) || (m_algo.type == A_NumCppSimpleTranspose) ) {
      m_pAlgoParameters->transposeB = true;
    } else if( m_algo.type == A_NumFortranSimpleTranspose ) {
      m_pAlgoParameters->transposeA = true;
    }
    switch(m_algo.type)
    {
      case A_NumFortran :
      case A_NumFortranSimple :
      case A_NumFortranSimpleTranspose :
      case A_NumFortranInternal :
      case A_NumFortranStrassen :
      case A_ExtCppEigen :
      case A_ExtCppArmadillo :
      case A_ExtCBLAS :
        m_pAlgoParameters->storage = numeric::TMatrixStorage::ColumnMajor;
      default:
        break;
    }
    TMatrixType mat_type = TMatrixType::Array;
    switch(m_algo.type)
    {
      case A_NumCppValarray :
      case A_NumCppValarrayTranspose :
        mat_type = TMatrixType::ValArray;
        break;
      case A_ExtCppBoost :
        mat_type = TMatrixType::BoostUblas;
        break;
      case A_ExtCppEigen :
        mat_type = TMatrixType::Eigen;
        break;
      case A_ExtCppMTL :
        mat_type = TMatrixType::MTL;
        break;
      case A_ExtCppArmadillo :
        mat_type = TMatrixType::Armadillo;
        break;
      default:
        mat_type = TMatrixType::Array;
        break;
    }
    TMatrixFlavour mat_flavour = TMatrixFlavour::Dense;
    //create input matrices
    log().debug("creating input matrices...");
    m_pAlgoParameters->a.reset(NewMatrix(m_precision.type,
          m_pfA.get(), false, m_pAlgoParameters->transposeA, m_pAlgoParameters->storage, mat_type, mat_flavour));
    m_pAlgoParameters->b.reset(NewMatrix(m_precision.type,
          m_pfB.get(), false, m_pAlgoParameters->transposeB, m_pAlgoParameters->storage, mat_type, mat_flavour));
    log().fdebug("found input matrices: A ( %zu x %zu ), B ( %zu x %zu )",
        m_pAlgoParameters->a->getRowsNum(), m_pAlgoParameters->a->getColumnsNum(),
        m_pAlgoParameters->b->getRowsNum(), m_pAlgoParameters->b->getColumnsNum());
    //sanity check(input can be used by selected algorithm)
    log().debug("sanity check of matrix sizes...");
    //check that sizes are valid
    if(!m_pAlgoParameters->initCsize()) {
      throw FileFormatValueBoundsError("Invalid matrix sizes in files, can't multiply them with selected algorithm",
          m_pfA->fileType(), m_pfA->fileName().c_str(), m_pfA->lineNum());
    }
    //initialize output structure[s] and possibly file[s]
    //create output matrix
    log().fdebug("creating output matrix C ( %zu x %zu )...", m_pAlgoParameters->nrows_C, m_pAlgoParameters->ncolumns_C);
    m_pAlgoParameters->c.reset(NewMatrix(m_precision.type,
          m_pAlgoParameters->nrows_C, m_pAlgoParameters->ncolumns_C, true, m_pAlgoParameters->storage, mat_type, mat_flavour));
    //read input data
    log().debug("reading input matrices...");
    readInput();
    //TODO: estimate output file size, check available disk space
    //log stats
    log().debug(SysUtil::getMemStats());
    //RUN_IO_ITER:
    //run selected algorithm
    log().debug("running main task...");
    matmul::perform(*m_pAlgoParameters,log());
    //output results
    log().debug("writing output matrix...");
    writeOutput();
    //POSTRUN:
    //finalize output file
    //log stats
    log().debug("have a nice day.");
  }

}
