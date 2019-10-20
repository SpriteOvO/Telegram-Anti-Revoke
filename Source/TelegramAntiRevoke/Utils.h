#pragma once

#include <Windows.h>
#include <string>
#include <vector>
#include <algorithm>
#include <functional>
#include <wininet.h>

#pragma comment(lib, "wininet.lib")

#define TO_STRING(v)							# v
#define PAGE_SIZE								( 0x1000 )
#define FORCE_EXIT()							{ TerminateProcess((HANDLE)-1, 0); ExitProcess(0); }
#define VEC_FIND(iter, vec, val)				( (iter = std::find(vec.begin(), vec.end(), val)) != vec.end() )
#define MAP_FIND(iter, map, val)				( (iter = map.find(val)) != map.end() )

namespace Text
{
	std::string			ToLowerA(const std::string &String);
	std::string			SubReplaceA(const std::string &Source, const std::string &Target, const std::string &New);
	std::vector<std::string> SplitByFlagA(const std::string &Source, const std::string &Flag);
	std::string			StringFormatA(const CHAR *Format, ...);
}

namespace Convert
{
	std::string			UnicodeToAnsi(const std::wstring &String);
}

namespace Internet
{
	std::string			RequestGetA(const std::string &HostName, INTERNET_PORT Port, const std::string &ObjectName, const std::string &AdditionalHeader = "Accept: */*\r\nAccept-Language: *\r\n");
}

namespace Thread
{
	template <typename T> void TimedExecute(DWORD Time, T ExecuteCallback)
	{
		static DWORD SaveCount = 0;
		DWORD TickCount = GetTickCount();

		if (SaveCount == 0) {
			SaveCount = TickCount;
			return;
		}

		if (TickCount - SaveCount >= Time) {
			ExecuteCallback();
			SaveCount = TickCount;
		}
	}
}

namespace Process
{
	std::string			GetCurrentName();
}

namespace Memory
{
	void				ReadProcess(HANDLE hProcess, PVOID TargetAddress, PVOID LocalBuffer, SIZE_T Size);
	std::vector<PVOID>	FindPatternEx(HANDLE hProcess, PVOID StartAddress, SIZE_T SearchSize, const CHAR Pattern[], const CHAR Mask[], DWORD Protect = PAGE_EXECUTE_READ);
	BOOLEAN				ForceOperate(PVOID Address, SIZE_T Size, const std::function<void()> &fnOperateCallback);
	std::vector<BYTE>	MakeCall(PVOID HookAddress, PVOID CallAddress);
	std::vector<BYTE>	MakeJmp(PVOID HookAddress, PVOID JmpAddress);
}

namespace Utils
{
	void				CreateConsole();

	template <typename T> PVOID GetFunctionAddress(T Function)
	{
		static_assert(sizeof(T) == sizeof(PVOID), "This parameter only accepts function.");

		union {
			T Function;
			PVOID Address;
		} Data;

		Data.Function = Function;
		return Data.Address;
	}

	template <typename T> T CallVirtual(PVOID Base, ULONG Index)
	{
		return (T)(((PVOID**)Base)[0][Index]);
	}
}