#include "stdafx.h"
#ifdef _GLOBALANNOUNCE
#include "ZGlobalAnnounce.h"
#include "EmojiManager.h"
#include "AnimatedGifTexture.h"
#include "ZConfiguration.h"
#include "RealSpace2.h"

// ============================================================
//  Helpers
// ============================================================

static BYTE LerpByte(BYTE a, BYTE b, float t)
{
	return (BYTE)((float)a + ((float)b - (float)a) * t);
}

static DWORD LerpColor(DWORD c1, DWORD c2, float t)
{
	if (t <= 0.0f) return c1;
	if (t >= 1.0f) return c2;
	BYTE a = LerpByte((c1 >> 24) & 0xFF, (c2 >> 24) & 0xFF, t);
	BYTE r = LerpByte((c1 >> 16) & 0xFF, (c2 >> 16) & 0xFF, t);
	BYTE g = LerpByte((c1 >> 8) & 0xFF, (c2 >> 8) & 0xFF, t);
	BYTE b = LerpByte((c1) & 0xFF, (c2) & 0xFF, t);
	return ((DWORD)a << 24) | ((DWORD)r << 16) | ((DWORD)g << 8) | (DWORD)b;
}

// ============================================================
//  Constructor / Clear / SetGlobalMessage
// ============================================================

ZGlobalAnnounce::ZGlobalAnnounce()
{
	m_bActive = false;
	m_dwStartTime = 0;
	m_dwLastUpdateTime = 0;
	m_fScrollOffset = 0;
	m_nTotalDisplayTime = 0;
	m_nSingleScrollTime = 0;
	m_bNeedScroll = false;
	m_nScrollDist = 0;
	m_szGlobalMsg[0] = 0;
	LoadConfig();
}

void ZGlobalAnnounce::Clear()
{
	m_szGlobalMsg[0] = 0;
	m_bActive = false;
	m_fScrollOffset = 0;
	m_nTotalDisplayTime = 0;
	m_nSingleScrollTime = 0;
	m_bNeedScroll = false;
	m_nScrollDist = 0;
}

void ZGlobalAnnounce::SetGlobalMessage(const char* pszMessage)
{
	if (!pszMessage || strlen(pszMessage) == 0)
		return;

	strncpy(m_szGlobalMsg, pszMessage, sizeof(m_szGlobalMsg) - 1);
	m_szGlobalMsg[sizeof(m_szGlobalMsg) - 1] = 0;

	m_bActive = true;
	m_dwStartTime = timeGetTime();
	m_dwLastUpdateTime = m_dwStartTime;
	m_fScrollOffset = 0;

	// Tính render width (có tính emoji)
	MFont* pFont = MFontManager::Get(m_Config.szFontName);
	if (!pFont) pFont = MFontManager::Get("FONTa10_O2Wht");

	int nScreenW = MGetWorkspaceWidth();
	int nScreenH = MGetWorkspaceHeight();
	float fScale = (float)nScreenH / 600.0f;
	int nPadScaled = (int)(m_Config.nBoxPadding * fScale);
	int nBoxInnerW = (int)(m_Config.fBoxWidth * (float)nScreenW) - (nPadScaled * 2);
	int nTextW = pFont ? CalcRenderWidth(pFont) : 200;

	m_bNeedScroll = (nTextW > nBoxInnerW);

	if (!m_bNeedScroll)
	{
		// Text vừa hộp → hiển thị theo thời gian cấu hình
		m_nTotalDisplayTime = m_Config.nDisplayTime;
		m_nScrollDist = 0;
		m_nSingleScrollTime = 0;
	}
	else
	{
		// Text dài → tính thời gian cuộn
		m_nScrollDist = nTextW - nBoxInnerW;
		int nFade = m_Config.nFadeTime;
		int nPause = m_Config.nScrollPause;
		int nScrollTime = (int)((float)m_nScrollDist / m_Config.fScrollSpeed * 1000.0f);

		// 1 lần cuộn = fade_in + pause + scroll + pause + fade_out(chỉ lần cuối)
		m_nSingleScrollTime = nPause + nScrollTime + nPause;

		// Tổng = fade_in + (repeat * single_scroll) + fade_out
		m_nTotalDisplayTime = nFade + (m_Config.nRepeatCount * m_nSingleScrollTime) + nFade;
	}
}

// ============================================================
//  Render width tính cả emoji
// ============================================================

int ZGlobalAnnounce::CalcRenderWidth(MFont* pFont)
{
	if (!pFont) return 0;
	const char* szText = m_szGlobalMsg;
	int nLen = (int)strlen(szText);
	int totalW = 0;
	int segStart = 0;
	char szSeg[512];

	bool bEmoji = ZGetConfiguration()->GetEtc()->bEmote && ZGetEmojiManager().IsLoaded();

	for (int i = 0; i < nLen; )
	{
		if (bEmoji)
		{
			EmojiMatchResult match = ZGetEmojiManager().FindEmoji(szText, i, nLen);
			if (match.pEntry)
			{
				// Đo segment text trước emoji
				if (i > segStart)
				{
					int segLen = i - segStart;
					if (segLen >= (int)sizeof(szSeg)) segLen = sizeof(szSeg) - 1;
					strncpy(szSeg, szText + segStart, segLen);
					szSeg[segLen] = 0;
					totalW += pFont->GetWidth(szSeg);
				}
				// Cộng width emoji
				int nEmojiW = match.pEntry->nWidth > 0 ? match.pEntry->nWidth : ZGetEmojiManager().GetSize(false);
				totalW += nEmojiW;
				i += match.nPatternLen;
				segStart = i;
				continue;
			}
		}
		i++;
	}

	// Segment còn lại
	if (segStart < nLen)
	{
		int segLen = nLen - segStart;
		if (segLen >= (int)sizeof(szSeg)) segLen = sizeof(szSeg) - 1;
		strncpy(szSeg, szText + segStart, segLen);
		szSeg[segLen] = 0;
		totalW += pFont->GetWidth(szSeg);
	}

	return totalW;
}

// ============================================================
//  Alpha & Color
// ============================================================

float ZGlobalAnnounce::CalcAlpha(DWORD dwNow)
{
	int dwElapsed = (int)(dwNow - m_dwStartTime);
	int nFade = m_Config.nFadeTime;

	if (dwElapsed >= m_nTotalDisplayTime)
		return 0.0f;
	if (dwElapsed < nFade)
		return (float)dwElapsed / (float)nFade;
	if (dwElapsed > (m_nTotalDisplayTime - nFade))
		return (float)(m_nTotalDisplayTime - dwElapsed) / (float)nFade;
	return 1.0f;
}

DWORD ZGlobalAnnounce::ApplyAlpha(DWORD dwColor, float fAlpha)
{
	int a = (int)((float)((dwColor >> 24) & 0xFF) * fAlpha);
	if (a > 255) a = 255;
	if (a < 0) a = 0;
	return ((DWORD)a << 24) | (dwColor & 0x00FFFFFF);
}

DWORD ZGlobalAnnounce::GetCyclingColor(DWORD dwNow, float fAlpha)
{
	if (!m_Config.bColorCycle || m_Config.nCycleColorCount < 2)
		return ApplyAlpha(m_Config.dwTextColor, fAlpha);

	// Tính vị trí trong chu kỳ màu
	int nCycleLen = m_Config.nColorCycleSpeed * m_Config.nCycleColorCount;
	int nTime = (int)(dwNow % (DWORD)nCycleLen);

	int nColorIdx = nTime / m_Config.nColorCycleSpeed;
	float fT = (float)(nTime % m_Config.nColorCycleSpeed) / (float)m_Config.nColorCycleSpeed;

	int nNext = (nColorIdx + 1) % m_Config.nCycleColorCount;
	DWORD dwColor = LerpColor(m_Config.dwCycleColors[nColorIdx], m_Config.dwCycleColors[nNext], fT);

	return ApplyAlpha(dwColor, fAlpha);
}

// ============================================================
//  Emoji rendering helpers
// ============================================================

void ZGlobalAnnounce::DrawEmojiStatic(MDrawContext* pDC, const EmojiEntry* pEntry, int x, int y, int w, int h)
{
	MBitmap* pBmp = MBitmapManager::Get(pEntry->filename.c_str());
	if (pBmp)
	{
		pDC->SetBitmap(pBmp);
		pDC->Draw(x, y, w, h);
	}
}

void ZGlobalAnnounce::DrawEmojiAnimated(const EmojiEntry* pEntry, int x, int y, int w, int h)
{
	AnimatedGifTexture* pAnimTex = ZGetEmojiManager().GetAnimatedTexture(pEntry->filename.c_str());
	if (!pAnimTex || !pAnimTex->IsValid()) return;

	LPDIRECT3DTEXTURE9 pFrameTex = pAnimTex->GetCurrentFrameTexture();
	if (!pFrameTex) return;

	LPDIRECT3DDEVICE9 pd3dDevice = RGetDevice();
	if (!pd3dDevice) return;

	float fx = (float)x, fy = (float)y, fw = (float)w, fh = (float)h;

	// Save render states
	DWORD dwPrevAB, dwPrevSB, dwPrevDB, dwPrevCO, dwPrevCA, dwPrevAO, dwPrevAA;
	pd3dDevice->GetRenderState(D3DRS_ALPHABLENDENABLE, &dwPrevAB);
	pd3dDevice->GetRenderState(D3DRS_SRCBLEND, &dwPrevSB);
	pd3dDevice->GetRenderState(D3DRS_DESTBLEND, &dwPrevDB);
	pd3dDevice->GetTextureStageState(0, D3DTSS_COLOROP, &dwPrevCO);
	pd3dDevice->GetTextureStageState(0, D3DTSS_COLORARG1, &dwPrevCA);
	pd3dDevice->GetTextureStageState(0, D3DTSS_ALPHAOP, &dwPrevAO);
	pd3dDevice->GetTextureStageState(0, D3DTSS_ALPHAARG1, &dwPrevAA);

	pd3dDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
	pd3dDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
	pd3dDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
	pd3dDevice->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_SELECTARG1);
	pd3dDevice->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
	pd3dDevice->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);
	pd3dDevice->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);

	pd3dDevice->SetTexture(0, pFrameTex);
	pd3dDevice->SetFVF(D3DFVF_XYZRHW | D3DFVF_TEX1);

	struct GIFVERT { float x, y, z, rhw, u, v; };
	GIFVERT verts[4] = {
		{ fx,      fy,      0.f, 1.f, 0.f, 0.f },
		{ fx + fw, fy,      0.f, 1.f, 1.f, 0.f },
		{ fx + fw, fy + fh, 0.f, 1.f, 1.f, 1.f },
		{ fx,      fy + fh, 0.f, 1.f, 0.f, 1.f },
	};
	pd3dDevice->DrawPrimitiveUP(D3DPT_TRIANGLEFAN, 2, verts, sizeof(GIFVERT));

	pd3dDevice->SetTexture(0, nullptr);
	pd3dDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, dwPrevAB);
	pd3dDevice->SetRenderState(D3DRS_SRCBLEND, dwPrevSB);
	pd3dDevice->SetRenderState(D3DRS_DESTBLEND, dwPrevDB);
	pd3dDevice->SetTextureStageState(0, D3DTSS_COLOROP, dwPrevCO);
	pd3dDevice->SetTextureStageState(0, D3DTSS_COLORARG1, dwPrevCA);
	pd3dDevice->SetTextureStageState(0, D3DTSS_ALPHAOP, dwPrevAO);
	pd3dDevice->SetTextureStageState(0, D3DTSS_ALPHAARG1, dwPrevAA);
}

// ============================================================
//  Vẽ text từng đoạn xen kẽ emoji
// ============================================================

void ZGlobalAnnounce::DrawTextWithEmoji(MDrawContext* pDC, int nStartX, int nTextY,
	DWORD dwTextColor, DWORD dwShadowColor)
{
	MFont* pFont = pDC->GetFont();
	if (!pFont) return;

	const char* szText = m_szGlobalMsg;
	int nLen = (int)strlen(szText);
	int nCurX = nStartX;
	int nFontH = pFont->GetHeight();
	char szSeg[512];
	int segStart = 0;

	bool bEmoji = ZGetConfiguration()->GetEtc()->bEmote && ZGetEmojiManager().IsLoaded();
	bool bShadow = m_Config.bShadow;

	for (int i = 0; i < nLen; )
	{
		bool bFoundEmoji = false;

		if (bEmoji)
		{
			EmojiMatchResult match = ZGetEmojiManager().FindEmoji(szText, i, nLen);
			if (match.pEntry)
			{
				bFoundEmoji = true;

				// Vẽ text tích lũy trước emoji
				if (i > segStart)
				{
					int segLen = i - segStart;
					if (segLen >= (int)sizeof(szSeg)) segLen = sizeof(szSeg) - 1;
					strncpy(szSeg, szText + segStart, segLen);
					szSeg[segLen] = 0;

					if (bShadow)
					{
						pDC->SetColor(MCOLOR(dwShadowColor));
						pDC->Text(nCurX + m_Config.nShadowOffset, nTextY + m_Config.nShadowOffset, szSeg);
					}
					pDC->SetColor(MCOLOR(dwTextColor));
					pDC->Text(nCurX, nTextY, szSeg);
					nCurX += pFont->GetWidth(szSeg);
				}

				// Vẽ emoji
				int nEmojiW = match.pEntry->nWidth > 0 ? match.pEntry->nWidth : ZGetEmojiManager().GetSize(false);
				int nEmojiH = match.pEntry->nHeight > 0 ? match.pEntry->nHeight : nEmojiW;
				int emojiY = nTextY + (nFontH - nEmojiH) / 2;

				if (match.pEntry->bAnimated)
					DrawEmojiAnimated(match.pEntry, nCurX, emojiY, nEmojiW, nEmojiH);
				else
					DrawEmojiStatic(pDC, match.pEntry, nCurX, emojiY, nEmojiW, nEmojiH);

				nCurX += nEmojiW;
				i += match.nPatternLen;
				segStart = i;
				continue;
			}
		}

		if (!bFoundEmoji)
			i++;
	}

	// Vẽ text còn lại
	if (segStart < nLen)
	{
		int segLen = nLen - segStart;
		if (segLen >= (int)sizeof(szSeg)) segLen = sizeof(szSeg) - 1;
		strncpy(szSeg, szText + segStart, segLen);
		szSeg[segLen] = 0;

		if (bShadow)
		{
			pDC->SetColor(MCOLOR(dwShadowColor));
			pDC->Text(nCurX + m_Config.nShadowOffset, nTextY + m_Config.nShadowOffset, szSeg);
		}
		pDC->SetColor(MCOLOR(dwTextColor));
		pDC->Text(nCurX, nTextY, szSeg);
	}
}

// ============================================================
//  DrawAnnounce — main draw
// ============================================================

void ZGlobalAnnounce::DrawAnnounce(MDrawContext* pDC)
{
	if (!m_bActive || strlen(m_szGlobalMsg) == 0)
		return;

	DWORD dwNow = timeGetTime();
	int dwElapsed = (int)(dwNow - m_dwStartTime);

	if (dwElapsed >= m_nTotalDisplayTime)
	{
		Clear();
		return;
	}

	float fAlpha = CalcAlpha(dwNow);
	if (fAlpha <= 0.01f)
		return;

	MFont* pFont = MFontManager::Get(m_Config.szFontName);
	if (!pFont) pFont = MFontManager::Get("FONTa10_O2Wht");
	if (!pFont) return;
	pDC->SetFont(pFont);

	int nScreenW = MGetWorkspaceWidth();
	int nScreenH = MGetWorkspaceHeight();
	float fScale = (float)nScreenH / 600.0f;	// Scale theo resolution gốc GunZ 800x600

	int nBoxW = (int)(m_Config.fBoxWidth * (float)nScreenW);
	int nBoxH = (int)(m_Config.nBoxHeight * fScale);
	int nBoxX = (nScreenW - nBoxW) / 2;
	int nBoxY = (int)(m_Config.fPosY * (float)nScreenH);
	int nPad = (int)(m_Config.nBoxPadding * fScale);
	int nBoxInnerW = nBoxW - (nPad * 2);

	int nRenderW = CalcRenderWidth(pFont);
	int nTextHeight = pFont->GetHeight();

	// === Vẽ nền + viền ===
	pDC->SetColor(MCOLOR(ApplyAlpha(m_Config.dwBgColor, fAlpha)));
	pDC->FillRectangle(nBoxX, nBoxY, nBoxW, nBoxH);
	pDC->SetColor(MCOLOR(ApplyAlpha(m_Config.dwBorderColor, fAlpha)));
	pDC->Rectangle(nBoxX, nBoxY, nBoxW, nBoxH);

	// === Tính vị trí text ===
	int nTextY = nBoxY + (nBoxH - nTextHeight) / 2;
	int nTextX;

	if (!m_bNeedScroll)
	{
		nTextX = nBoxX + nPad + (nBoxInnerW - nRenderW) / 2;
	}
	else
	{
		// Timeline: [fade] [repeat * (pause + scroll + pause)] [fade]
		int nFade = m_Config.nFadeTime;
		int nPause = m_Config.nScrollPause;
		int nScrollTime = m_nSingleScrollTime - (nPause * 2);

		// Thời gian trong vùng cuộn (sau fade_in)
		int nContentElapsed = dwElapsed - nFade;
		if (nContentElapsed < 0) nContentElapsed = 0;

		// Xác định đang ở lần lặp nào
		int nRepeatIdx = nContentElapsed / m_nSingleScrollTime;
		int nInRepeat = nContentElapsed % m_nSingleScrollTime;

		if (nRepeatIdx >= m_Config.nRepeatCount)
		{
			// Đã hết repeat → giữ ở cuối
			m_fScrollOffset = (float)m_nScrollDist;
		}
		else if (nInRepeat < nPause)
		{
			// Pause đầu → offset = 0
			m_fScrollOffset = 0;
		}
		else if (nInRepeat < (nPause + nScrollTime))
		{
			// Đang cuộn
			float fProgress = (float)(nInRepeat - nPause) / (float)nScrollTime;
			if (fProgress > 1.0f) fProgress = 1.0f;
			m_fScrollOffset = fProgress * (float)m_nScrollDist;
		}
		else
		{
			// Pause cuối → offset = max
			m_fScrollOffset = (float)m_nScrollDist;
		}

		nTextX = nBoxX + nPad - (int)m_fScrollOffset;
	}

	// === D3D Scissor Rect ===
	LPDIRECT3DDEVICE9 pd3dDevice = RGetDevice();
	DWORD dwPrevScissor = 0;
	RECT prevScissorRect = { 0, 0, 0, 0 };
	bool bScissorSet = false;

	if (pd3dDevice)
	{
		pd3dDevice->GetRenderState(D3DRS_SCISSORTESTENABLE, &dwPrevScissor);
		pd3dDevice->GetScissorRect(&prevScissorRect);

		RECT scissor;
		scissor.left = nBoxX + nPad;
		scissor.top = nBoxY;
		scissor.right = nBoxX + nPad + nBoxInnerW;
		scissor.bottom = nBoxY + nBoxH;

		pd3dDevice->SetRenderState(D3DRS_SCISSORTESTENABLE, TRUE);
		pd3dDevice->SetScissorRect(&scissor);
		bScissorSet = true;
	}

	// === Tính màu ===
	DWORD dwTextColor = GetCyclingColor(dwNow, fAlpha);
	DWORD dwShadowColor = ApplyAlpha(m_Config.dwShadowColor, fAlpha);

	// === Vẽ text + emoji ===
	DrawTextWithEmoji(pDC, nTextX, nTextY, dwTextColor, dwShadowColor);

	// === Khôi phục scissor ===
	if (bScissorSet && pd3dDevice)
	{
		pd3dDevice->SetRenderState(D3DRS_SCISSORTESTENABLE, dwPrevScissor);
		pd3dDevice->SetScissorRect(&prevScissorRect);
	}
}

// ============================================================
//  LoadConfig — đọc XML
// ============================================================

bool ZGlobalAnnounce::LoadConfig(const char* szFileName)
{
	MXmlDocument xmlDoc;
	xmlDoc.Create();

	if (!xmlDoc.LoadFromFile(szFileName))
	{
		xmlDoc.Destroy();
		return false;
	}

	MXmlElement rootElement, childElement;
	char szTagName[256];

	rootElement = xmlDoc.GetDocumentElement();
	int iCount = rootElement.GetChildNodeCount();

	for (int i = 0; i < iCount; i++)
	{
		childElement = rootElement.GetChildNode(i);
		childElement.GetTagName(szTagName);
		if (szTagName[0] == '#') continue;

		if (!strcmp(szTagName, "ANNOUNCE"))
		{
			char szBuf[512];

			if (childElement.GetAttribute(szBuf, "font", ""))
				if (strlen(szBuf) > 0)
					strncpy(m_Config.szFontName, szBuf, sizeof(m_Config.szFontName) - 1);

			if (childElement.GetAttribute(szBuf, "text_color", ""))
				if (strlen(szBuf) > 0) m_Config.dwTextColor = strtoul(szBuf, NULL, 16);
			if (childElement.GetAttribute(szBuf, "bg_color", ""))
				if (strlen(szBuf) > 0) m_Config.dwBgColor = strtoul(szBuf, NULL, 16);
			if (childElement.GetAttribute(szBuf, "border_color", ""))
				if (strlen(szBuf) > 0) m_Config.dwBorderColor = strtoul(szBuf, NULL, 16);
			if (childElement.GetAttribute(szBuf, "shadow_color", ""))
				if (strlen(szBuf) > 0) m_Config.dwShadowColor = strtoul(szBuf, NULL, 16);

			float fVal;
			childElement.GetAttribute(&fVal, "pos_y", m_Config.fPosY);
			m_Config.fPosY = fVal;
			childElement.GetAttribute(&fVal, "box_width", m_Config.fBoxWidth);
			m_Config.fBoxWidth = fVal;
			childElement.GetAttribute(&fVal, "scroll_speed", m_Config.fScrollSpeed);
			m_Config.fScrollSpeed = fVal;

			int nVal;
			childElement.GetAttribute(&nVal, "box_height", m_Config.nBoxHeight);
			m_Config.nBoxHeight = nVal;
			childElement.GetAttribute(&nVal, "display_time", m_Config.nDisplayTime);
			m_Config.nDisplayTime = nVal;
			childElement.GetAttribute(&nVal, "fade_time", m_Config.nFadeTime);
			m_Config.nFadeTime = nVal;
			childElement.GetAttribute(&nVal, "padding", m_Config.nBoxPadding);
			m_Config.nBoxPadding = nVal;
			childElement.GetAttribute(&nVal, "scroll_pause", m_Config.nScrollPause);
			m_Config.nScrollPause = nVal;
			childElement.GetAttribute(&nVal, "repeat", m_Config.nRepeatCount);
			m_Config.nRepeatCount = nVal;
			if (m_Config.nRepeatCount < 1) m_Config.nRepeatCount = 1;

			int nShadow = m_Config.bShadow ? 1 : 0;
			childElement.GetAttribute(&nShadow, "shadow", nShadow);
			m_Config.bShadow = (nShadow != 0);
			childElement.GetAttribute(&nVal, "shadow_offset", m_Config.nShadowOffset);
			m_Config.nShadowOffset = nVal;

			// Color cycling
			int nCycle = 0;
			childElement.GetAttribute(&nCycle, "color_cycle", 0);
			m_Config.bColorCycle = (nCycle != 0);
			childElement.GetAttribute(&nVal, "color_speed", m_Config.nColorCycleSpeed);
			m_Config.nColorCycleSpeed = nVal;
			if (m_Config.nColorCycleSpeed < 50) m_Config.nColorCycleSpeed = 50;

			// Parse colors="FFFF0000,FF00FF00,FF0000FF,..."
			m_Config.nCycleColorCount = 0;
			if (childElement.GetAttribute(szBuf, "colors", ""))
			{
				if (strlen(szBuf) > 0)
				{
					char* pToken = strtok(szBuf, ",");
					while (pToken && m_Config.nCycleColorCount < ANNOUNCE_MAX_COLORS)
					{
						// Bỏ khoảng trắng
						while (*pToken == ' ') pToken++;
						m_Config.dwCycleColors[m_Config.nCycleColorCount++] = strtoul(pToken, NULL, 16);
						pToken = strtok(NULL, ",");
					}
				}
			}

			break;
		}
	}

	xmlDoc.Destroy();
	return true;
}

#endif // _GLOBALANNOUNCE