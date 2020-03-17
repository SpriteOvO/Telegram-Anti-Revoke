#pragma once

#include <stdio.h>
#include <Windows.h>
#include <Psapi.h>
#include <vector>
#include <map>
#include <memory>

#include "../ThirdParty/MinHook/MinHook.h"
#include "../ThirdParty/jsoncpp/json.h"

#include "Utils.h"
#include "Logger.h"

using namespace std;

// Current version
#define AR_VERSION				"0.1.5"
#define AR_URL_ISSUES			"https://github.com/SpriteOvO/Telegram-Anti-Revoke/issues"
#define AR_URL_RELEASE			"/repos/SpriteOvO/Telegram-Anti-Revoke/releases/latest"
