#include "IUpdater.h"

#include <fstream>
#include <Windows.h>
#include <wininet.h>

#include <nlohmann/json.hpp>

#include "Logger.h"
#include "Config.h"
#include "Utils.h"

using json = nlohmann::json;

IUpdater &IUpdater::GetInstance()
{
    static IUpdater i;
    return i;
}

bool IUpdater::CheckUpdate()
{
    // Get releases data
    //

    // GitHub REST API limits the rate of unauthenticated requests to 60 per hour
    // See: https://docs.github.com/en/rest/overview/resources-in-the-rest-api#rate-limiting
    //
    // So we first try to request to Google Script API (we call it "bridge" here)
    // It will forward the request to GitHub REST API with authentication information
    //

    std::optional<std::string> Response = GetDataByBridge();
    if (!Response.has_value()) {
        LOG(Warn, "[Updater] GetDataByBridge() failed, try GetDataDirectly().");
    }
    else {
        if (ParseResponse(Response.value())) {
            LOG(Info, "[Updater] ParseResponse() successed. (ByBridge)");
            return true;
        }
        LOG(Warn, "[Updater] ParseResponse() failed, try Directly. (ByBridge)");
    }

    Response = GetDataDirectly();
    if (!Response.has_value()) {
        LOG(Warn, "[Updater] GetDataDirectly() failed.");
        return false;
    }

    if (!ParseResponse(Response.value())) {
        LOG(Warn, "[Updater] ParseResponse() failed. (Directly)");
        return false;
    }

    LOG(Info, "[Updater] ParseResponse() successed. (Directly)");
    return true;
}

bool IUpdater::ParseResponse(const std::string &Response)
{
    try {
        // Parse response
        //
        auto Root = json::parse(Response);
        const auto &Message = Root["message"];
        const auto &TagName = Root["tag_name"]; // Latest version
        const auto &HtmlUrl = Root["html_url"]; // Latest url
        const auto &Body = Root["body"];

        if (Message.is_string()) {
            LOG(Warn, "[Updater] Response has a message. message: {}", Message.get<std::string>());
        }

        if (!TagName.is_string() || !HtmlUrl.is_string() || !Body.is_string()) {
            LOG(Warn, "[Updater] Response fields invalid.");
            return false;
        }

        std::string HtmlUrlContent = HtmlUrl.get<std::string>();
        std::string TagNameContent = TagName.get<std::string>();
        std::string BodyContent = Body.get<std::string>();

        if (HtmlUrlContent.find(AR_REPO_URL) != 0) {
            LOG(Warn, "[Updater] html_url field invalid. html_url: {}", HtmlUrlContent);
            return false;
        }

        std::vector<std::string> vLocal = Text::SplitByFlag(AR_VERSION_STRING, ".");
        std::vector<std::string> vLatest = Text::SplitByFlag(TagNameContent, ".");

        if (vLocal.size() != 3 || vLatest.size() != 3) {
            LOG(Warn, "[Updater] Version format invalid. Local: {}, Latest: {}", AR_VERSION_STRING,
                TagNameContent);
            return false;
        }

        std::string LocalString =
            Text::Format("%03d%03d%03d", stoul(vLocal[0]), stoul(vLocal[1]), stoul(vLocal[2]));
        std::string LatestString =
            Text::Format("%03d%03d%03d", stoul(vLatest[0]), stoul(vLatest[1]), stoul(vLatest[2]));
        uint32_t LocalNumber = stoul(LocalString);
        uint32_t LatestNumber = stoul(LatestString);

        if (LocalNumber >= LatestNumber) {
            LOG(Info, "[Updater] No need to update. Local: {}, Latest: {}", LocalString,
                LatestString);
            return true;
        }

        LOG(Info, "[Updater] Need to update. Local: {}, Latest: {}", LocalString, LatestString);

        // Get Changelog
        //

        std::string ChangeLog;
        size_t ClBeginPos = BodyContent.find("Change log");

        if (ClBeginPos != std::string::npos) {
            // Find end of ChangeLog
            size_t ClEndPos = BodyContent.find("\r\n\r\n", ClBeginPos), ClCount;

            // If found, calc the size
            if (ClEndPos != std::string::npos) {
                ClCount = ClEndPos - ClBeginPos;
            }
            else {
                ClCount = std::string::npos;
            }

            ChangeLog = BodyContent.substr(ClBeginPos, ClCount) + "\n\n";
        }

        bool AllowSkip = [&]() {
            if (File::GetCurrentVersion() > 2008004) {
                return false;
            }

            auto MetaBegin = BodyContent.find("<meta>");
            auto MetaEnd = BodyContent.find("</meta>");
            if (MetaBegin == std::string::npos || MetaEnd == std::string::npos) {
                LOG(Warn, "[Updater] <meta> tag not found. Content: '{}'", BodyContent);
                return false;
            }

            MetaBegin += std::string{"<meta>"}.size();
            if (MetaBegin >= MetaEnd) {
                LOG(Warn, "[Updater] MetaBegin >= MetaEnd. Content: '{}'", BodyContent);
                return false;
            }

            std::string MetaJson = BodyContent.substr(MetaBegin, MetaEnd - MetaBegin);
            try {
                auto MetaRoot = json::parse(MetaJson);
                return MetaRoot["allow_skip"].get<bool>();
            }
            catch (json::exception &Exception) {
                LOG(Warn, "[Updater] AllowSkip() exception: '{}'. MetaJson: '{}'", Exception.what(),
                    MetaJson);
                return false;
            }
        }();
        LOG(Info, "[Updater] AllowSkip: {}", AllowSkip);

        constexpr auto ConfigFileName = "TAR-Config.json";

        if (AllowSkip) {
            try {
                std::ifstream ConfigFile{ConfigFileName};
                if (ConfigFile.good()) {
                    json ConfigRoot;
                    ConfigFile >> ConfigRoot;
                    if (ConfigRoot["skip_version"].get<std::string>() == TagNameContent) {
                        LOG(Info, "[Updater] Skip update {}", TagNameContent);
                        return true; // skip
                    }
                }
            }
            catch (const std::exception &Exception) {
                LOG(Warn, "[Updater] Skip read exception: {}", Exception.what());
            }
        }

        // Pop up the update message
        //

        std::string Msg = "A new version has been released.\n"
                          "\n"
                          "Current version: " AR_VERSION_STRING "\n"
                          "Latest version: " +
                          TagNameContent +
                          "\n"
                          "\n" +
                          ChangeLog +
                          "Do you want to go to GitHub to download the latest version?\n";

        if (MessageBoxA(nullptr, Msg.c_str(), "Anti-Revoke Plugin", MB_ICONQUESTION | MB_YESNO) ==
            IDYES) {
            system(("start " + HtmlUrlContent).c_str());
        }
        else if (AllowSkip) {
            Msg = "Do you want to skip this version?";
            if (MessageBoxA(
                    nullptr, Msg.c_str(), "Anti-Revoke Plugin", MB_ICONQUESTION | MB_YESNO) ==
                IDYES) {
                try {
                    json ConfigRoot;
                    {
                        std::ifstream Input{ConfigFileName};
                        if (Input.good()) {
                            ConfigRoot << Input;
                        }
                    }
                    ConfigRoot["skip_version"] = TagNameContent;
                    std::ofstream Output{ConfigFileName};
                    Output << ConfigRoot;
                }
                catch (const std::exception &Exception) {
                    LOG(Warn, "[Updater] Skip write exception: {}", Exception.what());
                }
            }
        }

        return true;
    }
    catch (json::exception &Exception) {
        LOG(Warn, "[Updater] Caught a json exception. What: {}, Response: {}", Exception.what(),
            Response);
        return false;
    }
}

std::optional<std::string> IUpdater::GetDataByBridge()
{
    std::string Response;
    uint32_t Status;
    bool IsSuccessed = Internet::HttpRequest(
        Response, Status, "POST", "script.google.com",
        "/macros/s/AKfycbxfGLfG3nXZOIE-t0zFIMGGylBbvj9dc1aiowtAvyh5YEZ69o0/exec",
        {{"Accept", "application/json"}, {"Content-Type", "application/json"}},
        "{\"forward_request\": \"" AR_LATEST_REQUEST "\"}");

    if (!IsSuccessed) {
        LOG(Warn, "[Updater] Internet::HttpRequest() failed. (ByBridge)");
        return std::nullopt;
    }

    if (Status != HTTP_STATUS_OK) {
        LOG(Warn, "[Updater] Response status is not 200. Status: {}, Response: {} (ByBridge)",
            Status, Response);
        return std::nullopt;
    }

    try {
        auto Root = json::parse(Response);
        const auto &BridgeErrorMessage = Root["bridge_error_message"];
        if (BridgeErrorMessage.is_string()) {
            LOG(Warn, "[Updater] bridge_error_message: {} (ByBridge)",
                BridgeErrorMessage.get<std::string>());
            return std::nullopt;
        }

        LOG(Info, "[Updater] Get data by bridge successed.");
        return Response;
    }
    catch (json::exception &Exception) {
        LOG(Warn, "[Updater] Caught a json exception. What: {}, Response: {}", Exception.what(),
            Response);
        return std::nullopt;
    }
}

std::optional<std::string> IUpdater::GetDataDirectly()
{
    std::string Response;
    uint32_t Status;
    bool IsSuccessed = Internet::HttpRequest(
        Response, Status, "GET", "api.github.com", AR_LATEST_REQUEST,
        {
            {"Accept", "application/vnd.github.v3+json"},
        });

    if (!IsSuccessed) {
        LOG(Warn, "[Updater] Internet::HttpRequest() failed. (Directly)");
        return std::nullopt;
    }

    if (Status != HTTP_STATUS_OK) {
        LOG(Warn, "[Updater] Response status is not 200. Status: {}, Response: {} (Directly)",
            Status, Response);
        return std::nullopt;
    }

    LOG(Info, "[Updater] Get data directly successed.");
    return Response;
}
