#include "stdafx.h"
#include <windows.h>
#include <wrl.h>
#include <WebView2.h>
#include <string>
#include <shlobj.h>
#pragma comment(lib, "shell32.lib")

using namespace Microsoft::WRL;

HWND g_hWebWnd = nullptr;
ComPtr<ICoreWebView2Controller> g_webController;
ComPtr<ICoreWebView2> g_webView;

std::wstring ToWide(const char* str)
{
    int len = MultiByteToWideChar(CP_UTF8, 0, str, -1, nullptr, 0);
    std::wstring wstr(len, L'\0');
    MultiByteToWideChar(CP_UTF8, 0, str, -1, &wstr[0], len);
    return wstr;
}

LRESULT CALLBACK WebProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg)
    {
    case WM_NCHITTEST:
    {
        // Chặn resize và viền (chỉ cho kéo thanh tiêu đề)
        LRESULT hit = DefWindowProc(hWnd, msg, wParam, lParam);
        if (hit == HTCAPTION)
            return hit;
        return HTCLIENT;
    }

    case WM_CLOSE:
        DestroyWindow(hWnd);
        return 0;

    case WM_DESTROY:
    {
        if (g_webController) g_webController->Close();
        g_webController = nullptr;
        g_webView = nullptr;
        g_hWebWnd = nullptr;

        while (ShowCursor(FALSE) >= 0);

        OutputDebugStringA("[WebView2] Window destroyed, cursor restored.\n");
        return 0;
    }
    }

    return DefWindowProc(hWnd, msg, wParam, lParam);
}

void ZShowWebPage(const char* url)
{
    if (g_hWebWnd)
    {
        SetForegroundWindow(g_hWebWnd);
        return;
    }

    HINSTANCE hInst = GetModuleHandle(NULL);
    WNDCLASSA wc = { 0 };
    wc.lpfnWndProc = WebProc;
    wc.hInstance = hInst;
    wc.lpszClassName = "GUNZ_WEBVIEW";
    RegisterClassA(&wc);

    int width = 900, height = 600;
    int x = (GetSystemMetrics(SM_CXSCREEN) - width) / 2;
    int y = (GetSystemMetrics(SM_CYSCREEN) - height) / 2;

    g_hWebWnd = CreateWindowExA(
        WS_EX_APPWINDOW,
        wc.lpszClassName,
        "Gunz WebViewer",
        WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_VISIBLE,
        x, y, width, height,
        NULL, NULL, hInst, NULL);

    if (!g_hWebWnd)
        return;

    std::wstring wurl = ToWide(url);
    while (ShowCursor(TRUE) < 0);

    char szPath[MAX_PATH];
    // Lấy đường dẫn Local AppData (C:\Users\Tên_User\AppData\Local)
    if (SUCCEEDED(SHGetFolderPathA(NULL, CSIDL_LOCAL_APPDATA, NULL, 0, szPath)))
    {
        // Tạo folder riêng cho game để tránh lẫn lộn
        strcat_s(szPath, "\\GunzTheDuel");
        CreateDirectoryA(szPath, NULL);

        // Tạo folder con cho WebView2
        strcat_s(szPath, "\\WebView2_Cache");
        CreateDirectoryA(szPath, NULL);
    }

    // Chuyển sang Wide String để ném vào CreateCoreWebView2EnvironmentWithOptions
    int nLen = MultiByteToWideChar(CP_ACP, 0, szPath, -1, NULL, 0);
    std::wstring wCachePath(nLen, L'\0');
    MultiByteToWideChar(CP_ACP, 0, szPath, -1, &wCachePath[0], nLen);

    CreateCoreWebView2EnvironmentWithOptions(
        nullptr,
        wCachePath.c_str(),  
        nullptr,
        Callback<ICoreWebView2CreateCoreWebView2EnvironmentCompletedHandler>(
            [wurl](HRESULT result, ICoreWebView2Environment* env) -> HRESULT

            {
                env->CreateCoreWebView2Controller(g_hWebWnd,
                    Callback<ICoreWebView2CreateCoreWebView2ControllerCompletedHandler>(
                        [wurl](HRESULT result, ICoreWebView2Controller* controller) -> HRESULT
                        {
                            if (!controller) return E_FAIL;

                            g_webController = controller;
                            g_webController->get_CoreWebView2(&g_webView);

                            ComPtr<ICoreWebView2Settings> settings;
                            if (SUCCEEDED(g_webView->get_Settings(&settings)))
                            {
                                settings->put_AreDefaultContextMenusEnabled(FALSE);
                                settings->put_AreDevToolsEnabled(FALSE);
                            }

                            RECT bounds;
                            GetClientRect(g_hWebWnd, &bounds);
                            g_webController->put_Bounds(bounds);

                            g_webView->Navigate(wurl.c_str());
                            return S_OK;
                        }).Get());
                return S_OK;
            }).Get());
}