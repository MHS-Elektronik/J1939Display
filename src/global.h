#ifndef __GLOBAL_H__
#define __GLOBAL_H__

/*
#define safe_min(a,b) (((a) < (b)) ? (a) : (b))
#define safe_strcp(dst, dst_max, src, count) do {memcpy(dst, src, safe_min(count, dst_max)); \
	((char*)dst)[safe_min(count, dst_max)-1] = 0;} while(0)
#define safe_strcpy(dst, dst_max, src) safe_strcp(dst, dst_max, src, strlen(src)+1)

#ifndef __WIN32__
  #define safe_sprintf snprintf
#else
  #if _MSC_VER >= 1500
    #define safe_sprintf(dst, dst_max, format, ...) _snprintf_s(dst, dst_max, _TRUNCATE, format, __VA_ARGS__)
  #else
    #define safe_sprintf _snprintf
  #endif
#endif*/


#ifdef __WIN32__
// ****** Windows
#include <windows.h>

#define MHS_LOCK_TYPE CRITICAL_SECTION
#define MHS_LOCK_INIT(x) InitializeCriticalSection((x))
#define MHS_LOCK_DESTROY(x) DeleteCriticalSection((x))
#define MHS_LOCK_ENTER(x) EnterCriticalSection((x))
#define MHS_LOCK_LEAVE(x) LeaveCriticalSection((x))

#define mhs_sleep(x) Sleep(x)
#define get_tick() GetTickCount()

#else
// ****** Linux
//#include <pthread.h>
//#include <unistd.h>
//#include <stdlib.h>
//#include <sys/time.h>
//#include <stdint.h>

#define CALLBACK_TYPE
#define MHS_LOCK_TYPE pthread_mutex_t
#define MHS_LOCK_INIT(x) pthread_mutex_init((x), NULL)
#define MHS_LOCK_DESTROY(x) pthread_mutex_destroy((x))
#define MHS_LOCK_ENTER(x) pthread_mutex_lock((x))
#define MHS_LOCK_LEAVE(x) pthread_mutex_unlock((x))

#define mhs_sleep(x) usleep((x) * 1000)

#endif

#endif
