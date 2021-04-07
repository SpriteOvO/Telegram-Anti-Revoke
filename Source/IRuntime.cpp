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
        _Data.Offset.Media = 0x54;
        _Data.Offset.SignedTimeText = 0x14;
        _Data.Offset.HistoryPeer = 0x7C;
    }
    // ver >= 2.4.1, ver < 2.6.0
    else if (_FileVersion >= 2004001 && _FileVersion < 2006000)
    {
        _Data.Offset.TimeText = 0x70;
        _Data.Offset.TimeWidth = 0x74;
        _Data.Offset.MainView = 0x5C;
        _Data.Offset.Media = 0x54;
        _Data.Offset.SignedTimeText = 0x10;    // changed
        _Data.Offset.HistoryPeer = 0x7C;       // maybe untested! (I forgot :)
    }
    // ver >= 2.6.0
    else if (_FileVersion >= 2006000)
    {
        _Data.Offset.TimeText = 0x78;          // changed
        _Data.Offset.TimeWidth = 0x7C;         // changed
        _Data.Offset.MainView = 0x60;          // changed
        _Data.Offset.Media = 0x5C;             // changed
        _Data.Offset.SignedTimeText = 0x10;
        _Data.Offset.HistoryPeer = 0x7C;       // untested!
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
}

bool IRuntime::InitDynamicData_DestroyMessage()
{
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
}

bool IRuntime::InitDynamicData_EditedIndex()
{
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
}

bool IRuntime::InitDynamicData_SignedIndex()
{
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
}

bool IRuntime::InitDynamicData_ReplyIndex()
{
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
}

bool IRuntime::InitDynamicData_LangInstance()
{
    using namespace std::chrono_literals;

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

    _Data.Index.ToHistoryMessage = (Offset / sizeof(void*)) - 1 /* Start from 0 */;

    return true;
}
