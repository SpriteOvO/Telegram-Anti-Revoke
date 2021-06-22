#include "IRuntime.h"

#include <spdlog/spdlog.h>

#include "Utils.h"


IRuntime& IRuntime::GetInstance()
{
    static IRuntime i;
    return i;
}

bool IRuntime::Initialize()
{
    _MainModule = (uintptr_t)GetModuleHandleW(L"Telegram.exe");
    _FileVersion = File::GetCurrentVersion();

    if (_MainModule == NULL || _FileVersion == 0) {
        return false;
    }

    spdlog::info("[IRuntime] Telegram version: {}", _FileVersion);

    if (!GetModuleInformation(GetCurrentProcess(), (HMODULE)_MainModule, &_MainModuleInfo, sizeof(_MainModuleInfo))) {
        return false;
    }

    return true;
}

bool IRuntime::InitFixedData()
{
    // ver < 2.4.0
    if (_FileVersion < 2004000) {
        return false;
    }
    // ver >= 2.4.0, ver < 2.4.1
    else if (_FileVersion >= 2004000 && _FileVersion < 2004001)
    {
        _Data.Offset.TimeText = 0x70;
        _Data.Offset.TimeWidth = 0x74;
        _Data.Offset.MainView = 0x5C;
        // _Data.Offset.Media = 0x54;
        _Data.Offset.SignedTimeText = 0x14;
        // _Data.Offset.HistoryPeer = 0x7C;
    }
    // ver >= 2.4.1, ver < 2.6.0
    else if (_FileVersion >= 2004001 && _FileVersion < 2006000)
    {
        _Data.Offset.TimeText = 0x70;
        _Data.Offset.TimeWidth = 0x74;
        _Data.Offset.MainView = 0x5C;
        // _Data.Offset.Media = 0x54;
        _Data.Offset.SignedTimeText = 0x10;    // changed
        // _Data.Offset.HistoryPeer = 0x7C;       // maybe untested! (I forgot :)
    }
    // ver >= 2.6.0
    else if (_FileVersion >= 2006000)
    {
        _Data.Offset.TimeText = 0x78;          // changed
        _Data.Offset.TimeWidth = 0x7C;         // changed
        _Data.Offset.MainView = 0x60;          // changed
        // _Data.Offset.Media = 0x5C;             // changed
        _Data.Offset.SignedTimeText = 0x10;
        // _Data.Offset.HistoryPeer = 0x7C;       // untested!
    }

    return true;
}

bool IRuntime::InitDynamicData()
{
    bool Result = false;

    Safe::TryExcept(
        [&]() {
#define INIT_DATA_AND_LOG(name)                                                 \
            if (!InitDynamicData_ ## name()) {                                  \
                spdlog::warn("[IRuntime] InitDynamicData_" # name "() failed.");       \
                return;                                                         \
            }                                                                   \
            else {                                                              \
                spdlog::info("[IRuntime] InitDynamicData_" # name "() succeeded.");    \
            }

            INIT_DATA_AND_LOG(MallocFree);
            INIT_DATA_AND_LOG(DestroyMessage);
            INIT_DATA_AND_LOG(EditedIndex);
            INIT_DATA_AND_LOG(SignedIndex);
            INIT_DATA_AND_LOG(ReplyIndex);
            INIT_DATA_AND_LOG(LangInstance);
            INIT_DATA_AND_LOG(ToHistoryMessage);

#undef INIT_DATA_AND_LOG
            Result = true;
        },
        [&](uint32_t ExceptionCode) {
            spdlog::warn("[IRuntime] InitDynamicData() caught an exception, code: {:#x}", ExceptionCode);
        }
    );

    return Result;
}

std::vector<uintptr_t> IRuntime::FindPatternInMainModule(const char Pattern[], const char Mask[])
{
    return Memory::FindPatternEx(GetCurrentProcess(), (void*)_MainModule, _MainModuleInfo.SizeOfImage, Pattern, Mask);
}

std::vector<uintptr_t> IRuntime::FindPatternInRange(uintptr_t StartAddress, size_t SearchSize, const char Pattern[], const char Mask[])
{
    return Memory::FindPatternEx(GetCurrentProcess(), (void*)StartAddress, SearchSize, Pattern, Mask);
}

// Some of the following instructions are taken from version 1.8.8
// Thanks to [采蘑菇的小蘑菇] for providing help with compiling Telegram.
//

bool IRuntime::InitDynamicData_MallocFree()
{
#if defined PLATFORM_X86

    /*
        void __cdecl __std_exception_copy(__std_exception_data *from, __std_exception_data *to)

        .text:01B7CAAE 8A 01                                   mov     al, [ecx]
        .text:01B7CAB0 41                                      inc     ecx
        .text:01B7CAB1 84 C0                                   test    al, al
        .text:01B7CAB3 75 F9                                   jnz     short loc_1B7CAAE
        .text:01B7CAB5 2B CA                                   sub     ecx, edx
        .text:01B7CAB7 53                                      push    ebx
        .text:01B7CAB8 56                                      push    esi
        .text:01B7CAB9 8D 59 01                                lea     ebx, [ecx+1]
        .text:01B7CABC 53                                      push    ebx             ; size

        // find this (internal malloc)
        //
        .text:01B7CABD E8 87 98 00 00                          call    _malloc

        .text:01B7CAC2 8B F0                                   mov     esi, eax
        .text:01B7CAC4 59                                      pop     ecx
        .text:01B7CAC5 85 F6                                   test    esi, esi
        .text:01B7CAC7 74 19                                   jz      short loc_1B7CAE2
        .text:01B7CAC9 FF 37                                   push    dword ptr [edi] ; source
        .text:01B7CACB 53                                      push    ebx             ; size_in_elements
        .text:01B7CACC 56                                      push    esi             ; destination
        .text:01B7CACD E8 B2 9E 01 00                          call    _strcpy_s
        .text:01B7CAD2 8B 45 0C                                mov     eax, [ebp+to]
        .text:01B7CAD5 8B CE                                   mov     ecx, esi
        .text:01B7CAD7 83 C4 0C                                add     esp, 0Ch
        .text:01B7CADA 33 F6                                   xor     esi, esi
        .text:01B7CADC 89 08                                   mov     [eax], ecx
        .text:01B7CADE C6 40 04 01                             mov     byte ptr [eax+4], 1
        .text:01B7CAE2
        .text:01B7CAE2                         loc_1B7CAE2:                            ; CODE XREF: ___std_exception_copy+2F↑j
        .text:01B7CAE2 56                                      push    esi             ; block

        // and find this (internal free)
        //
        .text:01B7CAE3 E8 7F 45 00 00                          call    _free

        .text:01B7CAE8 59                                      pop     ecx
        .text:01B7CAE9 5E                                      pop     esi
        .text:01B7CAEA 5B                                      pop     ebx
        .text:01B7CAEB EB 0B                                   jmp     short loc_1B7CAF8

        malloc		41 84 C0 75 F9 2B CA 53 56 8D 59 01 53 E8
        free		56 E8 ?? ?? ?? ?? 59 5E 5B EB
    */

    std::vector<uintptr_t> vMallocResult = FindPatternInMainModule("\x41\x84\xC0\x75\xF9\x2B\xCA\x53\x56\x8D\x59\x01\x53\xE8", "xxxxxxxxxxxxxx");
    if (vMallocResult.size() != 1) {
        spdlog::warn("[IRuntime] Search malloc failed.");
        return false;
    }

    std::vector<uintptr_t> vFreeResult = FindPatternInRange(vMallocResult.at(0), 0x50, "\x56\xE8\x00\x00\x00\x00\x59\x5E\x5B\xEB", "xx????xxxx");
    if (vFreeResult.size() != 1) {
        spdlog::warn("[IRuntime] Search free failed.");
        return false;
    }

    uintptr_t MallocCaller = vMallocResult.at(0) + 13;
    uintptr_t FreeCaller = vFreeResult.at(0) + 1;

    _Data.Function.Malloc = (FnMallocT)(MallocCaller + 5 + *(int32_t*)(MallocCaller + 1));
    _Data.Function.Free = (FnFreeT)(FreeCaller + 5 + *(int32_t*)(FreeCaller + 1));

    return true;

#elif defined PLATFORM_X64

    /*
        void __fastcall _std_exception_copy(const __std_exception_data *from, __std_exception_data *to)

        .text:0000000142B5CF5D 48 FF C7                                inc     rdi
        .text:0000000142B5CF60 80 3C 38 00                             cmp     byte ptr [rax+rdi], 0
        .text:0000000142B5CF64 75 F7                                   jnz     short loc_142B5CF5D
        .text:0000000142B5CF66 48 8D 4F 01                             lea     rcx, [rdi+1]    ; size

        // find this (internal malloc)
        //
        .text:0000000142B5CF6A E8 99 F5 00 00                          call    malloc

        .text:0000000142B5CF6F 48 8B D8                                mov     rbx, rax
        .text:0000000142B5CF72 48 85 C0                                test    rax, rax
        .text:0000000142B5CF75 74 1C                                   jz      short loc_142B5CF93
        .text:0000000142B5CF77 4C 8B 06                                mov     r8, [rsi]       ; source
        .text:0000000142B5CF7A 48 8D 57 01                             lea     rdx, [rdi+1]    ; size_in_elements
        .text:0000000142B5CF7E 48 8B C8                                mov     rcx, rax        ; destination
        .text:0000000142B5CF81 E8 96 5E 02 00                          call    strcpy_s
        .text:0000000142B5CF86 48 8B C3                                mov     rax, rbx
        .text:0000000142B5CF89 41 C6 46 08 01                          mov     byte ptr [r14+8], 1
        .text:0000000142B5CF8E 49 89 06                                mov     [r14], rax
        .text:0000000142B5CF91 33 DB                                   xor     ebx, ebx
        .text:0000000142B5CF93
        .text:0000000142B5CF93                         loc_142B5CF93:                          ; CODE XREF: __std_exception_copy+45↑j
        .text:0000000142B5CF93 48 8B CB                                mov     rcx, rbx        ; block

        // and find this (internal free)
        //
        .text:0000000142B5CF96 E8 59 F5 00 00                          call    free

        .text:0000000142B5CF9B EB 0A                                   jmp     short loc_142B5CFA7

        malloc: 48 FF C7 80 3C 38 00 75 F7 48 8D 4F 01 E8
        free: 48 8B CB E8 ?? ?? ?? ?? EB
    */

    std::vector<uintptr_t> vMallocResult = FindPatternInMainModule("\x48\xFF\xC7\x80\x3C\x38\x00\x75\xF7\x48\x8D\x4F\x01\xE8", "xxxxxxxxxxxxxx");
    if (vMallocResult.size() != 1) {
        spdlog::warn("[IRuntime] Search malloc failed.");
        return false;
    }

    std::vector<uintptr_t> vFreeResult = FindPatternInRange(vMallocResult.at(0), 0x50, "\x48\x8B\xCB\xE8\x00\x00\x00\x00\xEB", "xxxx????x");
    if (vFreeResult.size() != 1) {
        spdlog::warn("[IRuntime] Search free failed.");
        return false;
    }

    uintptr_t MallocCaller = vMallocResult.at(0) + 13;
    uintptr_t FreeCaller = vFreeResult.at(0) + 3;

    _Data.Function.Malloc = (FnMallocT)(MallocCaller + 5 + *(int32_t*)(MallocCaller + 1));
    _Data.Function.Free = (FnFreeT)(FreeCaller + 5 + *(int32_t*)(FreeCaller + 1));

    return true;

#else
# error "Unimplemented."
#endif
}

bool IRuntime::InitDynamicData_DestroyMessage()
{
#if defined PLATFORM_X86

    /*
        void __userpurge Data::Session::processMessagesDeleted(Data::Session *this@<ecx>, int a2@<ebp>, int a3@<edi>, int a4@<esi>, int channelId, QVector<MTPint> *data)

        .text:008CD8C1 8B 08                                   mov     this, [eax]
        .text:008CD8C3 8B 45 E8                                mov     eax, [ebp-18h]
        .text:008CD8C6 3B 48 04                                cmp     this, [eax+4]
        .text:008CD8C9 74 41                                   jz      short loc_8CD90C
        .text:008CD8CB 8B 49 0C                                mov     this, [this+0Ch]
        .text:008CD8CE 51                                      push    this            ; item
        .text:008CD8CF 8B C4                                   mov     eax, esp
        .text:008CD8D1 8B 71 10                                mov     esi, [this+10h]
        .text:008CD8D4 89 08                                   mov     [eax], this
        .text:008CD8D6 85 C9                                   test    this, this
        .text:008CD8D8 0F 84 A5 00 00 00                       jz      loc_8CD983
        .text:008CD8DE 8B 4D E0                                mov     this, [ebp-20h] ; this

        // find this
        //
        .text:008CD8E1 E8 9A 02 00 00                          call    ?destroyMessage@Session@Data@@QAEXV?$not_null@PAVHistoryItem@@@gsl@@@Z ; Data::Session::destroyMessage(gsl::not_null<HistoryItem *>)

        .text:008CD8E6 85 F6                                   test    esi, esi
        .text:008CD8E8 0F 84 0F 01 00 00                       jz      loc_8CD9FD
        .text:008CD8EE 80 BE 60 01 00 00 00                    cmp     byte ptr [esi+160h], 0
        .text:008CD8F5 75 69                                   jnz     short loc_8CD960
        .text:008CD8F7 8D 45 E4                                lea     eax, [ebp-1Ch]
        .text:008CD8FA 89 75 E4                                mov     [ebp-1Ch], esi
        .text:008CD8FD 50                                      push    eax             ; value
        .text:008CD8FE 8D 45 C8                                lea     eax, [ebp-38h]
        .text:008CD901 50                                      push    eax             ; result
        .text:008CD902 8D 4D B4                                lea     this, [ebp-4Ch] ; this
        .text:008CD905 E8 B6 3B D5 FF                          call    ?insert@?$flat_set@V?$not_null@PAVHistory@@@gsl@@U?$less@X@std@@@base@@QAE?AU?$pair@V?$flat_multi_set_iterator_impl@V?$not_null@PAVHistory@@@gsl@@V?$_Deque_iterator@V?$_Deque_val@U?$_Deque_simple_types@V?$flat_multi_set_const_wrap@V?$not_null@PAVHistory@@@gsl@@@base@@@std@@@std@@@std@@@base@@_N@std@@$$QAV?$not_null@PAVHistory@@@gsl@@@Z ; base::flat_set<gsl::not_null<History *>,std::less<void>>::insert(gsl::not_null<History *> &&)
        .text:008CD90A EB 54                                   jmp     short loc_8CD960

        8B 71 ?? 89 08 85 C9 0F 84 ?? ?? ?? ?? ?? ?? ?? E8

        ==========

        1.9.15 new :

        Telegram.exe+54069F - 51                    - push ecx
        Telegram.exe+5406A0 - 8B C4                 - mov eax,esp
        Telegram.exe+5406A2 - 89 08                 - mov [eax],ecx
        Telegram.exe+5406A4 - 8B CE                 - mov ecx,esi
        Telegram.exe+5406A6 - E8 D5751B00           - call Telegram.exe+6F7C80
        Telegram.exe+5406AB - 80 BE 58010000 00     - cmp byte ptr [esi+00000158],00 { 0 }
        Telegram.exe+5406B2 - 75 13                 - jne Telegram.exe+5406C7

        51 8B C4 89 08 8B CE E8 ?? ?? ?? ?? 80 BE ?? ?? ?? ?? 00
    */

    // ver < 1.9.15
    if (_FileVersion < 1009015)
    {
        std::vector<uintptr_t> vResult = FindPatternInMainModule("\x8B\x71\x00\x89\x08\x85\xC9\x0F\x84\x00\x00\x00\x00\x00\x00\x00\xE8", "xx?xxxxxx???????x");
        if (vResult.size() != 1) {
            spdlog::warn("[IRuntime] Search DestroyMessage failed.");
            return false;
        }

        _Data.Address.FnDestroyMessageCaller = (void*)(vResult.at(0) + 16);
    }
    // ver >= 1.9.15
    else if (_FileVersion >= 1009015)
    {
        std::vector<uintptr_t> vResult = FindPatternInMainModule("\x51\x8B\xC4\x89\x08\x8B\xCE\xE8\x00\x00\x00\x00\x80\xBE\x00\x00\x00\x00\x00", "xxxxxxxx????xx????x");
        if (vResult.size() != 1) {
            spdlog::warn("[IRuntime] Search new DestroyMessage failed.");
            return false;
        }

        _Data.Address.FnDestroyMessageCaller = (void*)(vResult.at(0) + 7);
    }

    return true;

#elif defined PLATFORM_X64

    /*
        void Data::Session::processMessagesDeleted(struct ChatIdType<2>, class QVector<class tl::int_type> const &)

        Telegram.exe+7ADDB2 - 74 1C                 - je Telegram.exe+7ADDD0
        Telegram.exe+7ADDB4 - 48 8B 09              - mov rcx,[rcx]
        Telegram.exe+7ADDB7 - 3B 42 10              - cmp eax,[rdx+10]
        Telegram.exe+7ADDBA - 74 17                 - je Telegram.exe+7ADDD3
        Telegram.exe+7ADDBC - 0F1F 40 00            - nop dword ptr [rax+00]
        Telegram.exe+7ADDC0 - 48 3B D1              - cmp rdx,rcx
        Telegram.exe+7ADDC3 - 74 0B                 - je Telegram.exe+7ADDD0
        Telegram.exe+7ADDC5 - 48 8B 52 08           - mov rdx,[rdx+08]
        Telegram.exe+7ADDC9 - 3B 42 10              - cmp eax,[rdx+10]
        Telegram.exe+7ADDCC - 75 F2                 - jne Telegram.exe+7ADDC0
        Telegram.exe+7ADDCE - EB 03                 - jmp Telegram.exe+7ADDD3
        Telegram.exe+7ADDD0 - 48 8B D5              - mov rdx,rbp
        Telegram.exe+7ADDD3 - 48 85 D2              - test rdx,rdx
        Telegram.exe+7ADDD6 - 49 0F44 D0            - cmove rdx,r8
        Telegram.exe+7ADDDA - 49 3B D0              - cmp rdx,r8
        Telegram.exe+7ADDDD - 74 4C                 - je Telegram.exe+7ADE2B
        Telegram.exe+7ADDDF - 48 8B 52 18           - mov rdx,[rdx+18]
        Telegram.exe+7ADDE3 - 48 85 D2              - test rdx,rdx
        Telegram.exe+7ADDE6 - 0F84 5D010000         - je Telegram.exe+7ADF49
        Telegram.exe+7ADDEC - 48 8B 5A 18           - mov rbx,[rdx+18]
        Telegram.exe+7ADDF0 - 48 85 DB              - test rbx,rbx
        Telegram.exe+7ADDF3 - 0F84 29010000         - je Telegram.exe+7ADF22
        Telegram.exe+7ADDF9 - 48 8B CB              - mov rcx,rbx

        // find this
        //
        Telegram.exe+7ADDFC - E8 2FAD2600           - call Telegram.exe+A18B30

        Telegram.exe+7ADE01 - 80 BB 28020000 00     - cmp byte ptr [rbx+00000228],00 { 0 }
        Telegram.exe+7ADE08 - 75 6D                 - jne Telegram.exe+7ADE77
        Telegram.exe+7ADE0A - 48 89 9C 24 C8000000  - mov [rsp+000000C8],rbx
        Telegram.exe+7ADE12 - 4C 8D 84 24 C8000000  - lea r8,[rsp+000000C8]
        Telegram.exe+7ADE1A - 48 8D 54 24 20        - lea rdx,[rsp+20]
        Telegram.exe+7ADE1F - 48 8D 4C 24 30        - lea rcx,[rsp+30]
        Telegram.exe+7ADE24 - E8 E7F4ACFF           - call Telegram.exe+27D310
        Telegram.exe+7ADE29 - EB 4C                 - jmp Telegram.exe+7ADE77
        Telegram.exe+7ADE2B - 48 85 FF              - test rdi,rdi
        Telegram.exe+7ADE2E - 74 47                 - je Telegram.exe+7ADE77
        Telegram.exe+7ADE30 - 80 BF 74010000 00     - cmp byte ptr [rdi+00000174],00 { 0 }
        Telegram.exe+7ADE37 - 74 3E                 - je Telegram.exe+7ADE77
        Telegram.exe+7ADE39 - 3B 87 70010000        - cmp eax,[rdi+00000170]
        Telegram.exe+7ADE3F - 7C 36                 - jl Telegram.exe+7ADE77
        Telegram.exe+7ADE41 - 48 8B 4F 48           - mov rcx,[rdi+48]
        Telegram.exe+7ADE45 - 48 85 C9              - test rcx,rcx
        Telegram.exe+7ADE48 - 0F84 86000000         - je Telegram.exe+7ADED4
        Telegram.exe+7ADE4E - 48 8B 89 F80D0000     - mov rcx,[rcx+00000DF8]
        Telegram.exe+7ADE55 - 48 8D 44 24 48        - lea rax,[rsp+48]
        Telegram.exe+7ADE5A - 48 89 84 24 C8000000  - mov [rsp+000000C8],rax
        Telegram.exe+7ADE62 - 48 89 AC 24 80000000  - mov [rsp+00000080],rbp
        Telegram.exe+7ADE6A - 4C 8D 44 24 48        - lea r8,[rsp+48]
        Telegram.exe+7ADE6F - 48 8B D7              - mov rdx,rdi
        Telegram.exe+7ADE72 - E8 09B1F8FF           - call Telegram.exe+738F80
        Telegram.exe+7ADE77 - 48 83 C6 04           - add rsi,04 { 4 }
        Telegram.exe+7ADE7B - 49 3B F7              - cmp rsi,r15
        Telegram.exe+7ADE7E - 0F85 CCFEFFFF         - jne Telegram.exe+7ADD50

        48 8B 5A 18 48 85 DB 0F 84 ?? ?? ?? ?? 48 8B CB E8 ?? ?? ?? ?? 80 BB ?? ?? ?? ?? 00
    */

    std::vector<uintptr_t> vResult = FindPatternInMainModule("\x48\x8B\x5A\x18\x48\x85\xDB\x0F\x84\x00\x00\x00\x00\x48\x8B\xCB\xE8\x00\x00\x00\x00\x80\xBB\x00\x00\x00\x00\x00", "xxxxxxxxx????xxxx????xx????x");
    if (vResult.size() != 1) {
        spdlog::warn("[IRuntime] Search DestroyMessage failed.");
        return false;
    }

    _Data.Address.FnDestroyMessageCaller = (void*)(vResult.at(0) + 16);

    return true;

#else
# error "Unimplemented."
#endif
}

bool IRuntime::InitDynamicData_EditedIndex()
{
#if defined PLATFORM_X86

    /*
        void __thiscall HistoryMessage::applyEdition(HistoryMessage *this, MTPDmessage *message)

        .text:00A4F320 55                                      push    ebp
        .text:00A4F321 8B EC                                   mov     ebp, esp
        .text:00A4F323 6A FF                                   push    0FFFFFFFFh
        .text:00A4F325 68 28 4F C8 01                          push    offset __ehhandler$?applyEdition@HistoryMessage@@UAEXABVMTPDmessage@@@Z
        .text:00A4F32A 64 A1 00 00 00 00                       mov     eax, large fs:0
        .text:00A4F330 50                                      push    eax
        .text:00A4F331 83 EC 0C                                sub     esp, 0Ch
        .text:00A4F334 53                                      push    ebx
        .text:00A4F335 56                                      push    esi
        .text:00A4F336 57                                      push    edi
        .text:00A4F337 A1 04 68 ED 02                          mov     eax, ___security_cookie
        .text:00A4F33C 33 C5                                   xor     eax, ebp
        .text:00A4F33E 50                                      push    eax
        .text:00A4F33F 8D 45 F4                                lea     eax, [ebp+var_C]
        .text:00A4F342 64 A3 00 00 00 00                       mov     large fs:0, eax
        .text:00A4F348 8B D9                                   mov     ebx, this
        .text:00A4F34A 8B 7D 08                                mov     edi, [ebp+message]
        .text:00A4F34D 8B 77 08                                mov     esi, [edi+8]
        .text:00A4F350 8D 47 48                                lea     eax, [edi+48h]

        .text:00A4F353 81 E6 00 80 00 00                       and     esi, 8000h
        .text:00A4F359 F7 DE                                   neg     esi
        .text:00A4F35B 1B F6                                   sbb     esi, esi
        .text:00A4F35D 23 F0                                   and     esi, eax
        .text:00A4F35F 74 65                                   jz      short loc_A4F3C6
        .text:00A4F361 81 4B 18 00 80 00 00                    or      dword ptr [ebx+18h], 8000h
        .text:00A4F368 8B 43 08                                mov     eax, [ebx+8]
        .text:00A4F36B 8B 38                                   mov     edi, [eax]

        // find this (RuntimeComponent<HistoryMessageEdited,HistoryItem>::Index()
        //
        .text:00A4F36D E8 6E 3A EA FF                          call    ?Index@?$RuntimeComponent@UHistoryMessageEdited@@VHistoryItem@@@@SAHXZ ; RuntimeComponent<HistoryMessageEdited,HistoryItem>::Index(void)

        .text:00A4F372 83 7C 87 08 04                          cmp     dword ptr [edi+eax*4+8], 4
        .text:00A4F377 73 28                                   jnb     short loc_A4F3A1
        .text:00A4F379 E8 62 3A EA FF                          call    ?Index@?$RuntimeComponent@UHistoryMessageEdited@@VHistoryItem@@@@SAHXZ ; RuntimeComponent<HistoryMessageEdited,HistoryItem>::Index(void)
        .text:00A4F37E 33 D2                                   xor     edx, edx

        E8 ?? ?? ?? ?? 83 7C 87 ?? ?? 73 ?? E8
    */

    std::vector<uintptr_t> vResult = FindPatternInMainModule("\xE8\x00\x00\x00\x00\x83\x7C\x87\x00\x00\x73\x00\xE8", "x????xxx??x?x");
    if (vResult.size() != 1) {
        spdlog::warn("[IRuntime] Search EditedIndex failed.");
        return false;
    }

    uintptr_t EditedIndexCaller = vResult.at(0);
    _Data.Function.EditedIndex = (FnIndexT)(EditedIndexCaller + 5 + *(int32_t*)(EditedIndexCaller + 1));

    return true;

#elif defined PLATFORM_X64

    /*
        void __fastcall HistoryMessage::applyEdition(HistoryMessage *__hidden this, const struct MTPDmessage *)

        Telegram.exe+A65456 - 44 8B 42 10           - mov r8d,[rdx+10]
        Telegram.exe+A6545A - 41 81 E0 00002000     - and r8d,00200000 { 2097152 }
        Telegram.exe+A65461 - 44 0B C0              - or r8d,eax
        Telegram.exe+A65464 - 44 89 41 28           - mov [rcx+28],r8d
        Telegram.exe+A65468 - F7 42 10 00800000     - test [rdx+10],00008000 { 32768 }
        Telegram.exe+A6546F - 48 8D 9A 98000000     - lea rbx,[rdx+00000098]
        Telegram.exe+A65476 - 49 0F44 DF            - cmove rbx,r15
        Telegram.exe+A6547A - 48 85 DB              - test rbx,rbx
        Telegram.exe+A6547D - 74 6F                 - je Telegram.exe+A654EE
        Telegram.exe+A6547F - 41 0FBA E8 0F         - bts r8d,0F { 15 }
        Telegram.exe+A65484 - 44 89 41 28           - mov [rcx+28],r8d
        Telegram.exe+A65488 - 48 8B 41 08           - mov rax,[rcx+08]
        Telegram.exe+A6548C - 48 8B 38              - mov rdi,[rax]
        Telegram.exe+A6548F - E8 8C0ED7FF           - call Telegram.exe+7D6320
        Telegram.exe+A65494 - 48 63 C8              - movsxd  rcx,eax
        Telegram.exe+A65497 - 48 83 7C CF 10 08     - cmp qword ptr [rdi+rcx*8+10],08 { 8 }
        Telegram.exe+A6549D - 73 26                 - jae Telegram.exe+A654C5

        // find this
        //
        Telegram.exe+A6549F - E8 7C0ED7FF           - call Telegram.exe+7D6320

        Telegram.exe+A654A4 - 8B C8                 - mov ecx,eax
        Telegram.exe+A654A6 - BA 01000000           - mov edx,00000001 { 1 }
        Telegram.exe+A654AB - 48 D3 E2              - shl rdx,cl
        Telegram.exe+A654AE - 48 8B 46 08           - mov rax,[rsi+08]
        Telegram.exe+A654B2 - 48 8B 08              - mov rcx,[rax]
        Telegram.exe+A654B5 - 48 0B 91 18020000     - or rdx,[rcx+00000218]
        Telegram.exe+A654BC - 48 8D 4E 08           - lea rcx,[rsi+08]
        Telegram.exe+A654C0 - E8 FB27EBFF           - call Telegram.exe+917CC0

        48 83 7C CF 10 08 73 ?? E8
    */

    std::vector<uintptr_t> vResult = FindPatternInMainModule("\x48\x83\x7C\xCF\x10\x08\x73\x00\xE8", "xxxxxxx?x");
    if (vResult.size() != 1) {
        spdlog::warn("[IRuntime] Search EditedIndex failed.");
        return false;
    }

    uintptr_t EditedIndexCaller = vResult.at(0) + 8;
    _Data.Function.EditedIndex = (FnIndexT)(EditedIndexCaller + 5 + *(int32_t*)(EditedIndexCaller + 1));

    return true;

#else
# error "Unimplemented."
#endif
}

bool IRuntime::InitDynamicData_SignedIndex()
{
#if defined PLATFORM_X86

    /*
        HistoryView__Message__refreshEditedBadge

        .text:009F109D                         loc_9F109D:                             ; CODE XREF: HistoryView__Message__refreshEditedBadge+69↑j
        .text:009F109D 8D 47 28                                lea     eax, [edi+28h]
        .text:009F10A0 50                                      push    eax
        .text:009F10A1 8D 4D E8                                lea     ecx, [ebp-18h]
        .text:009F10A4 E8 07 2E FC 00                          call    QDateTime__QDateTime
        .text:009F10A9 68 8C 88 3D 03                          push    offset gTimeFormat
        .text:009F10AE 8D 45 F0                                lea     eax, [ebp-10h]
        .text:009F10B1 C7 45 FC 00 00 00 00                    mov     dword ptr [ebp-4], 0
        .text:009F10B8 50                                      push    eax
        .text:009F10B9 8D 4D E8                                lea     ecx, [ebp-18h]
        .text:009F10BC E8 DF 91 FC 00                          call    QDateTime__toString
        .text:009F10C1 8D 4D E8                                lea     ecx, [ebp-18h]
        .text:009F10C4 C6 45 FC 02                             mov     byte ptr [ebp-4], 2
        .text:009F10C8 E8 43 31 FC 00                          call    QDateTime___QDateTime
        .text:009F10CD 8B 4D EC                                mov     ecx, [ebp-14h]
        .text:009F10D0 85 C9                                   test    ecx, ecx
        .text:009F10D2 74 12                                   jz      short loc_9F10E6
        .text:009F10D4 85 DB                                   test    ebx, ebx
        .text:009F10D6 0F 95 C0                                setnz   al
        .text:009F10D9 0F B6 C0                                movzx   eax, al
        .text:009F10DC 50                                      push    eax
        .text:009F10DD 8D 45 F0                                lea     eax, [ebp-10h]
        .text:009F10E0 50                                      push    eax
        .text:009F10E1 E8 AA BA 03 00                          call    HistoryMessageEdited__refresh
        .text:009F10E6
        .text:009F10E6                         loc_9F10E6:                             ; CODE XREF: HistoryView__Message__refreshEditedBadge+A2↑j
        .text:009F10E6 8B 46 08                                mov     eax, [esi+8]
        .text:009F10E9 8B 38                                   mov     edi, [eax]

        // find this (RuntimeComponent<HistoryMessageSigned,HistoryItem>::Index()
        //
        .text:009F10EB E8 30 62 FA FF                          call    RuntimeComponent_HistoryMessageSigned_HistoryItem___Index

        .text:009F10F0 8B 44 87 08                             mov     eax, [edi+eax*4+8]
        .text:009F10F4 83 CF FF                                or      edi, 0FFFFFFFFh
        .text:009F10F7 83 F8 04                                cmp     eax, 4
        .text:009F10FA 0F 82 F2 00 00 00                       jb      loc_9F11F2
        .text:009F1100 8B 76 08                                mov     esi, [esi+8]
        .text:009F1103 03 F0                                   add     esi, eax
        .text:009F1105 0F 84 E7 00 00 00                       jz      loc_9F11F2
        .text:009F110B 8B 45 EC                                mov     eax, [ebp-14h]
        .text:009F110E 85 C0                                   test    eax, eax
        .text:009F1110 74 1D                                   jz      short loc_9F112F
        .text:009F1112 85 DB                                   test    ebx, ebx
        .text:009F1114 74 19                                   jz      short loc_9F112F
        .text:009F1116 FF 35 74 3C C4 02                       push    ds:AllTextSelection_7
        .text:009F111C 8D 48 04                                lea     ecx, [eax+4]
        .text:009F111F 8D 45 E0                                lea     eax, [ebp-20h]
        .text:009F1122 50                                      push    eax
        .text:009F1123 E8 68 26 3F 00                          call    Ui__Text__String__toString
        .text:009F1128 8D 5F 06                                lea     ebx, [edi+6]
        .text:009F112B 8B 08                                   mov     ecx, [eax]
        .text:009F112D EB 1C                                   jmp     short loc_9F114B

        E8 ?? ?? ?? ?? 8B 44 87 08 83 CF FF
    */

    std::vector<uintptr_t> vResult = FindPatternInMainModule("\xE8\x00\x00\x00\x00\x8B\x44\x87\x08\x83\xCF\xFF", "x????xxxxxxx");
    if (vResult.size() != 1) {
        spdlog::warn("[IRuntime] Search SignedIndex failed.");
        return false;
    }

    uintptr_t SignedIndexCaller = vResult.at(0);
    _Data.Function.SignedIndex = (FnIndexT)(SignedIndexCaller + 5 + *(int32_t*)(SignedIndexCaller + 1));

    return true;

#elif defined PLATFORM_X64

    /*
        void __fastcall HistoryMessage::setPostAuthor(HistoryMessage *__hidden this, const struct QString *)

        .text:0000000140ADFF00 48 89 5C 24 10                          mov     [rsp+arg_8], rbx
        .text:0000000140ADFF05 48 89 74 24 18                          mov     [rsp+arg_10], rsi
        .text:0000000140ADFF0A 48 89 7C 24 20                          mov     [rsp+arg_18], rdi
        .text:0000000140ADFF0F 41 56                                   push    r14
        .text:0000000140ADFF11 48 83 EC 20                             sub     rsp, 20h
        .text:0000000140ADFF15 48 8B 41 08                             mov     rax, [rcx+8]
        .text:0000000140ADFF19 4C 8B F2                                mov     r14, rdx
        .text:0000000140ADFF1C 48 8B F9                                mov     rdi, rcx
        .text:0000000140ADFF1F 48 8B 18                                mov     rbx, [rax]
        .text:0000000140ADFF22 E8 E9 B4 FF FF                          call    ?Index@?$RuntimeComponent@UHistoryMessageSigned@@VHistoryItem@@@@SAHXZ ; RuntimeComponent<HistoryMessageSigned,HistoryItem>::Index(void)
        .text:0000000140ADFF27 4C 63 C0                                movsxd  r8, eax
        .text:0000000140ADFF2A 4A 63 44 C3 10                          movsxd  rax, dword ptr [rbx+r8*8+10h]
        .text:0000000140ADFF2F 83 F8 08                                cmp     eax, 8
        .text:0000000140ADFF32 72 09                                   jb      short loc_140ADFF3D
        .text:0000000140ADFF34 48 8B D8                                mov     rbx, rax
        .text:0000000140ADFF37 48 03 5F 08                             add     rbx, [rdi+8]
        .text:0000000140ADFF3B EB 02                                   jmp     short loc_140ADFF3F
        .text:0000000140ADFF3D                         ; ---------------------------------------------------------------------------
        .text:0000000140ADFF3D
        .text:0000000140ADFF3D                         loc_140ADFF3D:                          ; CODE XREF: HistoryMessage::setPostAuthor(QString const &)+32↑j
        .text:0000000140ADFF3D 33 DB                                   xor     ebx, ebx
        .text:0000000140ADFF3F
        .text:0000000140ADFF3F                         loc_140ADFF3F:                          ; CODE XREF: HistoryMessage::setPostAuthor(QString const &)+3B↑j
        .text:0000000140ADFF3F 49 8B 06                                mov     rax, [r14]
        .text:0000000140ADFF42 83 78 04 00                             cmp     dword ptr [rax+4], 0
        .text:0000000140ADFF46 75 48                                   jnz     short loc_140ADFF90
        .text:0000000140ADFF48 48 85 DB                                test    rbx, rbx
        .text:0000000140ADFF4B 0F 84 FA 00 00 00                       jz      loc_140AE004B
        .text:0000000140ADFF51 E8 BA B4 FF FF                          call    ?Index@?$RuntimeComponent@UHistoryMessageSigned@@VHistoryItem@@@@SAHXZ ; RuntimeComponent<HistoryMessageSigned,HistoryItem>::Index(void)
        .text:0000000140ADFF56 8B C8                                   mov     ecx, eax
        .text:0000000140ADFF58 41 B8 01 00 00 00                       mov     r8d, 1
        .text:0000000140ADFF5E 48 8B 47 08                             mov     rax, [rdi+8]
        .text:0000000140ADFF62 49 D3 E0                                shl     r8, cl
        .text:0000000140ADFF65 48 8D 4F 08                             lea     rcx, [rdi+8]    ; mask
        .text:0000000140ADFF69 49 F7 D0                                not     r8
        .text:0000000140ADFF6C 48 8B 10                                mov     rdx, [rax]
        .text:0000000140ADFF6F 48 8B 92 18 02 00 00                    mov     rdx, [rdx+218h]
        .text:0000000140ADFF76 49 23 D0                                and     rdx, r8
        .text:0000000140ADFF79 E8 82 F7 EA FF                          call    ?UpdateComponents@RuntimeComposerBase@@IEAA_N_K@Z ; RuntimeComposerBase::UpdateComponents(unsigned __int64)
        .text:0000000140ADFF7E 48 8B 4F 18                             mov     rcx, [rdi+18h]
        .text:0000000140ADFF82 48 85 C9                                test    rcx, rcx
        .text:0000000140ADFF85 0F 84 D6 00 00 00                       jz      loc_140AE0061
        .text:0000000140ADFF8B E9 AB 00 00 00                          jmp     loc_140AE003B

        E8 ?? ?? ?? ?? 4C 63 C0 4A 63 44 C3 10 83 F8 08 72 ?? 48 8B D8 48 03 5F 08 EB
    */

    std::vector<uintptr_t> vResult = FindPatternInMainModule("\xE8\x00\x00\x00\x00\x4C\x63\xC0\x4A\x63\x44\xC3\x10\x83\xF8\x08\x72\x00\x48\x8B\xD8\x48\x03\x5F\x08\xEB", "x????xxxxxxxxxxxx?xxxxxxxx");
    if (vResult.size() != 1) {
        spdlog::warn("[IRuntime] Search SignedIndex failed.");
        return false;
    }

    uintptr_t SignedIndexCaller = vResult.at(0);
    _Data.Function.SignedIndex = (FnIndexT)(SignedIndexCaller + 5 + *(int32_t*)(SignedIndexCaller + 1));

    return true;

#else
# error "Unimplemented."
#endif
}

bool IRuntime::InitDynamicData_ReplyIndex()
{
#if defined PLATFORM_X86

    /*
        HistoryView__Message__updatePressed

        .text:009EE632                         loc_9EE632:                             ; CODE XREF: HistoryView__Message__updatePressed+100↑j
        .text:009EE632 8B CF                                   mov     ecx, edi
        .text:009EE634 E8 27 13 00 00                          call    HistoryView__Message__displayFromName
        .text:009EE639 8B CF                                   mov     ecx, edi
        .text:009EE63B E8 00 14 00 00                          call    HistoryView__Message__displayForwardedFrom
        .text:009EE640 84 C0                                   test    al, al
        .text:009EE642 74 0E                                   jz      short loc_9EE652
        .text:009EE644 8D 4C 24 18                             lea     ecx, [esp+18h]
        .text:009EE648 E8 A3 75 BE FF                          call    gsl__not_null_Calls__Call__Delegate_____operator__
        .text:009EE64D E8 AE DF EE FF                          call    RuntimeComponent_HistoryMessageForwarded_HistoryItem___Index
        .text:009EE652
        .text:009EE652                         loc_9EE652:                             ; CODE XREF: HistoryView__Message__updatePressed+122↑j

        // find this (RuntimeComponent<HistoryMessageReply,HistoryItem>::Index()
        //
        .text:009EE652 E8 B9 52 FB FF                          call    RuntimeComponent_HistoryMessageReply_HistoryItem___Index

        .text:009EE657 8B 46 08                                mov     eax, [esi+8]
        .text:009EE65A 8B 38                                   mov     edi, [eax]
        .text:009EE65C E8 BF 53 FB FF                          call    RuntimeComponent_HistoryMessageVia_HistoryItem___Index
        .text:009EE661 8B 4C 87 08                             mov     ecx, [edi+eax*4+8]
        .text:009EE665 83 F9 04                                cmp     ecx, 4
        .text:009EE668 72 1D                                   jb      short loc_9EE687
        .text:009EE66A 8B 46 08                                mov     eax, [esi+8]
        .text:009EE66D 03 C1                                   add     eax, ecx
        .text:009EE66F 74 16                                   jz      short loc_9EE687
        .text:009EE671 8B 74 24 14                             mov     esi, [esp+14h]
        .text:009EE675 8B CE                                   mov     ecx, esi
        .text:009EE677 E8 E4 12 00 00                          call    HistoryView__Message__displayFromName
        .text:009EE67C 84 C0                                   test    al, al
        .text:009EE67E 75 07                                   jnz     short loc_9EE687
        .text:009EE680 8B CE                                   mov     ecx, esi
        .text:009EE682 E8 B9 13 00 00                          call    HistoryView__Message__displayForwardedFrom

        E8 ?? ?? ?? ?? E8 ?? ?? ?? ?? 8B 46 08 8B 38
    */

    std::vector<uintptr_t> vResult = FindPatternInMainModule("\xE8\x00\x00\x00\x00\xE8\x00\x00\x00\x00\x8B\x46\x08\x8B\x38", "x????x????xxxxx");
    if (vResult.size() != 1) {
        spdlog::warn("[IRuntime] Search ReplyIndex failed.");
        return false;
    }

    uintptr_t ReplyIndexCaller = vResult.at(0) + 5;
    _Data.Function.ReplyIndex = (FnIndexT)(ReplyIndexCaller + 5 + *(int32_t*)(ReplyIndexCaller + 1));

    return true;

#elif defined PLATFORM_X64

    /*
        bool __fastcall HistoryMessage::updateDependencyItem()

        .text:0000000140AE10D0 48 89 5C 24 18                          mov     [rsp+arg_10], rbx
        .text:0000000140AE10D5 57                                      push    rdi
        .text:0000000140AE10D6 48 83 EC 20                             sub     rsp, 20h
        .text:0000000140AE10DA 48 8B 41 08                             mov     rax, [rcx+8]
        .text:0000000140AE10DE 48 8B F9                                mov     rdi, rcx
        .text:0000000140AE10E1 48 8B 18                                mov     rbx, [rax]
        .text:0000000140AE10E4 E8 57 A2 FF FF                          call    ?Index@?$RuntimeComponent@UHistoryMessageReply@@VHistoryItem@@@@SAHXZ ; RuntimeComponent<HistoryMessageReply,HistoryItem>::Index(void)
        .text:0000000140AE10E9 48 63 D0                                movsxd  rdx, eax
        .text:0000000140AE10EC 48 63 44 D3 10                          movsxd  rax, dword ptr [rbx+rdx*8+10h]
        .text:0000000140AE10F1 83 F8 08                                cmp     eax, 8
        .text:0000000140AE10F4 72 6B                                   jb      short loc_140AE1161
        .text:0000000140AE10F6 48 8B D8                                mov     rbx, rax
        .text:0000000140AE10F9 48 03 5F 08                             add     rbx, [rdi+8]
        .text:0000000140AE10FD 74 62                                   jz      short loc_140AE1161

        E8 ?? ?? ?? ?? 48 63 D0 48 63 44 D3 10 83 F8 08 72 6B
    */

    std::vector<uintptr_t> vResult = FindPatternInMainModule("\xE8\x00\x00\x00\x00\x48\x63\xD0\x48\x63\x44\xD3\x10\x83\xF8\x08\x72\x6B", "x????xxxxxxxxxxxxx");
    if (vResult.size() != 1) {
        spdlog::warn("[IRuntime] Search ReplyIndex failed.");
        return false;
    }

    uintptr_t ReplyIndexCaller = vResult.at(0);
    _Data.Function.ReplyIndex = (FnIndexT)(ReplyIndexCaller + 5 + *(int32_t*)(ReplyIndexCaller + 1));

    return true;

#else
# error "Unimplemented."
#endif
}

bool IRuntime::InitDynamicData_LangInstance()
{
    using namespace std::chrono_literals;

#if defined PLATFORM_X86

    /*
        Lang::Instance *__cdecl Lang::Current()

        //////////////////////////////////////////////////

        2020.1.18 - 1.9.4

        Telegram.exe+6A74BB - 8B 0D 24A95B03        - mov ecx,[Telegram.exe+31FA924]
        Telegram.exe+6A74C1 - 03 C6                 - add eax,esi
        Telegram.exe+6A74C3 - 0FB7 C0               - movzx eax,ax
        Telegram.exe+6A74C6 - 85 C9                 - test ecx,ecx
        Telegram.exe+6A74C8 - 0F84 35010000         - je Telegram.exe+6A7603
        Telegram.exe+6A74CE - 8B 49 54              - mov ecx,[ecx+54]

        (byte)
        8B 0D ?? ?? ?? ?? 03 C6 0F B7 C0 85 C9 0F 84 ?? ?? ?? ?? 8B 49

        //////////////////////////////////////////////////

        2020.6.30 - 2.1.14 beta

        Telegram.exe+193CFC - 8B 0D 68687304        - mov ecx,[Telegram.exe+3C36868] { (04D45818) }
        Telegram.exe+193D02 - 03 C6                 - add eax,esi
        Telegram.exe+193D04 - 0FB7 C0               - movzx eax,ax
        Telegram.exe+193D07 - 85 C9                 - test ecx,ecx
        Telegram.exe+193D09 - 0F84 5F010000         - je Telegram.exe+193E6E
        Telegram.exe+193D0F - 8B 89 B4020000        - mov ecx,[ecx+000002B4]

        (uint32)
        8B 0D ?? ?? ?? ?? 03 C6 0F B7 C0 85 C9 0F 84 ?? ?? ?? ?? 8B
    */

    std::vector<uintptr_t> vResult;
    uint32_t LangInsOffset;

    // ver < 2.1.14
    if (_FileVersion < 2001014)
    {
        vResult = FindPatternInMainModule("\x8B\x0D\x00\x00\x00\x00\x03\xC6\x0F\xB7\xC0\x85\xC9\x0F\x84\x00\x00\x00\x00\x8B\x49", "xx????xxxxxxxxx????xx");
        if (vResult.empty()) {
            spdlog::warn("[IRuntime] Search LangInstance failed. (old)");
            return false;
        }

        LangInsOffset = (uint32_t)(*(uint8_t*)(vResult.at(0) + 21));

        // Check each result
        //
        for (uintptr_t Address : vResult)
        {
            if ((uint32_t)(*(uint8_t*)(Address + 21)) != LangInsOffset) {
                spdlog::warn("[IRuntime] Searched LangInstance index not sure. (old)");
                return false;
            }
        }
    }
    // ver >= 2.1.14
    else if (_FileVersion >= 2001014)
    {
        vResult = FindPatternInMainModule("\x8B\x0D\x00\x00\x00\x00\x03\xC6\x0F\xB7\xC0\x85\xC9\x0F\x84\x00\x00\x00\x00\x8B", "xx????xxxxxxxxx????x");
        if (vResult.empty()) {
            spdlog::warn("[IRuntime] Search LangInstance failed. (new)");
            return false;
        }

        LangInsOffset = *(uint32_t*)(vResult.at(0) + 21);

        // Check each result
        //
        for (uintptr_t Address : vResult)
        {
            if (*(uint32_t*)(Address + 21) != LangInsOffset) {
                spdlog::warn("[IRuntime] Searched LangInstance index not sure. (new)");
                return false;
            }
        }
    }

    uintptr_t pCoreAppInstance = *(uintptr_t*)(vResult.at(0) + 2);

#elif defined PLATFORM_X64

    /*
        Telegram.exe+BEACE0 - 40 53                 - push rbx
        Telegram.exe+BEACE2 - 48 83 EC 20           - sub rsp,20 { 32 }

        // find this (Application::Instance)
        //
        Telegram.exe+BEACE6 - 48 8B 05 F34CC704     - mov rax,[Telegram.exe+585F9E0] { (23F931E6690) }

        Telegram.exe+BEACED - 48 8B D9              - mov rbx,rcx
        Telegram.exe+BEACF0 - 48 85 C0              - test rax,rax
        Telegram.exe+BEACF3 - 0F84 86000000         - je Telegram.exe+BEAD7F

        // and this (std::unique_ptr<Lang::Instance> _langpack)
        //
        Telegram.exe+BEACF9 - 48 8B 80 90060000     - mov rax,[rax+00000690]

        Telegram.exe+BEAD00 - 0FB7 D2               - movzx edx,dx
        Telegram.exe+BEAD03 - 48 8B 48 70           - mov rcx,[rax+70]
        Telegram.exe+BEAD07 - 48 8B 40 78           - mov rax,[rax+78]
        Telegram.exe+BEAD0B - 48 2B C1              - sub rax,rcx
        Telegram.exe+BEAD0E - 48 C1 F8 03           - sar rax,03 { 3 }
        Telegram.exe+BEAD12 - 48 3B D0              - cmp rdx,rax
        Telegram.exe+BEAD15 - 73 41                 - jae Telegram.exe+BEAD58
        Telegram.exe+BEAD17 - 48 8B 04 D1           - mov rax,[rcx+rdx*8]
        Telegram.exe+BEAD1B - 48 8D 0C D1           - lea rcx,[rcx+rdx*8]
        Telegram.exe+BEAD1F - 48 89 03              - mov [rbx],rax
        Telegram.exe+BEAD22 - 48 3B CB              - cmp rcx,rbx

        48 8B 05 ?? ?? ?? ?? 48 8B D9 48 85 C0 0F 84 ?? ?? ?? ?? 48 8B 80
    */

    std::vector<uintptr_t> vResult = FindPatternInMainModule("\x48\x8B\x05\x00\x00\x00\x00\x48\x8B\xD9\x48\x85\xC0\x0F\x84\x00\x00\x00\x00\x48\x8B\x80", "xxx????xxxxxxxx????xxx");
    if (vResult.size() != 1) {
        spdlog::warn("[IRuntime] Search LangInstance failed. (new x64)");
        return false;
    }

    uintptr_t pCoreAppInstance = vResult.at(0) + 7 + *(int32_t*)(vResult.at(0) + 3);
    uint32_t LangInsOffset = *(uint32_t*)(vResult.at(0) + 22);

#else
# error "Unimplemented."
#endif

    uintptr_t CoreAppInstance = NULL;
    for (size_t i = 0; i < 20; ++i)
    {
        CoreAppInstance = *(uintptr_t*)pCoreAppInstance;
        if (CoreAppInstance != NULL) {
            break;
        }
        std::this_thread::sleep_for(1s);
    }

    if (CoreAppInstance == NULL) {
        spdlog::warn("[IRuntime] CoreAppInstance always nullptr.");
        return false;
    }

    _Data.Address.pLangInstance = *(LanguageInstance**)(CoreAppInstance + LangInsOffset);

    if (_Data.Address.pLangInstance == NULL) {
        spdlog::warn("[IRuntime] Searched pLangInstance is null.");
        return false;
    }

    return true;
}

bool IRuntime::InitDynamicData_ToHistoryMessage()
{
#if defined PLATFORM_X86

    /*
        Telegram.exe+724503 - 8B 49 20              - mov ecx,[ecx+20]
        Telegram.exe+724506 - 85 C9                 - test ecx,ecx
        Telegram.exe+724508 - 0F84 F2000000         - je Telegram.exe+724600
        Telegram.exe+72450E - 8B 01                 - mov eax,[ecx]
        Telegram.exe+724510 - FF 90 D8000000        - call dword ptr [eax+000000D8]
        Telegram.exe+724516 - 85 C0                 - test eax,eax

        8B 49 ?? 85 C9 0F 84 ?? ?? ?? ?? 8B 01 FF 90 ?? ?? ?? ?? 85 C0
    */

    std::vector<uintptr_t> vResult = FindPatternInMainModule("\x8B\x49\x00\x85\xC9\x0F\x84\x00\x00\x00\x00\x8B\x01\xFF\x90\x00\x00\x00\x00\x85\xC0", "xx?xxxx????xxxx????xx");
    if (vResult.empty()) {
        spdlog::warn("[IRuntime] Search toHistoryMessage index falied.");
        return false;
    }

    uint32_t Offset = *(uint32_t*)(vResult.at(0) + 15);

    if (Offset % sizeof(void*) != 0) {
        spdlog::warn("[IRuntime] Searched toHistoryMessage index invalid.");
        return false;
    }

    // Check each offset
    //
    for (uintptr_t Address : vResult)
    {
        if (*(uint32_t*)(Address + 15) != Offset) {
            spdlog::warn("[IRuntime] Searched toHistoryMessage index not sure.");
            return false;
        }
    }

#elif defined PLATFORM_X64

    /*
        std::optional<enum Storage::SharedMediaType> Media::View::OverlayWidget::sharedMediaType(void)const
    
        .text:0000000140D35693 48 83 7F 50 00                          cmp     qword ptr [rdi+50h], 0
        .text:0000000140D35698 74 2B                                   jz      short loc_140D356C5
        .text:0000000140D3569A 48 8B 06                                mov     rax, [rsi]
        .text:0000000140D3569D 48 8B CE                                mov     rcx, rsi

        // find this
        //
        .text:0000000140D356A0 FF 90 C8 01 00 00                       call    qword ptr [rax+1C8h]

        .text:0000000140D356A6 C6 43 01 01                             mov     byte ptr [rbx+1], 1
        .text:0000000140D356AA 48 85 C0                                test    rax, rax
        .text:0000000140D356AD 75 51                                   jnz     short loc_140D35700
        .text:0000000140D356AF C6 03 07                                mov     byte ptr [rbx], 7
        .text:0000000140D356B2 48 8B C3                                mov     rax, rbx
        .text:0000000140D356B5 48 8B 5C 24 40                          mov     rbx, [rsp+38h+arg_0]
        .text:0000000140D356BA 48 8B 74 24 48                          mov     rsi, [rsp+38h+arg_8]
        .text:0000000140D356BF 48 83 C4 30                             add     rsp, 30h
        .text:0000000140D356C3 5F                                      pop     rdi
        .text:0000000140D356C4 C3                                      retn

        FF 90 ?? ?? ?? ?? C6 43 01 01
    */

    std::vector<uintptr_t> vResult = FindPatternInMainModule("\xFF\x90\x00\x00\x00\x00\xC6\x43\x01\x01", "xx????xxxx");
    if (vResult.size() != 1) {
        spdlog::warn("[IRuntime] Search toHistoryMessage index falied.");
        return false;
    }

    uint32_t Offset = *(uint32_t*)(vResult.at(0) + 2);

#else
# error "Unimplemented."
#endif

    _Data.Index.ToHistoryMessage = (Offset / sizeof(void*)) - 1 /* Start from 0 */;

    return true;
}
