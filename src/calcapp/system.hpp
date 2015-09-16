#pragma once
#ifndef _UTIL_HPP
#define _UTIL_HPP
#include "config.h"

#include <string>

#include "calcapp/io.hpp"

using std::string;

namespace Calc {
   class SysUtil {
    static string getCurrentDirectory();
    static string getOSVersion();
    static string getCpuSpec();
    static string getBuildOptions();
    static unsigned getCpuCoresCount();
    static double getCurTimeSec();
    static bool getFreeDiskMB(double *SizeMB, string Name);
    static bool isEnoughDiskSpace(string fileName, double minMB, double * curMB = 0);
    static void throwOnOutOfDiskSpace(TFileType fileType, string fileName, double minMB);
   };
}

#endif /* _UTIL_HPP */
