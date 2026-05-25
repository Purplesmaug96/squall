#ifndef STORM_ERROR_HPP
#define STORM_ERROR_HPP

#include "storm/error/Macros.hpp"
#include "storm/error/Types.hpp"
#include "Core.hpp"

#if defined(WHOA_SYSTEM_WIN)
#include <winerror.h>
#include <wtypes.h>
#endif

#if defined(WHOA_SYSTEM_MAC) || defined(WHOA_SYSTEM_LINUX) || defined(WHOA_SYSTEM_ANDROID) || defined(WHOA_SYSTEM_WII)
#define ERROR_SUCCESS           0
#define ERROR_FILE_NOT_FOUND    2
#define ERROR_INVALID_HANDLE    6
#define ERROR_NOT_ENOUGH_MEMORY 8
#define ERROR_HANDLE_EOF        38
#define ERROR_INVALID_PARAMETER 87
typedef uint32_t DWORD;
typedef int BOOL;
#endif

[[noreturn]] void STORMCDECL SErrDisplayAppFatal(const char* format, ...);


#ifdef WHOA_DISPLAY_ERR_EXTRA_ARG
int32_t STORMAPI SErrDisplayError(uint32_t errorcode, const char* filename, int32_t linenumber, const char* description, int32_t recoverable, uint32_t exitcode, uint32_t a7 = 0x11111111);
#else
int32_t STORMAPI SErrDisplayError(uint32_t errorcode, const char* filename, int32_t linenumber, const char* description, int32_t recoverable, uint32_t exitcode);
#endif

int32_t STORMCDECL SErrDisplayErrorFmt(uint32_t errorcode, const char* filename, int32_t linenumber, int32_t recoverable, uint32_t exitcode, const char* format, ...);

#ifdef WHOA_STORM_FLAVOR_DIABLO2
int32_t STORMCDECL SErrIsDisplayingError();
#else
int32_t STORMAPI SErrIsDisplayingError();
#endif

void STORMAPI SErrPrepareAppFatal(const char* filename, int32_t linenumber);

void STORMAPI SErrSetLastError(uint32_t errorcode);

#ifdef WHOA_STORM_FLAVOR_DIABLO2
DWORD STORMCDECL SErrGetLastError();
#else
uint32_t STORMAPI SErrGetLastError();
#endif

#ifdef WHOA_STORM_FLAVOR_DIABLO2
BOOL STORMAPI SErrSuppressErrors(int32_t suppress);
#else
void STORMAPI SErrSuppressErrors(int32_t suppress);
#endif

int STORMAPI SErrGetErrorStr(DWORD dwMessageId, char* lpBuffer, DWORD nSize);

#endif
