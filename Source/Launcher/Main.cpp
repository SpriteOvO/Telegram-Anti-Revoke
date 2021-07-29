#include <Windows.h>
#include <tlhelp32.h>

#include <string>
#include <format>
#include <thread>
#include <fstream>
#include <filesystem>

#include <Config.h>


using namespace std::chrono_literals;

namespace fs = std::filesystem;

[[noreturn]] void FatalError(const std::string &content)
{
    MessageBoxA(
        nullptr,
        content.c_str(),
        "Telegram-Anti-Revoke Launcher",
        MB_ICONERROR
    );

    std::exit(1);
}

std::wstring ToLower(std::wstring source)
{
    std::transform(source.begin(), source.end(), source.begin(), tolower);
    return source;
}

enum class Arch : uint32_t
{
    Unknown,
    x86,
    x64,
};

Arch GetTargetArch(const fs::path &fullFilePath)
{
    DWORD binaryType = -1;

    if (!GetBinaryTypeW(fullFilePath.c_str(), &binaryType)) {
        return Arch::Unknown;
    }

    switch (binaryType)
    {
    case SCS_32BIT_BINARY:
        return Arch::x86;
    case SCS_64BIT_BINARY:
        return Arch::x64;
    default:
        return Arch::Unknown;
    }
}

ptrdiff_t GetLoadLibraryOffset()
{
    auto kernel32 = (uintptr_t)LoadLibraryW(L"kernel32.dll");
    if (kernel32 == 0) {
        FatalError("LoadLibraryW() kernel32.dll failed.");
    }

    auto apiAddress = (uintptr_t)GetProcAddress((HMODULE)kernel32, "LoadLibraryW");
    if (apiAddress == 0) {
        FatalError("GetProcAddress() LoadLibraryW failed.");
    }

    if (apiAddress < kernel32) {
        FatalError(
            std::format("Invalid address. kernel32: {}, apiAddress: {}", kernel32, apiAddress)
        );
    }

    return apiAddress - kernel32;
}

uintptr_t GetProcessModuleBase(uint32_t processId, std::wstring moduleName)
{
    HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, processId);
    if (snapshot == INVALID_HANDLE_VALUE) {
        return 0;
    }

    moduleName = ToLower(std::move(moduleName));

    uintptr_t result = 0;
    MODULEENTRY32W entry{};
    entry.dwSize = sizeof(entry);

    if (Module32FirstW(snapshot, &entry)) {
        do {
            if (moduleName == ToLower(entry.szModule)) {
                result = (uintptr_t)entry.modBaseAddr;
                break;
            }
        } while (Module32NextW(snapshot, &entry));
    }

    CloseHandle(snapshot);
    return result;
}

bool WriteMemory(HANDLE processHandle, void *target, const void *buffer, size_t size)
{
    SIZE_T written = 0;
    if (!WriteProcessMemory(processHandle, target, buffer, size, &written)) {
        return false;
    }
    return size == written;
}

int WinMain(HINSTANCE instance, HINSTANCE prevInstance, LPSTR cmdLine, int showCmd)
{
    if (!fs::exists("./Telegram.exe")) {
        FatalError(
            "\"Telegram.exe\" file not found.\n"
            "Please place this file in the \"Telegram.exe\" directory."
        );
    }

#if defined PLATFORM_X86
    fs::path corePath = "./TAR-Resources/x86.dll";
#elif defined PLATFORM_X64
    fs::path corePath = "./TAR-Resources/x64.dll";
#else
# error "Configurations error."
#endif

    if (!fs::exists(corePath)) {
        FatalError(
            "Resource files are missing.\n"
            "Did you forget to copy the \"TAR-Resources\" folder?"
        );
    }

    auto arch = GetTargetArch(fs::absolute("./Telegram.exe"));
    if (arch == Arch::Unknown) {
        FatalError("Invalid file \"Telegram.exe\".");
    }
#if defined PLATFORM_X86
    if (arch == Arch::x64) {
        FatalError(
            "The \"Telegram.exe\" in the current directory is x64 file.\n"
            "Please run \"TAR-Launcher-x64.exe\" to try again."
        );
    }
#elif defined PLATFORM_X64
    if (arch == Arch::x86) {
        FatalError(
            "The \"Telegram.exe\" in the current directory is x86 file.\n"
            "Please run \"TAR-Launcher-x86.exe\" to try again."
        );
    }
#else
# error "Configurations error."
#endif

    auto apiOffset = GetLoadLibraryOffset();

    STARTUPINFOA startupInfo{};
    startupInfo.cb = sizeof(startupInfo);

    PROCESS_INFORMATION processInfo{};

    bool isCreateSuccess = CreateProcessA(
        "Telegram.exe",
        nullptr,
        nullptr,
        nullptr,
        false,
        0,
        nullptr,
        nullptr,
        &startupInfo,
        &processInfo
    );
    if (!isCreateSuccess) {
        FatalError(std::format("CreateProcessA() failed. Last error code: {}", ::GetLastError()));
    }

    std::this_thread::sleep_for(3s);

    auto targetKernel32 = GetProcessModuleBase(processInfo.dwProcessId, L"kernel32.dll");
    if (targetKernel32 == 0) {
        // Telegram.exe exited at startup
        return 0;
    }

    auto targetLoadLibrary = targetKernel32 + apiOffset;

    auto targetBuffer = VirtualAllocEx(
        processInfo.hProcess,
        nullptr,
        0x1000,
        MEM_COMMIT,
        PAGE_READWRITE
    );
    if (targetBuffer == nullptr) {
        FatalError(std::format("VirtualAllocEx() failed. Last error code: {}", ::GetLastError()));
    }

    std::wstring targetFile = fs::absolute(corePath);

    if (!WriteMemory(
        processInfo.hProcess,
        targetBuffer,
        targetFile.c_str(),
        (targetFile.size() + 1) * sizeof(wchar_t))
    ) {
        FatalError(std::format("WriteMemory() failed. Last error code: {}", ::GetLastError()));
    }

    HANDLE remoteThread = CreateRemoteThread(
        processInfo.hProcess,
        nullptr,
        0,
        (LPTHREAD_START_ROUTINE)targetLoadLibrary,
        targetBuffer,
        0,
        nullptr
    );
    if (remoteThread == nullptr) {
        FatalError(
            std::format("CreateRemoteThread() failed. Last error code: {}", ::GetLastError())
        );
    }
    CloseHandle(remoteThread);

    CloseHandle(processInfo.hProcess);
    CloseHandle(processInfo.hThread);

    return 0;
}
