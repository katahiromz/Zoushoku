#include <windows.h>
#include <windowsx.h>
#include <stdlib.h>

static const TCHAR s_szName[] = TEXT("Zoushoku Window");
static HBITMAP s_hbm = NULL;
static HRGN s_hrgn = NULL;
static BITMAP s_bm;

HRGN DoCreateRgnFromBitmap(HBITMAP hbm, COLORREF rgb)
{
    HRGN ret = CreateRectRgn(0, 0, 0, 0);;
    BITMAP bm;
    if (!GetObject(hbm, sizeof(bm), &bm))
        return NULL;

    if (HDC hdc = CreateCompatibleDC(NULL))
    {
        HGDIOBJ hbmOld = SelectObject(hdc, hbm);
        {
            for (LONG y = 0; y < bm.bmHeight; ++y)
            {
                for (LONG x = 0; x < bm.bmWidth; ++x)
                {
                    if (GetPixel(hdc, x, y) != rgb)
                    {
                        HRGN hrgn = CreateRectRgn(x, y, x + 1, y + 1);
                        CombineRgn(ret, ret, hrgn, RGN_OR);
                        DeleteObject(hrgn);
                    }
                }
            }

        }
        SelectObject(hdc, hbmOld);
        DeleteDC(hdc);
    }

    return ret;
}

HWND DoCreateOne(void)
{
    INT cx = s_bm.bmWidth, cy = s_bm.bmHeight;
    INT x = rand() % (GetSystemMetrics(SM_CXSCREEN) - cx);
    INT y = rand() % (GetSystemMetrics(SM_CYSCREEN) - cy);

    HWND hwnd = CreateWindow(s_szName, s_szName, WS_POPUP,
                             x, y, cx, cy,
                             NULL, NULL, GetModuleHandle(NULL), NULL);
    if (!hwnd)
        return NULL;

    HRGN hRgn = CreateRectRgn(0, 0, 0, 0);
    CombineRgn(hRgn, s_hrgn, NULL, RGN_COPY);

    SetWindowRgn(hwnd, hRgn, TRUE);
    ShowWindow(hwnd, SW_SHOWNORMAL);
    UpdateWindow(hwnd);
    return hwnd;
}

BOOL OnCreate(HWND hwnd, LPCREATESTRUCT lpCreateStruct)
{
    return TRUE;
}

void OnTimer(HWND hwnd, UINT id)
{
    DoCreateOne();
}

void OnKey(HWND hwnd, UINT vk, BOOL fDown, int cRepeat, UINT flags)
{
    if (fDown && vk == VK_ESCAPE)
        PostQuitMessage(0);
}

void OnPaint(HWND hwnd)
{
    PAINTSTRUCT ps;
    if (HDC hDC = BeginPaint(hwnd, &ps))
    {
        if (HDC hMemDC = CreateCompatibleDC(hDC))
        {
            HGDIOBJ hbmOld = SelectObject(hMemDC, s_hbm);
            {
                BitBlt(hDC, 0, 0, s_bm.bmWidth, s_bm.bmHeight, hMemDC, 0, 0, SRCCOPY);
            }
            SelectObject(hMemDC, hbmOld);
            DeleteDC(hMemDC);
        }
        EndPaint(hwnd, &ps);
    }
}

void OnLButtonDown(HWND hwnd, BOOL fDoubleClick, int x, int y, UINT keyFlags)
{
    PostQuitMessage(0);
}

LRESULT CALLBACK
WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
        HANDLE_MSG(hwnd, WM_CREATE, OnCreate);
        HANDLE_MSG(hwnd, WM_TIMER, OnTimer);
        HANDLE_MSG(hwnd, WM_KEYDOWN, OnKey);
        HANDLE_MSG(hwnd, WM_LBUTTONDOWN, OnLButtonDown);
        HANDLE_MSG(hwnd, WM_PAINT, OnPaint);
    default:
        return DefWindowProc(hwnd, uMsg, wParam, lParam);
    }
    return 0;
}

INT WINAPI
WinMain(HINSTANCE   hInstance,
        HINSTANCE   hPrevInstance,
        LPSTR       lpCmdLine,
        INT         nCmdShow)
{
    srand(GetTickCount());

    s_hbm = LoadBitmap(hInstance, MAKEINTRESOURCE(1));
    GetObject(s_hbm, sizeof(s_bm), &s_bm);
    s_hrgn = DoCreateRgnFromBitmap(s_hbm, RGB(255, 0, 255));

    WNDCLASS wc = { 0, WindowProc };
    wc.hInstance = hInstance;
    wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_3DFACE + 1);
    wc.lpszClassName = s_szName;
    if (!RegisterClass(&wc))
        return -1;

    HWND hwnd = DoCreateOne();
    if (!hwnd)
        return -2;

    SetTimer(hwnd, 999, 500, NULL);

    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    DeleteObject(s_hbm);
    DeleteObject(s_hrgn);
    return 0;
}
