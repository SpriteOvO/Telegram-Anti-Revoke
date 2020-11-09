#pragma once

class Updater
{
public:
    static Updater& GetInstance();

    bool CheckUpdate();

private:
    bool GetDataByBridge(string &ReturnedResponse);
    bool GetDataDirectly(string &ReturnedResponse);

};
