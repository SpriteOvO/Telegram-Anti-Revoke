#include "Header.h"
#include "Telegram.h"
#include "Global.h"


void Session::ProcessRevoke(HistoryMessage* pMessage)
{
	SafeExcept([&]()
	{
		if (!pMessage->IsMessage()) {
			return;
		}

		QtString *pTimeText = pMessage->GetTimeText();
		if (pTimeText->IsValidTime())
		{
			SafeMutex(g::hMutex, [&]()
			{
				g::RevokedMessages.push_back(pMessage);
			});

			// g::Logger.TraceInfo("Successful push_back. Address: [" + Text::StringFormatA("0x%x", pMessage) + "]");
		}
		else {
			g::Logger.TraceWarn("A bad TimeText. Address: [" + Text::StringFormatA("0x%x", pMessage) + "]");
		}

	}, [](ULONG ExceptionCode)
	{
		g::Logger.TraceWarn("Function: [" __FUNCTION__ "] An exception was caught. Code: [" + Text::StringFormatA("0x%x", ExceptionCode) + "]");
	});
}

void ProcessItems()
{
	while (true)
	{
		Sleep(1000);

		static vector<HistoryMessage*> RevokedCache;

		SafeMutex(g::hMutex, [&]()
		{
			RevokedCache.assign(g::RevokedMessages.begin(), g::RevokedMessages.end());
		});

		for (HistoryMessage *pMessage : RevokedCache)
		{
			SafeExcept([&]()
			{
				QtString *pTimeText = NULL;
				HistoryMessageEdited *pEdited = pMessage->GetEdited();
				if (pEdited == NULL) {
					// Normal msg
					pTimeText = pMessage->GetTimeText();
				}
				else {
					// Edited msg
					// 编辑过的消息时间字符串不在 Item 内，而是由 EditedComponent 单独管理
					pTimeText = pEdited->GetTimeText();
				}

				if (pTimeText->IsEmpty() || pTimeText->Find(g::CurrentMark.Content) != wstring::npos) {
					// [Empty] This message isn't the current channel. 
					// [Found] This message is marked.
					return;
				}

				// Mark deleted
				wstring MarkedTime = g::CurrentMark.Content + pTimeText->GetText();
				pTimeText->Replace(MarkedTime.c_str());

				// Modify width
				HistoryViewElement *pMainView = pMessage->GetMainView();
				pMainView->SetWidth(pMainView->GetWidth() + g::CurrentMark.Width);
				pMessage->SetTimeWidth(pMessage->GetTimeWidth() + g::CurrentMark.Width);

				// 贴纸 和 LargeEmoji 的宽度是相对右对齐的
				// 所以需要多修改一个宽度，否则会导致消息整体左移。
				if (pMessage->IsSticker() || pMessage->IsLargeEmoji())
				{
					Media *pMainViewMedia = pMainView->GetMedia();
					if (pMainViewMedia != NULL) {
						pMainViewMedia->SetWidth(pMainViewMedia->GetWidth() + g::CurrentMark.Width);
					}
					else {
						// (For sticker) This may not be possible, but it takes time to prove.
						g::Logger.TraceWarn("Function: [" __FUNCTION__ "] MainView is nullptr. Address: [" + Text::StringFormatA("0x%x", pMessage) + "]");
					}
				}

			}, [&](ULONG ExceptionCode)
			{
				g::Logger.TraceWarn("Function: [" __FUNCTION__ "] An exception was caught. Code: [" + Text::StringFormatA("0x%x", ExceptionCode) + "] Address: [" + Text::StringFormatA("0x%x", pMessage) + "]");
			});
		}

		// 每 60 秒释放一次内存（几乎没用。。msg 貌似只会在自己主动删除消息时才会被释放，所以基本不会减少太多容量
		Thread::TimedExecute(60000, [&]()
		{
			vector<HistoryMessage*>().swap(RevokedCache);
		});
	}
}

void __cdecl FakeFree(void *block)
{
	SafeMutex(g::hMutex, [&]()
	{
		// when we delete a msg by ourselves, tg will free this memory block.
		// so, we will earse this msg from the vector.

		vector<HistoryMessage*>::iterator Iterator;

		if (VEC_FIND(Iterator, g::RevokedMessages, block)) {
			g::RevokedMessages.erase(Iterator);
		}
	});

	g::fnOriginalFree(block);
}