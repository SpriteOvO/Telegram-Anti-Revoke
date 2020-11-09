#pragma once

#include <string>


class Updater
{
public:
    static Updater& GetInstance();

    bool CheckUpdate();

private:
    bool GetDataByBridge(std::string &ReturnedResponse);
    bool GetDataDirectly(std::string &ReturnedResponse);

};
