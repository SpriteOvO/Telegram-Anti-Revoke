#pragma once

#include "QtString.h"


enum DocumentType {
	FileDocument = 0,
	VideoDocument = 1,
	SongDocument = 2,
	StickerDocument = 3,
	AnimatedDocument = 4,
	VoiceDocument = 5,
	RoundVideoDocument = 6,
	WallPaperDocument = 7,
};


class Object
{
public:
	INT GetWidth();
	void SetWidth(INT Value);

	PVOID VirtualTable = NULL;
	INT MaxWidth = 0;
	INT MinHeight = 0;
	INT Width = 0;
	INT Height = 0;
};

class DocumentData
{
public:
	ULONG GetType();
	BOOLEAN IsSticker();
};

class Media : public Object
{
public:
	DocumentData *GetDocument();
};

class HistoryViewElement : public Object
{
public:
	Media* GetMedia();
};

class HistoryMessageEdited
{
public:
	QtString* GetTimeText();
};

class HistoryMessageSigned
{
public:
	QtString* GetTimeText();
};

//class HistoryItem
//{
//public:
//
//};

class HistoryMessage /* : public HistoryItem */
{
public:
	BOOLEAN IsMessage();
	HistoryMessageEdited* GetEdited();
	HistoryMessageSigned* GetSigned();
	Media* GetMedia();
	BOOLEAN IsSticker();
	BOOLEAN IsLargeEmoji();
	HistoryViewElement* GetMainView();
	QtString* GetTimeText();
	INT GetTimeWidth();
	void SetTimeWidth(INT Value);
};

class Session
{
public:
	// Make the function conform to __thiscall rule.
	void ProcessRevoke(HistoryMessage* pMsg);
};

class LanguageInstance
{
public:
	QtString* GetId();
	QtString* GetPluralId();
	QtString* GetName();
	QtString* GetNativeName();
};
