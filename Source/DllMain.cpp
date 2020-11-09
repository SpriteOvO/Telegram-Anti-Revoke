// Credit: AheadLib
//
// Used to forward the original exported function back to the hijacked system DLL.
//

#include <string>
#include <Windows.h>

#include "ILogger.h"


#ifdef OS_WIN10

# pragma comment(linker, "/EXPORT:GetFileVersionInfoA=_AheadLib_GetFileVersionInfoA,@1")
# pragma comment(linker, "/EXPORT:GetFileVersionInfoByHandle=_AheadLib_GetFileVersionInfoByHandle,@2")
# pragma comment(linker, "/EXPORT:GetFileVersionInfoExA=_AheadLib_GetFileVersionInfoExA,@3")
# pragma comment(linker, "/EXPORT:GetFileVersionInfoExW=_AheadLib_GetFileVersionInfoExW,@4")
# pragma comment(linker, "/EXPORT:GetFileVersionInfoSizeA=_AheadLib_GetFileVersionInfoSizeA,@5")
# pragma comment(linker, "/EXPORT:GetFileVersionInfoSizeExA=_AheadLib_GetFileVersionInfoSizeExA,@6")
# pragma comment(linker, "/EXPORT:GetFileVersionInfoSizeExW=_AheadLib_GetFileVersionInfoSizeExW,@7")
# pragma comment(linker, "/EXPORT:GetFileVersionInfoSizeW=_AheadLib_GetFileVersionInfoSizeW,@8")
# pragma comment(linker, "/EXPORT:GetFileVersionInfoW=_AheadLib_GetFileVersionInfoW,@9")
# pragma comment(linker, "/EXPORT:VerFindFileA=_AheadLib_VerFindFileA,@10")
# pragma comment(linker, "/EXPORT:VerFindFileW=_AheadLib_VerFindFileW,@11")
# pragma comment(linker, "/EXPORT:VerInstallFileA=_AheadLib_VerInstallFileA,@12")
# pragma comment(linker, "/EXPORT:VerInstallFileW=_AheadLib_VerInstallFileW,@13")
# pragma comment(linker, "/EXPORT:VerLanguageNameA=_AheadLib_VerLanguageNameA,@14")
# pragma comment(linker, "/EXPORT:VerLanguageNameW=_AheadLib_VerLanguageNameW,@15")
# pragma comment(linker, "/EXPORT:VerQueryValueA=_AheadLib_VerQueryValueA,@16")
# pragma comment(linker, "/EXPORT:VerQueryValueW=_AheadLib_VerQueryValueW,@17")

#elif defined OS_WIN7

# pragma comment(linker, "/EXPORT:GetFileVersionInfoA=_AheadLib_GetFileVersionInfoA,@1")
# pragma comment(linker, "/EXPORT:GetFileVersionInfoByHandle=_AheadLib_GetFileVersionInfoByHandle,@2")
# pragma comment(linker, "/EXPORT:GetFileVersionInfoExW=_AheadLib_GetFileVersionInfoExW,@3")
# pragma comment(linker, "/EXPORT:GetFileVersionInfoSizeA=_AheadLib_GetFileVersionInfoSizeA,@4")
# pragma comment(linker, "/EXPORT:GetFileVersionInfoSizeExW=_AheadLib_GetFileVersionInfoSizeExW,@5")
# pragma comment(linker, "/EXPORT:GetFileVersionInfoSizeW=_AheadLib_GetFileVersionInfoSizeW,@6")
# pragma comment(linker, "/EXPORT:GetFileVersionInfoW=_AheadLib_GetFileVersionInfoW,@7")
# pragma comment(linker, "/EXPORT:VerFindFileA=_AheadLib_VerFindFileA,@8")
# pragma comment(linker, "/EXPORT:VerFindFileW=_AheadLib_VerFindFileW,@9")
# pragma comment(linker, "/EXPORT:VerInstallFileA=_AheadLib_VerInstallFileA,@10")
# pragma comment(linker, "/EXPORT:VerInstallFileW=_AheadLib_VerInstallFileW,@11")
# pragma comment(linker, "/EXPORT:VerLanguageNameA=_AheadLib_VerLanguageNameA,@12")
# pragma comment(linker, "/EXPORT:VerLanguageNameW=_AheadLib_VerLanguageNameW,@13")
# pragma comment(linker, "/EXPORT:VerQueryValueA=_AheadLib_VerQueryValueA,@14")
# pragma comment(linker, "/EXPORT:VerQueryValueW=_AheadLib_VerQueryValueW,@15")

#else
# error "Project configuration error. You must define OS_WIN10 or OS_WIN7 in Preprocessor Definitions."
#endif


namespace AheadLib
{
    HMODULE hOriginalModule = NULL;

    PVOID FnGetFileVersionInfoA = NULL;
    PVOID FnGetFileVersionInfoByHandle = NULL;
    PVOID FnGetFileVersionInfoExW = NULL;
    PVOID FnGetFileVersionInfoSizeA = NULL;
    PVOID FnGetFileVersionInfoSizeExW = NULL;
    PVOID FnGetFileVersionInfoSizeW = NULL;
    PVOID FnGetFileVersionInfoW = NULL;
    PVOID FnVerFindFileA = NULL;
    PVOID FnVerFindFileW = NULL;
    PVOID FnVerInstallFileA = NULL;
    PVOID FnVerInstallFileW = NULL;
    PVOID FnVerLanguageNameA = NULL;
    PVOID FnVerLanguageNameW = NULL;
    PVOID FnVerQueryValueA = NULL;
    PVOID FnVerQueryValueW = NULL;
#ifdef OS_WIN10
    // These two routines are not exported in Version.dll of Windows7
    //
    PVOID FnGetFileVersionInfoExA = NULL;
    PVOID FnGetFileVersionInfoSizeExA = NULL;
#endif

    FARPROC GetAddress(const CHAR *FunctionName)
    {
        FARPROC Address = GetProcAddress(hOriginalModule, FunctionName);
        if (Address == NULL) {
            ILogger::GetInstance().TraceError("Could not find [" + std::string(FunctionName) + "] function.");
            ExitProcess(0);
            return NULL;
        }

        return Address;
    }

    void InitializeAddresses()
    {
#define INIT_EXPORTED_FUNCTION(name)    if (Fn ## name == NULL) { Fn ## name = GetAddress(# name); }

        INIT_EXPORTED_FUNCTION(GetFileVersionInfoA);
        INIT_EXPORTED_FUNCTION(GetFileVersionInfoByHandle);
        INIT_EXPORTED_FUNCTION(GetFileVersionInfoExW);
        INIT_EXPORTED_FUNCTION(GetFileVersionInfoSizeA);
        INIT_EXPORTED_FUNCTION(GetFileVersionInfoSizeExW);
        INIT_EXPORTED_FUNCTION(GetFileVersionInfoSizeW);
        INIT_EXPORTED_FUNCTION(GetFileVersionInfoW);
        INIT_EXPORTED_FUNCTION(VerFindFileA);
        INIT_EXPORTED_FUNCTION(VerFindFileW);
        INIT_EXPORTED_FUNCTION(VerInstallFileA);
        INIT_EXPORTED_FUNCTION(VerInstallFileW);
        INIT_EXPORTED_FUNCTION(VerLanguageNameA);
        INIT_EXPORTED_FUNCTION(VerLanguageNameW);
        INIT_EXPORTED_FUNCTION(VerQueryValueA);
        INIT_EXPORTED_FUNCTION(VerQueryValueW);
#ifdef OS_WIN10
        INIT_EXPORTED_FUNCTION(GetFileVersionInfoExA);
        INIT_EXPORTED_FUNCTION(GetFileVersionInfoSizeExA);
#endif

#undef INIT_EXPORTED_FUNCTION
    }

    void Initialize()
    {
        if (hOriginalModule == NULL)
        {
            CHAR SystemPath[MAX_PATH];
            GetSystemDirectoryA(SystemPath, MAX_PATH);

            hOriginalModule = LoadLibraryA((std::string(SystemPath) + "\\version.dll").c_str());
            if (hOriginalModule == NULL) {
                ILogger::GetInstance().TraceError("Unable to load the original module.");
                ExitProcess(0);
                return;
            }
        }

        InitializeAddresses();
    }

    void Uninitialize()
    {
        if (hOriginalModule != NULL) {
            FreeLibrary(hOriginalModule);
            hOriginalModule = NULL;
        }
    }

} // namespace AheadLib

#define IMPL_EXPORTED_FUNCTION(name)                       \
    __declspec(naked) void __cdecl AheadLib_ ## name ()    \
    {                                                      \
        __asm pushad                                       \
        __asm call AheadLib::Initialize                    \
        __asm popad                                        \
        __asm jmp AheadLib::Fn ## name                     \
    }

extern "C"
{
    IMPL_EXPORTED_FUNCTION(GetFileVersionInfoA);
    IMPL_EXPORTED_FUNCTION(GetFileVersionInfoByHandle);
    IMPL_EXPORTED_FUNCTION(GetFileVersionInfoExW);
    IMPL_EXPORTED_FUNCTION(GetFileVersionInfoSizeA);
    IMPL_EXPORTED_FUNCTION(GetFileVersionInfoSizeExW);
    IMPL_EXPORTED_FUNCTION(GetFileVersionInfoSizeW);
    IMPL_EXPORTED_FUNCTION(GetFileVersionInfoW);
    IMPL_EXPORTED_FUNCTION(VerFindFileA);
    IMPL_EXPORTED_FUNCTION(VerFindFileW);
    IMPL_EXPORTED_FUNCTION(VerInstallFileA);
    IMPL_EXPORTED_FUNCTION(VerInstallFileW);
    IMPL_EXPORTED_FUNCTION(VerLanguageNameA);
    IMPL_EXPORTED_FUNCTION(VerLanguageNameW);
    IMPL_EXPORTED_FUNCTION(VerQueryValueA);
    IMPL_EXPORTED_FUNCTION(VerQueryValueW);
#ifdef OS_WIN10
    IMPL_EXPORTED_FUNCTION(GetFileVersionInfoExA);
    IMPL_EXPORTED_FUNCTION(GetFileVersionInfoSizeExA);
#endif
}

#undef IMPL_EXPORTED_FUNCTION


// Implemented in RealMain.cpp
//
BOOL WINAPI RealDllMain(HMODULE hModule, ULONG Reason, PVOID pReserved);

BOOL WINAPI DllMain(HMODULE hModule, ULONG Reason, PVOID pReserved)
{
    if (Reason == DLL_PROCESS_ATTACH) {
        return RealDllMain(hModule, Reason, pReserved);
    }
    else if (Reason == DLL_PROCESS_DETACH) {
        AheadLib::Uninitialize();
    }

    return TRUE;
}
