#pragma once

#include "QtString.h"


enum class DocumentType : uint32_t
{
    File = 0,
    Video = 1,
    Song = 2,
    Sticker = 3,
    Animated = 4,
    Voice = 5,
    RoundVideo = 6,
    WallPaper = 7,
};


class Object
{
public:
    int32_t GetWidth();
    void SetWidth(int32_t Value);

    void* VirtualTable = NULL;
    int32_t MaxWidth = 0;
    int32_t MinHeight = 0;
    int32_t Width = 0;
    int32_t Height = 0;
};

class DocumentData
{
public:
    DocumentType GetType();
    bool IsSticker();
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
    void SetMaxReplyWidth(int32_t Value);
    int32_t GetMaxReplyWidth();

};

class PeerData
{
public:
    bool IsChannel();

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

class HistoryMessage;

// In older versions of Telegram, the class name was Session
//
class History
{
public:
    PeerData* GetPeer();

    // Make the function conform to __thiscall rule.
    void OnDestroyMessage(HistoryMessage* pMessage);
};

//class HistoryItem
//{
//public:
//
//};

class HistoryMessage /* : public HistoryItem */
{
public:
    bool IsMessage();

    template <class CompT>
    CompT* GetComponent(uint32_t Index);

    HistoryMessageEdited* GetEdited();
    HistoryMessageSigned* GetSigned();
    HistoryMessageReply* GetReply();

    History* GetHistory();
    Media* GetMedia();
    bool IsSticker();
    bool IsLargeEmoji();
    HistoryViewElement* GetMainView();
    QtString* GetTimeText();
    int32_t GetTimeWidth();
    void SetTimeWidth(int32_t Value);
};

class LanguageInstance
{
public:
    QtString* GetId();
    QtString* GetPluralId();
    QtString* GetName();
    QtString* GetNativeName();
};
