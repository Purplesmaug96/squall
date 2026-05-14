#ifndef STORM_CORE_HPP
#define STORM_CORE_HPP

#include <cstdint>

#ifndef STORMAPI
#if defined(_MSC_VER)
    #define STORMAPI __stdcall
    #define STORMCDECL __cdecl
#else
    #define STORMAPI
    #define STORMCDECL
#endif
#endif

#if !defined(WHOA_SYSTEM_WIN)
#ifndef __windows_shim
typedef void* HANDLE;
typedef void* LPOVERLAPPED;
typedef struct HWND__* HWND;
#else
#include <winnt.h>
#include <minwinbase.h>
#include <winuser.h>
#endif
#endif

int32_t STORMAPI StormDestroy();

#endif
