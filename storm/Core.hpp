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
#ifndef libwin
typedef void* HANDLE;
typedef void* LPOVERLAPPED;
typedef struct HWND__* HWND;
#else
#include <winnt.h>
#include <minwinbase.h>
#include <winuser.h>
#endif
#endif

#ifdef WHOA_STORM_FLAVOR_DIABLO2
int32_t STORMCDECL StormDestroy();
#else
int32_t STORMAPI StormDestroy();
#endif

#endif
