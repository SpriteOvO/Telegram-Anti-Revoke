#include "Header.h"
#include "Global.h"
#include "QtString.h"
#include "Telegram.h"
#include "ILogger.h"


//////////////////////////////////////////////////

INT Object::GetWidth()
{
	return this->MaxWidth;
}
void Object::SetWidth(INT Value)
{
	this->MaxWidth = Value;
}

//////////////////////////////////////////////////

ULONG DocumentData::GetType()
{
	return *(INT*)((ULONG_PTR)this + 8);
}

BOOLEAN DocumentData::IsSticker()
{
	return GetType() == StickerDocument;
}

//////////////////////////////////////////////////

DocumentData* Media::GetDocument()
{
	// DocumentData *Media::document()
	return *(DocumentData**)((ULONG_PTR)this + 8);
}

//////////////////////////////////////////////////

Media* HistoryViewElement::GetMedia()
{
	return *(Media**)((ULONG_PTR)this + 0x24);
}

//////////////////////////////////////////////////

QtString* HistoryMessageEdited::GetTimeText()
{
	return (QtString*)((ULONG_PTR)this + 0x10);
}

//////////////////////////////////////////////////

QtString* HistoryMessageSigned::GetTimeText()
{
	return (QtString*)((ULONG_PTR)this + g::Offsets::SignedTimeText);
}

//////////////////////////////////////////////////

BOOLEAN PeerData::IsChannel()
{
	return (GetId() & PeerIdTypeMask) == PeerIdChannelShift;
}

PeerData::PeerId PeerData::GetId()
{
	return *(PeerId*)((ULONG_PTR)this + 0x8);
}

//////////////////////////////////////////////////

PeerData* History::GetPeer()
{
	return *(PeerData**)((ULONG_PTR)this + g::Offsets::HistoryPeer);
}

//////////////////////////////////////////////////

BOOLEAN HistoryMessage::IsMessage()
{
	// HistoryMessage *HistoryItem::toHistoryMessage()
	//
	// Join channel msg is HistoryItem, that's not an inheritance class.
	// It will cause a memory access crash, so we need to filter it out.

	typedef HistoryMessage*(*fntToHistoryMessage)(HistoryMessage *This);
	return Utils::CallVirtual<fntToHistoryMessage>(this, g::Offsets::Index_toHistoryMessage)(this) != NULL;
}

template <class CompT>
CompT* HistoryMessage::GetComponent(ULONG Index)
{
	CompT* Result = NULL;

	Safe::TryExcept(
		[&]()
		{
			PVOID *pData = *(PVOID**)((ULONG_PTR)this + 8);
			INT Offset = *(INT*)((ULONG_PTR)(*pData) + 4 * Index + 8);
			if (Offset >= 4) {
				Result = (CompT*)((ULONG_PTR)pData + Offset);
			}

		}, [&](ULONG ExceptionCode)
		{
			ILogger::GetInstance().TraceWarn("Function: [" __FUNCTION__ "] An exception was caught. Code: [" + Text::Format("0x%x", ExceptionCode) + "] Address: [" + Text::Format("0x%x", this) + "]");
		}
	);

	return Result;
}

HistoryMessageEdited* HistoryMessage::GetEdited()
{
	return GetComponent<HistoryMessageEdited>(g::fnEditedIndex());
}

HistoryMessageSigned* HistoryMessage::GetSigned()
{
	return GetComponent<HistoryMessageSigned>(g::fnSignedIndex());
}

HistoryMessageReply* HistoryMessage::GetReply()
{
	return GetComponent<HistoryMessageReply>(g::fnReplyIndex());
}

History* HistoryMessage::GetHistory()
{
	return *(History**)((ULONG_PTR)this + 0x10);
}

Media* HistoryMessage::GetMedia()
{
	return *(Media**)((ULONG_PTR)this + g::Offsets::Media);
}

BOOLEAN HistoryMessage::IsReply()
{
	return GetReply() != NULL;
}

BOOLEAN HistoryMessage::IsSticker()
{
	if (Media *pMedia = GetMedia()) {
		if (DocumentData *pData = pMedia->GetDocument()) {
			return pData->IsSticker();
		}
	}
	return FALSE;
}

BOOLEAN HistoryMessage::IsLargeEmoji()
{
	// if it's a LargeEmoji, [Item->Media] is nullptr, and [Item->MainView->Media] isn't nullptr.
	// if it's a Video, then [Item->Media] isn't nullptr.

	Media *pMedia = GetMedia();
	if (pMedia != NULL) {
		return FALSE;
	}

	HistoryViewElement *pMainView = GetMainView();
	if (pMainView == NULL) {
		return FALSE;
	}

	return pMainView->GetMedia() != NULL;
}

HistoryViewElement* HistoryMessage::GetMainView()
{
	return *(HistoryViewElement**)((ULONG_PTR)this + g::Offsets::MainView);
}

QtString* HistoryMessage::GetTimeText()
{
	return (QtString*)((ULONG_PTR)this + g::Offsets::TimeText);
}

INT HistoryMessage::GetTimeWidth()
{
	return *(INT*)((ULONG_PTR)this + g::Offsets::TimeWidth);
}
void HistoryMessage::SetTimeWidth(INT Value)
{
	*(INT*)((ULONG_PTR)this + g::Offsets::TimeWidth) = Value;
}

//////////////////////////////////////////////////

QtString* LanguageInstance::GetId()
{
	return (QtString*)((ULONG_PTR)this + 0x4);
}

QtString* LanguageInstance::GetPluralId()
{
	return (QtString*)((ULONG_PTR)this + 0x8);
}

QtString* LanguageInstance::GetName()
{
	return (QtString*)((ULONG_PTR)this + 0x14);
}

QtString* LanguageInstance::GetNativeName()
{
	return (QtString*)((ULONG_PTR)this + 0x18);
}

//////////////////////////////////////////////////
