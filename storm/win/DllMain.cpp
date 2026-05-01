#include <windows.h>
#include <stdio.h>

void OpenConsoleAndStdout() {
    // Try to attach to parent console first (optional)
    AttachConsole(ATTACH_PARENT_PROCESS);

    // If no console, allocate a new one
    if (!GetConsoleWindow()) {
        AllocConsole();
    }

    // Reopen C std streams to the console device
    freopen("CONIN$",  "r", stdin);
    freopen("CONOUT$", "w", stdout);
    freopen("CONOUT$", "w", stderr);

    // Optionally set console code page
    SetConsoleOutputCP(CP_UTF8);
}

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved) {
    switch (fdwReason) {
        case DLL_PROCESS_ATTACH:
            OpenConsoleAndStdout();
            printf("Hello world from squallstorm!\n");
            break;
        case DLL_THREAD_ATTACH:
            // Code to run when a new thread is created
            break;
        case DLL_THREAD_DETACH:
            // Code to run when a thread exits cleanly
            break;
        case DLL_PROCESS_DETACH:
            // Code to run when the DLL is unloaded
            break;
    }
    return TRUE; // Successful initialization
}
