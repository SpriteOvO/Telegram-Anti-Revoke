#pragma once

#include <fstream>
#include <mutex>


class ILogger
{
public:
    static ILogger& GetInstance();

    ILogger();

    void TraceInfo(const std::string &Content);
    void TraceWarn(const std::string &Content);
    void TraceError(const std::string &Content, bool bReport = true);

private:
    std::mutex _Mutex;
    std::ofstream _File;

    void TraceText(const std::string &Content);

};
