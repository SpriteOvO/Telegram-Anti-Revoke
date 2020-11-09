#pragma once

#include <string>


class IUpdater
{
public:
    static IUpdater& GetInstance();

    bool CheckUpdate();

private:
    bool GetDataByBridge(std::string &ReturnedResponse);
    bool GetDataDirectly(std::string &ReturnedResponse);

};
