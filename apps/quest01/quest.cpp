#include "quest.hpp"
#include "matmul.hpp"

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
    m_output.filetype = output;
    //parse filename for C
#ifdef HAVE_BOOST
    m_output.filename = argMap["out-C"].as<string>();
#endif
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
    m_algo.type = algo;
    return true;
  }


  QuestApp::QuestApp(const QuestAppOptions& opt)
    : CliApp(dynamic_cast<const CliAppOptions&>(opt))
    , m_input(opt.getInOpts()),m_output(opt.getOutOpts()),m_algo(opt.getAlgoOpts())
    , m_pAlgoParameters(new matmul::AlgoParameters({m_threading,m_precision,m_algo,nullptr,nullptr,nullptr
          ,false,false,numeric::TMatrixStorage::RowMajor,0,0}))
    , m_pfA(new InFileText(m_input.filename_A,m_input.filetype,true))
    , m_pfB(new InFileText(m_input.filename_B,m_input.filetype,true))
    , m_pfC(new OutFileText(m_output.filename,m_output.filetype,false))
  {
  }

  QuestApp::QuestApp(const QuestAppOptions& opt, ProgressCtrl* pc)
    : CliApp(dynamic_cast<const CliAppOptions&>(opt), pc)
    , m_input(opt.getInOpts()),m_output(opt.getOutOpts()),m_algo(opt.getAlgoOpts())
    , m_pAlgoParameters(new matmul::AlgoParameters({m_threading,m_precision,m_algo,nullptr,nullptr,nullptr
          ,false,false,numeric::TMatrixStorage::RowMajor,0,0}))
    , m_pfA(new InFileText(m_input.filename_A,m_input.filetype,true))
    , m_pfB(new InFileText(m_input.filename_B,m_input.filetype,true))
    , m_pfC(new OutFileText(m_output.filename,m_output.filetype,false))
  {
  }

  QuestApp::QuestApp(ProgressCtrl* pc):
    CliApp(pc)
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
          ,false,false,numeric::TMatrixStorage::RowMajor,0,0}));
    m_pfA.reset(new InFileText(m_input.filename_A,m_input.filetype,true));
    m_pfB.reset(new InFileText(m_input.filename_B,m_input.filetype,true));
    m_pfC.reset(new OutFileText(m_output.filename,m_output.filetype,false));
  }

  void QuestApp::readInput()
  {
    //read input matrices from files
    m_pAlgoParameters->a->readFromFile(*m_pfA,m_pAlgoParameters->transposeA);
    m_pAlgoParameters->b->readFromFile(*m_pfB,m_pAlgoParameters->transposeB);
  }

  void QuestApp::run()
  {
    //PRERUN:
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
        m_pAlgoParameters->storage = numeric::TMatrixStorage::ColumnMajor;
      default:
        break;
    }
    //create input matrices
    m_pAlgoParameters->a.reset(NewMatrix(m_pAlgoParameters->Popt.type,
          m_pfA.get(), false, m_pAlgoParameters->transposeA, m_pAlgoParameters->storage));
    m_pAlgoParameters->b.reset(NewMatrix(m_pAlgoParameters->Popt.type,
          m_pfB.get(), false, m_pAlgoParameters->transposeB, m_pAlgoParameters->storage));
    //sanity check(input can be used by selected algorithm)
    //check that sizes are valid
    bool canMultiply = false;
    if(m_pAlgoParameters->transposeB) {
      canMultiply = ( m_pAlgoParameters->a->getColumnsNum() == m_pAlgoParameters->b->getColumnsNum() );
      m_pAlgoParameters->nrows_C = m_pAlgoParameters->a->getRowsNum();
      m_pAlgoParameters->ncolumns_C = m_pAlgoParameters->b->getRowsNum();
    } else if(m_pAlgoParameters->transposeA) {
      canMultiply = ( m_pAlgoParameters->a->getRowsNum() == m_pAlgoParameters->b->getRowsNum() );
      m_pAlgoParameters->nrows_C = m_pAlgoParameters->a->getColumnsNum();
      m_pAlgoParameters->ncolumns_C = m_pAlgoParameters->b->getColumnsNum();
    } else {
      canMultiply = ( m_pAlgoParameters->a->getColumnsNum() == m_pAlgoParameters->b->getRowsNum() );
      m_pAlgoParameters->nrows_C = m_pAlgoParameters->a->getRowsNum();
      m_pAlgoParameters->ncolumns_C = m_pAlgoParameters->b->getColumnsNum();
    }
    if(!canMultiply) {
      throw FileFormatValueBoundsError("Invalid matrix sizes in files, can't multiply them with selected algorithm",
          m_pfA->fileType(), m_pfA->fileName().c_str(), m_pfA->lineNum());
    }
    //initialize output structures and possibly file
    //create output matrix
    m_pAlgoParameters->c.reset(NewMatrix(m_pAlgoParameters->Popt.type,
          m_pAlgoParameters->nrows_C, m_pAlgoParameters->ncolumns_C, true, m_pAlgoParameters->storage));
    //read input data
    readInput();
    //estimate output file size, check available disk space
    //log stats
    //RUN_IO_ITER:
    //run selected algorithm
    matmul::perform(*m_pAlgoParameters);
    //output results
    //m_pAlgoParameters->c->writeToFile(*m_pfC);
    //POSTRUN:
    //finalize output file
    //log stats
  }

}
