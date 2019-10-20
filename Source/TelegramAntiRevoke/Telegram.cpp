#include "Header.h"
#include "Global.h"
#include "QtString.h"
#include "Telegram.h"


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

BOOLEAN HistoryMessage::IsMessage()
{
	// HistoryMessage *HistoryItem::toHistoryMessage()
	//
	// Join channel msg is HistoryItem, that's not an inheritance class.
	// It will cause a memory access crash, so we need to filter it out.

	typedef HistoryMessage*(*fntToHistoryMessage)(HistoryMessage *This, ULONG a);
	return Utils::CallVirtual<fntToHistoryMessage>(this, 33)(this, 1) != NULL;
}

HistoryMessageEdited* HistoryMessage::GetEdited()
{
	HistoryMessageEdited* Result = NULL;

	SafeExcept([&]()
	{
		PVOID *pData = *(PVOID**)((ULONG_PTR)this + 8);
		INT Offset = *(INT*)((ULONG_PTR)(*pData) + 4 * g::fnGetEditedIndex() + 8);
		if (Offset < 4) {
			Result = NULL;
		}
		else {
			Result = (HistoryMessageEdited*)((ULONG_PTR)pData + Offset);
		}

	}, [&](ULONG ExceptionCode)
	{
		g::Logger.TraceWarn("Function: [" __FUNCTION__ "] An exception was caught. Code: [" + Text::StringFormatA("0x%x", ExceptionCode) + "] Address: [" + Text::StringFormatA("0x%x", this) + "]");
		// MessageBoxA(NULL, ("Function: [" __FUNCTION__ "] An exception was caught. Code: [" + Text::StringFormatA("0x%x", ExceptionCode) + "]").c_str(), "", 0);
	});

	return Result;
}

Media* HistoryMessage::GetMedia()
{
	return *(Media**)((ULONG_PTR)this + 0x54);
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
	return *(HistoryViewElement**)((ULONG_PTR)this + 0x5C);
}

QtString* HistoryMessage::GetTimeText()
{
	return (QtString*)((ULONG_PTR)this + 0x98);
}

INT HistoryMessage::GetTimeWidth()
{
	return *(INT*)((ULONG_PTR)this + 0x9C);
}
void HistoryMessage::SetTimeWidth(INT Value)
{
	*(INT*)((ULONG_PTR)this + 0x9C) = Value;
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
