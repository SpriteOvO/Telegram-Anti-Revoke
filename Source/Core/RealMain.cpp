#include <Windows.h>

#include "Config.h"
#include "Logger.h"
#include "IUpdater.h"
#include "IRuntime.h"
#include "IAntiRevoke.h"
#include "Utils.h"

bool CheckProcess()
{
    std::string CurrentName = File::GetCurrentName();
    if (Text::ToLower(CurrentName) != "telegram.exe") {
        LOG(Warn, "This is not a Telegram process. \"{}\"", CurrentName);
        return false;
    }
    return true;
}

ULONG WINAPI Initialize(PVOID pParameter)
{
#ifdef _DEBUG
    MessageBoxW(nullptr, L"Initialize", L"Anti-Revoke Plugin", MB_ICONINFORMATION);
#endif

    Logger::Initialize();

    if (!CheckProcess()) {
        return TRUE;
    }

    LOG(Info, "Running. Version: \"{}\", Platform: \"{}\"", AR_VERSION_STRING, AR_PLATFORM_STR);

    auto &Runtime = IRuntime::GetInstance();
    auto &AntiRevoke = IAntiRevoke::GetInstance();

    if (!Runtime.Initialize()) {
        LOG(Critical, "[IRuntime] Initialize failed.");
        return 0;
    }

    IUpdater::GetInstance().CheckUpdate();

    if (!Runtime.InitFixedData()) {
        LOG(Error,
            "The version of Telegram you are using has been deprecated by the plugin.\nPlease update your Telegram client.");
        return 0;
    }

    if (!Runtime.InitDynamicData()) {
        LOG(Critical, "[IRuntime] InitDynamicData() failed.");
        return 0;
    }

    AntiRevoke.InitMarker();
    AntiRevoke.SetupHooks();
    AntiRevoke.ProcessBlockedMessages();

    return 0;
}

BOOL WINAPI RealDllMain(HMODULE hModule, ULONG Reason, PVOID pReserved)
{
    if (Reason == DLL_PROCESS_ATTACH) {
        DisableThreadLibraryCalls(hModule);

        HANDLE hThread = CreateThread(nullptr, 0, Initialize, nullptr, 0, nullptr);
        if (hThread == nullptr) {
            Logger::DoError(
                "CreateThread() failed. ErrorCode: " + std::to_string(::GetLastError()), true);
            return FALSE;
        }

        CloseHandle(hThread);
    }

    return TRUE;
}
