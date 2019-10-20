// An easy log manager

#include "Header.h"


LoggerManager::LoggerManager()
{
	this->hMutex = CreateMutex(NULL, FALSE, NULL);
	this->File.open("ArLog.txt", ios::app);

	TraceInfo("");
	TraceInfo("Running. Version: [" + string(AR_VERSION) + "]");
}

LoggerManager::~LoggerManager()
{
	Close();
}

void LoggerManager::TraceText(string Content)
{
	if (!this->File.is_open()) {
		return;
	}

	SYSTEMTIME LocalTime;
	CHAR TimeBuffer[64] = { 0 };

	GetLocalTime(&LocalTime);
	sprintf_s(TimeBuffer, "[%d.%02d.%02d-%02d:%02d:%02d] ", LocalTime.wYear, LocalTime.wMonth, LocalTime.wDay, LocalTime.wHour, LocalTime.wMinute, LocalTime.wSecond);

	Content = Text::SubReplaceA(Content, "\n", "[\\n]");

	SafeMutex(this->hMutex, [&]()
	{
		this->File << string(TimeBuffer) + Content << endl;
	});
}

void LoggerManager::TraceInfo(string Content)
{
	TraceText("[Info]  " + Content);
}

void LoggerManager::TraceWarn(string Content)
{
	TraceText("[Warn]  " + Content);
#ifdef _DEBUG
	MessageBoxA(NULL, Content.c_str(), "Anti-Revoke Plugin", MB_ICONWARNING);
#endif
}

void LoggerManager::TraceError(string Content, BOOLEAN NeedReport)
{
	TraceText("[Error] " + Content);
	Close();

	/*
		An error has occurred!

		[error content]

		Please help us fix this problem.
		Click "OK" to go to GitHub to submit a question.
		Thank you very much.
	*/ 
	string Msg;
	if (NeedReport) {
		Msg = "An error has occurred!\n"
			"Please help us fix this problem.\n"
			"\n" + 
			Content + "\n"
			"\n"
			"Click \"OK\" to go to GitHub to submit a question.\n"
			"Thank you very much.";
	}
	else {
		Msg = Content;
	}

	MessageBoxA(NULL, Msg.c_str(), "Anti-Revoke Plugin", MB_ICONERROR);

	if (NeedReport) {
		// Pop up Github issues tracker

		// 这里用 ShellExecute 会导致阻塞卡死，猜测可能和跨模块 malloc/free 一样，不同的 Runtime 库导致的。
		// 所以用 system 代替了，反正效果一样。

		// ShellExecute(NULL, L"open", L"https://github.com/SpriteOvO/Telegram-Anti-Revoke/issues", NULL, NULL, SW_SHOWNORMAL);
		system("start " AR_URL_ISSUES);
	}

	FORCE_EXIT();
}

void LoggerManager::Close()
{
	if (this->File.is_open()) {
		this->File.close();
	}
}