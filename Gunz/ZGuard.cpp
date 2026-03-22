#include "stdafx.h"
#include "ZGuard.h"
#include "windowsx.h"

typedef BOOL(WINAPI* lpfnSetLayeredWindowAttributes)
(HWND hWnd, COLORREF cr, BYTE bAlpha, DWORD dwFlags);

lpfnSetLayeredWindowAttributes g_pSetLayeredWindowAttributes;

#define WS_EX_LAYERED 0x00080000 

static LRESULT CALLBACK ExtWndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	static CSplash* spl = NULL;
	if (uMsg == WM_CREATE)
	{
		spl = (CSplash*)((LPCREATESTRUCT)lParam)->lpCreateParams;
	}
	if (spl)
		return spl->WindowProc(hwnd, uMsg, wParam, lParam);
	else
		return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

LRESULT CALLBACK CSplash::WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
		HANDLE_MSG(hwnd, WM_PAINT, OnPaint);
	}

	return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

void CSplash::OnPaint(HWND hwnd)
{
	if (!m_hBitmap)
		return;

	PAINTSTRUCT ps;
	HDC hDC = BeginPaint(hwnd, &ps);

	RECT   rect;
	::GetClientRect(m_hwnd, &rect);

	HDC hMemDC = ::CreateCompatibleDC(hDC);
	HBITMAP hOldBmp = (HBITMAP)::SelectObject(hMemDC, m_hBitmap);

	BitBlt(hDC, 0, 0, m_dwWidth, m_dwHeight, hMemDC, 0, 0, SRCCOPY);

	::SelectObject(hMemDC, hOldBmp);

	::DeleteDC(hMemDC);

	EndPaint(hwnd, &ps);
}

void CSplash::Init()
{
	m_hwnd = NULL;
	m_lpszClassName = TEXT("SPLASH");
	m_colTrans = 0;
	HMODULE hUser32 = GetModuleHandle(TEXT("USER32.DLL"));

	g_pSetLayeredWindowAttributes = (lpfnSetLayeredWindowAttributes)
		GetProcAddress(hUser32, "SetLayeredWindowAttributes");
}

CSplash::CSplash()
{
	Init();
}

CSplash::CSplash(LPCTSTR lpszFileName, COLORREF colTrans)
{
	Init();

	SetBitmap(lpszFileName);
	SetTransparentColor(colTrans);
}

CSplash::~CSplash()
{
	FreeResources();
}

HWND CSplash::RegAndCreateWindow()
{

	WNDCLASSEX wndclass;
	wndclass.cbSize = sizeof(wndclass);
	wndclass.style = CS_BYTEALIGNCLIENT | CS_BYTEALIGNWINDOW;
	wndclass.lpfnWndProc = ExtWndProc;
	wndclass.cbClsExtra = 0;
	wndclass.cbWndExtra = DLGWINDOWEXTRA;
	wndclass.hInstance = ::GetModuleHandle(NULL);
	wndclass.hIcon = NULL;
	wndclass.hCursor = ::LoadCursor(NULL, IDC_WAIT);
	wndclass.hbrBackground = (HBRUSH)::GetStockObject(LTGRAY_BRUSH);
	wndclass.lpszMenuName = NULL;
	wndclass.lpszClassName = m_lpszClassName;
	wndclass.hIconSm = NULL;

	if (!RegisterClassEx(&wndclass))
		return NULL;

	DWORD nScrWidth = ::GetSystemMetrics(SM_CXFULLSCREEN);
	DWORD nScrHeight = ::GetSystemMetrics(SM_CYFULLSCREEN);

	int x = (nScrWidth - m_dwWidth) / 2;
	int y = (nScrHeight - m_dwHeight) / 2;
	m_hwnd = ::CreateWindowEx(WS_EX_TOPMOST | WS_EX_TOOLWINDOW, m_lpszClassName,
		TEXT("Banner"), WS_POPUP, x, y,
		m_dwWidth, m_dwHeight, NULL, NULL, NULL, this);

	if (m_hwnd)
	{
		MakeTransparent();
		ShowWindow(m_hwnd, SW_SHOW);
		UpdateWindow(m_hwnd);
	}
	return m_hwnd;
}

int CSplash::DoLoop()
{

	if (!m_hwnd)
		ShowSplash();

	MSG msg;
	while (GetMessage(&msg, NULL, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	return msg.wParam;

}

void CSplash::ShowSplash()
{
	CloseSplash();
	RegAndCreateWindow();
}


DWORD CSplash::SetBitmap(LPCTSTR lpszFileName)
{

	HBITMAP    hBitmap = NULL;
	hBitmap = (HBITMAP)::LoadImage(0, lpszFileName, IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
	return SetBitmap(hBitmap);
}

DWORD CSplash::SetBitmap(HBITMAP hBitmap)
{
	int nRetValue;
	BITMAP  csBitmapSize;

	FreeResources();

	if (hBitmap)
	{
		m_hBitmap = hBitmap;
		nRetValue = ::GetObject(hBitmap, sizeof(csBitmapSize), &csBitmapSize);
		if (nRetValue == 0)
		{
			FreeResources();
			return 0;
		}
		m_dwWidth = (DWORD)csBitmapSize.bmWidth;
		m_dwHeight = (DWORD)csBitmapSize.bmHeight;
	}

	return 1;
}

void CSplash::FreeResources()
{
	if (m_hBitmap)
		::DeleteObject(m_hBitmap);
	m_hBitmap = NULL;
}

int CSplash::CloseSplash()
{

	if (m_hwnd)
	{
		DestroyWindow(m_hwnd);
		m_hwnd = 0;
		UnregisterClass(m_lpszClassName, ::GetModuleHandle(NULL));
		return 1;
	}
	return 0;
}

bool CSplash::SetTransparentColor(COLORREF col)
{
	m_colTrans = col;

	return MakeTransparent();
}

bool CSplash::MakeTransparent()
{
	if (m_hwnd && g_pSetLayeredWindowAttributes && m_colTrans)
	{
		SetWindowLong(m_hwnd, GWL_EXSTYLE, GetWindowLong(m_hwnd, GWL_EXSTYLE) | WS_EX_LAYERED);
		g_pSetLayeredWindowAttributes(m_hwnd, m_colTrans, 0, LWA_COLORKEY);
	}
	return TRUE;
}


void SplashShow() {
	CSplash splash1(TEXT(".\\EasyAntiCheat\\Localization\\Protect.bmp"), RGB(128, 128, 128));
	splash1.ShowSplash();
	Sleep(3000);
	splash1.CloseSplash();
}

#ifdef _ANTIHACK
// HITBOX
float ZGuard::HIT_HEAD_15 = 15.f;
float ZGuard::HIT_BODY_30 = 30.f;
float ZGuard::HIT_LEGS_20 = 20.f;

// BULLET COUNT
int ZGuard::SHOTGUN_BULLET_COUNT = 12;

// SHOTGUN RANGE
float ZGuard::SHOTGUN_DIFFUSE_RANGE = 0.1f;

bool IsAddressHookedGuard(unsigned long address) {
	BYTE* offsetValue = (BYTE*)address;
	return (*offsetValue == 0xE8 || *offsetValue == 0xE9 || *offsetValue == 0x7E || *offsetValue == 0x74 || *offsetValue == 0xC3);
}

//void CheckMacroWindows()
//{
//    // Gom tất cả các Define XorStr vào đây
//    const char* szBlacklist[] = { 
//        MEMORYVIEWER, 
//        MACROLG, 
//        MACRORZ, 
//        MACROCS // Thêm các chuỗi khác tương tự
//    };
//
//    int nSize = sizeof(szBlacklist) / sizeof(szBlacklist[0]);
//
//    for (int i = 0; i < nSize; i++)
//    {
//        if (FindWindowA(NULL, szBlacklist[i]))
//        {
//            #ifdef _HWID
//                char Log[64];
//                sprintf(Log, EXIT_TEXT);
//                mlog("Detected: %s - %s\n", szBlacklist[i], Log);
//
//                if (ZApplication::GetGameInterface()) {
//                    ZApplication::GetGameInterface()->ScreenZGuard();
//                }
//                
//                Sleep(5000); // 15s hơi lâu, 5s là đủ để nó nhìn thấy thông báo rồi
//                ExitProcess(NULL);
//            #else
//                ExitProcess(NULL);
//            #endif
//            break;
//        }
//    }
//}

void ZGuard::GuardInit()
{
	SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_LOWEST);

	MemoryManagement* pMem = new MemoryManagement((HMODULE)SCAN_START, CODE_SIZE);

	DWORD getTickCount = (DWORD)GetProcAddress(GetModuleHandleA(KERNEL32), GETTICKCOUNT);
	DWORD queryPerformanceCounter = (DWORD)GetProcAddress(GetModuleHandleA(KERNEL32), QUERYPERFORMANCECOUNTER);
	DWORD timeGetTime = (DWORD)GetProcAddress(GetModuleHandleA(WINMM), TIMEGETTIME);

	while (true)
	{
		/////////////////////////////////////////////////////////////////////////////////////////////////////////
		// CHECK
		if (getTickCount != (DWORD)GetProcAddress(GetModuleHandleA(KERNEL32), GETTICKCOUNT)
			|| queryPerformanceCounter != (DWORD)GetProcAddress(GetModuleHandleA(KERNEL32), QUERYPERFORMANCECOUNTER)
			|| timeGetTime != (DWORD)GetProcAddress(GetModuleHandleA(WINMM), TIMEGETTIME)) {
#ifdef _HWID
			if (ZGetGameInterface())
			{
				ZPostBanMe(WHT_UNDERCLOCK);
			}
			else
			{
				char Log[64];
				sprintf(Log, EXIT_TEXT);
				mlog("%s\n", Log);
				ZApplication::GetGameInterface()->ScreenZGuard();
				Sleep(15000);
				ExitProcess(NULL);
			}
#else
			ExitProcess(NULL);
#endif
		}
		/////////////////////////////////////////////////////////////////////////////////////////////////////////
		// UNDERLOCK
		if (IsAddressHookedGuard(getTickCount) || IsAddressHookedGuard(queryPerformanceCounter) || IsAddressHookedGuard(timeGetTime)) {
#ifdef _HWID
			if (ZGetGameInterface())
			{
				ZPostBanMe(WHT_UNDERCLOCK);
			}
			else
			{
				char Log[64];
				sprintf(Log, EXIT_TEXT);
				mlog("%s\n", Log);
				ZApplication::GetGameInterface()->ScreenZGuard();
				Sleep(15000);
				ExitProcess(NULL);
			}
#else
			ExitProcess(NULL);
#endif
		}
		/////////////////////////////////////////////////////////////////////////////////////////////////////////
		// HOOKS
		if (GetModuleHandleA(WIREFRAMEDLL) != NULL || GetModuleHandleA(HOOKDLL) != NULL || GetModuleHandleA(SOFTSPDLL) != NULL || GetModuleHandleA(SPEEDHACKDLL) != NULL || GetModuleHandleA(MYHOOKDLL) != NULL) {
#ifdef _HWID
			if (ZGetGameInterface())
			{
				ZPostBanMe(WHT_HOOKS);
			}
			else
			{
				char Log[64];
				sprintf(Log, EXIT_TEXT);
				mlog("%s\n", Log);
				ZApplication::GetGameInterface()->ScreenZGuard();
				Sleep(15000);
				ExitProcess(NULL);
			}
#else
			ExitProcess(NULL);
#endif
		}
		/////////////////////////////////////////////////////////////////////////////////////////////////////////
		// CHEAT ENGINE
		if (FindWindow(NULL, MEMORYVIEWER))
		{
#ifdef _HWID
			/*if (ZGetGameInterface())
			{
				ZPostBanMe(WHT_CHEATENGINE);
			}
			else
			{*/
				char Log[64];
				sprintf(Log, EXIT_TEXT);
				mlog("%s\n", Log);
				ZApplication::GetGameInterface()->ScreenZGuard();
				Sleep(15000);
				ExitProcess(NULL);
			/*}*/
#else
			ExitProcess(NULL);
#endif
		}
		///////////////////////////////////////////////////////////////////////////////////////////////////////////
		// MACRO
		if (FindWindow(NULL, MACRORZ) || FindWindow(NULL, MACROCS) || FindWindow(NULL, MACROLG))
		{
#ifdef _HWID
			/*if (ZGetGameInterface())
			{
				ZPostBanMe(WHT_CHEATENGINE);
			}
			else
			{*/
			char Log[64];
			sprintf(Log, EXIT_TEXT);
			mlog("%s\n", Log);
			ZApplication::GetGameInterface()->ScreenZGuard();
			Sleep(15000);
			ExitProcess(NULL);
			/*}*/
#else
			ExitProcess(NULL);
#endif
		}
		/////////////////////////////////////////////////////////////////////////////////////////////////////////
		// PROGRAMS
		if (FindWindow(NULL, AUTOITV3) || FindWindow(THUNDERRT6FORMDC, DXKSAUTO) || (OpenMutex(SYNCHRONIZE, FALSE, SBMUTEX) != NULL)) {
#ifdef _HWID
			if (ZGetGameInterface())
			{
				ZPostBanMe(WHT_PROGRAMS);
			}
			else
			{
				char Log[64];
				sprintf(Log, EXIT_TEXT);
				mlog("%s\n", Log);
				ZApplication::GetGameInterface()->ScreenZGuard();
				Sleep(15000);
				ExitProcess(NULL);
			}
#else
			ExitProcess(NULL);
#endif
		}
		/////////////////////////////////////////////////////////////////////////////////////////////////////////
		// TASK
		if (FindWindow(NULL, TASK) || FindWindow(NULL, PL) || FindWindow(NULL, TCP) || FindWindow(NULL, CURR) != NULL) {
#ifdef _HWID
				char Log[64];
				sprintf(Log, EXIT_TEXT);
				mlog("%s\n", Log);
				ZApplication::GetGameInterface()->ScreenZGuard();
				Sleep(15000);
				ExitProcess(NULL);
			
#else
			ExitProcess(NULL);
#endif
		}
		/////////////////////////////////////////////////////////////////////////////////////////////////////////
		// HITBOX
		if (ZGuard::HIT_HEAD_15 != 15.f || ZGuard::HIT_BODY_30 != 30.f || ZGuard::HIT_LEGS_20 != 20.f)
		{
			if (!ZGetMyInfo()->IsAdminGrade())
			{
#ifdef _HWID
				if (ZGetGameInterface())
				{
					ZPostBanMe(WHT_HITBOX);
				}
				else
				{
					char Log[64];
					sprintf(Log, EXIT_TEXT);
					mlog("%s\n", Log);
					ZApplication::GetGameInterface()->ScreenZGuard();
					Sleep(15000);
					ExitProcess(NULL);
				}
#else
				ExitProcess(NULL);
#endif
			}
		}
		/////////////////////////////////////////////////////////////////////////////////////////////////////////
		// BULLET COUNT
		if (ZGuard::SHOTGUN_BULLET_COUNT != 12)
		{
			if (!ZGetMyInfo()->IsAdminGrade())
			{
#ifdef _HWID
				if (ZGetGameInterface())
				{
					ZPostBanMe(WHT_BULLET);
				}
				else
				{
					char Log[64];
					sprintf(Log, EXIT_TEXT);
					mlog("%s\n", Log);
					ZApplication::GetGameInterface()->ScreenZGuard();
					Sleep(15000);
					ExitProcess(NULL);
				}
#else
				ExitProcess(NULL);
#endif
			}
		}
		/////////////////////////////////////////////////////////////////////////////////////////////////////////
		// SHOTGUN RANGE
		if (ZGuard::SHOTGUN_DIFFUSE_RANGE != 0.1f)
		{
			if (!ZGetMyInfo()->IsAdminGrade())
			{
#ifdef _HWID
				if (ZGetGameInterface())
				{
					ZPostBanMe(WHT_RANGE);
				}
				else
				{
					char Log[64];
					sprintf(Log, EXIT_TEXT);
					mlog("%s\n", Log);
					ZApplication::GetGameInterface()->ScreenZGuard();
					Sleep(15000);
					ExitProcess(NULL);
				}
#else
				ExitProcess(NULL);
#endif
			}
		}
		/////////////////////////////////////////////////////////////////////////////////////////////////////////
		// HP/AP
		if (ZGetGame() && ZGetGame()->m_pMyCharacter != NULL)
		{
			if (!ZGetMyInfo()->IsAdminGrade())
			{
				if (!ZGetGameClient()->GetMatchStageSetting()->IsModifierUsed(MMOD_ROLLTHEDICE))
				{
					if (!ZGetGame()->GetMatch()->GetMatchType() == MMATCH_GAMETYPE_MODE_STAFF)
					{
						if (!ZGetGame()->GetMatch()->GetMatchType() == MMATCH_GAMETYPE_VAMPIRE)
						{
							if ((!ZGetGameClient()->GetMatchStageSetting()->IsForcedHPAP()))
							{
								{
									if (((int)(ZGetGame()->m_pMyCharacter->GetHP()) > 132) || (((int)ZGetGame()->m_pMyCharacter->GetAP()) > 132))
									{
#ifdef _HWID
										if (ZGetGameInterface())
										{
											ZApplication::GetGameInterface()->ScreenZGuard();
											ZPostBanMe(WHT_HPAP);
											Sleep(15000);
											ExitProcess(NULL);
										}
#else
										ExitProcess(NULL);
#endif
									}
								}
							}
						}
					}
				}
			}
		}
	}
}
#endif