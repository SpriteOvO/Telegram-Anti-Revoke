#pragma once

#include <string>

/*
    由于直接引用 Qt 静态库，注入会出现找不到 DLL 文件的情况。
    将 Qt 和 VC Runtime 相关 DLL 都放到 根目录 或 SysWOW64 下依旧找不到（雾
    所以自己实现了一个简单的 QString 类

    Direct reference to the Qt static library will cause the problem that the DLL file cannot be found after injection.
    So I implemented a simple QString class.
*/

struct QtArrayData
{
    int32_t ref;
    int32_t size;
    uint32_t alloc : 31;
    uint32_t capacityReserved : 1;
    ptrdiff_t offset; // in bytes from beginning of header
};

class QtString
{
public:
    QtString();
    QtString(const wchar_t *String);

    bool IsValidTime();
    wchar_t* GetText();
    bool IsEmpty();
    size_t Find(const std::wstring &String);
    int32_t GetRefCount();
    void MakeString(const wchar_t *String);
    void Swap(QtString *Dst);
    void Replace(const wchar_t *NewContent);
    void Clear();

private:
    QtArrayData *d = NULL;

};
