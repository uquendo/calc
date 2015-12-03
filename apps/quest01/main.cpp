#include "quest.hpp"

#include <iostream>
#include <iomanip>
#include <string>
#include <ctime>

int main(int argc, char** argv)
{
  try
  {
    Calc::QuestAppOptions calcOpt;
    if ( ! calcOpt.processOptions(argc, argv) )
      return 0;

    //std::cout.precision(calcOpt.getPrecOpts().print_precision);
    Calc::CliProgress pc(calcOpt.getLogOpts());
    std::string cmdLine("Command line: ");
    for ( int i = 0; i < argc; i++ )
    {
      cmdLine.append(argv[i]).append(" ");
    }
    pc.log().debug(cmdLine);
    std::time_t t;
    time(&t);
    pc.log().fdebug("Launch time: %s", ctime(&t));
//    pc.log().debug(calcOpt.About());

    Calc::QuestApp calc(calcOpt,&pc);
    calc.run();
  }
  catch (std::exception& e)
  {
    std::cerr << "Exception in " << argv[0] << " : " << e.what() << std::endl;
  }
  catch (...)
  {
    std::cerr << "Exception in " << argv[0] << " : " << std::endl;
  }
  return 0;
}

