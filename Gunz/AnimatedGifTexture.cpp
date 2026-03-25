#include "stdafx.h"
#include "AnimatedGifTexture.h"
#include "gif_load.h"
#include "MDebug.h"

AnimatedGifTexture::AnimatedGifTexture()
    : m_nWidth(0)
    , m_nHeight(0)
    , m_nTotalDurationMs(0)
    , m_pd3dDevice(nullptr)
    , m_dwStartTime(0)
{
}

AnimatedGifTexture::~AnimatedGifTexture()
{
    Destroy();
}

bool AnimatedGifTexture::CreateFromMemory(const uint8_t* pData, int nDataSize, LPDIRECT3DDEVICE9 pd3dDevice)
{
    if (!pData || nDataSize <= 0 || !pd3dDevice)
        return false;

    Destroy();

    m_pd3dDevice = pd3dDevice;

    // Keep raw data for device reset
    m_RawGifData.assign(pData, pData + nDataSize);

    if (!CreateTexturesFromGifData())
    {
        Destroy();
        return false;
    }

    m_dwStartTime = timeGetTime();
    return true;
}

bool AnimatedGifTexture::CreateTexturesFromGifData()
{
    if (m_RawGifData.empty() || !m_pd3dDevice)
        return false;

    // Decode GIF
    GifFileData gif;
    if (!GifLoad(&gif, m_RawGifData.data(), (int)m_RawGifData.size()))
    {
        mlog("AnimatedGifTexture: GIF decode failed\n");
        return false;
    }

    m_nWidth = gif.width;
    m_nHeight = gif.height;

    int cumulative = 0;

    for (int i = 0; i < gif.frameCount; i++)
    {
        // Create D3D texture for this frame
        LPDIRECT3DTEXTURE9 pTex = nullptr;
        HRESULT hr = m_pd3dDevice->CreateTexture(
            m_nWidth, m_nHeight, 1,
            0,                    // no special usage
            D3DFMT_A8R8G8B8,     // 32-bit ARGB
            D3DPOOL_MANAGED,      // managed pool for simplicity
            &pTex, nullptr
        );

        if (FAILED(hr) || !pTex)
        {
            mlog("AnimatedGifTexture: CreateTexture failed for frame %d\n", i);
            GifFree(&gif);
            return false;
        }

        // Lock texture and copy RGBA -> ARGB pixel data
        D3DLOCKED_RECT lr;
        hr = pTex->LockRect(0, &lr, nullptr, 0);
        if (FAILED(hr))
        {
            pTex->Release();
            mlog("AnimatedGifTexture: LockRect failed for frame %d\n", i);
            GifFree(&gif);
            return false;
        }

        const uint8_t* src = gif.frames[i].pixels;
        uint8_t* dst = (uint8_t*)lr.pBits;

        for (int y = 0; y < m_nHeight; y++)
        {
            uint32_t* dstRow = (uint32_t*)(dst + y * lr.Pitch);
            const uint8_t* srcRow = src + y * m_nWidth * 4;

            for (int x = 0; x < m_nWidth; x++)
            {
                uint8_t r = srcRow[x * 4 + 0];
                uint8_t g = srcRow[x * 4 + 1];
                uint8_t b = srcRow[x * 4 + 2];
                uint8_t a = srcRow[x * 4 + 3];
                // D3DFMT_A8R8G8B8: ARGB layout
                dstRow[x] = (a << 24) | (r << 16) | (g << 8) | b;
            }
        }

        pTex->UnlockRect(0);

        // Build frame entry
        Frame frame;
        frame.pTexture = pTex;
        frame.delayMs = gif.frames[i].delayMs;
        cumulative += frame.delayMs;
        frame.cumulativeMs = cumulative;
        m_Frames.push_back(frame);
    }

    m_nTotalDurationMs = cumulative;

    GifFree(&gif);

    mlog("AnimatedGifTexture: Loaded %d frames (%dx%d, total %dms)\n",
        (int)m_Frames.size(), m_nWidth, m_nHeight, m_nTotalDurationMs);

    return true;
}

void AnimatedGifTexture::Destroy()
{
    for (auto& f : m_Frames)
    {
        if (f.pTexture)
        {
            f.pTexture->Release();
            f.pTexture = nullptr;
        }
    }
    m_Frames.clear();
    m_RawGifData.clear();
    m_nWidth = 0;
    m_nHeight = 0;
    m_nTotalDurationMs = 0;
    m_pd3dDevice = nullptr;
    m_dwStartTime = 0;
}

int AnimatedGifTexture::GetCurrentFrame() const
{
    if (m_Frames.empty() || m_nTotalDurationMs <= 0)
        return 0;

    DWORD elapsed = timeGetTime() - m_dwStartTime;
    int timeInLoop = (int)(elapsed % (DWORD)m_nTotalDurationMs);

    // Binary search for the right frame
    int lo = 0, hi = (int)m_Frames.size() - 1;
    while (lo < hi)
    {
        int mid = (lo + hi) / 2;
        if (m_Frames[mid].cumulativeMs <= timeInLoop)
            lo = mid + 1;
        else
            hi = mid;
    }
    return lo;
}

LPDIRECT3DTEXTURE9 AnimatedGifTexture::GetCurrentFrameTexture() const
{
    if (m_Frames.empty()) return nullptr;
    int idx = GetCurrentFrame();
    return m_Frames[idx].pTexture;
}

void AnimatedGifTexture::OnLostDevice()
{
    // D3DPOOL_MANAGED textures survive device lost,
    // but if using DEFAULT pool, release here
    // Currently using MANAGED, so nothing needed
}

void AnimatedGifTexture::OnResetDevice()
{
    // If we need to recreate (e.g. switched to DEFAULT pool):
    // Destroy();
    // CreateTexturesFromGifData();
    // m_dwStartTime = timeGetTime();
}
