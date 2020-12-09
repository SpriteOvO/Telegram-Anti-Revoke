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
        Logger.TraceError("The version of Telegram you are using has been deprecated by the plugin.\nPlease update your Telegram client.", false);
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
        return false;
    }
    return true;
}

BOOL WINAPI RealDllMain(HMODULE hModule, ULONG Reason, PVOID pReserved)
{
    if (Reason == DLL_PROCESS_ATTACH)
    {
        DisableThreadLibraryCalls(hModule);
        // Utils::CreateConsole();

        if (!CheckProcess()) {
            return TRUE;
        }

        HANDLE hThread = CreateThread(NULL, 0, Initialize, NULL, 0, NULL);
        if (hThread == NULL) {
            ILogger::GetInstance().TraceError("CreateThread() failed. ErrorCode: " + std::to_string(GetLastError()));
            ExitProcess(0);
            return FALSE;
        }

        CloseHandle(hThread);
    }

    return TRUE;
}
