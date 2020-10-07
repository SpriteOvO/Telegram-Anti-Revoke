#pragma once

#include <stdio.h>
#include <Windows.h>
#include <Psapi.h>
#include <vector>
#include <map>
#include <set>
#include <memory>
#include <mutex>

#include "ThirdParty/MinHook/MinHook.h"
#include "ThirdParty/jsoncpp/json.h"

#include "Utils.h"
#include "Logger.h"

using namespace std;

// Current version
#define AR_VERSION           "0.2.6"

#define AR_OWNER_REPO        "SpriteOvO/Telegram-Anti-Revoke"
#define AR_REPO_URL          "https://github.com/" AR_OWNER_REPO
#define AR_ISSUES_URL        AR_REPO_URL "/issues"
#define AR_LATEST_REQUEST    "/repos/" AR_OWNER_REPO "/releases/latest"
