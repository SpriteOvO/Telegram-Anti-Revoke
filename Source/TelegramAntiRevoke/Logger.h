#pragma once

#include <fstream>


class LoggerManager
{
public:
	LoggerManager();
	~LoggerManager();

	void TraceText(string Content);
	void TraceInfo(string Content);
	void TraceWarn(string Content);
	void TraceError(string Content, BOOLEAN NeedReport = TRUE);

	void Close();

private:
	HANDLE hMutex;
	ofstream File;

};