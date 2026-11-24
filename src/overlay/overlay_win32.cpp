#ifdef _WIN32

#include <windows.h>

/**
 * Entry point for the overlay DLL on Windows.
 */
BOOL WINAPI DllMain(HINSTANCE instance, DWORD reason, [[maybe_unused]] LPVOID reserved) {
    if (reason == DLL_PROCESS_ATTACH) {
        DisableThreadLibraryCalls(instance);
    }
    return TRUE;
}

#endif // _WIN32
