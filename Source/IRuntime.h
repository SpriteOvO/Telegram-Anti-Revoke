#pragma once

#include <cstdint>
#include <vector>
#include <Windows.h>
#include <Psapi.h>

#include "Telegram.h"


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
            uint32_t Media;
            uint32_t SignedTimeText;
            uint32_t HistoryPeer;
        } Offset;

        struct {
            uint32_t ToHistoryMessage;
        } Index; // Virtual call index

        struct {
            FnMallocT Malloc = NULL;
            FnFreeT Free = NULL;
            FnIndexT EditedIndex = NULL;
            FnIndexT SignedIndex = NULL;
            FnIndexT ReplyIndex = NULL;
        } Function;

        struct {
            void* FnDestroyMessageCaller = NULL;
            LanguageInstance *pLangInstance = NULL;
        } Address;
    };

    static IRuntime& GetInstance();

    const auto &GetData() { return _Data; }

    bool Initialize();

    bool InitFixedData();
    bool InitDynamicData();

private:
    uintptr_t _MainModule = NULL;
    uint32_t _FileVersion = 0;
    MODULEINFO _MainModuleInfo = { 0 };

    DataT _Data;

    std::vector<uintptr_t> FindPatternInMainModule(const char Pattern[], const char Mask[]);
    std::vector<uintptr_t> FindPatternInRange(uintptr_t StartAddress, size_t SearchSize, const char Pattern[], const char Mask[]);

    bool InitDynamicData_MallocFree();
    bool InitDynamicData_DestroyMessage();
    bool InitDynamicData_EditedIndex();
    bool InitDynamicData_SignedIndex();
    bool InitDynamicData_ReplyIndex();
    bool InitDynamicData_LangInstance();
    bool InitDynamicData_ToHistoryMessage();

};
