#include "Main.h"
#include "DirectX.h"

//--------------------------------------------------------------------------------------
// �ÓI�����o
//--------------------------------------------------------------------------------------
HWND Window::g_hWnd = nullptr;//�E�B���h�E�n���h��
int Window::g_iClientWidth = 800;//�N���C�A���g�̈�̉���
int Window::g_iClientHeight = 600;//�N���C�A���g�̈�̍���
double Window::g_dFps = 0;//FPS
double Window::g_dFrameTime = 0;//1�t���[��������̎���

//--------------------------------------------------------------------------------------
// �O���錾
//--------------------------------------------------------------------------------------
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

//--------------------------------------------------------------------------------------
// wWinMain()�֐��F�G���g���[�|�C���g
//--------------------------------------------------------------------------------------
int WINAPI wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR lpCmdLine, _In_ int nCmdShow)
{
    if (FAILED(CoInitialize(nullptr)))//COM�̏�����
        return 0;

    Window win;

    if (FAILED(win.InitWindow(hInstance, nCmdShow)))
        return 0;

    DirectX11 dx;

    if (FAILED(dx.InitDevice()))
        return 0;

    win.InitFps();

    // ���C�����b�Z�[�W���[�v
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

    CoUninitialize();//COM�̏I������

    return (int)msg.wParam;
}

//--------------------------------------------------------------------------------------
// WndProc()�֐��F�E�B���h�E�v���V�[�W��
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
// Window::InitWindow()�֐��F�E�B���h�E�̕\��
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
    g_hWnd = CreateWindow(L"WindowClass", L"DirectX11 - �ȈՃy�C���g�\�t�g",//��---�ύX---
        WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX,
        CW_USEDEFAULT, CW_USEDEFAULT, rc.right - rc.left, rc.bottom - rc.top, nullptr, nullptr, hInstance,
        nullptr);
    if (!g_hWnd)
        return E_FAIL;

    ShowWindow(g_hWnd, nCmdShow);

    return S_OK;
}

//--------------------------------------------------------------------------------------
// Window::InitFps()�֐��FFPS�v���̏�����
//--------------------------------------------------------------------------------------
void Window::InitFps()
{
    QueryPerformanceFrequency(&m_freq);
    QueryPerformanceCounter(&m_starttime);//���݂̎��Ԃ��擾�i1�t���[���ځj
}

//--------------------------------------------------------------------------------------
// Window::CalculationFps()�֐��FFPS�v��
//--------------------------------------------------------------------------------------
void Window::CalculationFps()
{
    if (m_iCount == 60)//�J�E���g��60�̎��̏���
    {
        QueryPerformanceCounter(&m_nowtime);//���݂̎��Ԃ��擾�i60�t���[���ځj
        //FPS = 1�b / 1�t���[���̕`��ɂ����鎞��
        //    = 1000ms / ((���݂̎���ms - 1�t���[���ڂ̎���ms) / 60�t���[��)
        g_dFps = 1000.0 / (static_cast<double>((m_nowtime.QuadPart - m_starttime.QuadPart) * 1000 / m_freq.QuadPart) / 60.0);
        m_iCount = 0;//�J�E���g�������l�ɖ߂�
        m_starttime = m_nowtime;//1�t���[���ڂ̎��Ԃ����݂̎��Ԃɂ���
    }
    m_iCount++;//�J�E���g+1
}

//--------------------------------------------------------------------------------------
// Window::CalculationSleep()�֐��FSleep�����鎞�Ԃ̌v�Z
//--------------------------------------------------------------------------------------
void Window::CalculationSleep()
{
    QueryPerformanceCounter(&m_nowtime);//���݂̎��Ԃ��擾
    //Sleep�����鎞��ms = 1�t���[���ڂ��猻�݂̃t���[���܂ł̕`��ɂ�����ׂ�����ms - 1�t���[���ڂ��猻�݂̃t���[���܂Ŏ��ۂɂ�����������ms
    //                  = (1000ms / 60)*�t���[���� - (���݂̎���ms - 1�t���[���ڂ̎���ms)
    DWORD dwSleepTime = static_cast<DWORD>((1000.0 / 60.0) * m_iCount - (m_nowtime.QuadPart - m_starttime.QuadPart) * 1000 / m_freq.QuadPart);
    if (dwSleepTime > 0 && dwSleepTime < 18)//�傫���ϓ����Ȃ����SleepTime��1�`17�̊Ԃɔ[�܂�
    {
        timeBeginPeriod(1);
        Sleep(dwSleepTime);
        timeEndPeriod(1);
    }
    else//�傫���ϓ����������ꍇ
    {
        timeBeginPeriod(1);
        Sleep(1);
        timeEndPeriod(1);
    }
}

//--------------------------------------------------------------------------------------
// Window::CalculationFrameTime()�֐��F1�t���[��������̎��Ԃ̌v��
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
// Window::GethWnd()�֐��FhWnd�̎擾
//--------------------------------------------------------------------------------------
HWND Window::GethWnd()
{
    return g_hWnd;
}

//--------------------------------------------------------------------------------------
// Window::GetClientWidth()�֐��F�N���C�A���g�̈�̉����̎擾
//--------------------------------------------------------------------------------------
int Window::GetClientWidth()
{
    return g_iClientWidth;
}

//--------------------------------------------------------------------------------------
// Window::GetClientHeight()�֐��F�N���C�A���g�̈�̍����̎擾
//--------------------------------------------------------------------------------------
int Window::GetClientHeight()
{
    return g_iClientHeight;
}

//--------------------------------------------------------------------------------------
// Window::GetFps()�֐��FFPS�̎擾
//--------------------------------------------------------------------------------------
double Window::GetFps()
{
    return g_dFps;
}

//--------------------------------------------------------------------------------------
// Window::GetFrameTime()�֐��F1�t���[��������̎��Ԃ̎擾
//--------------------------------------------------------------------------------------
double Window::GetFrameTime()
{
    return g_dFrameTime;
}