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

class HistoryMessageReply
{
public:
};

//class HistoryItem
//{
//public:
//
//};

class PeerData
{
public:
	BOOLEAN IsChannel();

private:
	using PeerId = uint64_t;

	static constexpr auto PeerIdMask         = PeerId(0xFFFFFFFFULL);
	static constexpr auto PeerIdTypeMask     = PeerId(0xF00000000ULL);
	static constexpr auto PeerIdUserShift    = PeerId(0x000000000ULL);
	static constexpr auto PeerIdChatShift    = PeerId(0x100000000ULL);
	static constexpr auto PeerIdChannelShift = PeerId(0x200000000ULL);
	static constexpr auto PeerIdFakeShift    = PeerId(0xF00000000ULL);

	PeerId GetId();

};

class History
{
public:
	PeerData* GetPeer();

};

class HistoryMessage /* : public HistoryItem */
{
public:
	BOOLEAN IsMessage();

	template <class CompT>
	CompT* GetComponent(ULONG Index);

	HistoryMessageEdited* GetEdited();
	HistoryMessageSigned* GetSigned();
	HistoryMessageReply* GetReply();

	History* GetHistory();
	Media* GetMedia();
	BOOLEAN IsReply();
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
