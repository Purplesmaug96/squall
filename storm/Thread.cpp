#include "storm/Thread.hpp"

#if defined(WHOA_SYSTEM_WIN)
#include <windows.h>
#endif

#if defined(WHOA_SYSTEM_MAC) || defined(WHOA_SYSTEM_LINUX) || defined(WHOA_SYSTEM_ANDROID)
#include <pthread.h>
#endif

long unsigned int SGetCurrentThreadId() {
#if defined(WHOA_SYSTEM_WIN)
    return GetCurrentThreadId();
#endif

#if defined(WHOA_SYSTEM_MAC) || defined(WHOA_SYSTEM_LINUX) || defined(WHOA_SYSTEM_ANDROID)
    return (long unsigned int)(pthread_self());
#endif
}
