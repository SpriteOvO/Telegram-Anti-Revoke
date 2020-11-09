#include <Windows.h>

#include "ILogger.h"
#include "IUpdater.h"
#include "IRuntime.h"
#include "IAntiRevoke.h"
#include "Utils.h"


ULONG WINAPI Initialize(PVOID pParameter)
{
#ifdef _DEBUG
    MessageBoxW(NULL, L"Initialize", L"Anti-Revoke Plugin", MB_ICONINFORMATION);
#endif

    auto &Logger = ILogger::GetInstance();
    auto &Runtime = IRuntime::GetInstance();
    auto &AntiRevoke = IAntiRevoke::GetInstance();

    if (!Runtime.Initialize()) {
        Logger.TraceError("[IRuntime] Initialize failed.");
        return 0;
    }

    IUpdater::GetInstance().CheckUpdate();
    
    if (!Runtime.InitFixedData()) {
        Logger.TraceError("You are using a version of Telegram that is deprecated by the plugin.\nPlease update your Telegram.", false);
        return 0;
    }

    if (!Runtime.InitDynamicData()) {
        Logger.TraceError("[IRuntime] InitDynamicData() failed.");
        return 0;
    }

    AntiRevoke.InitMarker();
    AntiRevoke.SetupHooks();
    AntiRevoke.ProcessBlockedMessages();

    return 0;
}

bool CheckProcess()
{
    std::string CurrentName = File::GetCurrentName();
    if (Text::ToLower(CurrentName) != "telegram.exe") {
        ILogger::GetInstance().TraceWarn("This is not a Telegram process. [" + CurrentName + "]");
        return FALSE;
    }
    return TRUE;
}

BOOL WINAPI RealDllMain(HMODULE hModule, ULONG Reason, PVOID pReserved)
{
    switch (Reason)
    {
    case DLL_PROCESS_ATTACH:
        DisableThreadLibraryCalls(hModule);
        // Utils::CreateConsole();
        
        if (CheckProcess()) {
            CloseHandle(CreateThread(NULL, 0, Initialize, NULL, 0, NULL));
        }

        break;
    }

    return TRUE;
}
