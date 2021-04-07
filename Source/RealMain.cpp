#include <Windows.h>

#include <spdlog/spdlog.h>

#include "Config.h"
#include "Logger.h"
#include "IUpdater.h"
#include "IRuntime.h"
#include "IAntiRevoke.h"
#include "Utils.h"


ULONG WINAPI Initialize(PVOID pParameter)
{
#ifdef _DEBUG
    MessageBoxW(NULL, L"Initialize", L"Anti-Revoke Plugin", MB_ICONINFORMATION);
#endif

    Logger::Initialize();

    spdlog::info("Running. Version: \"{}\"", AR_VERSION_STRING);

    auto &Runtime = IRuntime::GetInstance();
    auto &AntiRevoke = IAntiRevoke::GetInstance();

    if (!Runtime.Initialize()) {
        spdlog::critical("[IRuntime] Initialize failed.");
        return 0;
    }

    IUpdater::GetInstance().CheckUpdate();
    
    if (!Runtime.InitFixedData()) {
        spdlog::error("The version of Telegram you are using has been deprecated by the plugin.\nPlease update your Telegram client.");
        return 0;
    }

    if (!Runtime.InitDynamicData()) {
        spdlog::critical("[IRuntime] InitDynamicData() failed.");
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
        spdlog::warn("This is not a Telegram process. \"{}\"", CurrentName);
        return false;
    }
    return true;
}

BOOL WINAPI RealDllMain(HMODULE hModule, ULONG Reason, PVOID pReserved)
{
    if (Reason == DLL_PROCESS_ATTACH)
    {
        DisableThreadLibraryCalls(hModule);

        if (!CheckProcess()) {
            return TRUE;
        }

        HANDLE hThread = CreateThread(NULL, 0, Initialize, NULL, 0, NULL);
        if (hThread == NULL) {
            spdlog::critical("CreateThread() failed. ErrorCode: {}", ::GetLastError());
            std::exit(0);
            return FALSE;
        }

        CloseHandle(hThread);
    }

    return TRUE;
}
