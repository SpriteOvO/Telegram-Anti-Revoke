#pragma once

#include <string>
#include <vector>
#include <functional>
#include <Windows.h>


class NonCopyable
{
protected:
    NonCopyable() = default;

    NonCopyable(const NonCopyable &) = delete;
    NonCopyable& operator=(const NonCopyable &) = delete;
};

class NonMovable
{
protected:
    NonMovable() = default;

    NonMovable(NonMovable &&) = delete;
    NonMovable& operator=(NonMovable &&) = delete;
};

namespace File
{
    uint32_t GetCurrentVersion();
    std::string GetCurrentName();

} // namespace File

namespace Text
{
    std::string ToLower(const std::string &String);
    std::string SubReplace(const std::string &Source, const std::string &Target, const std::string &New);
    std::vector<std::string> SplitByFlag(const std::string &Source, const std::string &Flag);
    std::string Format(const char *Format, ...);

} // namespace Text

namespace Convert
{
    std::string UnicodeToAnsi(const std::wstring &String);

} // namespace Convert

namespace Internet
{
    bool HttpRequest(std::string &Response, uint32_t &Status, const std::string &HttpVerb, const std::string &HostName, const std::string &ObjectName, const std::vector<std::pair<std::string, std::string>> &Headers, const std::string &PostData = std::string{});

} // namespace Internet

namespace Safe
{
    template<typename T1, typename T2>
    bool TryExcept(T1 TryCallback, T2 ExceptCallback)
    {
        __try {
            TryCallback();
            return true;
        }
        __except (EXCEPTION_EXECUTE_HANDLER) {
            ExceptCallback(GetExceptionCode());
            return false;
        }
    }

} // namespace Safe

namespace Memory
{
    void ReadProcess(HANDLE hProcess, void* TargetAddress, void* LocalBuffer, size_t Size);
    std::vector<uintptr_t> FindPatternEx(HANDLE hProcess, void* StartAddress, size_t SearchSize, const char Pattern[], const char Mask[], ULONG Protect = PAGE_EXECUTE_READ);
    bool ForceOperate(void* Address, size_t Size, const std::function<void()> &FnCallback);
    std::vector<uint8_t> MakeCall(void* HookAddress, void* CallAddress);
    std::vector<uint8_t> MakeJmp(void* HookAddress, void* JmpAddress);

} // namespace Memory

namespace Utils
{
    void CreateConsole();

    // HACK for get member function address
    //
    template<typename FnT>
    void* GetFunctionAddress(FnT Function)
    {
        static_assert(sizeof(FnT) == sizeof(void*), "This parameter only accepts function.");

        union {
            FnT Function;
            void* Address;
        } Data;

        Data.Function = Function;
        return Data.Address;
    }

    template <typename FnT>
    FnT CallVirtual(void* Base, uint32_t Index)
    {
        return (FnT)(((void***)Base)[0][Index]);
    }

} // namespace Utils