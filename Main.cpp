#include "Main.h"
#include "DirectX.h"

//--------------------------------------------------------------------------------------
// 静的メンバ
//--------------------------------------------------------------------------------------
HWND Window::g_hWnd = nullptr;//ウィンドウハンドル
int Window::g_iClientWidth = 800;//クライアント領域の横幅
int Window::g_iClientHeight = 600;//クライアント領域の高さ
double Window::g_dFps = 0;//FPS
double Window::g_dFrameTime = 0;//1フレームあたりの時間

//--------------------------------------------------------------------------------------
// 前方宣言
//--------------------------------------------------------------------------------------
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

//--------------------------------------------------------------------------------------
// wWinMain()関数：エントリーポイント
//--------------------------------------------------------------------------------------
int WINAPI wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR lpCmdLine, _In_ int nCmdShow)
{
    if (FAILED(CoInitialize(nullptr)))//COMの初期化
        return 0;

    Window win;

    if (FAILED(win.InitWindow(hInstance, nCmdShow)))
        return 0;

    DirectX11 dx;

    if (FAILED(dx.InitDevice()))
        return 0;

    win.InitFps();

    // メインメッセージループ
    MSG msg = { 0 };
    while (WM_QUIT != msg.message)
    {
        if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
        else
        {
            win.CalculationFps();

            win.CalculationFrameTime();

            dx.Render();

            win.CalculationSleep();
        }
    }

    CoUninitialize();//COMの終了処理

    return (int)msg.wParam;
}

//--------------------------------------------------------------------------------------
// WndProc()関数：ウィンドウプロシージャ
//--------------------------------------------------------------------------------------
LRESULT CALLBACK WndProc(HWND hWnd, UINT uiMessage, WPARAM wParam, LPARAM lParam)
{
    PAINTSTRUCT ps;
    HDC hdc;

    switch (uiMessage)
    {
    case WM_PAINT:
        hdc = BeginPaint(hWnd, &ps);
        EndPaint(hWnd, &ps);
        break;

    case WM_DESTROY:
        PostQuitMessage(0);
        break;

    default:
        return DefWindowProc(hWnd, uiMessage, wParam, lParam);
    }

    return 0;
}

//--------------------------------------------------------------------------------------
// Window::InitWindow()関数：ウィンドウの表示
//--------------------------------------------------------------------------------------
HRESULT Window::InitWindow(HINSTANCE hInstance, int nCmdShow)
{
    WNDCLASSEX wcex;
    wcex.cbSize = sizeof(WNDCLASSEX);
    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = WndProc;
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = 0;
    wcex.hInstance = hInstance;
    wcex.hIcon = nullptr;
    wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wcex.lpszMenuName = nullptr;
    wcex.lpszClassName = L"WindowClass";
    wcex.hIconSm = nullptr;
    if (!RegisterClassEx(&wcex))
        return E_FAIL;

    RECT rc = { 0, 0, g_iClientWidth, g_iClientHeight };
    AdjustWindowRect(&rc, WS_OVERLAPPEDWINDOW, FALSE);
    g_hWnd = CreateWindow(L"WindowClass", L"DirectX11 - 簡易ペイントソフト",//★---変更---
        WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX,
        CW_USEDEFAULT, CW_USEDEFAULT, rc.right - rc.left, rc.bottom - rc.top, nullptr, nullptr, hInstance,
        nullptr);
    if (!g_hWnd)
        return E_FAIL;

    ShowWindow(g_hWnd, nCmdShow);

    return S_OK;
}

//--------------------------------------------------------------------------------------
// Window::InitFps()関数：FPS計測の初期化
//--------------------------------------------------------------------------------------
void Window::InitFps()
{
    QueryPerformanceFrequency(&m_freq);
    QueryPerformanceCounter(&m_starttime);//現在の時間を取得（1フレーム目）
}

//--------------------------------------------------------------------------------------
// Window::CalculationFps()関数：FPS計測
//--------------------------------------------------------------------------------------
void Window::CalculationFps()
{
    if (m_iCount == 60)//カウントが60の時の処理
    {
        QueryPerformanceCounter(&m_nowtime);//現在の時間を取得（60フレーム目）
        //FPS = 1秒 / 1フレームの描画にかかる時間
        //    = 1000ms / ((現在の時間ms - 1フレーム目の時間ms) / 60フレーム)
        g_dFps = 1000.0 / (static_cast<double>((m_nowtime.QuadPart - m_starttime.QuadPart) * 1000 / m_freq.QuadPart) / 60.0);
        m_iCount = 0;//カウントを初期値に戻す
        m_starttime = m_nowtime;//1フレーム目の時間を現在の時間にする
    }
    m_iCount++;//カウント+1
}

//--------------------------------------------------------------------------------------
// Window::CalculationSleep()関数：Sleepさせる時間の計算
//--------------------------------------------------------------------------------------
void Window::CalculationSleep()
{
    QueryPerformanceCounter(&m_nowtime);//現在の時間を取得
    //Sleepさせる時間ms = 1フレーム目から現在のフレームまでの描画にかかるべき時間ms - 1フレーム目から現在のフレームまで実際にかかった時間ms
    //                  = (1000ms / 60)*フレーム数 - (現在の時間ms - 1フレーム目の時間ms)
    DWORD dwSleepTime = static_cast<DWORD>((1000.0 / 60.0) * m_iCount - (m_nowtime.QuadPart - m_starttime.QuadPart) * 1000 / m_freq.QuadPart);
    if (dwSleepTime > 0 && dwSleepTime < 18)//大きく変動がなければSleepTimeは1～17の間に納まる
    {
        timeBeginPeriod(1);
        Sleep(dwSleepTime);
        timeEndPeriod(1);
    }
    else//大きく変動があった場合
    {
        timeBeginPeriod(1);
        Sleep(1);
        timeEndPeriod(1);
    }
}

//--------------------------------------------------------------------------------------
// Window::CalculationFrameTime()関数：1フレームあたりの時間の計測
//--------------------------------------------------------------------------------------
void Window::CalculationFrameTime()
{
    static int iFlg;
    if (iFlg == 0)
    {
        QueryPerformanceCounter(&m_frametime_a);
        iFlg = 1;
    }
    QueryPerformanceCounter(&m_frametime_b);
    g_dFrameTime = (m_frametime_b.QuadPart - m_frametime_a.QuadPart) * 1000.0 / m_freq.QuadPart;
    m_frametime_a = m_frametime_b;
}

//--------------------------------------------------------------------------------------
// Window::GethWnd()関数：hWndの取得
//--------------------------------------------------------------------------------------
HWND Window::GethWnd()
{
    return g_hWnd;
}

//--------------------------------------------------------------------------------------
// Window::GetClientWidth()関数：クライアント領域の横幅の取得
//--------------------------------------------------------------------------------------
int Window::GetClientWidth()
{
    return g_iClientWidth;
}

//--------------------------------------------------------------------------------------
// Window::GetClientHeight()関数：クライアント領域の高さの取得
//--------------------------------------------------------------------------------------
int Window::GetClientHeight()
{
    return g_iClientHeight;
}

//--------------------------------------------------------------------------------------
// Window::GetFps()関数：FPSの取得
//--------------------------------------------------------------------------------------
double Window::GetFps()
{
    return g_dFps;
}

//--------------------------------------------------------------------------------------
// Window::GetFrameTime()関数：1フレームあたりの時間の取得
//--------------------------------------------------------------------------------------
double Window::GetFrameTime()
{
    return g_dFrameTime;
}