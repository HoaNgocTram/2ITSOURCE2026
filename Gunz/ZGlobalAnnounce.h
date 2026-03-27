#ifndef _ZGLOBALANNOUNCE_H
#define _ZGLOBALANNOUNCE_H

#ifdef _GLOBALANNOUNCE
#include <string>

class MDrawContext;
class MFont;
struct EmojiEntry;

#define ANNOUNCE_MAX_COLORS		8

struct ZAnnounceConfig {
	char	szFontName[64];
	DWORD	dwTextColor;
	DWORD	dwBgColor;
	DWORD	dwBorderColor;
	DWORD	dwShadowColor;
	float	fPosY;
	float	fBoxWidth;
	int		nBoxHeight;
	int		nDisplayTime;
	int		nFadeTime;
	float	fScrollSpeed;
	bool	bShadow;
	int		nShadowOffset;
	int		nBoxPadding;
	int		nScrollPause;
	int		nRepeatCount;			// Số lần lặp cuộn cho text dài (1 = chạy 1 lần)
	bool	bColorCycle;			// Bật nhấp nháy màu
	int		nColorCycleSpeed;		// ms chuyển từ màu này sang màu kế (VD: 300)
	DWORD	dwCycleColors[ANNOUNCE_MAX_COLORS];
	int		nCycleColorCount;

	ZAnnounceConfig() {
		strcpy(szFontName, "FONTa10_O2Wht");
		dwTextColor = 0xFFFFFF00;
		dwBgColor = 0xC0000000;
		dwBorderColor = 0xFF888888;
		dwShadowColor = 0xAA000000;
		fPosY = 0.04f;
		fBoxWidth = 0.50f;
		nBoxHeight = 30;
		nDisplayTime = 8000;
		nFadeTime = 500;
		fScrollSpeed = 100.0f;
		bShadow = true;
		nShadowOffset = 1;
		nBoxPadding = 10;
		nScrollPause = 1000;
		nRepeatCount = 1;
		bColorCycle = false;
		nColorCycleSpeed = 300;
		nCycleColorCount = 0;
	}
};

class ZGlobalAnnounce {
protected:
	char			m_szGlobalMsg[512];
	DWORD			m_dwStartTime;
	bool			m_bActive;
	float			m_fScrollOffset;
	DWORD			m_dwLastUpdateTime;
	int				m_nTotalDisplayTime;
	int				m_nSingleScrollTime;	// Thời gian 1 lần cuộn (ms)
	bool			m_bNeedScroll;
	int				m_nScrollDist;

	ZAnnounceConfig	m_Config;

	float CalcAlpha(DWORD dwNow);
	DWORD ApplyAlpha(DWORD dwColor, float fAlpha);
	DWORD GetCyclingColor(DWORD dwNow, float fAlpha);
	int CalcRenderWidth(MFont* pFont);
	void DrawTextWithEmoji(MDrawContext* pDC, int nStartX, int nTextY, DWORD dwTextColor, DWORD dwShadowColor);
	void DrawEmojiStatic(MDrawContext* pDC, const EmojiEntry* pEntry, int x, int y, int w, int h);
	void DrawEmojiAnimated(const EmojiEntry* pEntry, int x, int y, int w, int h);

public:
	ZGlobalAnnounce();
	~ZGlobalAnnounce() { Clear(); }

	const char* GetGlobalMessage() { return m_szGlobalMsg; }
	bool IsActive() { return m_bActive; }

	void SetGlobalMessage(const char* pszMessage);
	void Clear();
	void DrawAnnounce(MDrawContext* pDC);
	bool LoadConfig(const char* szFileName = "interface/default/announce.xml");
	ZAnnounceConfig& GetConfig() { return m_Config; }
};

#endif
#endif