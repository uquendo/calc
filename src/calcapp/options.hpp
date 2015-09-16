#pragma once
#ifndef _OPTIONS_HPP
#define _OPTIONS_HPP
#include "config.h"

#include <string>
#include <iostream>

//common long options
#define HELP_OPT "help"
#define VERBOSE_OPT "verbose"
#define THREADS_OPT "threads"
#define ALGO_OPT "algorithm"
#define PRECISION_OPT "precision"
#define THREADING_OPT "threading"

namespace Calc {
    struct LoggingOptions {
        bool verbose;
        int loglevel;
        bool profile;
        bool progress;
        std::string logfile;
    };

    struct ThreadingOptions {
        TThreading type;
        unsigned num;
    };

    struct PrecisionOptions {
        TPrecision type;
        unsigned decimal_digits;
        std::streamsize precision;
    };

    struct ThreadingOptName {
        const char * name;
        const char * opt;
        TThreading type;
    };

    extern ThreadingOptName _threading_opt_names[];

    struct PrecisionOptName {
        const char * name;
        const char * opt; 
        TPrecision type;
    };

    extern PrecisionOptName _precision_opt_names[];

class AppOptions {
private:
        LoggingOptions m_logging;
        ThreadingOptions m_threading;
        PrecisionOptions m_precision;
        //define for derived class for example:
        //InputOptions m_input;
        //OutputOptions m_output;
        //AlgoOptions m_algorithm; 
public:
        AppOptions();
        virtual ~AppOptions();
        //set options from command line arguments
        virtual bool processOptions(int argc, char* argv[]);
        //const access functions
        const LoggingOptions& getLogOpts() const { return m_logging; };
        const ThreadingOptions& getThreadOpts() const { return m_threading; };
        const PrecisionOptions& getPrecOpts() const { return m_precision; };
        //const InputOptions& getInOpts() const { return m_input; };
        //const OutputOptions& getOutOpts() const { return m_output; };
        //const AlgoOptions& getAlgoOpts() const { return m_algorithm; };
};

}

#endif /* _OPTIONS_HPP */
