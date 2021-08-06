#pragma once

#include <cstdint>
#include <vector>
#include <Windows.h>
#include <Psapi.h>

#include <sigmatch/sigmatch.hpp>

#include "Telegram.h"

using namespace sigmatch_literals;

using FnMallocT = void*(__cdecl *)(unsigned int size);
using FnFreeT = void(__cdecl *)(void *block);
using FnIndexT = int(__cdecl *)();

class IRuntime
{
public:
    struct DataT
    {
        struct {
            uint32_t TimeText;
            uint32_t TimeWidth;
            uint32_t MainView;
            // uint32_t Media;
            uint32_t SignedTimeText;
            // uint32_t HistoryPeer;
        } Offset;

        struct {
            uint32_t ToHistoryMessage;
        } Index; // Virtual call index

        struct {
            FnMallocT Malloc = nullptr;
            FnFreeT Free = nullptr;
            FnIndexT EditedIndex = nullptr;
            FnIndexT SignedIndex = nullptr;
            FnIndexT ReplyIndex = nullptr;
        } Function;

        struct {
            void* FnDestroyMessageCaller = nullptr;
            LanguageInstance *pLangInstance = nullptr;
        } Address;
    };

    static IRuntime& GetInstance();

    const auto &GetData() { return _Data; }

    bool Initialize();

    bool InitFixedData();
    bool InitDynamicData();

private:
    sigmatch::this_process_target _ThisProcess;
    sigmatch::search_context _MainModule;
    uint32_t _FileVersion = 0;

    DataT _Data;

    bool InitDynamicData_MallocFree();
    bool InitDynamicData_DestroyMessage();
    bool InitDynamicData_EditedIndex();
    bool InitDynamicData_SignedIndex();
    bool InitDynamicData_ReplyIndex();
    bool InitDynamicData_LangInstance();
    bool InitDynamicData_ToHistoryMessage();

};
