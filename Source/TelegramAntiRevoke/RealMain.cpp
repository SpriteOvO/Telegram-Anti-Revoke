#include "Header.h"
#include "Telegram.h"
#include "Global.h"
#include "AntiRevoke.h"


namespace g
{
	LoggerManager Logger;
	ULONG_PTR MainModule = NULL;

	fntMalloc fnMalloc = NULL;
	fntFree fnFree = NULL;
	fntFree fnOriginalFree = NULL;
	fntGetEditedIndex fnGetEditedIndex = NULL;
	fntGetCurrentInstance fnGetCurrentInstance = NULL;
	PVOID RevokeByServer = NULL;
	PVOID OriginalRevoke = NULL;

	HANDLE hMutex = 0;
	vector<HistoryMessage*> RevokedMessages;

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
				{ L"Traditional", L"已刪除 ", 7 * 6 }
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
	PVOID			HookAddress = g::RevokeByServer;
	PVOID			TargetAddress = NULL;
	vector<BYTE>	Shellcode;

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
	if (Status)
	{
		// Enable Hook
		if (MH_CreateHook(g::fnFree, FakeFree, (PVOID*)&g::fnOriginalFree) != MH_OK) {
			return FALSE;
		}

		return MH_EnableHook(g::fnFree) == MH_OK;
	}
	else
	{
		// Restore hook
		return MH_DisableHook(g::fnFree) == MH_OK;
	}
}

void InitMarkLanguage()
{
	SafeExcept([]()
	{
		LanguageInstance *Instance = g::fnGetCurrentInstance();
		if (Instance == NULL) {
			g::Logger.TraceWarn("Get language instance failed.");
			return;
		}

		//printf("Instance: %p\n", Instance);
		//printf("GetId         : %ws\n", Instance->GetId()->GetText());
		//printf("GetPluralId   : %ws\n", Instance->GetPluralId()->GetText());
		//printf("GetName       : %ws\n", Instance->GetName()->GetText());
		//printf("GetNativeName : %ws\n", Instance->GetNativeName()->GetText());

		wstring CurrentPluralId = Instance->GetPluralId()->GetText();
		wstring CurrentName = Instance->GetName()->GetText();
		map<wstring, vector<MARK_INFO>>::iterator Iterator;

		// find language
		if (!MAP_FIND(Iterator, g::MultiLanguageMarks, CurrentPluralId)) {
			g::Logger.TraceWarn(string("An unadded language. PluralId: [") + Convert::UnicodeToAnsi(CurrentPluralId + wstring(L"] Name: [") + CurrentName) + string("]"));
			return;
		}

		vector<MARK_INFO> Sublanguages = g::MultiLanguageMarks[CurrentPluralId];

		// default sublanguage
		g::CurrentMark = Sublanguages[0];

		// multiple sublanguages
		if (Sublanguages.size() > 1)
		{
			for (MARK_INFO Language : Sublanguages)
			{
				if (CurrentName.find(Language.LangName) != wstring::npos) {
					// found sub language
					g::CurrentMark = Language;
					break;
				}
			}
		}

	}, [](ULONG ExceptionCode)
	{
		g::Logger.TraceWarn("Function: [" __FUNCTION__ "] An exception was caught. Code: [" + Text::StringFormatA("0x%x", ExceptionCode) + "]");
	});
}

BOOLEAN SearchSigns()
{
	// 以下的所有汇编取自 1.8.8 版本
	// Thanks to [采蘑菇的小蘑菇] for providing help with compiling Telegram.

	MODULEINFO MainModuleInfo = { 0 };
	if (!GetModuleInformation(GetCurrentProcess(), (HMODULE)g::MainModule, &MainModuleInfo, sizeof(MainModuleInfo))) {
		return FALSE;
	}

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
		.text:01B7CAE3 E8 7F 45 00 00                          call    _free

		.text:01B7CAE8 59                                      pop     ecx
		.text:01B7CAE9 5E                                      pop     esi
		.text:01B7CAEA 5B                                      pop     ebx
		.text:01B7CAEB EB 0B                                   jmp     short loc_1B7CAF8

		malloc		41 84 C0 75 F9 2B CA 53 56 8D 59 01 53 E8
		free		56 E8 ?? ?? ?? ?? 59 5E 5B EB
	*/
	vector<PVOID> vCallMalloc = Memory::FindPatternEx(GetCurrentProcess(), (PVOID)g::MainModule, MainModuleInfo.SizeOfImage, "\x41\x84\xC0\x75\xF9\x2B\xCA\x53\x56\x8D\x59\x01\x53\xE8", "xxxxxxxxxxxxxx");
	if (vCallMalloc.size() != 1) {
		g::Logger.TraceWarn("Search malloc falied.");
		return FALSE;
	}

	vector<PVOID> vCallFree = Memory::FindPatternEx(GetCurrentProcess(), vCallMalloc[0], 0x50, "\x56\xE8\x00\x00\x00\x00\x59\x5E\x5B\xEB", "xx????xxxx");
	if (vCallFree.size() != 1) {
		g::Logger.TraceWarn("Search free falied.");
		return FALSE;
	}

	ULONG_PTR CallMalloc = (ULONG_PTR)vCallMalloc[0];
	ULONG_PTR CallFree = (ULONG_PTR)vCallFree[0];

	g::fnMalloc = (fntMalloc)(CallMalloc + 18 + *(INT*)(CallMalloc + 14));
	g::fnFree = (fntFree)(CallFree + 6 + *(INT*)(CallFree + 2));


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
	*/
	vector<PVOID> vCallDestroyMessage = Memory::FindPatternEx(GetCurrentProcess(), (PVOID)g::MainModule, MainModuleInfo.SizeOfImage, "\x8B\x71\x00\x89\x08\x85\xC9\x0F\x84\x00\x00\x00\x00\x00\x00\x00\xE8", "xx?xxxxxx???????x");
	if (vCallDestroyMessage.size() != 1) {
		g::Logger.TraceWarn("Search DestroyMessage falied.");
		return FALSE;
	}

	ULONG_PTR CallDestroyMessage = (ULONG_PTR)vCallDestroyMessage[0];
	g::RevokeByServer = (PVOID)(CallDestroyMessage + 16);

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
		.text:00A4F36D E8 6E 3A EA FF                          call    ?Index@?$RuntimeComponent@UHistoryMessageEdited@@VHistoryItem@@@@SAHXZ ; RuntimeComponent<HistoryMessageEdited,HistoryItem>::Index(void)

		.text:00A4F372 83 7C 87 08 04                          cmp     dword ptr [edi+eax*4+8], 4
		.text:00A4F377 73 28                                   jnb     short loc_A4F3A1
		.text:00A4F379 E8 62 3A EA FF                          call    ?Index@?$RuntimeComponent@UHistoryMessageEdited@@VHistoryItem@@@@SAHXZ ; RuntimeComponent<HistoryMessageEdited,HistoryItem>::Index(void)
		.text:00A4F37E 33 D2                                   xor     edx, edx

		E8 ?? ?? ?? ?? 83 7C 87 ?? ?? 73 ?? E8
	*/
	vector<PVOID> vCallApplyEdition = Memory::FindPatternEx(GetCurrentProcess(), (PVOID)g::MainModule, MainModuleInfo.SizeOfImage, "\xE8\x00\x00\x00\x00\x83\x7C\x87\x00\x00\x73\x00\xE8", "x????xxx??x?x");
	if (vCallApplyEdition.size() != 1) {
		g::Logger.TraceWarn("Search ApplyEdition falied.");
		return FALSE;
	}

	ULONG_PTR CallApplyEdition = (ULONG_PTR)vCallApplyEdition[0];
	g::fnGetEditedIndex = (fntGetEditedIndex)(CallApplyEdition + 5 + *(INT*)(CallApplyEdition + 1));


	/*
		Lang::Instance *__cdecl Lang::Current()

		.text:00B6FBD0 51                                      push    ecx
		.text:00B6FBD1 A1 44 1F 3B 03                          mov     eax, ?Instance@Application@Core@@0PAV12@A.ptr_ ; Core::Application * Core::Application::Instance
		.text:00B6FBD6 85 C0                                   test    eax, eax
		.text:00B6FBD8 74 05                                   jz      short loc_B6FBDF
		.text:00B6FBDA 8B 40 54                                mov     eax, [eax+54h]
		.text:00B6FBDD 59                                      pop     ecx
		.text:00B6FBDE C3                                      retn
		.text:00B6FBDF                         ; ---------------------------------------------------------------------------
		.text:00B6FBDF
		.text:00B6FBDF                         loc_B6FBDF:                             ; CODE XREF: Lang::Current(void)+8↑j
		.text:00B6FBDF 68 58 03 00 00                          push    358h            ; line
		.text:00B6FBE4 BA 49 00 00 00                          mov     edx, 49h        ; size
		.text:00B6FBE9 B9 B0 EB C7 02                          mov     ecx, offset aDMyprojectTele_25 ; "D:\\MyProject\\Telegram\\tdesktop\\Tele"...
		.text:00B6FBEE E8 2D F7 9C FF                          call    ?extract_basename@assertion@base@@YAPBDPBDI@Z ; base::assertion::extract_basename(char const *,uint)
		.text:00B6FBF3 8B D0                                   mov     edx, eax        ; file
		.text:00B6FBF5 B9 00 ED C7 02                          mov     ecx, offset aApplicationIns ; "\"Application::Instance != nullptr\""
		.text:00B6FBFA E8 41 F7 9C FF                          call    ?fail@assertion@base@@YAXPBD0H@Z ; base::assertion::fail(char const *,char const *,int)

		51 A1 ?? ?? ?? ?? 85 C0 74 05 8B 40 ?? 59 C3
	*/
	vector<PVOID> vCallCurrent = Memory::FindPatternEx(GetCurrentProcess(), (PVOID)g::MainModule, MainModuleInfo.SizeOfImage, "\x51\xA1\x00\x00\x00\x00\x85\xC0\x74\x05\x8B\x40\x00\x59\xC3", "xx????xxxxxx?xx");
	if (vCallCurrent.size() != 1) {
		g::Logger.TraceWarn("Search GetCurrentInstance falied.");
		return FALSE;
	}

	g::fnGetCurrentInstance = (fntGetCurrentInstance)vCallCurrent[0];


	//printf("Call_Malloc: %p\n", (PVOID)Call_Malloc);
	//printf("Call_Free  : %p\n", (PVOID)Call_Free);
	//printf("fnMalloc   : %p\n", g::fnMalloc);
	//printf("fnFree     : %p\n", g::fnFree);
	//printf("RevokeByServer : %p\n", g::RevokeByServer);
	//printf("fnGetIndex : %p\n", g::fnGetIndex);
	//printf("fnGetValue : %p\n", g::fnGetLanguageValue);
	//printf("---------------\n");

	return TRUE;
}

void CheckUpdate()
{
	string LatestData = Internet::RequestGetA("api.github.com", 80, AR_URL_RELEASE);
	if (LatestData.empty()) {
		g::Logger.TraceWarn("Check Update failed. LatestData is empty.");
		return;
	}

	Json::CharReaderBuilder Builder;
	Json::Value Root;
	Json::String Errors;
	unique_ptr<Json::CharReader> pReader(Builder.newCharReader());

	if (!pReader->parse(LatestData.c_str(), LatestData.c_str() + LatestData.size(), &Root, &Errors)) {
		g::Logger.TraceWarn("Check Update failed. Reader->parse() failed. { " + Errors + " }");
		return;
	}

	string Message = Root["message"].asString();
	string LatestVersion = Root["tag_name"].asString();	// like "1.3.0"
	string LatestUrl = Root["html_url"].asString();

	if (!Message.empty() || LatestVersion.empty() || LatestUrl.empty()) {
		// if falied, message is "Not Found".
		g::Logger.TraceWarn("Check Update failed. message or tag_name or html_url is bad. { message: [" + Message + "] tag_name: [" + LatestVersion + "] html_url: [" + LatestUrl + "] }");
		return;
	}


	vector<string> vLocal = Text::SplitByFlagA(AR_VERSION, ".");
	vector<string> vLatest = Text::SplitByFlagA(LatestVersion, ".");
	if (vLocal.size() != 3 || vLatest.size() != 3) {
		g::Logger.TraceWarn("Update check failed. Vector size is bad. Local: " + string(AR_VERSION) + " Latest: " + LatestVersion);
		return;
	}

	// 将 1.21.3 格式化为 001021003，然后转为整数，对比版本
	string LocalString = Text::StringFormatA("%03d%03d%03d", stoul(vLocal[0]), stoul(vLocal[1]), stoul(vLocal[2]));
	string LatestString = Text::StringFormatA("%03d%03d%03d", stoul(vLatest[0]), stoul(vLatest[1]), stoul(vLatest[2]));
	ULONG LocalNumber = stoul(LocalString);
	ULONG LatestNumber = stoul(LatestString);

	if (LocalNumber >= LatestNumber) {
		// no need update.
		g::Logger.TraceInfo("No need update. Local: [" + LocalString + "] Latest: [" + LatestString + "]");
		return;
	}

	g::Logger.TraceInfo("Need update. Local: [" + LocalString + "] Latest: [" + LatestString + "]");

	/*
		A new version has been released.

		Current version: 1.2.3
		Latest version: 1.3.0

		Do you want to go to GitHub to download the latest version?
	*/
	string Msg = 
		"A new version has been released.\n"
		"\n"
		"Current version: " + string(AR_VERSION) + "\n"
		"Latest version: " + LatestVersion + "\n"
		"\n"
		"Do you want to go to GitHub to download the latest version?\n";

	if (MessageBoxA(NULL, Msg.c_str(), "Anti-Revoke Plugin", MB_ICONQUESTION | MB_YESNO) == IDYES) {
		system(("start " + LatestUrl).c_str());
	}
}

DWORD WINAPI Initialize(PVOID pParameter)
{
	g::MainModule = (ULONG_PTR)GetModuleHandle(L"Telegram.exe");
	g::hMutex = CreateMutex(NULL, FALSE, NULL);

	CheckUpdate();

	if (!SearchSigns()) {
		g::Logger.TraceError("SearchSigns() failed.");
		return 0;
	}

	MH_STATUS Status = MH_Initialize();
	if (Status != MH_OK) {
		g::Logger.TraceError(string("MH_Initialize() failed.\n") + MH_StatusToString(Status));
		return 0;
	}

	InitMarkLanguage();

	if (!HookMemoryFree(TRUE)) {
		g::Logger.TraceError("HookMemoryFree() failed.");
		return 0;
	}

	if (!HookRevoke(TRUE)) {
		g::Logger.TraceError("HookRevoke() failed.");
		return 0;
	}

	ProcessItems();

	return 0;
}

BOOLEAN CheckProcess()
{
	string CurrentName = Process::GetCurrentName();
	if (Text::ToLowerA(CurrentName) != "telegram.exe") {
		g::Logger.TraceError("This is not a Telegram process. [" + CurrentName + "]", FALSE);
		return FALSE;
	}
	return TRUE;
}

BOOL WINAPI RealDllMain(HINSTANCE hModule, DWORD dwReason, PVOID pReserved)
{
	switch (dwReason)
	{
	case DLL_PROCESS_ATTACH:
		DisableThreadLibraryCalls(hModule);
		// Utils::CreateConsole();

		if (!CheckProcess()) {
			return FALSE;
		}

		CloseHandle(CreateThread(NULL, 0, Initialize, NULL, 0, NULL));

		break;
	}

	return TRUE;
}
