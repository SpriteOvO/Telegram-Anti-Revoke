#include "Header.h"
#include "QtString.h"
#include "Global.h"


QtString::QtString()
{
}

QtString::QtString(const WCHAR *String)
{
	MakeString(String);
}

BOOLEAN QtString::IsValidTime()
{
	// Check valid
	return d != NULL && !IsBadReadPtr(d, sizeof(PVOID)) && !IsBadReadPtr((PVOID)((ULONG_PTR)d + d->offset), sizeof(QtArrayData) + 12) &&
		GetRefCount() <= 1 &&
		wcslen(GetText()) <= 8; // Fixed for 12h format. ("12:34 AM" / "12:34 PM")
}

WCHAR* QtString::GetText()
{
	return (WCHAR*)((ULONG_PTR)d + d->offset);
}

BOOLEAN QtString::IsEmpty()
{
	return wcscmp(GetText(), L"") == 0;
}

//INT QtString::Find(const WCHAR *String)
//{
//	WCHAR *TimeText = GetText();
//	WCHAR *Result = wcsstr(TimeText, String);
//
//	return (Result == NULL) ? -1 : (Result - TimeText);
//}

SIZE_T QtString::Find(wstring String)
{
	return wstring(GetText()).find(String);
}

INT QtString::GetRefCount()
{
	return d->ref;
}

void QtString::MakeString(const WCHAR *String)
{
	SIZE_T Length = wcslen(String);
	SIZE_T StrBytes = (Length + 1) * sizeof(WCHAR);
	SIZE_T DataBytes = sizeof(QtArrayData) + StrBytes;

	// Thanks to [dummycatz] for pointing out the reason for the error in malloc/free memory across modules.
	d = (QtArrayData *)g::fnMalloc(DataBytes);
	RtlZeroMemory(d, DataBytes);

	d->ref = 1;
	d->size = Length;
	d->alloc = Length + 1;
	d->capacityReserved = 0;
	d->offset = sizeof(QtArrayData);

	RtlCopyMemory(GetText(), String, StrBytes);
}

void QtString::Swap(QtString *Dst)
{
	QtArrayData *SaveData = Dst->d;

	Dst->d = d;
	d = SaveData;
}

void QtString::Replace(const WCHAR *NewContent)
{
	QtString NewText(NewContent);
	NewText.Swap(this);
	NewText.Clear();
}

void QtString::Clear()
{
	g::fnOriginalFree(d);
}
