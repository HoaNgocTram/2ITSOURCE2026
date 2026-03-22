#include "stdafx.h"
#ifdef _GLOBALANNOUNCE
#include "ZGlobalAnnounce.h"

ZGlobalAnnounce::ZGlobalAnnounce()
{
	m_dwTime = 0;
	m_dwTimeDelay = 0;
	Clear();
}

void ZGlobalAnnounce::Clear()
{
	m_szGlobalMsg[0] = NULL;
}
//Custom: This will be seen in Lobby
void ZGlobalAnnounce::DrawAnnounceLobby(MLabel* pLabel)
{
	if (m_dwTimeDelay == 0)
		return;

	if ((timeGetTime() - m_dwTime) < m_dwTimeDelay)
	{
		{
			char szMsg[512] = { 0 };
			sprintf(szMsg, "%s", GetGlobalMessage());

			if (strlen(szMsg) <= 0)
				return;

			pLabel->SetText(szMsg);
			pLabel->SetVisible(true);
		}
	}
	else
	{
		m_dwTimeDelay = 0;
		pLabel->SetVisible(false);
		Clear();
	}
}
//Custom: This will be seen in In-Game
void ZGlobalAnnounce::DrawAnnounce(MDrawContext* pDC)
{
	if (m_dwTimeDelay == 0)
		return;

	if ((timeGetTime() - m_dwTime) < m_dwTimeDelay)
	{
		MFont *pFont = MFontManager::Get("FONTa10_O2Wht");
		if (pFont == NULL)
			return;

		char szMsg[512] = { 0 };
		sprintf(szMsg, "%s", GetGlobalMessage());

		if (strlen(szMsg) <= 0)
			return;

		pDC->SetFont(pFont);
		pDC->SetColor(MCOLOR(0xFF00FF00));
		TextRelative(pDC, 350.f / 800.f, 23.f / 600.f, szMsg);
	}
	else
	{
		m_dwTime = 0;
		m_dwTimeDelay = 0;
		Clear();
	}
}
#endif