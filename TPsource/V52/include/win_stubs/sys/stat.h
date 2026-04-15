/* Redirect Windows sys\stat.h to real POSIX sys/stat.h */
#ifndef _SYS_STAT_STUB
#define _SYS_STAT_STUB
#include_next <sys/stat.h>
#endif
