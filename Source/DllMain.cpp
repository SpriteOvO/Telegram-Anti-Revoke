// Credit: AheadLib
//
// Used to forward the original exported function back to the hijacked system DLL.
//

#include <Windows.h>

#include <string>

#include "Logger.h"


#if defined OS_WIN10

#define ORIGINAL_EXPORTS_CALLBACKER(callback)    \
    callback(GetFileVersionInfoA,        1);     \
    callback(GetFileVersionInfoByHandle, 2);     \
    callback(GetFileVersionInfoExA,      3);     \
    callback(GetFileVersionInfoExW,      4);     \
    callback(GetFileVersionInfoSizeA,    5);     \
    callback(GetFileVersionInfoSizeExA,  6);     \
    callback(GetFileVersionInfoSizeExW,  7);     \
    callback(GetFileVersionInfoSizeW,    8);     \
    callback(GetFileVersionInfoW,        9);     \
    callback(VerFindFileA,               10);    \
    callback(VerFindFileW,               11);    \
    callback(VerInstallFileA,            12);    \
    callback(VerInstallFileW,            13);    \
    callback(VerLanguageNameA,           14);    \
    callback(VerLanguageNameW,           15);    \
    callback(VerQueryValueA,             16);    \
    callback(VerQueryValueW,             17);

#elif defined OS_WIN7

#define ORIGINAL_EXPORTS_CALLBACKER(callback)    \
    callback(GetFileVersionInfoA,        1);     \
    callback(GetFileVersionInfoByHandle, 2);     \
    callback(GetFileVersionInfoExW,      3);     \
    callback(GetFileVersionInfoSizeA,    4);     \
    callback(GetFileVersionInfoSizeExW,  5);     \
    callback(GetFileVersionInfoSizeW,    6);     \
    callback(GetFileVersionInfoW,        7);     \
    callback(VerFindFileA,               8);     \
    callback(VerFindFileW,               9);     \
    callback(VerInstallFileA,            10);    \
    callback(VerInstallFileW,            11);    \
    callback(VerLanguageNameA,           12);    \
    callback(VerLanguageNameW,           13);    \
    callback(VerQueryValueA,             14);    \
    callback(VerQueryValueW,             15);

#else
# error "Project configuration error. You must define OS_WIN10 or OS_WIN7 in Preprocessor Definitions."
#endif

// Export proxy functions
//
#if defined PLATFORM_X86
# define EXPORT_PROXY_FUNCTION(name, ordinal)    __pragma(comment(linker, "/EXPORT:" # name "=_asm_proxy_" # name ",@" # ordinal))
#elif defined PLATFORM_X64
# define EXPORT_PROXY_FUNCTION(name, ordinal)    __pragma(comment(linker, "/EXPORT:" # name "=asm_proxy_" # name ",@" # ordinal))
#endif
ORIGINAL_EXPORTS_CALLBACKER(EXPORT_PROXY_FUNCTION);
#undef EXPORT_PROXY_FUNCTION

namespace Proxy
{
    HMODULE hOriginalModule = NULL;

    // Declare original exported functions address
    //
#define DECLARE_ORIGINAL_EXPORT_ADDRESS(name, ordinal)    extern "C" void* Proxy_OEFn ## name = NULL;
    ORIGINAL_EXPORTS_CALLBACKER(DECLARE_ORIGINAL_EXPORT_ADDRESS);
#undef DECLARE_ORIGINAL_EXPORT_ADDRESS

    void* GetExportedAddress(const char *SymbolName)
    {
        void* Address = GetProcAddress(hOriginalModule, SymbolName);
        if (Address == NULL) {
            Logger::DoError("Cannot find \"" + std::string{SymbolName} + "\" exported symbol.", true);
        }

        return Address;
    }

    extern "C" void Proxy_Initialize()
    {
        // Initialize hOriginalModule
        //
        if (hOriginalModule == NULL)
        {
            char SystemPath[MAX_PATH];
            GetSystemDirectoryA(SystemPath, MAX_PATH);

            hOriginalModule = LoadLibraryA((std::string{SystemPath} + "\\version.dll").c_str());
            if (hOriginalModule == NULL) {
                Logger::DoError("Unable to load the original module.", true);
            }
        }

        // Initialize original exported functions address
        //
#define INIT_ORIGINAL_EXPORT_ADDRESS(name, ordinal)    if (Proxy_OEFn ## name == NULL) { Proxy_OEFn ## name = GetExportedAddress(# name); }
        ORIGINAL_EXPORTS_CALLBACKER(INIT_ORIGINAL_EXPORT_ADDRESS);
#undef INIT_ORIGINAL_EXPORT_ADDRESS
    }

    void Deinitialize()
    {
        if (hOriginalModule != NULL) {
            FreeLibrary(hOriginalModule);
            hOriginalModule = NULL;
        }
    }

} // namespace Proxy


// Implemented in RealMain.cpp
//
BOOL WINAPI RealDllMain(HMODULE hModule, ULONG Reason, PVOID pReserved);

BOOL WINAPI DllMain(HMODULE hModule, ULONG Reason, PVOID pReserved)
{
    if (Reason == DLL_PROCESS_ATTACH) {
        return RealDllMain(hModule, Reason, pReserved);
    }
    else if (Reason == DLL_PROCESS_DETACH) {
        Proxy::Deinitialize();
    }

    return TRUE;
}
