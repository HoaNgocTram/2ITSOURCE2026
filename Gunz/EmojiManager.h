#pragma once
#include <vector>
#include <string>
#include <map>

// ============================================================
//  EmojiManager — load emojis from XML, match patterns, draw
//  Supports static PNG and animated GIF emojis
// ============================================================

class AnimatedGifTexture; // forward declaration

struct EmojiEntry
{
	std::string pattern;     // ":D", "^_^", "O:)", ":v", etc.
	std::string filename;    // "1.png", "diamond.gif"
	bool bNeedSpaceBefore;   // true = needs space (or start of text) before pattern
	int nWidth;              // custom width (0 = use default size)
	int nHeight;             // custom height (0 = use default size)
	bool bAnimated;          // true if this is an animated GIF emoji
};

struct EmojiMatchResult
{
	const EmojiEntry* pEntry;
	int nPatternLen;         // how many chars matched
};

class EmojiManager
{
public:
	static EmojiManager& GetInstance();

	bool LoadFromXML(const char* szFileName);
	bool IsLoaded() const { return m_bLoaded; }

	// Find emoji at position in char* string (for old chat / TextMultiLine)
	EmojiMatchResult FindEmoji(const char* szText, int nPos, int nLen) const;

	// Find emoji at position in wchar_t* string (for NewChat / DrawTextN)
	EmojiMatchResult FindEmojiW(const wchar_t* szText, int nPos, int nLen) const;

	// Get draw size based on context
	int GetSize(bool bIngame) const;

	// Get actual line height: max(fontHeight, tallest emoji in line)
	// For char* strings (lobby/old chat)
	int GetLineHeight(const char* szText, int nLen, int nFontHeight, bool bIngame) const;
	// For wchar_t* strings (NewChat)
	int GetLineHeightW(const wchar_t* szText, int nLen, int nFontHeight, bool bIngame) const;

	// Get animated GIF texture by filename (returns nullptr if not animated or not found)
	AnimatedGifTexture* GetAnimatedTexture(const char* szFilename) const;

	// Access
	const std::vector<EmojiEntry>& GetEmojis() const { return m_Emojis; }
	int GetLobbySize() const { return m_nLobbySize; }
	int GetIngameSize() const { return m_nIngameSize; }

	// Cleanup animated textures (call on shutdown)
	void Destroy();

private:
	EmojiManager() = default;
	~EmojiManager();

	std::vector<EmojiEntry> m_Emojis;
	int m_nLobbySize = 16;
	int m_nIngameSize = 14;
	bool m_bLoaded = false;

	// Map of filename -> AnimatedGifTexture for GIF emojis
	std::map<std::string, AnimatedGifTexture*> m_AnimatedTextures;

	// Helper: check if filename ends with .gif
	static bool IsGifFile(const std::string& filename);

	// Load a GIF emoji: decode and create D3D textures
	bool LoadAnimatedEmoji(const std::string& filename, const char* szBasePath);
};

// Convenience macro
#define ZGetEmojiManager() EmojiManager::GetInstance()

// ============================================================
//  Legacy support — keep weapons/blank for kill feed system
// ============================================================
struct Emoji
{
	char A, B, C;
	char name[20];
	Emoji(char a, char b, char c, char* Name)
	{
		A = a; B = b; C = c;
		memset(name, 0, 20);
		strcpy(name, Name);
	}
	Emoji(char a, char b, char* Name)
	{
		A = a; B = b; C = ' ';
		memset(name, 0, 20);
		strcpy(name, Name);
	}
};

typedef std::vector<Emoji*> Emojis;

extern Emojis m_Weapons;
extern Emojis m_Blank;
