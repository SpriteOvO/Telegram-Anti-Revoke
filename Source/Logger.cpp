// An easy log manager

#include "Header.h"


LoggerManager::LoggerManager()
{
	this->File.open("ArLog.txt", ofstream::out | ofstream::trunc);

	TraceInfo("");
	TraceInfo("Running. Version: [" AR_VERSION "]");
}

LoggerManager::~LoggerManager()
{
	Close();
}

void LoggerManager::TraceText(const string &Content)
{
	if (!this->File.is_open()) {
		return;
	}

	SYSTEMTIME LocalTime;
	CHAR TimeBuffer[64] = { 0 };

	GetLocalTime(&LocalTime);
	sprintf_s(TimeBuffer, "[%d.%02d.%02d-%02d:%02d:%02d] ", LocalTime.wYear, LocalTime.wMonth, LocalTime.wDay, LocalTime.wHour, LocalTime.wMinute, LocalTime.wSecond);

	string Result = Text::SubReplace(Content, "\n", "[\\n]");

	std::lock_guard<std::mutex> Lock(this->Mutex);
	this->File << string(TimeBuffer) + Result << endl;
}

void LoggerManager::TraceInfo(const string &Content)
{
	TraceText("[Info]  " + Content);
}

void LoggerManager::TraceWarn(const string &Content)
{
	TraceText("[Warn]  " + Content);
#ifdef _DEBUG
	MessageBoxA(NULL, Content.c_str(), "Anti-Revoke Plugin", MB_ICONWARNING);
#endif
}

void LoggerManager::TraceError(const string &Content, BOOLEAN bReport)
{
	TraceText("[Error] " + Content);
	Close();

	/*
		An error has occurred!
		Please help us fix this problem.
		--------------------------------------------------

		[error content]

		--------------------------------------------------
		Click "Abort" or "Ignore" will pop up GitHub issue tracker page.
		You can submit this information to us there.
		Thank you very much.
	*/
	string Msg;
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

	INT Result;
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
		FORCE_EXIT();
	}
}

void LoggerManager::Close()
{
	if (this->File.is_open()) {
		this->File.close();
	}
}