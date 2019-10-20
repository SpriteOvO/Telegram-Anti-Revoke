#pragma once

//#include <QtCore/qstring.h>
//#pragma comment(lib, "Qt5Cored.lib")

/*
	由于直接引用 Qt 静态库，注入会出现找不到 DLL 文件的情况。
	将 Qt 和 VC Runtime 相关 DLL 都放到 根目录 或 SysWOW64 下依旧找不到（雾
	所以自己实现了一个简单的 QString 类
*/

struct QtArrayData
{
	INT ref;
	INT size;
	UINT alloc : 31;
	UINT capacityReserved : 1;
	INT offset; // in bytes from beginning of header
};

class QtString
{
public:
	QtString();
	QtString(const WCHAR *String);

	BOOLEAN IsValidTime();
	WCHAR* GetText();
	BOOLEAN IsEmpty();
	SIZE_T Find(wstring String);
	INT GetRefCount();
	void MakeString(const WCHAR *String);
	void Swap(QtString *Dst);
	void Replace(const WCHAR *NewContent);
	void Clear();

private:
	QtArrayData *d = NULL;

};