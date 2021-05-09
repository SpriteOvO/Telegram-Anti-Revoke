#pragma once

#include <string>


namespace Logger
{
    [[noreturn]] void DoError(const std::string &Content, bool bReport);

    void Initialize();

} // namespace Logger
