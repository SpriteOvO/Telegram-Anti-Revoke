#pragma once

#include <fstream>


class LoggerManager
{
public:
	LoggerManager();
	~LoggerManager();

	void TraceText(std::string Content);
	void TraceInfo(std::string Content);
	void TraceWarn(std::string Content);
	void TraceError(std::string Content, BOOLEAN bReport = TRUE);

	void Close();

private:
	HANDLE hMutex;
	std::ofstream File;

};