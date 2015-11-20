#include "calcapp/app.hpp"
#include "calcapp/exception.hpp"

#include <iostream>

namespace Calc {

//TODO: STUBS!
App::App():App(nullptr){
}

App::App(ProgressCtrl* p):m_pProgress(p){
  if(p==nullptr)
    throw InternalError(FERR_GENERAL_ERROR, "App instance failed to initialise");
}

}
