#pragma once

class HistoryMessage;
class QtString;

// TODO: Remove me
//
typedef void(__cdecl *fntFree)(void *block);

struct MARK_INFO
{
    wstring LangName;
    wstring Content;
    INT Width;
};

namespace g
{
    extern fntFree fnOriginalFree;
    extern mutex Mutex;
    extern set<HistoryMessage*> RevokedMessages;
    extern MARK_INFO CurrentMark;
};

