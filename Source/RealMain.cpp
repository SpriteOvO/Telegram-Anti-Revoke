#include "Header.h"
#include "Telegram.h"
#include "Global.h"
#include "AntiRevoke.h"
#include "Updater.h"
#include "ILogger.h"
#include "IRuntime.h"


namespace g
{
    PVOID OriginalRevoke = NULL;
    fntFree fnOriginalFree = NULL;

    mutex Mutex;
    set<HistoryMessage*> RevokedMessages;

    /*
        English
            GetId         : en
            GetPluralId   : en
            GetName       : English
            GetNativeName : English

        Simplified Chinese
            GetId         : classic-zh-cn
            GetPluralId   : zh
            GetName       : Chinese (Simplified, @zh_CN)
            GetNativeName : [bad string]

        Traditional Chinese
            GetId         : zhhant-hk
            GetPluralId   : zh
            GetName       : Chinese (Traditional, Hong Kong)
            GetNativeName : [bad string]

        Japanese
            GetId         : ja-raw
            GetPluralId   : ja
            GetName       : Japanese
            GetNativeName : [bad string]

        Korean
            GetId         : ko
            GetPluralId   : ko
            GetName       : Korean
            GetNativeName : [bad string]

        So we use PluralId and Name.
    */
    map<wstring, vector<MARK_INFO>> MultiLanguageMarks =
    {
        {
            L"en",
            {
                { L"English", L"deleted ", 8 * 6 }
            }
        },

        {
            L"zh",
            {
                { L"Simplified", L"已删除 ", 7 * 6 },
                { L"Traditional", L"已刪除 ", 7 * 6 },
                { L"Cantonese", L"刪咗 ", 5 * 6 }			// Thanks @Rongronggg9, #29
            }
        },

        {
            L"ja",
            {
                { L"Japanese", L"削除された ", 11 * 6 }
            }
        },

        {
            L"ko",
            {
                { L"Korean", L"삭제 ", 5 * 6 }
            }
        }

        // For more languages or corrections, please go to the GitHub issue submission.
    };

    MARK_INFO CurrentMark = MultiLanguageMarks[L"en"][0];
};


BOOLEAN HookRevoke(BOOLEAN Status)
{
    PVOID HookAddress = IRuntime::GetInstance().GetData().Address.FnDestroyMessageCaller;
    PVOID TargetAddress = NULL;
    vector<BYTE> Shellcode;

    if (Status)
    {
        // Enable Hook
        // Save the original revoke function.
        g::OriginalRevoke = (PVOID)((ULONG_PTR)HookAddress + 5 + *(INT*)((ULONG_PTR)HookAddress + 1));

        TargetAddress = Utils::GetFunctionAddress(&Session::ProcessRevoke);
    }
    else
    {
        // Restore hook
        TargetAddress = g::OriginalRevoke;
    }

    Shellcode = Memory::MakeCall(HookAddress, TargetAddress);

    return Memory::ForceOperate(HookAddress, Shellcode.size(), [&]()
    {
        RtlCopyMemory(HookAddress, Shellcode.data(), Shellcode.size());
    });
}

BOOLEAN HookMemoryFree(BOOLEAN Status)
{
    auto FnFree = IRuntime::GetInstance().GetData().Function.Free;

    if (Status)
    {
        // Enable Hook
        //
        if (MH_CreateHook(FnFree, DetourFree, (PVOID*)&g::fnOriginalFree) != MH_OK) {
            return FALSE;
        }

        return MH_EnableHook(FnFree) == MH_OK;
    }
    else
    {
        // Restore hook
        //
        return MH_DisableHook(FnFree) == MH_OK;
    }
}

void InitMarkLanguage()
{
    auto &Logger = ILogger::GetInstance();

    Safe::TryExcept([&]()
    {
        LanguageInstance *pLangInstance = IRuntime::GetInstance().GetData().Address.pLangInstance;

        //printf("pLangInstance : %p\n", pLangInstance);
        //printf("GetId         : %ws\n", Instance->GetId()->GetText());
        //printf("GetPluralId   : %ws\n", Instance->GetPluralId()->GetText());
        //printf("GetName       : %ws\n", Instance->GetName()->GetText());
        //printf("GetNativeName : %ws\n", Instance->GetNativeName()->GetText());

        wstring CurrentPluralId = pLangInstance->GetPluralId()->GetText();
        wstring CurrentName = pLangInstance->GetName()->GetText();

        // Fix for irregularly named language packages
        if (CurrentPluralId == L"yue" && CurrentName == L"Cantonese") {
            CurrentPluralId = L"zh";
        }

        // find language
        auto Iterator = g::MultiLanguageMarks.find(CurrentPluralId);
        if (Iterator == g::MultiLanguageMarks.end()) {
            Logger.TraceWarn(string("An unadded language. PluralId: [") + Convert::UnicodeToAnsi(CurrentPluralId + wstring(L"] Name: [") + CurrentName) + string("]"));
            return;
        }

        const vector<MARK_INFO> &Sublanguages = Iterator->second;

        // default sublanguage
        g::CurrentMark = Sublanguages[0];

        // multiple sublanguages
        if (Sublanguages.size() > 1)
        {
            for (const MARK_INFO &Language : Sublanguages)
            {
                if (CurrentName.find(Language.LangName) != wstring::npos) {
                    // found sub language
                    g::CurrentMark = Language;
                    break;
                }
            }
        }

    }, [&](ULONG ExceptionCode)
    {
        Logger.TraceWarn("Function: [" __FUNCTION__ "] An exception was caught. Code: [" + Text::Format("0x%x", ExceptionCode) + "]");
    });
}

DWORD WINAPI Initialize(PVOID pParameter)
{
#ifdef _DEBUG
    MessageBoxW(NULL, L"Initialize", L"Anti-Revoke Plugin", MB_ICONINFORMATION);
#endif

    auto &Logger = ILogger::GetInstance();
    auto &Runtime = IRuntime::GetInstance();

    if (!Runtime.Initialize()) {
        Logger.TraceError("[IRuntime] Initialize failed.");
        return 0;
    }

    Updater::GetInstance().CheckUpdate();
    
    if (!Runtime.InitFixedData()) {
        Logger.TraceError("You are using a version of Telegram that is deprecated by the plugin.\nPlease update your Telegram.", false);
        return 0;
    }

    if (!Runtime.InitDynamicData()) {
        Logger.TraceError("[IRuntime] InitDynamicData() failed.");
        return 0;
    }

    MH_STATUS Status = MH_Initialize();
    if (Status != MH_OK) {
        Logger.TraceError(string("MH_Initialize() failed.\n") + MH_StatusToString(Status));
        return 0;
    }

    InitMarkLanguage();

    if (!HookMemoryFree(TRUE)) {
        Logger.TraceError("HookMemoryFree() failed.");
        return 0;
    }

    if (!HookRevoke(TRUE)) {
        Logger.TraceError("HookRevoke() failed.");
        return 0;
    }

    ProcessItems();

    return 0;
}

BOOLEAN CheckProcess()
{
    string CurrentName = File::GetCurrentName();
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
