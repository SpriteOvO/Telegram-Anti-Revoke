#include "Utils.h"

using namespace std;

namespace File
{
	string GetCurrentFullNameA()
	{
		CHAR Buffer[MAX_PATH] = { 0 };
		if (GetModuleFileNameA(NULL, Buffer, MAX_PATH) == 0) {
			return "";
		}

		return string(Buffer);
	}

	string GetCurrentName()
	{
		string FullName = File::GetCurrentFullNameA();
		if (FullName.empty()) {
			return "";
		}

		SIZE_T Position = FullName.rfind("\\");
		if (Position == string::npos) {
			return "";
		}

		Position++;
		return FullName.substr(Position, FullName.size() - Position);
	}

	ULONG GetCurrentVersion()
	{
		string FullName = GetCurrentFullNameA();

		DWORD InfoSize = GetFileVersionInfoSizeA(FullName.c_str(), NULL);
		if (!InfoSize) {
			return 0;
		}

		unique_ptr<CHAR[]> Buffer(new CHAR[InfoSize]);
		if (!GetFileVersionInfoA(FullName.c_str(), 0, InfoSize, Buffer.get())) {
			return 0;
		}

		VS_FIXEDFILEINFO *pVsInfo = NULL;
		UINT VsInfoSize = sizeof(VS_FIXEDFILEINFO);
		if (!VerQueryValueA(Buffer.get(), "\\", (PVOID*)&pVsInfo, &VsInfoSize)) {
			return 0;
		}

		string TelegramVersion = Text::Format("%03hu%03hu%03hu", HIWORD(pVsInfo->dwFileVersionMS), LOWORD(pVsInfo->dwFileVersionMS), HIWORD(pVsInfo->dwFileVersionLS));
		return stoul(TelegramVersion);
	}

} // namespace File

namespace Text
{
	string ToLower(const string &String)
	{
		string Result = String;
		transform(Result.begin(), Result.end(), Result.begin(), tolower);
		return Result;
	}

	string SubReplace(const string &Source, const string &Target, const string &New)
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

	vector<string> SplitByFlag(const string &Source, const string &Flag)
	{
		vector<string> Result;
		SIZE_T BeginPos = 0, EndPos = Source.find(Flag);

		while (EndPos != string::npos)
		{
			Result.emplace_back(Source.substr(BeginPos, EndPos - BeginPos));

			BeginPos = EndPos + Flag.size();
			EndPos = Source.find(Flag, BeginPos);
		}

		if (BeginPos != Source.length()) {
			Result.emplace_back(Source.substr(BeginPos));
		}

		return Result;
	}

	string Format(const CHAR *Format, ...)
	{
		va_list VaList;
		CHAR Buffer[0x200] = { 0 };

		va_start(VaList, Format);
		vsprintf_s(Buffer, Format, VaList);
		va_end(VaList);

		return string(Buffer);
	}

} // namespace Text

namespace Convert
{
	string UnicodeToAnsi(const wstring &String)
	{
		string Result;
		INT Length = WideCharToMultiByte(CP_ACP, 0, String.c_str(), (INT)String.length(), NULL, 0, NULL, NULL);
		Result.resize(Length);
		WideCharToMultiByte(CP_ACP, 0, String.c_str(), (INT)String.length(), (CHAR*)Result.data(), Length, NULL, NULL);
		return Result;
	}

} // namespace Convert

namespace Internet
{
	BOOLEAN HttpRequest(string &Response, ULONG &Status, const string &HttpVerb, const string &HostName, const string &ObjectName, const vector<pair<string, string>> &Headers, const string &PostData)
	{
		if (HttpVerb != "GET" && HttpVerb != "POST") {
			return FALSE;
		}

		if (HttpVerb != "POST" && !PostData.empty()) {
			return FALSE;
		}

		BOOLEAN Result = FALSE;
		HINTERNET hInternet = NULL, hConnect = NULL, hRequest = NULL;

		do
		{
			hInternet = InternetOpenA("Mozilla/4.0 (compatible; MSIE 6.0; Windows NT 5.0)", INTERNET_OPEN_TYPE_PRECONFIG, NULL, NULL, 0);
			if (hInternet == NULL) {
				break;
			}

			hConnect = InternetConnectA(hInternet, HostName.c_str(), INTERNET_DEFAULT_HTTPS_PORT, NULL, NULL, INTERNET_SERVICE_HTTP, 0, 0);
			if (hConnect == NULL) {
				break;
			}

			hRequest = HttpOpenRequestA(hConnect, HttpVerb.c_str(), ObjectName.c_str(), "HTTP/1.1", NULL, NULL, INTERNET_FLAG_NO_CACHE_WRITE | INTERNET_FLAG_RELOAD | INTERNET_FLAG_SECURE, 0);
			if (hRequest == NULL) {
				break;
			}

			string HeadersText;
			for (const auto &[HeaderName, HeaderValue] : Headers) {
				HeadersText += HeaderName + ": " + HeaderValue + "\r\n";
			}

			if (!HttpSendRequestA(hRequest, HeadersText.c_str(), -1, (PVOID)PostData.c_str(), (ULONG)PostData.size())) {
				break;
			}

			ULONG StatusSize = sizeof(Status), Index = 0;
			if (!HttpQueryInfoA(hRequest, HTTP_QUERY_STATUS_CODE | HTTP_QUERY_FLAG_NUMBER, &Status, &StatusSize, &Index)) {
				break;
			}

			Response.clear();

			while (TRUE)
			{
#define ONCE_READ_SIZE	( 0x100 )
				ULONG BytesRead = 0;
				CHAR TempBuffer[ONCE_READ_SIZE + 1];
				RtlZeroMemory(TempBuffer, sizeof(TempBuffer));

				if (!InternetReadFile(hRequest, TempBuffer, ONCE_READ_SIZE, &BytesRead)) {
					goto EXIT;
				}

				if (BytesRead == 0) {
					break;
				}

				// We must fill a zero ending here.
				// Because the `InternetReadFile` API will write more data than `BytesRead` if the buffer is large enough!
				TempBuffer[BytesRead] = '\0';

				Response += TempBuffer;
#undef ONCE_READ_SIZE
			}

			Result = TRUE;

		} while (FALSE);


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

		return Result;
	}

} // namespace Internet

namespace Memory
{
	void ReadProcess(HANDLE hProcess, PVOID TargetAddress, PVOID LocalBuffer, SIZE_T Size)
	{
		SIZE_T Bytes;
		ReadProcessMemory(hProcess, TargetAddress, LocalBuffer, Size, &Bytes);
	}

	vector<PVOID> FindPatternEx(HANDLE hProcess, PVOID StartAddress, SIZE_T SearchSize, const CHAR Pattern[], const CHAR Mask[], DWORD Protect)
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
							Result.emplace_back((PVOID)(MatchAddress + InPage - InSign - Header));
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

	BOOLEAN ForceOperate(PVOID Address, SIZE_T Size, const function<void()> &fnOperateCallback)
	{
		ULONG SaveProtect;

		if (!VirtualProtect(Address, Size, PAGE_EXECUTE_READWRITE, &SaveProtect)) {
			return FALSE;
		}

		fnOperateCallback();

		return VirtualProtect(Address, Size, SaveProtect, &SaveProtect);
	}

	vector<BYTE> MakeCall(PVOID HookAddress, PVOID CallAddress)
	{
		vector<BYTE> MakeCode = { 0xE8, 0x00, 0x00, 0x00, 0x00 };
		*(ULONG*)(MakeCode.data() + 1) = (ULONG)((ULONG_PTR)CallAddress - (ULONG_PTR)HookAddress - 5);
		return MakeCode;
	}

	vector<BYTE> MakeJmp(PVOID HookAddress, PVOID JmpAddress)
	{
		vector<BYTE> MakeCode = { 0xE9, 0x00, 0x00, 0x00, 0x00 };
		*(ULONG*)(MakeCode.data() + 1) = (ULONG)((ULONG_PTR)JmpAddress - (ULONG_PTR)HookAddress - 5);
		return MakeCode;
	}

} // namespace Memory

namespace Utils
{
	void CreateConsole()
	{
		FILE *hStream = NULL;
		AllocConsole();
		freopen_s(&hStream, "CONOUT$", "w", stdout);
	}

} // namespace Utils