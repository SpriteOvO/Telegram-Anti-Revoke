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

using namespace std;


template <typename T> void SafeMutex(HANDLE hMutex, T Callback)
{
	WaitForSingleObject(hMutex, INFINITE);
	Callback();
	ReleaseMutex(hMutex);
}

template <typename T1, typename T2> BOOLEAN SafeExcept(T1 TryCallback, T2 ExceptCallback)
{
	__try
	{
		TryCallback();
		return TRUE;
	}
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
		ExceptCallback(GetExceptionCode());
		return FALSE;
	}
}

#include "Logger.h"

// Current version
#define AR_VERSION				"0.1.0"
#define AR_URL_ISSUES			"https://github.com/SpriteOvO/Telegram-Anti-Revoke/issues"
#define AR_URL_ISSUES			"https://github.com/SpriteOvO/Telegram-Anti-Revoke/issues"
#define AR_URL_RELEASE			"/repos/SpriteOvO/Telegram-Anti-Revoke/releases/latest"
