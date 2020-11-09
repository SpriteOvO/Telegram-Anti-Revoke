#include "ILogger.h"

#include <Windows.h>

#include "Config.h"
#include "Utils.h"


ILogger& ILogger::GetInstance()
{
    static ILogger i;
    return i;
}

ILogger::ILogger()
{
    _File.open("ArLog.txt", std::ofstream::out | std::ofstream::trunc);

    TraceInfo("Running. Version: [" AR_VERSION "]");
}

void ILogger::TraceText(const std::string &Content)
{
    if (!_File.is_open()) {
        return;
    }

    SYSTEMTIME LocalTime;
    char TimeBuffer[64] = { 0 };

    GetLocalTime(&LocalTime);
    sprintf_s(TimeBuffer, "[%d.%02d.%02d-%02d:%02d:%02d] ", LocalTime.wYear, LocalTime.wMonth, LocalTime.wDay, LocalTime.wHour, LocalTime.wMinute, LocalTime.wSecond);

    std::string Result = Text::SubReplace(Content, "\n", "[\\n]");

    std::lock_guard<std::mutex> Lock(_Mutex);
    _File << std::string(TimeBuffer) + Result << std::endl;
}

void ILogger::TraceInfo(const std::string &Content)
{
    TraceText("[Info]  " + Content);
}

void ILogger::TraceWarn(const std::string &Content)
{
    TraceText("[Warn]  " + Content);
#ifdef _DEBUG
    MessageBoxA(NULL, Content.c_str(), "Anti-Revoke Plugin", MB_ICONWARNING);
#endif
}

void ILogger::TraceError(const std::string &Content, bool bReport)
{
    TraceText("[Error] " + Content);

    std::string Msg;
    if (bReport) {
        Msg = "An error has occurred!\n"
            "Please help us fix this problem.\n"
            "--------------------------------------------------\n"
            "\n" +
            Content + "\n"
            "\n"
            "--------------------------------------------------\n"
            "Click \"Abort\" or \"Ignore\" will pop up GitHub issue tracker page.\n"
            "You can submit this information to us there.\n"
            "Thank you very much.";
    }
    else {
        Msg = Content;
    }

    int Result;
    do
    {
        Result = MessageBoxA(NULL, Msg.c_str(), "Anti-Revoke Plugin", MB_ABORTRETRYIGNORE | MB_ICONERROR);
        // lol..
    } while (Result == IDRETRY);


    if (bReport) {
        // Pop up Github issues tracker

        // Using ShellExecute will cause freezing. I guess it may be caused by different Runtime libraries like cross-module malloc/free.
        // So replace ShellExecute with system, there is not much difference anyway.

        // ShellExecute(NULL, L"open", L"https://github.com/SpriteOvO/Telegram-Anti-Revoke/issues", NULL, NULL, SW_SHOWNORMAL);
        system("start " AR_ISSUES_URL);
    }

    if (Result == IDABORT) {
        ExitProcess(0);
    }
}
