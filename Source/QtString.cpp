#include "QtString.h"
#include "IRuntime.h"
#include "IAntiRevoke.h"


QtString::QtString()
{
}

QtString::QtString(const wchar_t *String)
{
    MakeString(String);
}

bool QtString::IsValidTime()
{
    // Check valid
    return d != NULL && !IsBadReadPtr(d, sizeof(void*)) && !IsBadReadPtr((void*)((uintptr_t)d + d->offset), sizeof(QtArrayData) + 12) &&
        GetRefCount() <= 1 &&
        wcslen(GetText()) <= 8; // Fixed for 12h format. ("12:34 AM" / "12:34 PM")
}

wchar_t* QtString::GetText()
{
    return (wchar_t*)((uintptr_t)d + d->offset);
}

bool QtString::IsEmpty()
{
    return wcscmp(GetText(), L"") == 0;
}

size_t QtString::Find(const std::wstring &String)
{
    return std::wstring(GetText()).find(String);
}

int32_t QtString::GetRefCount()
{
    return d->ref;
}

void QtString::MakeString(const wchar_t *String)
{
    size_t Length = wcslen(String);
    size_t StrBytes = (Length + 1) * sizeof(wchar_t);
    size_t DataBytes = sizeof(QtArrayData) + StrBytes;

    // Thanks to [dummycatz] for pointing out the cause of cross-module malloc/free crash.
    //
    d = (QtArrayData *)IRuntime::GetInstance().GetData().Function.Malloc(DataBytes);
    memset(d, 0, DataBytes);

    d->ref = 1;
    d->size = Length;
    d->alloc = Length + 1;
    d->capacityReserved = 0;
    d->offset = sizeof(QtArrayData);

    memcpy(GetText(), String, StrBytes);
}

void QtString::Swap(QtString *Dst)
{
    QtArrayData *SaveData = Dst->d;

    Dst->d = d;
    d = SaveData;
}

void QtString::Replace(const wchar_t *NewContent)
{
    QtString NewText(NewContent);
    NewText.Swap(this);
    NewText.Clear();
}

void QtString::Clear()
{
    IAntiRevoke::GetInstance().CallFree(d);
}
