#pragma once

#include <unordered_set>
#include <mutex>

#include "Telegram.h"
#include "IRuntime.h"


using FnDestroyMessageT = void(__thiscall *)(History *pHistory, HistoryMessage *pMessage);

class IAntiRevoke
{
public:
	static IAntiRevoke& GetInstance();

	void InitMarker();
	void SetupHooks();
	void ProcessBlockedMessages();

    void CallFree(void* Block);

private:
    struct MarkDataT {
        std::wstring LangName;
        std::wstring Content;
        int32_t Width;
    };

    MarkDataT _MarkData;
    FnDestroyMessageT _FnOriginalDestroyMessage;
    FnFreeT _FnOriginalFree;
    std::mutex _Mutex;
    std::unordered_set<HistoryMessage*> _BlockedMessages;


    bool HookFreeFunction();
    bool HookRevokeFunction();

    void OnFree(void* Block);
    void OnDestroyMessage(History *pHistory, HistoryMessage* pMessage);

	// Callback
	//
	static void __cdecl Callback_DetourFree(void *Block);

    friend class History;

};
