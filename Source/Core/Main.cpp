#if defined OS_WIN
    #include "OS/Windows/DllMain.h"
#endif

#if defined OS_WIN

BOOL WINAPI DllMain(HMODULE hModule, ULONG Reason, PVOID pReserved)
{
    return OS::Windows::DllMain::Entry(hModule, Reason, pReserved);
}

#endif
