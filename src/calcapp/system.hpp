#pragma once
#ifndef _UTIL_HPP
#define _UTIL_HPP
#include "config.h"

#include <string>

#include "calcapp/io.hpp"

namespace Calc {

namespace SysUtil {
    std::string getCurrentDirectory();
    std::string getOSVersion();
    std::string getCpuSpec();
    std::string getBuildOptions();
    unsigned getCpuCoresCount();
    double getCurTimeSec();
    bool getFreeDiskMB(double *SizeMB, std::string Name);
    bool isEnoughDiskSpace(std::string fileName, double minMB, double * curMB = 0);
    void throwOnOutOfDiskSpace(TFileType fileType, std::string fileName, double minMB);
}

}

#endif /* _UTIL_HPP */
