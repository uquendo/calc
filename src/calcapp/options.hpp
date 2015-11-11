#pragma once
#ifndef _OPTIONS_HPP
#define _OPTIONS_HPP
#include "config.h"

#include <cstddef>
#include <string>

#include "numeric/parallel.hpp"
#include "numeric/real.hpp"
#include "calcapp/log.hpp"

using namespace numeric;
using std::string;
using std::ptrdiff_t;

//common long options
#define HELP_OPT "help"
#define ABOUT_OPT "version"
#define VERBOSE_OPT "verbose"
#define LOGFILE_OPT "logfile"
#define THREADS_OPT "threads"
#define ALGO_OPT "algorithm"
#define PRECISION_OPT "precision"
#define DIGITS_OPT "digits"
#define THREADING_OPT "threading"

namespace Calc {
    struct LoggingOptions {
        bool verbose;
        Logger::LogLevel level;
        bool profile;
        bool progress;
        string filename;
    };

    struct ThreadingOptions {
        TThreading type;
        unsigned num;
    };

    struct PrecisionOptions {
        TPrecision type;
        unsigned decimal_digits;
        ptrdiff_t print_precision; //for output i.e. via ios_base::precision(streamsize)
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
protected:
        std::string m_AppName;
        std::string m_AppVersion;
        LoggingOptions m_logging;
        ThreadingOptions m_threading;
        PrecisionOptions m_precision;
        //define for derived class for example:
        //InputOptions m_input;
        //OutputOptions m_output;
        //AlgoOptions m_algorithm; 
public:
        AppOptions(std::string AppName = std::string("SomeApp"), std::string AppVersion = std::string("0.1"));
        virtual ~AppOptions();
        //set options from command line arguments
        virtual bool processOptions(int argc, char* argv[]) = 0;
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
