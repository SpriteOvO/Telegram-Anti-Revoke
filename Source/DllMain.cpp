// Credit: AheadLib
//
// Used to forward the original exported function back to the hijacked system DLL.
//

#include <string>
#include <Windows.h>

#include "ILogger.h"


#if defined OS_WIN10

#define ORIGINAL_EXPORTED_LIST(invoke)       \
    invoke(GetFileVersionInfoA, 1);          \
    invoke(GetFileVersionInfoExA, 3);        \
    invoke(GetFileVersionInfoExW, 4);        \
    invoke(GetFileVersionInfoSizeA, 5);      \
    invoke(GetFileVersionInfoSizeExA, 6);    \
    invoke(GetFileVersionInfoSizeExW, 7);    \
    invoke(GetFileVersionInfoSizeW, 8);      \
    invoke(GetFileVersionInfoW, 9);          \
    invoke(VerFindFileA, 10);                \
    invoke(VerFindFileW, 11);                \
    invoke(VerInstallFileA, 12);             \
    invoke(VerInstallFileW, 13);             \
    invoke(VerLanguageNameA, 14);            \
    invoke(VerLanguageNameW, 15);            \
    invoke(VerQueryValueA, 16);              \
    invoke(VerQueryValueW, 17);

#elif defined OS_WIN7

#define ORIGINAL_EXPORTED_LIST(invoke)       \
    invoke(GetFileVersionInfoA, 1);          \
    invoke(GetFileVersionInfoExW, 3);        \
    invoke(GetFileVersionInfoSizeA, 4);      \
    invoke(GetFileVersionInfoSizeExW, 5);    \
    invoke(GetFileVersionInfoSizeW, 6);      \
    invoke(GetFileVersionInfoW, 7);          \
    invoke(VerFindFileA, 8);                 \
    invoke(VerFindFileW, 9);                 \
    invoke(VerInstallFileA, 10);             \
    invoke(VerInstallFileW, 11);             \
    invoke(VerLanguageNameA, 12);            \
    invoke(VerLanguageNameW, 13);            \
    invoke(VerQueryValueA, 14);              \
    invoke(VerQueryValueW, 15);

#else
# error "Project configuration error. You must define OS_WIN10 or OS_WIN7 in Preprocessor Definitions."
#endif

// Export forwarding functions
//
#define EXPORT_FORWARDING_FUNCTION(name, ordinal)    __pragma(comment(linker, "/EXPORT:" # name "=_Forwarder_" # name ",@" # ordinal))
ORIGINAL_EXPORTED_LIST(EXPORT_FORWARDING_FUNCTION);
#undef EXPORT_FORWARDING_FUNCTION

namespace Forwarder
{
    HMODULE hOriginalModule = NULL;

    // Declare original exported functions address
    //
#define DECLARE_ORIGINAL_EXPORTED_ADDRESS(name, ordinal)    void* Fn ## name = NULL;
    ORIGINAL_EXPORTED_LIST(DECLARE_ORIGINAL_EXPORTED_ADDRESS);
#undef DECLARE_ORIGINAL_EXPORTED_ADDRESS

    void* GetExportedAddress(const char *ExportedName)
    {
        void* Address = GetProcAddress(hOriginalModule, ExportedName);
        if (Address == NULL) {
            ILogger::GetInstance().TraceError("Could not find [" + std::string(ExportedName) + "] function.");
            ExitProcess(0);
            return NULL;
        }

        return Address;
    }

    void Initialize()
    {
        // Initialize hOriginalModule
        //
        if (hOriginalModule == NULL)
        {
            char SystemPath[MAX_PATH];
            GetSystemDirectoryA(SystemPath, MAX_PATH);

            hOriginalModule = LoadLibraryA((std::string(SystemPath) + "\\version.dll").c_str());
            if (hOriginalModule == NULL) {
                ILogger::GetInstance().TraceError("Unable to load the original module.");
                ExitProcess(0);
                return;
            }
        }

        // Initialize original exported functions address
        //
#define INIT_ORIGINAL_EXPORTED_ADDRESS(name, ordinal)    if (Fn ## name == NULL) { Fn ## name = GetExportedAddress(# name); }
        ORIGINAL_EXPORTED_LIST(INIT_ORIGINAL_EXPORTED_ADDRESS);
#undef INIT_ORIGINAL_EXPORTED_ADDRESS
    }

    void Uninitialize()
    {
        if (hOriginalModule != NULL) {
            FreeLibrary(hOriginalModule);
            hOriginalModule = NULL;
        }
    }

} // namespace AheadLib

// Implement forwarding functions
//
#define IMPL_FORWARDING_FUNCTION(name, ordinal)                        \
    extern "C" __declspec(naked) void __cdecl Forwarder_ ## name ()    \
    {                                                                  \
        __asm pushad                                                   \
        __asm call Forwarder::Initialize                               \
        __asm popad                                                    \
        __asm jmp Forwarder::Fn ## name                                \
    }
ORIGINAL_EXPORTED_LIST(IMPL_FORWARDING_FUNCTION);
#undef IMPL_FORWARDING_FUNCTION

// Implemented in RealMain.cpp
//
BOOL WINAPI RealDllMain(HMODULE hModule, ULONG Reason, PVOID pReserved);

BOOL WINAPI DllMain(HMODULE hModule, ULONG Reason, PVOID pReserved)
{
    if (Reason == DLL_PROCESS_ATTACH) {
        return RealDllMain(hModule, Reason, pReserved);
    }
    else if (Reason == DLL_PROCESS_DETACH) {
        Forwarder::Uninitialize();
    }

    return TRUE;
}
