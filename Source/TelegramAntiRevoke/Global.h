#pragma once

class HistoryMessage;
class QtString;
class LoggerManager;
class LanguageInstance;

typedef LPVOID(__cdecl *fntMalloc)(unsigned int size);
typedef void(__cdecl *fntFree)(void *block);
typedef INT(__cdecl *fntGetEditedIndex)();
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
	extern fntGetEditedIndex fnGetEditedIndex;
	extern HANDLE hMutex;
	extern vector<HistoryMessage*> RevokedMessages;
	extern BOOLEAN IsDetach;
	extern MARK_INFO CurrentMark;
};

