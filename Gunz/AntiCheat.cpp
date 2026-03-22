#include "stdafx.h"
#include "AntiCheat.h"
#include "CriticalSection.h"
#include "MemoryManagement.h"
#include "WindowUI.h"
#ifdef _ANTIHACK
AntiCheat::AntiCheat()
{
	m_hInstance = nullptr;
}

AntiCheat::~AntiCheat()
{
}

typedef struct _UI_THREAD_PARAM
{
	gui::window* pWndPtr;
	bool* pbCompleted;
} SUIThreadParam, * PUIThreadParam;

bool AntiCheat::OnLauncherInit()
{
	auto bRet = false;
	auto bCompletedUIRoutine = false;
	auto pUIThreadTimer = CTimer<std::chrono::milliseconds>();

	// Init UI
	auto LauncherUIRoutine = [](LPVOID lpParam) -> DWORD
	{
		auto param = reinterpret_cast<SUIThreadParam*>(lpParam);
		if (!param)
			return 0;

		auto pLauncherWnd = gui::window::singleton();
		if (!pLauncherWnd)
		{
			mlog("[Anti-Cheat] UI Thread class initilization fail!\n");

			return 0;
		}

		pLauncherWnd->set_instance(ZGetAntiCheat()->GetMainWnd());
		param->pWndPtr = pLauncherWnd;

		if (!pLauncherWnd->assemble("nm_updater_class", "nm_updater", gui::rectangle(0, 0, 290, 118), MAKEINTRESOURCEA(NM_ICON_IMAGE)))
		{
			MLog("[Anti-Cheat] UI Thread assemble initilization fail!\n");
			return 0;
		}
		if (param->pbCompleted) *param->pbCompleted = true;

		gui::execute();
		return 0;
	};

	pUIThreadTimer.reset();
	auto pUIThreadParam = std::make_shared<SUIThreadParam>();
	pUIThreadParam->pbCompleted = &bCompletedUIRoutine;
	auto hThread = CreateThread(nullptr, 0, LauncherUIRoutine, pUIThreadParam.get(), 0, nullptr);

	if (!IS_VALID_HANDLE(hThread))
	{
		MLog("[Anti-Cheat]: Fail Create UI Thread\n");
		goto _Complete;
	}

	MLog("[Anti-Cheat] Launcher UI thread create step completed!\n");

	while (bCompletedUIRoutine == false || !pUIThreadParam->pWndPtr)
	{
		if (pUIThreadTimer.diff() > 5000)
		{
			MLog("[Anti-Cheat] Fail Create UIRoutine.\n");
			goto _Complete;
		}

		Sleep(100);
	}

	mlog("[Anti-Cheat] Launcher UI thread init step completed!\n");

	// Set progressbar position
	pUIThreadParam->pWndPtr->GetProgressBarInstance()->set_position(2);
	Sleep(1000);
	// Set progressbar position
	pUIThreadParam->pWndPtr->GetProgressBarInstance()->set_position(50);
	Sleep(1000);
	// Set progressbar position
	pUIThreadParam->pWndPtr->GetProgressBarInstance()->set_position(75);
	Sleep(1000);
	// Set progressbar position
	pUIThreadParam->pWndPtr->GetProgressBarInstance()->set_position(100);
	Sleep(500);
	bRet = true;
_Complete:

	if (pUIThreadParam->pWndPtr && pUIThreadParam->pWndPtr->get_handle())
		SendMessageA(pUIThreadParam->pWndPtr->get_handle(), WM_CLOSE, NULL, NULL);
	mlog("[Anti-Cheat] Launcher UI destroy step completed!\n");

	mlog("[Anti-Cheat] Launcher routine completed!\n");
	return bRet;
}

void AntiCheat::Main()
{
	MemoryManagement* pMem = new MemoryManagement((HMODULE)SCAN_START, CODE_SIZE);
	auto pTimer = CTimer<std::chrono::milliseconds>();

	while (true) 
	{
		if (pTimer.diff() > 2000)
		{
			if (pMem->MemoryEdited()) 
			{
				if (ZGetGameInterface()) 
				{
					char test[10] = { "Edit" };
					mlog("%s", test);
#ifdef _HWID
					ZPostBanMe(WHT_MEMORY);
#endif
					ExitAPP();
				}
				else {
					char test[5] = { "23" };
					mlog("%s", test);
					exit(NULL);
				}
			}

			pTimer.reset();
		}

		// Reduce cpu usage
		auto dwRandNumber = rand() % 6000;
		Sleep(dwRandNumber);
	}
}

void AntiCheat::SetMainWnd(HINSTANCE hInstance)
{
	m_hInstance = hInstance;
}

HINSTANCE AntiCheat::GetMainWnd()
{
	return m_hInstance;
}

AntiCheat* AntiCheat::GetInstance()
{
	static AntiCheat instance;
	return &instance;
}
#endif