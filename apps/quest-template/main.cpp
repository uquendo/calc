#include "quest.hpp"

#include <iostream>
#include <iomanip>
#include <string>
#include <ctime>

using namespace std;

int main(int argc, char** argv){
	try {
        Calc::QuestAppOptions calcOpt;
		if ( ! calcOpt.processOptions(argc, argv) )
			return 0;

        //cout.precision(calcOpt.getPrecOpts().precision);
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

	} catch(exception& e) {
		cerr << "Exception in : " << e.what() << endl;
	} catch (...) {
		cerr << "Exception in " << endl;
	}
    return 0;
}

