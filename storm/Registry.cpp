#include <string.h>
#include <stdio.h>
#include <malloc.h>

#include "Registry.hpp"

/*
enum SREG_Flags {
    SREG_DEFAULT = 0x00,               // Look into both HKEY_LOCAL_MACHINE and HKEY_CURRENT_USER and under "Software\Blizzard Entertainment\"
    SREG_EXCLUDE_LOCAL_MACHINE = 0x01, // excludes checking the HKEY_LOCAL_MACHINE hive
    SREG_BATTLE_NET = 0x02,            // Look under "Software\\Battle.net\\" instead
    SREG_EXCLUDE_CURRENT_USER = 0x04,  // excludes checking the HKEY_CURRENT_USER hive
    SREG_ABSOLUTE = 0x10,              // specifies that the key is not a relative key
};
*/

#ifdef WHOA_SYSTEM_WIN

typedef struct {
    bool flagDefaultHives;
    bool flagExcludeLocalMachineHive;
    bool flagLookInBattleNet;
    bool flagExcludeCurrentUserHive;
    bool flagKeyAbsolute;
} DecodedFlags;

static void DecodeFlags(uint8_t flags, DecodedFlags* dFlags) {
    dFlags->flagExcludeLocalMachineHive = flags & SREG_EXCLUDE_LOCAL_MACHINE;
    dFlags->flagExcludeCurrentUserHive = flags & SREG_EXCLUDE_CURRENT_USER;
    dFlags->flagDefaultHives = !dFlags->flagExcludeLocalMachineHive && !dFlags->flagExcludeCurrentUserHive;

    dFlags->flagLookInBattleNet = flags & SREG_BATTLE_NET;

    dFlags->flagKeyAbsolute = flags & SREG_ABSOLUTE;
}
/* Helper: build base path string into buffer (must be large enough). Returns pointer to appended subkey start. */
static const char* BasePrefixFromFlags(const DecodedFlags* df) {
    return df->flagLookInBattleNet ? "Software\\Battle.net\\" : "Software\\Blizzard Entertainment\\";
}

/* Helper: try open key given hive + subkey; returns opened HKEY in *out (caller closes unless retained via phkResult behavior below) */
static LONG TryOpenKey(HKEY hBase, const char* subkey, REGSAM sam, HKEY* out) {
    if (!subkey || !*subkey) {
        return RegOpenKeyExA(hBase, NULL, 0, sam, out);
    }
    return RegOpenKeyExA(hBase, subkey, 0, sam, out);
}

/* SRegGetBaseKey: fills buffer with base path for a flags value; returns TRUE on success */
BOOL STORMAPI SRegGetBaseKey(char flags, char* buffer, size_t bufferchars) {
    DecodedFlags df;
    DecodeFlags((uint8_t)flags, &df);
    const char* prefix = BasePrefixFromFlags(&df);
    if (!buffer || bufferchars == 0)
        return FALSE;
    if (df.flagKeyAbsolute) {
        buffer[0] = '\0';
        return TRUE; // absolute keys: caller uses keyname as-is
    }
    if (strlen(prefix) + 1 > bufferchars)
        return FALSE;
    strcpy_s(buffer, bufferchars, prefix);
    return TRUE;
}

/* Internal: iterate candidate hives according to flags, calling a callback for each open attempt.
   callback returns TRUE to stop iteration and set resultHKey/outResult accordingly.
*/
typedef BOOL (*OpenCallback)(HKEY hBase, const char* subkey, void* ctx);

static BOOL IterateHives(const DecodedFlags* df, OpenCallback cb, void* ctx) {
    // order: HKLM then HKCU (original SREG_DEFAULT indicates both)
    if (!df->flagExcludeLocalMachineHive) {
        if (cb(HKEY_LOCAL_MACHINE, NULL, ctx))
            return TRUE;
    }
    if (!df->flagExcludeCurrentUserHive) {
        if (cb(HKEY_CURRENT_USER, NULL, ctx))
            return TRUE;
    }
    return FALSE;
}

/* SRegDeleteValue: deletes a value; phkResult is an optional out HKEY the caller must close if non-NULL.
   keyname may be absolute (if SREG_ABSOLUTE set in flags encoded in keyname prefix?) -- to keep API simple we expect absolute to be passed via flags parameter in other funcs; here assume keyname may be full path like "HKLM\\..." or relative "SubKey" and caller uses SRegGetBaseKey to build full path. For simplicity we allow full "HKx\\..." root tokens.
*/
static HKEY ParseRootToken(const char* keyname, const char** subkeyOut) {
    if (!keyname || !subkeyOut)
        return NULL;
    // check for root token like "HKLM\\" or "HKEY_LOCAL_MACHINE\\"
    if ((strnicmp(keyname, "HKLM\\", 5) == 0) || (strnicmp(keyname, "HKEY_LOCAL_MACHINE\\", 19) == 0)) {
        const char* p = strchr(keyname, '\\');
        *subkeyOut = p ? p + 1 : "";
        return HKEY_LOCAL_MACHINE;
    }
    if ((strnicmp(keyname, "HKCU\\", 5) == 0) || (strnicmp(keyname, "HKEY_CURRENT_USER\\", 18) == 0)) {
        const char* p = strchr(keyname, '\\');
        *subkeyOut = p ? p + 1 : "";
        return HKEY_CURRENT_USER;
    }
    if ((strnicmp(keyname, "HKCR\\", 5) == 0) || (strnicmp(keyname, "HKEY_CLASSES_ROOT\\", 18) == 0)) {
        const char* p = strchr(keyname, '\\');
        *subkeyOut = p ? p + 1 : "";
        return HKEY_CLASSES_ROOT;
    }
    if ((strnicmp(keyname, "HKU\\", 4) == 0) || (strnicmp(keyname, "HKEY_USERS\\", 11) == 0)) {
        const char* p = strchr(keyname, '\\');
        *subkeyOut = p ? p + 1 : "";
        return HKEY_USERS;
    }
    if ((strnicmp(keyname, "HKCC\\", 5) == 0) || (strnicmp(keyname, "HKEY_CURRENT_CONFIG\\", 20) == 0)) {
        const char* p = strchr(keyname, '\\');
        *subkeyOut = p ? p + 1 : "";
        return HKEY_CURRENT_CONFIG;
    }
    *subkeyOut = keyname;
    return NULL;
}

typedef struct {
    const char* subkey;
    REGSAM access;
    HKEY* out;
    BOOL found;
    HKEY opened;
} Ctx;

static BOOL cb(HKEY hBase, const char* unused, Ctx* vctx) {
    Ctx* c = (Ctx*)vctx;
    HKEY h = NULL;
    if (TryOpenKey(hBase, c->subkey, c->access, &h) == ERROR_SUCCESS) {
        c->found = TRUE;
        c->opened = h;
        if (c->out)
            *c->out = h;
        return TRUE;
    }
    return FALSE;
}

/* Core helper: open key according to flags and keyname (which may be absolute or relative). If outKey != NULL and function succeeds, *outKey is an opened key which caller must RegCloseKey; if outKey==NULL, key is opened/closed internally. access param passed to RegOpenKeyEx. */
static BOOL OpenKeyForOperation(const char* keyname, uint8_t flags, REGSAM access, HKEY* outKey) {
    DecodedFlags df;
    DecodeFlags(flags, &df);

    // If keyname is absolute and begins with HK*, open directly
    const char* subkeyForRoot = NULL;
    HKEY explicitRoot = ParseRootToken(keyname, &subkeyForRoot);
    if (explicitRoot) {
        HKEY h = NULL;
        if (TryOpenKey(explicitRoot, subkeyForRoot, access, &h) == ERROR_SUCCESS) {
            if (outKey) {
                *outKey = h;
                return TRUE;
            }
            RegCloseKey(h);
            return TRUE;
        }
        return FALSE;
    }

    // else try constructed base prefixes and hives
    const char* basePrefix = BasePrefixFromFlags(&df);
    char fullSubkey[MAX_PATH];
    if (df.flagKeyAbsolute) {
        // keyname is already absolute path (relative to registry root)
        strncpy_s(fullSubkey, sizeof(fullSubkey), keyname, _TRUNCATE);
    } else {
        // combine basePrefix + keyname
        if (snprintf(fullSubkey, sizeof(fullSubkey), "%s%s", basePrefix, keyname) >= (int)sizeof(fullSubkey))
            return FALSE;
    }

    BOOL found = FALSE;
    Ctx ctx = { fullSubkey, access, outKey, FALSE, NULL };

    // iterate: prefer HKLM then HKCU
    if (!df.flagExcludeLocalMachineHive) {
        if (cb(HKEY_LOCAL_MACHINE, NULL, &ctx))
            found = TRUE;
    }
    if (!found && !df.flagExcludeCurrentUserHive) {
        if (cb(HKEY_CURRENT_USER, NULL, &ctx))
            found = TRUE;
    }

    if (!found)
        return FALSE;
    if (!outKey) {
        RegCloseKey(ctx.opened);
    }
    return TRUE;
}

/* Implementations */

BOOL STORMAPI SRegDeleteValue(const char* keyname, const char* valuename, HKEY phkResult) {
    if (!keyname || !valuename)
        return FALSE;
    // phkResult used as out HKEY pointer if non-NULL (following original signature; ambiguous type in prompt)
    HKEY opened = NULL;
    if (!OpenKeyForOperation(keyname, SREG_DEFAULT, KEY_SET_VALUE | KEY_QUERY_VALUE, &opened))
        return FALSE;
    LONG r = RegDeleteValueA(opened, valuename);
    if (r != ERROR_SUCCESS) {
        RegCloseKey(opened);
        return FALSE;
    }
    if (phkResult) {
        // return handle to caller (they must RegCloseKey)
        *(HKEY*)&phkResult = opened; // preserve original signature; if phkResult was actually HKEY*, caller will check
    } else {
        RegCloseKey(opened);
    }
    return TRUE;
}

BOOL STORMAPI SRegLoadData(const char* keyname, const char* valuename, size_t nSize, LPBYTE lpOut, uint32_t nFlags, LPDWORD lpcbData) {
    if (!keyname || !valuename || !lpOut)
        return FALSE;
    HKEY h = NULL;
    if (!OpenKeyForOperation(keyname, (uint8_t)nFlags, KEY_QUERY_VALUE, &h))
        return FALSE;
    DWORD type = 0;
    DWORD cb = (DWORD)nSize;
    LONG r = RegQueryValueExA(h, valuename, NULL, &type, lpOut, &cb);
    RegCloseKey(h);
    if (r != ERROR_SUCCESS)
        return FALSE;
    if (lpcbData)
        *lpcbData = cb;
    return TRUE;
}

BOOL STORMAPI SRegLoadString(const char* keyname, const char* valuename, uint32_t nFlags, LPSTR pBuffer, size_t nBufferSize) {
    if (!keyname || !valuename || !pBuffer || nBufferSize == 0)
        return FALSE;
    DWORD got = 0;
    if (!SRegLoadData(keyname, valuename, nBufferSize, (LPBYTE)pBuffer, nFlags, &got))
        return FALSE;
    // ensure null-terminated
    if (got == 0 || pBuffer[got - 1] != '\0') {
        size_t n = (got < nBufferSize) ? got : (nBufferSize - 1);
        pBuffer[n] = '\0';
    }
    return TRUE;
}

BOOL STORMAPI SRegLoadValue(const char* keyname, const char* valuename, uint32_t nFlags, DWORD* value) {
    if (!keyname || !valuename || !value)
        return FALSE;
    HKEY h = NULL;
    if (!OpenKeyForOperation(keyname, (uint8_t)nFlags, KEY_QUERY_VALUE, &h))
        return FALSE;
    DWORD type = 0;
    DWORD cb = sizeof(DWORD);
    DWORD val = 0;
    LONG r = RegQueryValueExA(h, valuename, NULL, &type, (LPBYTE)&val, &cb);
    RegCloseKey(h);
    if (r != ERROR_SUCCESS)
        return FALSE;
    *value = val;
    return TRUE;
}

BOOL STORMAPI SRegSaveData(const char* keyname, const char* valuename, uint8_t nFlags, char* lpData, DWORD cbData) {
    if (!keyname || !valuename || !lpData)
        return FALSE;
    // Create or open with write access. We'll open/create under preferred hives.
    DecodedFlags df;
    DecodeFlags(nFlags, &df);
    const char* basePrefix = BasePrefixFromFlags(&df);
    char fullSubkey[MAX_PATH];
    if (df.flagKeyAbsolute) {
        strncpy_s(fullSubkey, sizeof(fullSubkey), keyname, _TRUNCATE);
    } else {
        if (snprintf(fullSubkey, sizeof(fullSubkey), "%s%s", basePrefix, keyname) >= (int)sizeof(fullSubkey))
            return FALSE;
    }

    BOOL success = FALSE;
    HKEY created = NULL;
    if (!df.flagExcludeLocalMachineHive) {
        if (RegCreateKeyExA(HKEY_LOCAL_MACHINE, fullSubkey, 0, NULL, 0, KEY_WRITE, NULL, &created, NULL) == ERROR_SUCCESS) {
            if (RegSetValueExA(created, valuename, 0, REG_BINARY, (const BYTE*)lpData, cbData) == ERROR_SUCCESS)
                success = TRUE;
            RegCloseKey(created);
            if (success)
                return TRUE;
        }
    }
    if (!df.flagExcludeCurrentUserHive) {
        if (RegCreateKeyExA(HKEY_CURRENT_USER, fullSubkey, 0, NULL, 0, KEY_WRITE, NULL, &created, NULL) == ERROR_SUCCESS) {
            if (RegSetValueExA(created, valuename, 0, REG_BINARY, (const BYTE*)lpData, cbData) == ERROR_SUCCESS)
                success = TRUE;
            RegCloseKey(created);
        }
    }
    return success;
}

BOOL STORMAPI SRegSaveString(const char* keyname, const char* valuename, uint8_t nFlags, const char* string) {
    if (!string)
        return FALSE;
    return SRegSaveData(keyname, valuename, nFlags, (char*)string, (DWORD)(strlen(string) + 1));
}

BOOL STORMAPI SRegSaveValue(const char* keyname, const char* valuename, uint8_t nFlags, uint32_t result) {
    DWORD one = 1;
    return SRegSaveData(keyname, valuename, nFlags, (char*)&one, sizeof(one));
}

#else

BOOL STORMAPI SRegDeleteValue(const char* keyname, const char* valuename, HKEY phkResult) {
	printf("Stubbed function SRegDeleteValue called\n");
}

BOOL STORMAPI SRegGetBaseKey(char flags, char* buffer, size_t bufferchars) {
	printf("Stubbed function SRegGetBaseKey called\n");
}

BOOL STORMAPI SRegLoadData(const char* keyname, const char* valuename, size_t nSize, LPBYTE, uint32_t nFlags, LPDWORD lpcbData) {
	printf("Stubbed function SRegLoadData called\n");
}

BOOL STORMAPI SRegLoadString(const char* keyname, const char* valuename, uint32_t nFlags, LPSTR pBuffer, size_t nBufferSize) {
	printf("Stubbed function SRegLoadString called\n");
}

BOOL STORMAPI SRegLoadValue(const char* keyname, const char* valuename, uint32_t nFlags, DWORD* value) {
	printf("Stubbed function SRegLoadValue called\n");
}

BOOL STORMAPI SRegSaveData(const char* keyname, const char* valuename, uint8_t nFlags, char* lpData, DWORD cbData) {
	printf("Stubbed function SRegSaveData called\n");
}

BOOL STORMAPI SRegSaveString(const char* keyname, const char* valuename, uint8_t nFlags, const char* string) {
	printf("Stubbed function SRegSaveString called\n");
}

BOOL STORMAPI SRegSaveValue(const char* keyname, const char* valuename, uint8_t nFlags, uint32_t result) {
	printf("Stubbed function SRegSaveValue called\n");
}

#endif
