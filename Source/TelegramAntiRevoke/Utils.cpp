#include "Utils.h"

using namespace std;


namespace Text
{
	string			ToLowerA(const string &String)
	{
		string Result = String;
		transform(Result.begin(), Result.end(), Result.begin(), tolower);
		return Result;
	}

	string			SubReplaceA(const string &Source, const string &Target, const string &New)
	{
		string Result = Source;
		while (true)
		{
			SIZE_T Pos = Result.find(Target);
			if (Pos == string::npos) {
				break;
			}
			Result.replace(Pos, Target.size(), New);
		}
		return Result;
	}

	vector<string>	SplitByFlagA(const string &Source, const string &Flag)
	{
		vector<string> Result;
		SIZE_T BeginPos = 0, EndPos = Source.find(Flag);

		while (EndPos != string::npos)
		{
			Result.push_back(Source.substr(BeginPos, EndPos - BeginPos));

			BeginPos = EndPos + Flag.size();
			EndPos = Source.find(Flag, BeginPos);
		}

		if (BeginPos != Source.length()) {
			Result.push_back(Source.substr(BeginPos));
		}

		return Result;
	}

	string			StringFormatA(const CHAR *Format, ...)
	{
		va_list VaList;
		CHAR Buffer[0x200] = { 0 };

		va_start(VaList, Format);
		vsprintf_s(Buffer, Format, VaList);
		va_end(VaList);

		return string(Buffer);
	}
}

namespace Convert
{
	string			UnicodeToAnsi(const wstring &UnicodeString)
	{
		INT Length = WideCharToMultiByte(CP_ACP, 0, UnicodeString.c_str(), -1, NULL, 0, NULL, NULL);

		CHAR *AnsiBuffer = (CHAR*)malloc((Length + 1));
		RtlZeroMemory(AnsiBuffer, Length + 1);

		Length = WideCharToMultiByte(CP_ACP, 0, UnicodeString.c_str(), -1, AnsiBuffer, Length, NULL, NULL);

		string Out = string(AnsiBuffer);
		free(AnsiBuffer);

		return Out;
	}
}

namespace Internet
{
	string			RequestGetA(const string &HostName, INTERNET_PORT Port, const string &ObjectName, const string &AdditionalHeader)
	{
#define ONCE_READ_SIZE	( 0x100 )

		HINTERNET hInternet = NULL, hConnect = NULL, hRequest = NULL;
		string Buffer;

		// 打开句柄
		hInternet = InternetOpenA("Mozilla/4.0 (compatible; MSIE 6.0; Windows NT 5.0)", INTERNET_OPEN_TYPE_DIRECT, NULL, NULL, 0);
		if (hInternet == NULL) {
			goto EXIT;
		}

		// 连接主机
		hConnect = InternetConnectA(hInternet, HostName.c_str(), Port, NULL, NULL, INTERNET_SERVICE_HTTP, 0, 0);
		if (hConnect == NULL) {
			goto EXIT;
		}

		// 创建请求
		hRequest = HttpOpenRequestA(hConnect, "GET", ObjectName.c_str(), "HTTP/1.1", NULL, NULL, INTERNET_FLAG_RELOAD, 0);
		if (hRequest == NULL) {
			goto EXIT;
		}

		// 发送请求
		if (!HttpSendRequestA(hRequest, AdditionalHeader.c_str(), (DWORD)AdditionalHeader.size(), NULL, 0)) {
			goto EXIT;
		}

		// 读取数据
		while (true)
		{
			DWORD BytesRead = 0;
			CHAR TempBuffer[ONCE_READ_SIZE + 1];
			RtlZeroMemory(TempBuffer, sizeof(TempBuffer));

			InternetReadFile(hRequest, TempBuffer, ONCE_READ_SIZE, &BytesRead);
			if (BytesRead == 0) {
				break;
			}

			TempBuffer[BytesRead] = '\0';
			Buffer += TempBuffer;
		}

	EXIT:
		if (hRequest != NULL) {
			InternetCloseHandle(hRequest);
		}
		if (hConnect != NULL) {
			InternetCloseHandle(hConnect);
		}
		if (hInternet != NULL) {
			InternetCloseHandle(hInternet);
		}

		return Buffer;
	}
}

namespace Safe
{
	void			Mutex(HANDLE hMutex, std::function<void()> Callback)
	{
		WaitForSingleObject(hMutex, INFINITE);
		Callback();
		ReleaseMutex(hMutex);
	}
}

namespace Process
{
	string			GetCurrentName()
	{
		CHAR Buffer[MAX_PATH] = { 0 };
		if (GetModuleFileNameA(NULL, Buffer, MAX_PATH) == 0) {
			return "";
		}

		string FileName = Buffer;
		SIZE_T Position = FileName.rfind("\\");
		if (Position == string::npos) {
			return "";
		}

		Position++;
		return FileName.substr(Position, FileName.size() - Position);
	}
}

namespace Memory
{
	void			ReadProcess(HANDLE hProcess, PVOID TargetAddress, PVOID LocalBuffer, SIZE_T Size)
	{
		SIZE_T Bytes;
		ReadProcessMemory(hProcess, TargetAddress, LocalBuffer, Size, &Bytes);
	}

	vector<PVOID>	FindPatternEx(HANDLE hProcess, PVOID StartAddress, SIZE_T SearchSize, const CHAR Pattern[], const CHAR Mask[], DWORD Protect)
	{
		vector<PVOID> Result;

		if (StartAddress == NULL || SearchSize == 0 || Pattern == NULL || Mask == NULL) {
			return Result;
		}

		// 处理 Mask 头部通配情况
		SIZE_T Header = 0;
		while (Mask[Header] == '?') {
			Header++;
		}
		Pattern += Header;
		Mask += Header;

		PVOID QueryAddress = StartAddress;
		PVOID EndAddress = (PVOID)((ULONG_PTR)StartAddress + SearchSize);

		while (true)
		{
			MEMORY_BASIC_INFORMATION Mbi;
			if (VirtualQueryEx(hProcess, QueryAddress, &Mbi, sizeof(MEMORY_BASIC_INFORMATION)) != sizeof(MEMORY_BASIC_INFORMATION)) {
				goto Next;
			}
			if (Mbi.RegionSize == 0) {
				break;
			}
			if (Mbi.State != MEM_COMMIT || Mbi.Protect != Protect) {
				goto Next;
			}


			for (ULONG_PTR MatchAddress = (ULONG_PTR)QueryAddress; MatchAddress < (ULONG_PTR)QueryAddress + Mbi.RegionSize; MatchAddress += PAGE_SIZE)
			{
				CHAR PageData[PAGE_SIZE];

				SIZE_T RamainSize = ((ULONG_PTR)EndAddress - MatchAddress);
				SIZE_T PageSize = RamainSize < PAGE_SIZE ? RamainSize : PAGE_SIZE;
				if (PageSize == 0) {
					return Result;
				}

				ReadProcess(hProcess, (PVOID)MatchAddress, PageData, PageSize);

				for (SIZE_T InSign = 0, InPage = 0; InPage < PageSize; InPage++)
				{
					// 对比字节
					if (PageData[InPage] == Pattern[InSign] || Mask[InSign] == '?')
					{
						// 判断是否到字节尾
						if (Mask[InSign + 1] == '\0')
						{
							// 表示定位到符合的特征码地址 push头地址
							Result.push_back((PVOID)(MatchAddress + InPage - InSign - Header));
							goto ROLLBACK;
						}
						else {
							// 特征字节下标递增
							InSign++;
						}
					}
					else
					{
					ROLLBACK:
						InPage -= InSign;
						InSign = 0;
					}

					if ((PVOID)(MatchAddress + InPage) >= EndAddress) {
						return Result;
					}
				}

				// 地址递增 读下一页
			}

		Next:
			QueryAddress = (PVOID)((ULONG_PTR)QueryAddress + Mbi.RegionSize);
			if (QueryAddress >= EndAddress) {
				break;
			}
		}

		return Result;
	}

	BOOLEAN			ForceOperate(PVOID Address, SIZE_T Size, const function<void()> &fnOperateCallback)
	{
		ULONG SaveProtect;

		if (!VirtualProtect(Address, Size, PAGE_EXECUTE_READWRITE, &SaveProtect)) {
			return FALSE;
		}

		fnOperateCallback();

		return VirtualProtect(Address, Size, SaveProtect, &SaveProtect);
	}

	vector<BYTE>	MakeCall(PVOID HookAddress, PVOID CallAddress)
	{
		vector<BYTE> MakeCode = { 0xE8, 0x00, 0x00, 0x00, 0x00 };
		*(ULONG*)(MakeCode.data() + 1) = (ULONG)((ULONG_PTR)CallAddress - (ULONG_PTR)HookAddress - 5);
		return MakeCode;
	}

	vector<BYTE>	MakeJmp(PVOID HookAddress, PVOID JmpAddress)
	{
		vector<BYTE> MakeCode = { 0xE9, 0x00, 0x00, 0x00, 0x00 };
		*(ULONG*)(MakeCode.data() + 1) = (ULONG)((ULONG_PTR)JmpAddress - (ULONG_PTR)HookAddress - 5);
		return MakeCode;
	}
}

namespace Utils
{
	void			CreateConsole()
	{
		FILE *hStream = NULL;
		AllocConsole();
		freopen_s(&hStream, "CONOUT$", "w", stdout);
	}
}