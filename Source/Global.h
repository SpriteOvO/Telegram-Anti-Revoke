#pragma once

class HistoryMessage;
class QtString;
class LoggerManager;
class LanguageInstance;

typedef LPVOID(__cdecl *fntMalloc)(unsigned int size);
typedef void(__cdecl *fntFree)(void *block);
typedef INT(__cdecl *fntIndex)();
typedef LanguageInstance*(__cdecl *fntGetCurrentInstance)();


struct MARK_INFO
{
	wstring LangName;
	wstring Content;
	INT Width;
};

namespace g
{
	extern LoggerManager Logger;
	extern fntMalloc fnMalloc;
	extern fntFree fnOriginalFree;
	extern fntIndex fnEditedIndex;
	extern fntIndex fnSignedIndex;
	extern mutex Mutex;
	extern set<HistoryMessage*> RevokedMessages;
	namespace Offsets
	{
		extern ULONG TimeText;
		extern ULONG TimeWidth;
		extern ULONG MainView;
		extern ULONG Media;

		extern ULONG Index_toHistoryMessage;
	}
	extern MARK_INFO CurrentMark;
};

