#include "calcapp/app.hpp"
#include "calcapp/exception.hpp"

#include <iostream>

#ifdef HAVE_TCMALLOC
#include <google/malloc_extension.h>
#endif

namespace Calc {

void App::init_helper(const ProgressCtrl * const p)
{
#ifdef HAVE_TCMALLOC
  MallocExtension::instance()->Initialize();
#endif
  if(p==nullptr)
    throw InternalError(FERR_GENERAL_ERROR, "App instance failed to initialise");
}

App::App(const AppOptions& opt, ProgressCtrl * const p):
  m_AppName(opt.getAppName()),m_AppVersion(opt.getAppVersion())
  ,m_logging(opt.getLogOpts()),m_threading(opt.getThreadOpts()),m_precision(opt.getPrecOpts())
  ,m_pProgress(p)
{
  init_helper(m_pProgress);
}

App::App(ProgressCtrl * const p):m_pProgress(p)
{
  init_helper(m_pProgress);
}

App::App():App(nullptr){
}

}
