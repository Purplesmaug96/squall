#pragma once

#ifdef WHOA_SYSTEM_WIN
#include <Windows.h>
#else
#ifndef libwin
#include <stdint.h>
typedef uint32_t BOOL;
typedef void* HKEY;
typedef uint8_t* LPBYTE;
typedef uint32_t DWORD;
typedef DWORD* LPDWORD;
typedef char* LPSTR;
typedef long LONG;
#define TRUE true
#define FALSE false
#else
#include <windows.h>
#endif
#endif

#include "Core.hpp"

enum SREG_Flags {
    SREG_DEFAULT = 0x00,               // Look into both HKEY_LOCAL_MACHINE and HKEY_CURRENT_USER and under "Software\Blizzard Entertainment\"
    SREG_EXCLUDE_LOCAL_MACHINE = 0x01, // excludes checking the HKEY_LOCAL_MACHINE hive
    SREG_BATTLE_NET = 0x02,            // Look under "Software\\Battle.net\\" instead
    SREG_EXCLUDE_CURRENT_USER = 0x04,  // excludes checking the HKEY_CURRENT_USER hive
    SREG_ABSOLUTE = 0x10,              // specifies that the key is not a relative key
};

/*

/// Imported by ['D2Direct3D.dll', 'D2DDraw.dll']
D2FUNC_DLL_NP(STORM, SRegLoadData, BOOL, __stdcall, (const char* keyname, const char* valuename, size_t nSize, LPBYTE lpData, uint32_t nFlags, LPDWORD lpcbData), 0x25840); // Storm.#421

/// Imported by ['D2Launch.dll', 'D2Direct3D.dll', 'Fog.dll', 'D2DDraw.dll', 'D2Client.dll']
D2FUNC_DLL_NP(STORM, SRegLoadString, BOOL, __stdcall, (const char* keyname, const char* valuename, uint32_t nFlags, LPSTR pBuffer, size_t nBufferSize), 0x25a00); // Storm.#422

/// Imported by ['D2gfx.dll', 'D2Launch.dll', 'D2Direct3D.dll', 'Fog.dll', 'D2CMP.dll', 'D2Multi.dll', 'D2DDraw.dll', 'D2sound.dll', 'D2Game.dll', 'D2Client.dll']
D2FUNC_DLL_NP(STORM, SRegLoadValue, BOOL, __stdcall, (const char* keyname, const char* valuename, uint32_t nFlags, DWORD* value), 0x25ac0); // Storm.#423

/// Not imported by any .dll
D2FUNC_DLL_NP(STORM, SRegSaveData, BOOL, __stdcall, (char* keyname, char* valuename, BYTE flags, BYTE* lpData, DWORD cbData), 0x25ba0); // Storm.#424

/// Imported by ['D2Launch.dll', 'Fog.dll', 'D2Client.dll']
D2FUNC_DLL_NP(STORM, SRegSaveString, BOOL, __stdcall, (const char* keyname, const char* valuename, BYTE flags, const char* string), 0x25d10); // Storm.#425

/// Imported by ['D2Launch.dll', 'D2Multi.dll', 'D2sound.dll', 'D2Client.dll']
D2FUNC_DLL_NP(STORM, SRegSaveValue, BOOL, __stdcall, (const char* keyname, const char* valuename, BYTE flags, DWORD result), 0x25d80); // Storm.#426

/// Not imported by any .dll
D2FUNC_DLL_NP(STORM, SRegGetBaseKey, BOOL, __stdcall, (char flags, char* buffer, size_t bufferchars), 0x257e0); // Storm.#427

/// Not imported by any .dll
D2FUNC_DLL_NP(STORM, SRegDeleteValue, BOOL, __stdcall, (char* keyname, char* valuename, HKEY phkResult), 0x25660); // Storm.#428

*/

// SRegDeleteValue and SRegSaveData have non-const char* as key/valuenames in D2MOO, but i've changed them to const char*

BOOL STORMAPI SRegDeleteValue(const char* keyname, const char* valuename, HKEY phkResult);
BOOL STORMAPI SRegGetBaseKey(char flags, char* buffer, size_t bufferchars);
BOOL STORMAPI SRegLoadData(const char* keyname, const char* valuename, size_t nSize, LPBYTE, uint32_t nFlags, LPDWORD lpcbData);
BOOL STORMAPI SRegLoadString(const char* keyname, const char* valuename, uint32_t nFlags, LPSTR pBuffer, size_t nBufferSize);
BOOL STORMAPI SRegLoadValue(const char* keyname, const char* valuename, uint32_t nFlags, DWORD* value);
BOOL STORMAPI SRegSaveData(const char* keyname, const char* valuename, uint8_t nFlags, char* lpData, DWORD cbData);
BOOL STORMAPI SRegSaveString(const char* keyname, const char* valuename, uint8_t nFlags, const char* string);
BOOL STORMAPI SRegSaveValue(const char* keyname, const char* valuename, uint8_t nFlags, uint32_t unused);
