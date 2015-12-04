#include "calcapp/system.hpp"
#include "calcapp/exception.hpp"
#include "calcapp/log.hpp"

#include "numeric/parallel.hpp"

#include <cctype>
#include <iostream>
#include <string>
#include <sstream>
#include <fstream>
#include <algorithm>
#include <iostream>
#include <vector>
#include <thread>
#include <chrono>

#ifdef HAVE_BOOST
#include "boost/filesystem/operations.hpp"
#include "boost/format.hpp"
#endif

#ifdef _WIN32
#include <windows.h>
#include <intrin.h>
#endif

#ifdef __linux
#include <sys/utsname.h>
#include <gnu/libc-version.h>
#endif

#ifdef HAVE_TCMALLOC
#include <google/malloc_extension.h>
#endif

namespace Calc {

namespace SysUtil {

std::string getMemStats()
{
  std::string r="";
#ifdef HAVE_TCMALLOC
  char buf[LINE_BUF_SIZE];
  MallocExtension::instance()->GetStats(&buf[0],LINE_BUF_SIZE);
  r.append(&buf[0]);
#endif
  return r;
}

bool getFreeDiskMB(double *SizeMB, std::string Name)
{
#ifdef HAVE_BOOST
  boost::filesystem::path p(Name);
  p = boost::filesystem::absolute(p);
  if ( p.has_parent_path() )
    p = p.parent_path();
  boost::filesystem::space_info s = boost::filesystem::space(p);
  *SizeMB = (double) s.free / 1024.0 / 1024.0;
  return true;
#endif
  return false;
}

bool isEnoughDiskSpace(const std::string  fileName, double minMB, double * curMB)
{
  double mb=0.0;
  if(!getFreeDiskMB(&mb, fileName))
    return true;
  if ( curMB )
    *curMB = mb;
  return ( mb >= minMB );
}

void throwOnOutOfDiskSpace(TFileType fileType, const std::string fileName, double minMB)
{
  double mb;
  if ( ! isEnoughDiskSpace(fileName, minMB, &mb) )
    throw OutOfDiskSpaceError("No enough free space to write file", fileType, fileName.c_str(), (unsigned long) mb, (unsigned long) minMB);
}

unsigned getCpuCoresCount() {
  return (unsigned) numeric::hardware_concurrency();
}

double getCurTimeSec()
{
  double r = 0.0;
  r = std::chrono::duration_cast< std::chrono::milliseconds > (
        std::chrono::system_clock::now().time_since_epoch() ).count()/ 1000.0;
  return r;
}

#ifdef _WIN32

typedef BOOL (WINAPI *LPFN_ISWOW64PROCESS) (HANDLE, PBOOL);

inline BOOL IsWow64()
{
    BOOL bIsWow64 = FALSE;

    LPFN_ISWOW64PROCESS fnIsWow64Process = (LPFN_ISWOW64PROCESS)GetProcAddress(
            GetModuleHandle("kernel32"),"IsWow64Process");

    if (fnIsWow64Process != NULL)
    {
        if (!fnIsWow64Process(GetCurrentProcess(),&bIsWow64))
        {
            // handle error?
        }
    }
    return bIsWow64!=FALSE;
}

#endif

std::string getOSVersion()
{
#ifdef _WIN32
   OSVERSIONINFOEX osvi;
   BOOL bOsVersionInfoEx;

   std::string bit=IsWow64()?" 64bit":" 32bit";
   
   // Try calling GetVersionEx using the OSVERSIONINFOEX structure.
   // If that fails, try using the OSVERSIONINFO structure.

   ZeroMemory(&osvi, sizeof(OSVERSIONINFOEX));
   osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);

   if( !(bOsVersionInfoEx = GetVersionEx ((OSVERSIONINFO *) &osvi)) )
   {
      osvi.dwOSVersionInfoSize = sizeof (OSVERSIONINFO);
      if (! GetVersionEx ( (OSVERSIONINFO *) &osvi) ) 
         return "Unknown";
   }

   switch (osvi.dwPlatformId)
   {
      // Test for the Windows NT product family.
      case VER_PLATFORM_WIN32_NT:

      // Test for the specific product.
    if ( osvi.dwMajorVersion == 10 && osvi.dwMinorVersion == 0 )
         return "Microsoft Windows 10"+bit;

    if ( osvi.dwMajorVersion == 6 && osvi.dwMinorVersion == 3 )
         return "Microsoft Windows 8.1"+bit;

    if ( osvi.dwMajorVersion == 6 && osvi.dwMinorVersion == 2 )
         return "Microsoft Windows 8"+bit;

    if ( osvi.dwMajorVersion == 6 && osvi.dwMinorVersion == 1 )
         return "Microsoft Windows 7"+bit;
  
    if ( osvi.dwMajorVersion == 6 && osvi.dwMinorVersion == 0 )
         return "Microsoft Windows Vista"+bit;

      if ( osvi.dwMajorVersion == 5 && osvi.dwMinorVersion == 2 )
         return "Microsoft Windows Server 2003"+bit;

      if ( osvi.dwMajorVersion == 5 && osvi.dwMinorVersion == 1 )
         return "Microsoft Windows XP"+bit;

      if ( osvi.dwMajorVersion == 5 && osvi.dwMinorVersion == 0 )
         return "Microsoft Windows 2000";

      if ( osvi.dwMajorVersion <= 4 )
         return "Microsoft Windows NT";

      // Test for the Windows Me/98/95.
      case VER_PLATFORM_WIN32_WINDOWS:

      if (osvi.dwMajorVersion == 4 && osvi.dwMinorVersion == 0)
      {
          return "Microsoft Windows 95";
      } 

      if (osvi.dwMajorVersion == 4 && osvi.dwMinorVersion == 10)
      {
          return "Microsoft Windows 98";
      } 

      if (osvi.dwMajorVersion == 4 && osvi.dwMinorVersion == 90)
      {
          return "Microsoft Windows Millennium Edition";
      } 
      break;

      case VER_PLATFORM_WIN32s:
        return "Microsoft Win32s";
   }

  return "Microsoft Windows";
#elif __linux
  utsname u;
  if ( uname(&u) != 0 ) {
    Logger::system().warning("Error detecting linux version");
    return "Unknown";
  }
  
  return std::string(u.sysname) + " " + u.release + " " + u.version + " " + u.machine + "; libc version: " + gnu_get_libc_version();
#else
   return "Unknown OS";
#endif
}

std::string getCpuSpec()
{
  std::string r="";
#if __linux
  try{
    std::regex rg("model name[[:space:]]*:[[:space:]]*(.*)");
    r= IOUtil::fileGrep("/proc/cpuinfo", rg, true, 1);
  }catch(std::regex_error& e){
    //beware of pre-4.9 gcc! see for example https://stackoverflow.com/a/12665408
//    std::cerr << e.what() << e.code() << std::endl;
  }
  return r;
#elif _MSC_VER
  int cpuInfo[4] = {-1};
  __cpuid(cpuInfo, 0);
# ifdef HAVE_BOOST
  r = str(boost::format("cpuid: %08x %08x %08x %08x") % cpuInfo[0] % cpuInfo[1] % cpuInfo[2] % cpuInfo[3]);
# else
  r.append("cpuid: ");
  r.append(IOUtil::to_string_hex(cpuInfo[0])).append(" ").append(IOUtil::to_string_hex(cpuInfo[1])).append(" ");
  r.append(IOUtil::to_string_hex(cpuInfo[2])).append(" ").append(IOUtil::to_string_hex(cpuInfo[3]));
# endif
  return r;
#else
  return "Unknown OS";
#endif
}

std::string getBuildOptions()
{
  //here goes macro frenzy
  std::string r = "";
  r+="build toolchain : " CMAKE_CXX_COMPILER_ID " " CMAKE_CXX_COMPILER_VERSION "\n";
  r+="build type : " CMAKE_BUILD_TYPE "\n";
  //git revision
#ifdef INFO_GIT_SHA1
  r+="git sha1 : " INFO_GIT_SHA1 "\n";
#endif
#ifdef INFO_GIT_REFSPEC
//  r+="git refspec : " INFO_GIT_REFSPEC "\n";
#endif
  //BUILD_* options
  r+="build options : ";
#ifdef INFO_BUILD_OPTIONS
  r+=INFO_BUILD_OPTIONS;
#endif
  r+="\n";
  //HAVE_* options
  r+="third parties options : ";
#ifdef INFO_HAVE_OPTIONS
  r+=INFO_HAVE_OPTIONS;
#endif
  r+="\n";
  //build string
#if defined(INFO_C_FLAGS) && defined (INFO_CXX_FLAGS) && defined(INFO_COMPILE_OPTIONS)
  r+="CMAKE_C_FLAGS: " INFO_C_FLAGS "\n";
  r+="CMAKE_CXX_FLAGS: " INFO_CXX_FLAGS "\n";
  r+="extra compilation options: " INFO_COMPILE_OPTIONS "\n";
#endif
  return r;
}

std::string getCurrentDirectory()
{
  std::string r = "";
#ifdef HAVE_BOOST
  r = boost::filesystem::absolute(boost::filesystem::path(".")).string();
#endif
  return r;
}

}

}

