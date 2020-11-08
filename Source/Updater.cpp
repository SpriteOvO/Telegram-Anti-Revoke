#include "Header.h"
#include "Global.h"
#include "Updater.h"

Updater& Updater::GetInstance()
{
	static Updater Instance;
	return Instance;
}

BOOLEAN Updater::CheckUpdate()
{
	string Response;

	// Get releases data
	//
	if (!GetDataByBridge(Response))
	{
		g::Logger.TraceWarn("[Updater] GetDataByBridge() failed, try GetDataDirectly().");

		if (!GetDataDirectly(Response)) {
			g::Logger.TraceWarn("[Updater] GetDataDirectly() failed.");
			return FALSE;
		}
	}

	// Parse response
	//
	Json::CharReaderBuilder Builder;
	Json::Value Root;
	Json::String Errors;
	unique_ptr<Json::CharReader> pReader(Builder.newCharReader());

	if (!pReader->parse(Response.c_str(), Response.c_str() + Response.size(), &Root, &Errors)) {
		g::Logger.TraceWarn("[Updater] Parse response failed. JsonError: " + Errors + " Response: " + Response);
		return FALSE;
	}

	Json::Value &Message = Root["message"];
	Json::Value &TagName = Root["tag_name"]; // Latest version
	Json::Value &HtmlUrl = Root["html_url"]; // Latest url
	Json::Value &Body = Root["body"];

	if (Message.isString()) {
		g::Logger.TraceWarn("[Updater] Response has a message. message: " + Message.asString());
	}

	if (!TagName.isString() || !HtmlUrl.isString() || !Body.isString()) {
		g::Logger.TraceWarn("[Updater] Response fields invalid.");
		return FALSE;
	}

	std::string HtmlUrlContent = HtmlUrl.asString();
	std::string TagNameContent = TagName.asString();
	std::string BodyContent = Body.asString();

	if (HtmlUrlContent.find(AR_REPO_URL) != 0) {
		g::Logger.TraceWarn("[Updater] html_url field invalid. html_url: " + HtmlUrlContent);
		return FALSE;
	}

	vector<string> vLocal = Text::SplitByFlag(AR_VERSION, ".");
	vector<string> vLatest = Text::SplitByFlag(TagNameContent, ".");

	if (vLocal.size() != 3 || vLatest.size() != 3) {
		g::Logger.TraceWarn("[Updater] Version format invalid. Local: " AR_VERSION " Latest: " + TagNameContent);
		return FALSE;
	}

	string LocalString = Text::Format("%03d%03d%03d", stoul(vLocal[0]), stoul(vLocal[1]), stoul(vLocal[2]));
	string LatestString = Text::Format("%03d%03d%03d", stoul(vLatest[0]), stoul(vLatest[1]), stoul(vLatest[2]));
	ULONG LocalNumber = stoul(LocalString);
	ULONG LatestNumber = stoul(LatestString);

	if (LocalNumber >= LatestNumber) {
		g::Logger.TraceInfo("[Updater] No need to update. Local: " + LocalString + " Latest: " + LatestString);
		return TRUE;
	}

	g::Logger.TraceInfo("[Updater] Need to update. Local: " + LocalString + " Latest: " + LatestString);

	// Get Changelog
	//

	string ChangeLog;
	SIZE_T ClBeginPos = BodyContent.find("Change log");

	if (ClBeginPos != string::npos)
	{
		// Find end of ChangeLog
		SIZE_T ClEndPos = BodyContent.find("\r\n\r\n", ClBeginPos), ClCount;

		// If found, calc the size
		if (ClEndPos != string::npos) {
			ClCount = ClEndPos - ClBeginPos;
		}
		else {
			ClCount = string::npos;
		}

		ChangeLog = BodyContent.substr(ClBeginPos, ClCount) + "\n\n";
	}

	// Pop up the update message
	//

	string Msg =
		"A new version has been released.\n"
		"\n"
		"Current version: " AR_VERSION "\n"
		"Latest version: " + TagNameContent + "\n"
		"\n" +
		ChangeLog +
		"Do you want to go to GitHub to download the latest version?\n";

	if (MessageBoxA(NULL, Msg.c_str(), "Anti-Revoke Plugin", MB_ICONQUESTION | MB_YESNO) == IDYES) {
		system(("start " + HtmlUrlContent).c_str());
	}

	return TRUE;
}

BOOLEAN Updater::GetDataByBridge(string &ReturnedResponse)
{
	BOOLEAN Result = FALSE;

	Safe::TryExcept(
		[&]()
		{
			string Response;
			ULONG Status;
			BOOLEAN IsSuccessed = Internet::HttpRequest(
				Response,
				Status,
				"POST",
				"script.google.com",
				"/macros/s/AKfycbxfGLfG3nXZOIE-t0zFIMGGylBbvj9dc1aiowtAvyh5YEZ69o0/exec",
				{
					{ "Accept" , "application/json" },
					{ "Content-Type" , "application/json" }
				},
				"{\"forward_request\": \"" AR_LATEST_REQUEST "\"}"
			);

			if (!IsSuccessed) {
				g::Logger.TraceWarn("[Updater] Internet::HttpRequest() failed. (ByBridge)");
				return;
			}

			if (Status != HTTP_STATUS_OK) {
				g::Logger.TraceWarn("[Updater] Response status is not 200. Status: " + to_string(Status) + " Response: " + Response + " (ByBridge)");
				return;
			}

			Json::CharReaderBuilder Builder;
			Json::Value Root;
			Json::String Errors;
			unique_ptr<Json::CharReader> pReader(Builder.newCharReader());

			if (!pReader->parse(Response.c_str(), Response.c_str() + Response.size(), &Root, &Errors)) {
				g::Logger.TraceWarn("[Updater] Parse response failed. JsonError: " + Errors + " Response: " + Response + " (ByBridge)");
				return;
			}

			Json::Value &BridgeErrorMessage = Root["bridge_error_message"];
			if (BridgeErrorMessage.isString()) {
				g::Logger.TraceWarn("[Updater] bridge_error_message: " + BridgeErrorMessage.asString() + " (ByBridge)");
				return;
			}

			ReturnedResponse = Response;
			Result = TRUE;

			g::Logger.TraceInfo("[Updater] Get data by bridge successed.");
		},
		[](ULONG ExceptionCode)
		{
			g::Logger.TraceWarn("[Updater] An exception was caught. ExceptionCode: " + Text::Format("0x%x", ExceptionCode) + " (ByBridge)");
		}
	);

	return Result;
}

BOOLEAN Updater::GetDataDirectly(string &ReturnedResponse)
{
	string Response;
	ULONG Status;
	BOOLEAN IsSuccessed = Internet::HttpRequest(
		Response,
		Status,
		"GET",
		"api.github.com",
		AR_LATEST_REQUEST,
		{
			{ "Accept" , "application/vnd.github.v3+json" },
		}
	);

	if (!IsSuccessed) {
		g::Logger.TraceWarn("[Updater] Internet::HttpRequest() failed. (Directly)");
		return FALSE;
	}

	if (Status != HTTP_STATUS_OK) {
		g::Logger.TraceWarn("[Updater] Response status is not 200. Status: " + to_string(Status) + " Response: " + Response + " (Directly)");
		return FALSE;
	}

	ReturnedResponse = Response;

	g::Logger.TraceInfo("[Updater] Get data directly successed.");
	return TRUE;
}
