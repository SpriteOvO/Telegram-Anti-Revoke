#include "IAntiRevoke.h"

#include <unordered_map>

#include <MinHook.h>
#include <spdlog/spdlog.h>

#include "IRuntime.h"
#include "Utils.h"


IAntiRevoke& IAntiRevoke::GetInstance()
{
    static IAntiRevoke i;
    return i;
}

void IAntiRevoke::InitMarker()
{
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

    std::unordered_map<std::wstring, std::vector<MarkDataT>> MultiLangMarks =
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

    // Set default lang
    //
    _MarkData = MultiLangMarks[L"en"].at(0);

    Safe::TryExcept(
        [&]()
        {
            LanguageInstance *pLangInstance = IRuntime::GetInstance().GetData().Address.pLangInstance;

            std::wstring CurrentPluralId = pLangInstance->GetPluralId()->GetText();
            std::wstring CurrentName = pLangInstance->GetName()->GetText();

            // Fix for irregularly named language packages
            //
            if (CurrentPluralId == L"yue" && CurrentName == L"Cantonese") {
                CurrentPluralId = L"zh";
            }

            // Find language
            //
            auto Iterator = MultiLangMarks.find(CurrentPluralId);
            if (Iterator == MultiLangMarks.end()) {
                spdlog::warn("An unadded language. PluralId: \"{}\", Name: \"{}\"", Convert::UnicodeToAnsi(CurrentPluralId), Convert::UnicodeToAnsi(CurrentName));
                return;
            }

            const std::vector<MarkDataT> &Sublanguages = Iterator->second;

            // Set default sublanguage
            //
            _MarkData = Sublanguages.at(0);

            // If has multiple sublanguages
            //
            if (Sublanguages.size() > 1)
            {
                for (const MarkDataT &Language : Sublanguages)
                {
                    if (CurrentName.find(Language.LangName) != std::wstring::npos)
                    {
                        // Found sublanguage, set it
                        //
                        _MarkData = Language;
                        break;
                    }
                }
            }

        }, [&](ULONG ExceptionCode)
        {
            spdlog::warn("Function: [" __FUNCTION__ "] An exception was caught. Code: {:#x}", ExceptionCode);
        }
    );
}

void IAntiRevoke::SetupHooks()
{
    MH_STATUS Status = MH_Initialize();
    if (Status != MH_OK) {
        spdlog::critical("[IAntiRevoke] MH_Initialize() failed. Status: {}", MH_StatusToString(Status));
        return;
    }

    if (!HookFreeFunction()) {
        spdlog::critical("[IAntiRevoke] HookFreeFunction() failed.");
        return;
    }

    if (!HookRevokeFunction()) {
        spdlog::critical("[IAntiRevoke] HookRevokeFunction() failed.");
        return;
    }
}

void IAntiRevoke::ProcessBlockedMessages()
{
    while (true)
    {
        Sleep(1000);

        std::lock_guard<std::mutex> Lock(_Mutex);

        for (HistoryMessage *pMessage : _BlockedMessages)
        {
            Safe::TryExcept(
                [&]()
                {
                    QtString *pTimeText = NULL;
                    HistoryMessageEdited *pEdited = pMessage->GetEdited();
                    HistoryMessageSigned *pSigned = pMessage->GetSigned();

                    // Signed msg take precedence over Edited msg, and TG uses the Signed text when both exist.
                    //
                    if (pSigned != NULL) {
                        // Signed msg
                        pTimeText = pSigned->GetTimeText();
                    }
                    else if (pEdited != NULL) {
                        // Edited msg
                        pTimeText = pEdited->GetTimeText();
                    }
                    else {
                        // Normal msg
                        pTimeText = pMessage->GetTimeText();
                    }

                    //  vvvvvvvvvvvvvvvvvvvv TODO: This is a workaround, try to hook HistoryMessage's destructor to improve.
                    if (pTimeText == NULL ||
                        pTimeText->IsEmpty() || // This message content hasn't been cached by Telegram.
                        pTimeText->Find(_MarkData.Content) != std::wstring::npos /* This message is marked. */)
                    {
                        return;
                    }

                    // Mark "deleted"
                    //

                    std::wstring MarkedTime;

                    if (pSigned != NULL)
                    {
                        // Signed msg text: "<author>, <time>" ("xxx, 10:20")
                        //
                        std::wstring OriginalString = pTimeText->GetText();
                        size_t Pos = OriginalString.rfind(L", ");
                        if (Pos == std::wstring::npos) {
                            return;
                        }

                        MarkedTime = OriginalString.substr(0, Pos + 2) + _MarkData.Content + OriginalString.substr(Pos + 2);
                    }
                    else {
                        MarkedTime = _MarkData.Content + pTimeText->GetText();
                    }

                    pTimeText->Replace(MarkedTime.c_str());

                    // Modify width
                    //

                    HistoryViewElement *pMainView = pMessage->GetMainView();
                    if (pMainView == NULL) {
                        return;
                    }

                    pMainView->SetWidth(pMainView->GetWidth() + _MarkData.Width);
                    pMessage->SetTimeWidth(pMessage->GetTimeWidth() + _MarkData.Width);

                    Media *pMainViewMedia = pMainView->GetMedia();
                    if (pMainViewMedia != NULL) {
                        pMainViewMedia->SetWidth(pMainViewMedia->GetWidth() + _MarkData.Width);
                    }

                    HistoryMessageReply *pReply = pMessage->GetReply();
                    if (pReply != NULL) {
                        pReply->SetMaxReplyWidth(pReply->GetMaxReplyWidth() + _MarkData.Width);
                    }

                }, [&](ULONG ExceptionCode)
                {
                    spdlog::warn("Function: [" __FUNCTION__ "] An exception was caught. Code: {:#x}, Address: {}", ExceptionCode, (void*)pMessage);
                }
            );
        }
    }
}

void IAntiRevoke::CallFree(void* Block)
{
    _FnOriginalFree(Block);
}

bool IAntiRevoke::HookFreeFunction()
{
    auto FnFree = IRuntime::GetInstance().GetData().Function.Free;

    if (MH_CreateHook(FnFree, &IAntiRevoke::Callback_DetourFree, (void**)&_FnOriginalFree) != MH_OK) {
        return false;
    }

    return MH_EnableHook(FnFree) == MH_OK;
}

bool IAntiRevoke::HookRevokeFunction()
{
    void* HookAddress = IRuntime::GetInstance().GetData().Address.FnDestroyMessageCaller;
    void* TargetAddress = Utils::GetFunctionAddress(&History::OnDestroyMessage);
    std::vector<uint8_t> Shellcode = Memory::MakeCall(HookAddress, TargetAddress);

    // Save the original function
    //
    _FnOriginalDestroyMessage = (FnDestroyMessageT)((uintptr_t)HookAddress + 5 + *(int32_t*)((uintptr_t)HookAddress + 1));

    return Memory::ForceOperate(
        HookAddress,
        Shellcode.size(),
        [&]() {
            memcpy(HookAddress, Shellcode.data(), Shellcode.size());
        }
    );
}

void IAntiRevoke::OnFree(void *Block)
{
    std::unique_lock<std::mutex> Lock(_Mutex);

    // When we delete a msg by ourselves, Telegram will free this memory block.
    // So, we need to earse this msg from the vector.
    //

    _BlockedMessages.erase((HistoryMessage*)Block);
    Lock.unlock();

    CallFree(Block);
}

void IAntiRevoke::OnDestroyMessage(History *pHistory, HistoryMessage* pMessage)
{
    Safe::TryExcept(
        [&]()
        {
            // TODO: Allow revoke BOT messages in non-private chats.
            //

            if (!pMessage->IsMessage()) {
                return;
            }

            QtString *pTimeText = pMessage->GetTimeText();
            if (!pTimeText->IsValidTime()) {
                spdlog::warn("A bad TimeText. Address: {}", (void*)pMessage);
                return;
            }

            spdlog::debug("Caught a deleted meesage. Address: {}", (void*)pMessage);

            std::lock_guard<std::mutex> Lock(_Mutex);
            _BlockedMessages.insert(pMessage);
        },
        [&](ULONG ExceptionCode)
        {
            spdlog::warn("Function: [" __FUNCTION__ "] An exception was caught. Code: {:#x}", ExceptionCode);
        }
    );
}

void __cdecl IAntiRevoke::Callback_DetourFree(void *Block)
{
    IAntiRevoke::GetInstance().OnFree(Block);
}
