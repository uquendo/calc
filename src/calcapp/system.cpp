#include "calcapp/system.hpp"
#include "calcapp/exception.hpp"
#include "calcapp/log.hpp"

#include <cctype>
#include <iostream>
#include <string>
#include <sstream>
#include <fstream>
#include <algorithm>
#include <iostream>
#include <vector>

#include "boost/date_time/posix_time/posix_time.hpp"
#include "boost/filesystem/operations.hpp"
#include "boost/thread/thread.hpp"
#include "boost/format.hpp"

#ifdef _WIN32
#include <windows.h>
#include <intrin.h>
#endif

#ifdef __linux
#include <sys/utsname.h>
#include <gnu/libc-version.h>
#endif

using namespace std;

namespace Calc {

bool SysUtil::getFreeDiskMB(double *SizeMB, string Name)
{
	boost::filesystem::path p(Name);
	p = boost::filesystem::absolute(p);
	if ( p.has_parent_path() )
		p = p.parent_path();
	boost::filesystem::space_info s = boost::filesystem::space(p);
	*SizeMB = (double) s.free / 1024.0 / 1024.0;
	return true;
}

bool SysUtil::isEnoughDiskSpace(const std::string  fileName, double minMB, double * curMB)
{
	double mb;
	SysUtil::getFreeDiskMB(&mb, fileName);
	if ( curMB )
		*curMB = mb;
	return ( mb >= minMB );
}

void SysUtil::throwOnOutOfDiskSpace(TFileType fileType, const std::string fileName, double minMB)
{
	double mb;
	if ( ! isEnoughDiskSpace(fileName, minMB, &mb) )
		throw OutOfDiskSpaceError("No enough free space to write file", fileType, fileName.c_str(), (unsigned long) mb, (unsigned long) minMB);
}

unsigned SysUtil::getCpuCoresCount() {
	return (int) boost::thread::hardware_concurrency();
}

double SysUtil::getCurTimeSec()
{
	return (boost::posix_time::microsec_clock::local_time() 
				- boost::posix_time::ptime(boost::gregorian::date(1970,1,1))
			).total_milliseconds() / 1000.0;
}

#ifdef _WIN32

typedef BOOL (WINAPI *LPFN_ISWOW64PROCESS) (HANDLE, PBOOL);

BOOL IsWow64()
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

string SysUtil::getOSVersion() {
#ifdef _WIN32
   OSVERSIONINFOEX osvi;
   BOOL bOsVersionInfoEx;

   string bit=IsWow64()?" 64bit":" 32bit";
   
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
	
	return string(u.sysname) + " " + u.release + " " + u.version + " " + u.machine + "; libc version: " + gnu_get_libc_version();
#else
   return "Unknown OS";
#endif
}

string SysUtil::getCpuSpec() {
#if __linux
	return IOUtil::fileGrep("/proc/cpuinfo", boost::regex("^model name\\S*:\\S*(.*)$"), true);
#elif _MSC_VER
	int cpuInfo[4] = {-1};
	__cpuid(cpuInfo, 0);
	return str(boost::format("cpuid: %08x %08x %08x %08x") % cpuInfo[0] % cpuInfo[1] % cpuInfo[2] % cpuInfo[3]);
#else
	return "Unknown OS";
#endif
}

string SysUtil::getCurrentDirectory() 
{
	return boost::filesystem::absolute(boost::filesystem::path(".")).string();
}

}

