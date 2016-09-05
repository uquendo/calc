#include "quest.hpp"
#include "newton.hpp"

#include "calcapp/system.hpp"

#include <cassert>
#include <cstring>

using std::string;

namespace Calc {

  QuestAppOptions::QuestAppOptions():
    CliAppOptions(string(APP_NAME),string(APP_VERSION))
  {
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

  bool QuestAppOptions::parseOptions(int argc, char* argv[]){
    bool result = CliAppOptions::parseOptions(argc,argv);
    return result;
  }

  QuestApp::QuestApp(const QuestAppOptions& opt)
    : CliApp(dynamic_cast<const CliAppOptions&>(opt))
    , m_pAlgoParameters(new newton::AlgoParameters({m_threading,m_precision,ctrl()}))
  {
  }

  QuestApp::QuestApp(const QuestAppOptions& opt, ProgressCtrl* pc)
    : CliApp(dynamic_cast<const CliAppOptions&>(opt), pc)
    , m_pAlgoParameters(new newton::AlgoParameters({m_threading,m_precision,ctrl()}))
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

  const string QuestApp::Summary() const
  {
    std::string s;
    //TODO: rewrite using getNameByType(...) helpers
    s.append("Running newton solver algorithm");
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

  void QuestApp::run()
  {
    //PRERUN:
    log().debug(Summary());
    //init structures
    //log().debug("allocating values table...");
    //log stats
    //log().debug(SysUtil::getMemStats());
    log().debug("running main task...");
    newton::perform(*m_pAlgoParameters,log());
    //log stats
    log().debug("have a nice day.");
  }

}
