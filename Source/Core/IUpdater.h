#pragma once

#include <string>
#include <optional>


class IUpdater
{
public:
    static IUpdater& GetInstance();

    bool CheckUpdate();

private:
    bool ParseResponse(const std::string &Response);

    std::optional<std::string> GetDataByBridge();
    std::optional<std::string> GetDataDirectly();

};
