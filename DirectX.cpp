#include "Main.h"
#include "DirectX.h"

//--------------------------------------------------------------------------------------
// DirectX11::DirectX11()�֐��F�R���X�g���N�^
//--------------------------------------------------------------------------------------
DirectX11::DirectX11()
{

}

//--------------------------------------------------------------------------------------
// DirectX11::~DirectX11�֐��F�f�X�g���N�^
//--------------------------------------------------------------------------------------
DirectX11::~DirectX11()
{

}

//--------------------------------------------------------------------------------------
// DirectX11::InitDevice()�FDirectX�֌W�̏�����
//--------------------------------------------------------------------------------------
HRESULT DirectX11::InitDevice()
{
    //------------------------------------------------------------
    // DirectX11��Direct2D 1.1�̏�����
    //------------------------------------------------------------
    HRESULT hr = S_OK;

    UINT uiDeviceFlags = D3D11_CREATE_DEVICE_BGRA_SUPPORT;//DirectX11���Direct2D���g�p���邽�߂ɕK�v
#ifdef _DEBUG
    uiDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

    D3D_DRIVER_TYPE drivertypes[] =
    {
        D3D_DRIVER_TYPE_HARDWARE,
        D3D_DRIVER_TYPE_WARP,
        D3D_DRIVER_TYPE_REFERENCE,
    };
    UINT uiDriverTypesNum = ARRAYSIZE(drivertypes);

    D3D_FEATURE_LEVEL featurelevels[] =
    {
        D3D_FEATURE_LEVEL_11_1,
        D3D_FEATURE_LEVEL_11_0,
        D3D_FEATURE_LEVEL_10_1,
        D3D_FEATURE_LEVEL_10_0,
    };
    UINT uiFeatureLevelsNum = ARRAYSIZE(featurelevels);

    Microsoft::WRL::ComPtr<ID3D11Device> D3DDevice;
    D3D_DRIVER_TYPE drivertype = D3D_DRIVER_TYPE_NULL;
    D3D_FEATURE_LEVEL featurelevel = D3D_FEATURE_LEVEL_11_0;
    for (UINT uiDriverTypeIndex = 0; uiDriverTypeIndex < uiDriverTypesNum; uiDriverTypeIndex++)
    {
        drivertype = drivertypes[uiDriverTypeIndex];
        hr = D3D11CreateDevice(nullptr, drivertype, nullptr, uiDeviceFlags, featurelevels, uiFeatureLevelsNum,
            D3D11_SDK_VERSION, &D3DDevice, &featurelevel, &m_D3DDeviceContext);//&D3DDevice &m_D3DDeviceContext ������

        if (hr == E_INVALIDARG)
        {
            hr = D3D11CreateDevice(nullptr, drivertype, nullptr, uiDeviceFlags, &featurelevels[1], uiFeatureLevelsNum - 1,
                D3D11_SDK_VERSION, &D3DDevice, &featurelevel, &m_D3DDeviceContext);//&D3DDevice &m_D3DDeviceContext ������
        }

        if (SUCCEEDED(hr))
            break;
    }
    if (FAILED(hr))
        return hr;

    Microsoft::WRL::ComPtr<IDXGIDevice2> DXGIDevice2;
    hr = D3DDevice.As(&DXGIDevice2);//D3DDevice->QueryInterface()�ł͂Ȃ�D3DDevice.As()�A&DXGIDevice2 ������
    if (FAILED(hr))
        return hr;

    Microsoft::WRL::ComPtr<ID2D1Factory1> D2DFactory1;
    D2D1_FACTORY_OPTIONS factoryoptions = {};
#ifdef _DEBUG
    factoryoptions.debugLevel = D2D1_DEBUG_LEVEL_INFORMATION;
#endif
    hr = D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, factoryoptions, D2DFactory1.GetAddressOf());//&D2DFactory1�ł͂Ȃ�D2DFactory1.GetAddressOf()
    if (FAILED(hr))
        return hr;

    Microsoft::WRL::ComPtr<ID2D1Device> D2D1Device;
    hr = D2DFactory1->CreateDevice(DXGIDevice2.Get(), &D2D1Device);//DXGIDevice2�ł͂Ȃ�DXGIDevice2.Get()�A&D2D1Device ������
    if (FAILED(hr))
        return hr;

    hr = D2D1Device->CreateDeviceContext(D2D1_DEVICE_CONTEXT_OPTIONS_NONE, &m_D2DDeviceContext);//&m_D2DDeviceContext ������
    if (FAILED(hr))
        return hr;

    Microsoft::WRL::ComPtr<IDXGIAdapter> DXGIAdapter;
    hr = DXGIDevice2->GetAdapter(&DXGIAdapter);//&DXGIAdapter ������
    if (FAILED(hr))
        return hr;

    Microsoft::WRL::ComPtr<IDXGIFactory2> DXGIFactory2;
    hr = DXGIAdapter->GetParent(IID_PPV_ARGS(&DXGIFactory2));//&DXGIFactory2 ������
    if (FAILED(hr))
        return hr;

    DXGI_SWAP_CHAIN_DESC1 desc = {};
    desc.Width = Window::GetClientWidth();
    desc.Height = Window::GetClientHeight();
    desc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
    desc.Stereo = FALSE;
    desc.SampleDesc.Count = 1;
    desc.SampleDesc.Quality = 0;
    desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    desc.BufferCount = 2;
    desc.Scaling = DXGI_SCALING_STRETCH;
    desc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
    desc.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;

    hr = DXGIFactory2->CreateSwapChainForHwnd(D3DDevice.Get(), Window::GethWnd(), &desc, nullptr, nullptr, &m_DXGISwapChain1);//D3DDevice�ł͂Ȃ�D3DDevice.Get()�A&m_DXGISwapChain1 ������
    if (FAILED(hr))
        return hr;

    (void)DXGIDevice2->SetMaximumFrameLatency(1);

    DXGIFactory2->MakeWindowAssociation(Window::GethWnd(), DXGI_MWA_NO_ALT_ENTER);//Alt+Enter���t���X�N���[���𖳌�

    Microsoft::WRL::ComPtr<IDXGISurface2> DXGISurface2;
    hr = m_DXGISwapChain1->GetBuffer(0, IID_PPV_ARGS(&DXGISurface2));//&DXGISurface2 ������
    if (FAILED(hr))
        return hr;

    hr = m_D2DDeviceContext->CreateBitmapFromDxgiSurface(DXGISurface2.Get(),//DXGISurface2�ł͂Ȃ�DXGISurface2.Get()
        D2D1::BitmapProperties1(D2D1_BITMAP_OPTIONS_TARGET | D2D1_BITMAP_OPTIONS_CANNOT_DRAW,
            D2D1::PixelFormat(DXGI_FORMAT_B8G8R8A8_UNORM, D2D1_ALPHA_MODE_IGNORE)), &m_D2DBitmap1);//&m_D2DBitmap1 ������
    if (FAILED(hr))
        return hr;

    //--------------------------���ύX��--------------------------
    //��Ɨp�r�b�g�}�b�v�̍쐬
    D2D1_SIZE_U pixsize = m_D2DBitmap1->GetPixelSize();
    hr = m_D2DDeviceContext->CreateBitmap(pixsize, nullptr, pixsize.width * 4,
        D2D1::BitmapProperties1(D2D1_BITMAP_OPTIONS_TARGET,
            D2D1::PixelFormat(DXGI_FORMAT_B8G8R8A8_UNORM, D2D1_ALPHA_MODE_PREMULTIPLIED)), &m_D2DBackBitmap1);//&m_D2DBackBitmap1 ������
    if (FAILED(hr))
        return hr;

    m_D2DDeviceContext->SetTarget(m_D2DBackBitmap1.Get());//��Ɨp�r�b�g�}�b�v�ɕ`��^�[�Q�b�g�Ƃ��ڂ�
    m_D2DDeviceContext->BeginDraw();
    m_D2DDeviceContext->Clear(D2D1::ColorF(0, 0, 0, 0));//�ŏ������N���A���Ă���
    m_D2DDeviceContext->EndDraw();
    //--------------------------���ύX��--------------------------

    Microsoft::WRL::ComPtr<ID3D11Texture2D> D3DTexture2D;
    hr = m_DXGISwapChain1->GetBuffer(0, IID_PPV_ARGS(&D3DTexture2D));//&D3DTexture2D ������
    if (FAILED(hr))
        return hr;

    hr = D3DDevice->CreateRenderTargetView(D3DTexture2D.Get(), nullptr, &m_D3DRenderTargetView);//D3DTexture2D�ł͂Ȃ�D3DTexture2D.Get()�A&m_D3DRenderTargetView ������
    if (FAILED(hr))
        return hr;

    m_D3DDeviceContext->OMSetRenderTargets(1, m_D3DRenderTargetView.GetAddressOf(), nullptr);//&m_D3DRenderTargetView�ł͂Ȃ�m_D3DRenderTargetView.GetAddressOf()

    D3D11_VIEWPORT vp;
    vp.Width = (FLOAT)Window::GetClientWidth();
    vp.Height = (FLOAT)Window::GetClientHeight();
    vp.MinDepth = 0.0f;
    vp.MaxDepth = 1.0f;
    vp.TopLeftX = 0;
    vp.TopLeftY = 0;
    m_D3DDeviceContext->RSSetViewports(1, &vp);

    //------------------------------------------------------------
    // DirectWrite�̏�����
    //------------------------------------------------------------
    Microsoft::WRL::ComPtr<IDWriteFactory> DWriteFactory;
    hr = DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED, __uuidof(IDWriteFactory), &DWriteFactory);//&DWriteFactory ������
    if (FAILED(hr))
        return hr;

    //�֐�CreateTextFormat()
    //��1�����F�t�H���g���iL"���C���I", L"Arial", L"Meiryo UI"���j
    //��2�����F�t�H���g�R���N�V�����inullptr�j
    //��3�����F�t�H���g�̑����iDWRITE_FONT_WEIGHT_NORMAL, DWRITE_FONT_WEIGHT_BOLD���j
    //��4�����F�t�H���g�X�^�C���iDWRITE_FONT_STYLE_NORMAL, DWRITE_FONT_STYLE_OBLIQUE, DWRITE_FONT_STYLE_ITALIC�j
    //��5�����F�t�H���g�̕��iDWRITE_FONT_STRETCH_NORMAL,DWRITE_FONT_STRETCH_EXTRA_EXPANDED���j
    //��6�����F�t�H���g�T�C�Y�i20, 30���j
    //��7�����F���P�[�����iL""�j
    //��8�����F�e�L�X�g�t�H�[�}�b�g�i&g_pTextFormat�j
    hr = DWriteFactory->CreateTextFormat(L"���C���I", nullptr, DWRITE_FONT_WEIGHT_NORMAL, DWRITE_FONT_STYLE_NORMAL, DWRITE_FONT_STRETCH_NORMAL, 20, L"", &m_DWriteTextFormat);//&m_DWriteTextFormat ������
    if (FAILED(hr))
        return hr;

    //�֐�SetTextAlignment()
    //��1�����F�e�L�X�g�̔z�u�iDWRITE_TEXT_ALIGNMENT_LEADING�F�O, DWRITE_TEXT_ALIGNMENT_TRAILING�F��, DWRITE_TEXT_ALIGNMENT_CENTER�F�����j
    hr = m_DWriteTextFormat->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_LEADING);
    if (FAILED(hr))
        return hr;

    //�֐�CreateSolidColorBrush()
    //��1�����F�t�H���g�F�iD2D1::ColorF(D2D1::ColorF::Black)�F��, D2D1::ColorF(D2D1::ColorF(0.0f, 0.2f, 0.9f, 1.0f))�FRGBA�w��j
    hr = m_D2DDeviceContext->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Black), &m_D2DSolidBrush);//&m_D2DSolidBrush ������
    if (FAILED(hr))
        return hr;

    //--------------------------���ύX��--------------------------

    //--------------------------���ύX��--------------------------

    return S_OK;
}

//--------------------------------------------------------------------------------------
// DirectX11::Render()�FDirectX�֌W�̕`��
//--------------------------------------------------------------------------------------
void DirectX11::Render()
{
    m_D3DDeviceContext->ClearRenderTargetView(m_D3DRenderTargetView.Get(), DirectX::Colors::Aquamarine);//m_D3DRenderTargetView�ł͂Ȃ�m_D3DRenderTargetView.Get()

    //--------------------------���ύX��--------------------------

    //------------------------------------------------------------
    // �L�[���͊֌W
    //------------------------------------------------------------
    //�}�E�X����
    int iMouseL = 0, iMouseR = 0;
    if (GetAsyncKeyState(VK_LBUTTON) & 0x8000) iMouseL = 1;
    if (GetAsyncKeyState(VK_RBUTTON) & 0x8000) iMouseR = 1;

    POINT mousepoint_a;
    static FLOAT fMouseX_a, fMouseY_a;

    POINT mousepoint_b;
    static FLOAT fMouseX_b, fMouseY_b;

    //------------------------------------------------------------
    // ��������
    //------------------------------------------------------------
    //FPS�\���p
    WCHAR wcText1[256] = { 0 };
    swprintf(wcText1, 256, L"FPS=%lf", Window::GetFps());

    //------------------------------------------------------------
    // 2D�`��i��Ɨp�r�b�g�}�b�v�ɕ`��j
    //------------------------------------------------------------
    static int iDrawStart = 1;
    if (iMouseL)//�}�E�X���{�^����������Ă���Ƃ�
    {
        if (iDrawStart)//�}�E�X���{�^���������ꂽ���̃}�E�X���W�擾
        {
            GetCursorPos(&mousepoint_b);//�}�E�X�̃X�N���[�����W�擾
            ScreenToClient(Window::GethWnd(), &mousepoint_b);//�X�N���[�����W���N���C�A���g���W�i�A�v���P�[�V�����̍����(0, 0)�j�ɕϊ�
            fMouseX_b = static_cast<FLOAT>(mousepoint_b.x);
            fMouseY_b = static_cast<FLOAT>(mousepoint_b.y);

            iDrawStart = 0;
        }
        GetCursorPos(&mousepoint_a);//�}�E�X�̃X�N���[�����W�擾
        ScreenToClient(Window::GethWnd(), &mousepoint_a);//�X�N���[�����W���N���C�A���g���W�i�A�v���P�[�V�����̍����(0, 0)�j�ɕϊ�
        fMouseX_a = static_cast<FLOAT>(mousepoint_a.x);
        fMouseY_a = static_cast<FLOAT>(mousepoint_a.y);

        if (fMouseX_a != fMouseX_b || fMouseY_a != fMouseY_b)//�}�E�X���W�ɕύX���������ꍇ�A����`��
        {
            m_D2DDeviceContext->SetTarget(m_D2DBackBitmap1.Get());//��Ɨp�r�b�g�}�b�v�ɕ`��^�[�Q�b�g�Ƃ��ڂ�
            m_D2DDeviceContext->BeginDraw();
            m_D2DSolidBrush->SetColor(D2D1::ColorF(0.0f, 0.0f, 1.0f, 1.0f));//RGBA
            m_D2DDeviceContext->DrawLine(D2D1::Point2F(fMouseX_b, fMouseY_b), D2D1::Point2F(fMouseX_a, fMouseY_a), m_D2DSolidBrush.Get(), 1);//����`��
            m_D2DDeviceContext->EndDraw();

            fMouseX_b = fMouseX_a;
            fMouseY_b = fMouseY_a;
        }
    }
    else//�}�E�X���{�^����������Ă��Ȃ��Ƃ�
    {
        iDrawStart = 1;
    }

    if (iMouseL == 0 && iMouseR == 1)//�}�E�X���{�^����������Ă��Ȃ��@���@�}�E�X�E�{�^����������Ă���Ƃ���Ɨp�r�b�g�}�b�v���N���A����
    {
        m_D2DDeviceContext->SetTarget(m_D2DBackBitmap1.Get());//��Ɨp�r�b�g�}�b�v�ɕ`��^�[�Q�b�g�Ƃ��ڂ�
        m_D2DDeviceContext->BeginDraw();
        m_D2DDeviceContext->Clear(D2D1::ColorF(0, 0, 0, 0));//�N���A
        m_D2DDeviceContext->EndDraw();
    }

    //------------------------------------------------------------
    // 2D�`��i�E�B���h�E�ɕ`��j
    //------------------------------------------------------------
    m_D2DDeviceContext->SetTarget(m_D2DBitmap1.Get());//�E�B���h�E�\���p�r�b�g�}�b�v�ɕ`��^�[�Q�b�g�Ƃ��ڂ�
    m_D2DDeviceContext->BeginDraw();
    m_D2DDeviceContext->DrawBitmap(m_D2DBackBitmap1.Get());//��Ɨp�r�b�g�}�b�v��`��
    //�֐�DrawBitmap()
    //��1�����F�����_�����O����r�b�g�}�b�v�im_D2DBitmap�ł͂Ȃ�m_D2DBitmap.Get()�j
    //��2�����F�r�b�g�}�b�v�̈ʒu�̍��W�iD2D1::RectF(��, ��, �E, ��)�j
    //��3�����F�s�����x�i0.0f�`1.0f�j
    //��4�����F�r�b�g�}�b�v���g��k���܂��͉�]�����ꍇ�Ɏg�p�����ԃ��[�h�iD2D1_BITMAP_INTERPOLATION_MODE_NEAREST_NEIGHBOR�F�h�b�g�G��[�M�U�M�U]
    //                                                                         D2D1_BITMAP_INTERPOLATION_MODE_LINEAR�F�ʐ^��[�Ȃ߂炩]�j
    //��5�����F�g���~���O�iD2D1::RectF(��, ��, �E, ��), nullptr�F�C���[�W�S�̂̏ꍇ�j
    //m_D2DDeviceContext->DrawBitmap(m_D2DBitmap.Get(), D2D1::RectF(fPosX, fPosY, fPosX + 512, fPosY + 512), 1.0f, D2D1_BITMAP_INTERPOLATION_MODE_NEAREST_NEIGHBOR, D2D1::RectF(0, 0, iPixSize, iPixSize));
    m_D2DSolidBrush->SetColor(D2D1::ColorF(0, 0, 0, 1));//RGBA
    m_D2DDeviceContext->DrawText(wcText1, ARRAYSIZE(wcText1) - 1, m_DWriteTextFormat.Get(), D2D1::RectF(0, 0, 800, 20), m_D2DSolidBrush.Get(), D2D1_DRAW_TEXT_OPTIONS_NONE);//m_DWriteTextFormat�ł͂Ȃ�m_DWriteTextFormat.Get()
    m_D2DDeviceContext->EndDraw();
    //--------------------------���ύX��--------------------------

    m_DXGISwapChain1->Present(0, 0);
}