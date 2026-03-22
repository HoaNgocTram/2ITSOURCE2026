#ifndef _ZGLOBALANNOUNCE_H
#define _ZGLOBALANNOUNCE_H

#ifdef _GLOBALANNOUNCE
#include <string>
using namespace std;

class MDrawContext;

//Custom: 5 second
#define TIME_DELAY	5000

class ZGlobalAnnounce {
protected:
	char			m_szGlobalMsg[128];
	DWORD			m_dwTime;
	DWORD			m_dwTimeDelay;

public:
	ZGlobalAnnounce();
	~ZGlobalAnnounce()					{ Clear(); }

	const char* GetGlobalMessage()						{ return m_szGlobalMsg; }
	void SetGlobalMessage(const char* pszMessage)		{ strcpy(m_szGlobalMsg, pszMessage); m_dwTime = timeGetTime(); m_dwTimeDelay = TIME_DELAY; }
	void Clear();

	void DrawAnnounceLobby(MLabel* pLabel);
	void DrawAnnounce(MDrawContext* pDC);
};
#endif
#endif
