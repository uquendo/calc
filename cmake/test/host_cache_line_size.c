#include <stdio.h>
#include <stdint.h>
#include <stddef.h>
#include <stdlib.h>
#include <malloc.h>

//headers for runtime cache line size detection
#if defined(_WIN32)
# include <windows.h>
#elif defined(__linux__)
# include <unistd.h>
#elif defined(__APPLE__)
# include <sys/sysctl.h>
#endif

#define DEFAULT_CACHE_LINE_SIZE 64

size_t host_cache_line_size()
{
  size_t line_size = 0;
#if defined(_WIN32)
  DWORD buffer_size = 0;
  DWORD i = 0;
  SYSTEM_LOGICAL_PROCESSOR_INFORMATION * buffer = 0;
  GetLogicalProcessorInformation(0, &buffer_size);
  buffer = (SYSTEM_LOGICAL_PROCESSOR_INFORMATION *)malloc(buffer_size);
  GetLogicalProcessorInformation(&buffer[0], &buffer_size);
  for (i = 0; i != buffer_size / sizeof(SYSTEM_LOGICAL_PROCESSOR_INFORMATION); ++i) {
      if (buffer[i].Relationship == RelationCache && buffer[i].Cache.Level == 1) {
          line_size = buffer[i].Cache.LineSize;
          break;
      }
  }
  free(buffer);
#elif defined(__linux__)
  long sysconf_line_size = sysconf(_SC_LEVEL1_DCACHE_LINESIZE);
  if(sysconf_line_size != -1)
    line_size = sysconf_line_size;
#elif defined(__APPLE__)
  size_t sizeof_line_size = sizeof(line_size);
  sysctlbyname("hw.cachelinesize", &line_size, &sizeof_line_size, 0, 0);
#endif
  if(line_size > 0)
    return line_size;
  else
    return DEFAULT_CACHE_LINE_SIZE;
}

int main()
{
  return host_cache_line_size();
}
