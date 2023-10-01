#pragma once

#include <windows.h>
#pragma comment(lib,"winmm.lib")

//--------------------------------------------------------------------------------------
// WindowÉNÉâÉXÅFWindowä÷åW
//--------------------------------------------------------------------------------------
class Window
{
public:
    HRESULT InitWindow(HINSTANCE hInstance, int nCmdShow);
    void InitFps();
    void CalculationFps();
    void CalculationSleep();
    void CalculationFrameTime();

    static HWND GethWnd();
    static int GetClientWidth();
    static int GetClientHeight();
    static double GetFps();
    static double GetFrameTime();
private:
    LARGE_INTEGER m_freq = { 0 };
    LARGE_INTEGER m_starttime = { 0 };
    LARGE_INTEGER m_nowtime = { 0 };
    LARGE_INTEGER m_frametime_a = { 0 };
    LARGE_INTEGER m_frametime_b = { 0 };
    int m_iCount = 0;

    static HWND g_hWnd;
    static int g_iClientWidth;
    static int g_iClientHeight;
    static double g_dFps;
    static double g_dFrameTime;
};