#include "stdafx.h"
#include "EmojiManager.h"
#include "AnimatedGifTexture.h"
#include "MXml.h"
#include "MZFileSystem.h"
#include "MDebug.h"
#include "ZApplication.h"
#include "RealSpace2.h"

EmojiManager& EmojiManager::GetInstance()
{
	static EmojiManager instance;
	return instance;
}

EmojiManager::~EmojiManager()
{
	Destroy();
}

void EmojiManager::Destroy()
{
	for (auto& pair : m_AnimatedTextures)
	{
		if (pair.second)
		{
			pair.second->Destroy();
			delete pair.second;
		}
	}
	m_AnimatedTextures.clear();
	m_Emojis.clear();
	m_bLoaded = false;
}

bool EmojiManager::IsGifFile(const std::string& filename)
{
	if (filename.size() < 4) return false;
	std::string ext = filename.substr(filename.size() - 4);
	return (stricmp(ext.c_str(), ".gif") == 0);
}

bool EmojiManager::LoadAnimatedEmoji(const std::string& filename, const char* szBasePath)
{
	// Already loaded?
	if (m_AnimatedTextures.find(filename) != m_AnimatedTextures.end())
		return true;

	// Build full path: basePath + filename
	std::string fullPath = std::string(szBasePath) + filename;

	// Read file via MZFile
	MZFile mzFile;
	if (!mzFile.Open(fullPath.c_str(), ZApplication::GetFileSystem()))
	{
		if (!mzFile.Open(fullPath.c_str(), (MZFileSystem*)NULL))
		{
			mlog("EmojiManager: Cannot open GIF file %s\n", fullPath.c_str());
			return false;
		}
	}

	unsigned long fileSize = mzFile.GetLength();
	uint8_t* fileData = new uint8_t[fileSize];
	mzFile.Read(fileData, fileSize);

	// Create animated texture
	AnimatedGifTexture* pAnimTex = new AnimatedGifTexture();
	if (!pAnimTex->CreateFromMemory(fileData, (int)fileSize, RGetDevice()))
	{
		mlog("EmojiManager: Failed to create animated texture for %s\n", filename.c_str());
		delete pAnimTex;
		delete[] fileData;
		return false;
	}

	delete[] fileData;
	m_AnimatedTextures[filename] = pAnimTex;

	mlog("EmojiManager: Loaded animated emoji '%s' (%d frames)\n",
		filename.c_str(), pAnimTex->GetFrameCount());

	return true;
}

AnimatedGifTexture* EmojiManager::GetAnimatedTexture(const char* szFilename) const
{
	auto it = m_AnimatedTextures.find(szFilename);
	if (it != m_AnimatedTextures.end())
		return it->second;
	return nullptr;
}

bool EmojiManager::LoadFromXML(const char* szFileName)
{
	// Clean up previous data (including animated textures)
	Destroy();

	MXmlDocument xmlDoc;
	xmlDoc.Create();

	MZFile mzFile;
	if (!mzFile.Open(szFileName, (MZFileSystem*)NULL))
	{
		// Try with filesystem
		if (!mzFile.Open(szFileName, ZApplication::GetFileSystem()))
		{
			mlog("EmojiManager: Cannot open %s\n", szFileName);
			xmlDoc.Destroy();
			return false;
		}
	}

	char* buffer = new char[mzFile.GetLength() + 1];
	buffer[mzFile.GetLength()] = 0;
	mzFile.Read(buffer, mzFile.GetLength());

	if (!xmlDoc.LoadFromMemory(buffer))
	{
		mlog("EmojiManager: Failed to parse %s\n", szFileName);
		xmlDoc.Destroy();
		delete[] buffer;
		return false;
	}

	MXmlElement rootElement = xmlDoc.GetDocumentElement();

	// Read global attributes
	char szTemp[64];
	if (rootElement.GetAttribute(szTemp, "lobby_size"))
		m_nLobbySize = atoi(szTemp);
	if (rootElement.GetAttribute(szTemp, "ingame_size"))
		m_nIngameSize = atoi(szTemp);

	// Extract base path from XML filename (e.g. "Interface/default/emojis/")
	std::string basePath = szFileName;
	size_t lastSlash = basePath.find_last_of("/\\");
	if (lastSlash != std::string::npos)
		basePath = basePath.substr(0, lastSlash + 1);
	else
		basePath = "";

	int nStaticCount = 0;
	int nAnimatedCount = 0;

	int nCount = rootElement.GetChildNodeCount();
	for (int i = 0; i < nCount; i++)
	{
		MXmlElement elem = rootElement.GetChildNode(i);

		char szTag[64];
		elem.GetTagName(szTag);
		if (szTag[0] == '#') continue;
		if (stricmp(szTag, "EMOJI") != 0) continue;

		EmojiEntry entry;
		char szPattern[64] = "";
		char szFile[64] = "";
		char szSpace[16] = "true";
		char szWidth[16] = "0";
		char szHeight[16] = "0";

		elem.GetAttribute(szPattern, "pattern");
		elem.GetAttribute(szFile, "file");
		elem.GetAttribute(szSpace, "space");
		elem.GetAttribute(szWidth, "w");
		elem.GetAttribute(szHeight, "h");

		if (szPattern[0] == 0 || szFile[0] == 0) continue;

		entry.pattern = szPattern;
		entry.filename = szFile;
		entry.bNeedSpaceBefore = (stricmp(szSpace, "false") != 0); // default true
		entry.nWidth = atoi(szWidth);
		entry.nHeight = atoi(szHeight);

		// Auto-detect animated GIF by file extension
		entry.bAnimated = IsGifFile(entry.filename);

		if (entry.bAnimated)
		{
			// Load GIF and create D3D textures for all frames
			if (LoadAnimatedEmoji(entry.filename, basePath.c_str()))
				nAnimatedCount++;
			else
				entry.bAnimated = false; // Fallback: treat as static if load fails
		}
		else
		{
			nStaticCount++;
		}

		m_Emojis.push_back(entry);
	}

	xmlDoc.Destroy();
	delete[] buffer;

	m_bLoaded = true;
	mlog("EmojiManager: Loaded %d emojis (%d static, %d animated) from %s (lobby=%d, ingame=%d)\n",
		(int)m_Emojis.size(), nStaticCount, nAnimatedCount, szFileName, m_nLobbySize, m_nIngameSize);

	return true;
}

int EmojiManager::GetSize(bool bIngame) const
{
	return bIngame ? m_nIngameSize : m_nLobbySize;
}

EmojiMatchResult EmojiManager::FindEmoji(const char* szText, int nPos, int nLen) const
{
	EmojiMatchResult result = { nullptr, 0 };

	for (const auto& entry : m_Emojis)
	{
		int patLen = (int)entry.pattern.size();
		if (nPos + patLen > nLen) continue;

		// Check space before if needed
		if (entry.bNeedSpaceBefore && nPos > 0 && szText[nPos - 1] != ' ')
			continue;

		// Match pattern (case-insensitive)
		bool bMatch = true;
		for (int j = 0; j < patLen; j++)
		{
			if (tolower((unsigned char)szText[nPos + j]) != tolower((unsigned char)entry.pattern[j]))
			{
				bMatch = false;
				break;
			}
		}

		if (bMatch)
		{
			result.pEntry = &entry;
			result.nPatternLen = patLen;
			return result;
		}
	}

	return result;
}

EmojiMatchResult EmojiManager::FindEmojiW(const wchar_t* szText, int nPos, int nLen) const
{
	EmojiMatchResult result = { nullptr, 0 };

	for (const auto& entry : m_Emojis)
	{
		int patLen = (int)entry.pattern.size();
		if (nPos + patLen > nLen) continue;

		// Check space before if needed
		if (entry.bNeedSpaceBefore && nPos > 0 && szText[nPos - 1] != L' ')
			continue;

		// Match pattern (case-insensitive, compare wchar vs char — emojis are all ASCII)
		bool bMatch = true;
		for (int j = 0; j < patLen; j++)
		{
			if (towlower(szText[nPos + j]) != towlower((wchar_t)entry.pattern[j]))
			{
				bMatch = false;
				break;
			}
		}

		if (bMatch)
		{
			result.pEntry = &entry;
			result.nPatternLen = patLen;
			return result;
		}
	}

	return result;
}
