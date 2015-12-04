#pragma once
#ifndef _UTIL_HPP
#define _UTIL_HPP
#include "config.h"

#include <string>

#include "calcapp/io.hpp"
#include "numeric/parallel.hpp"

namespace Calc {

namespace SysUtil {
    std::string getCurrentDirectory();
    std::string getOSVersion();
    std::string getCpuSpec();
    std::string getBuildOptions();
    std::string getMemStats();
    unsigned getCpuCoresCount();
    double getCurTimeSec();
    bool getFreeDiskMB(double *SizeMB, std::string Name);
    bool isEnoughDiskSpace(std::string fileName, double minMB, double * curMB = nullptr);
    void throwOnOutOfDiskSpace(TFileType fileType, std::string fileName, double minMB);
    void initThreadingBackend(const numeric::TThreading type, const unsigned n = 0);
}

}

#endif /* _UTIL_HPP */
