#include "quest.hpp"

#include <iostream>
#include <iomanip>
#include <string>
#include <ctime>

int main(int argc, char** argv){
	try {
        Calc::QuestAppOptions calcOpt;
		if ( ! calcOpt.processOptions(argc, argv) )
			return 0;

        //std::cout.precision(calcOpt.getPrecOpts().precision);
        Calc::CliProgress pc(calcOpt.getLogOpts());

        std::string cmdLine("Command line: ");
        for ( int i = 0; i < argc; i++ ) {
            cmdLine.append(argv[i]).append(" ");
        }
        pc.log().debug(cmdLine.c_str());
        time_t t;
        time(&t);
        pc.log().fdebug("Launch time: %s", ctime(&t));

        Calc::QuestApp calc(&pc);
        calc.setOptions(calcOpt);
        calc.readInput();
        calc.run();

	} catch(std::exception& e) {
        std::cerr << "Exception in : " << e.what() << std::endl;
	} catch (...) {
        std::cerr << "Exception in " << std::endl;
	}
    return 0;
}

