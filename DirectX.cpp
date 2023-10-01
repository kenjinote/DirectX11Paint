#include "Main.h"
#include "DirectX.h"

//--------------------------------------------------------------------------------------
// DirectX11::DirectX11()関数：コンストラクタ
//--------------------------------------------------------------------------------------
DirectX11::DirectX11()
{

}

//--------------------------------------------------------------------------------------
// DirectX11::~DirectX11関数：デストラクタ
//--------------------------------------------------------------------------------------
DirectX11::~DirectX11()
{

}

//--------------------------------------------------------------------------------------
// DirectX11::InitDevice()：DirectX関係の初期化
//--------------------------------------------------------------------------------------
HRESULT DirectX11::InitDevice()
{
    //------------------------------------------------------------
    // DirectX11とDirect2D 1.1の初期化
    //------------------------------------------------------------
    HRESULT hr = S_OK;

    UINT uiDeviceFlags = D3D11_CREATE_DEVICE_BGRA_SUPPORT;//DirectX11上でDirect2Dを使用するために必要
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
            D3D11_SDK_VERSION, &D3DDevice, &featurelevel, &m_D3DDeviceContext);//&D3DDevice &m_D3DDeviceContext 初期化

        if (hr == E_INVALIDARG)
        {
            hr = D3D11CreateDevice(nullptr, drivertype, nullptr, uiDeviceFlags, &featurelevels[1], uiFeatureLevelsNum - 1,
                D3D11_SDK_VERSION, &D3DDevice, &featurelevel, &m_D3DDeviceContext);//&D3DDevice &m_D3DDeviceContext 初期化
        }

        if (SUCCEEDED(hr))
            break;
    }
    if (FAILED(hr))
        return hr;

    Microsoft::WRL::ComPtr<IDXGIDevice2> DXGIDevice2;
    hr = D3DDevice.As(&DXGIDevice2);//D3DDevice->QueryInterface()ではなくD3DDevice.As()、&DXGIDevice2 初期化
    if (FAILED(hr))
        return hr;

    Microsoft::WRL::ComPtr<ID2D1Factory1> D2DFactory1;
    D2D1_FACTORY_OPTIONS factoryoptions = {};
#ifdef _DEBUG
    factoryoptions.debugLevel = D2D1_DEBUG_LEVEL_INFORMATION;
#endif
    hr = D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, factoryoptions, D2DFactory1.GetAddressOf());//&D2DFactory1ではなくD2DFactory1.GetAddressOf()
    if (FAILED(hr))
        return hr;

    Microsoft::WRL::ComPtr<ID2D1Device> D2D1Device;
    hr = D2DFactory1->CreateDevice(DXGIDevice2.Get(), &D2D1Device);//DXGIDevice2ではなくDXGIDevice2.Get()、&D2D1Device 初期化
    if (FAILED(hr))
        return hr;

    hr = D2D1Device->CreateDeviceContext(D2D1_DEVICE_CONTEXT_OPTIONS_NONE, &m_D2DDeviceContext);//&m_D2DDeviceContext 初期化
    if (FAILED(hr))
        return hr;

    Microsoft::WRL::ComPtr<IDXGIAdapter> DXGIAdapter;
    hr = DXGIDevice2->GetAdapter(&DXGIAdapter);//&DXGIAdapter 初期化
    if (FAILED(hr))
        return hr;

    Microsoft::WRL::ComPtr<IDXGIFactory2> DXGIFactory2;
    hr = DXGIAdapter->GetParent(IID_PPV_ARGS(&DXGIFactory2));//&DXGIFactory2 初期化
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

    hr = DXGIFactory2->CreateSwapChainForHwnd(D3DDevice.Get(), Window::GethWnd(), &desc, nullptr, nullptr, &m_DXGISwapChain1);//D3DDeviceではなくD3DDevice.Get()、&m_DXGISwapChain1 初期化
    if (FAILED(hr))
        return hr;

    (void)DXGIDevice2->SetMaximumFrameLatency(1);

    DXGIFactory2->MakeWindowAssociation(Window::GethWnd(), DXGI_MWA_NO_ALT_ENTER);//Alt+Enter時フルスクリーンを無効

    Microsoft::WRL::ComPtr<IDXGISurface2> DXGISurface2;
    hr = m_DXGISwapChain1->GetBuffer(0, IID_PPV_ARGS(&DXGISurface2));//&DXGISurface2 初期化
    if (FAILED(hr))
        return hr;

    hr = m_D2DDeviceContext->CreateBitmapFromDxgiSurface(DXGISurface2.Get(),//DXGISurface2ではなくDXGISurface2.Get()
        D2D1::BitmapProperties1(D2D1_BITMAP_OPTIONS_TARGET | D2D1_BITMAP_OPTIONS_CANNOT_DRAW,
            D2D1::PixelFormat(DXGI_FORMAT_B8G8R8A8_UNORM, D2D1_ALPHA_MODE_IGNORE)), &m_D2DBitmap1);//&m_D2DBitmap1 初期化
    if (FAILED(hr))
        return hr;

    //--------------------------★変更↓--------------------------
    //作業用ビットマップの作成
    D2D1_SIZE_U pixsize = m_D2DBitmap1->GetPixelSize();
    hr = m_D2DDeviceContext->CreateBitmap(pixsize, nullptr, pixsize.width * 4,
        D2D1::BitmapProperties1(D2D1_BITMAP_OPTIONS_TARGET,
            D2D1::PixelFormat(DXGI_FORMAT_B8G8R8A8_UNORM, D2D1_ALPHA_MODE_PREMULTIPLIED)), &m_D2DBackBitmap1);//&m_D2DBackBitmap1 初期化
    if (FAILED(hr))
        return hr;

    m_D2DDeviceContext->SetTarget(m_D2DBackBitmap1.Get());//作業用ビットマップに描画ターゲットとを移す
    m_D2DDeviceContext->BeginDraw();
    m_D2DDeviceContext->Clear(D2D1::ColorF(0, 0, 0, 0));//最初だけクリアしておく
    m_D2DDeviceContext->EndDraw();
    //--------------------------★変更↑--------------------------

    Microsoft::WRL::ComPtr<ID3D11Texture2D> D3DTexture2D;
    hr = m_DXGISwapChain1->GetBuffer(0, IID_PPV_ARGS(&D3DTexture2D));//&D3DTexture2D 初期化
    if (FAILED(hr))
        return hr;

    hr = D3DDevice->CreateRenderTargetView(D3DTexture2D.Get(), nullptr, &m_D3DRenderTargetView);//D3DTexture2DではなくD3DTexture2D.Get()、&m_D3DRenderTargetView 初期化
    if (FAILED(hr))
        return hr;

    m_D3DDeviceContext->OMSetRenderTargets(1, m_D3DRenderTargetView.GetAddressOf(), nullptr);//&m_D3DRenderTargetViewではなくm_D3DRenderTargetView.GetAddressOf()

    D3D11_VIEWPORT vp;
    vp.Width = (FLOAT)Window::GetClientWidth();
    vp.Height = (FLOAT)Window::GetClientHeight();
    vp.MinDepth = 0.0f;
    vp.MaxDepth = 1.0f;
    vp.TopLeftX = 0;
    vp.TopLeftY = 0;
    m_D3DDeviceContext->RSSetViewports(1, &vp);

    //------------------------------------------------------------
    // DirectWriteの初期化
    //------------------------------------------------------------
    Microsoft::WRL::ComPtr<IDWriteFactory> DWriteFactory;
    hr = DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED, __uuidof(IDWriteFactory), &DWriteFactory);//&DWriteFactory 初期化
    if (FAILED(hr))
        return hr;

    //関数CreateTextFormat()
    //第1引数：フォント名（L"メイリオ", L"Arial", L"Meiryo UI"等）
    //第2引数：フォントコレクション（nullptr）
    //第3引数：フォントの太さ（DWRITE_FONT_WEIGHT_NORMAL, DWRITE_FONT_WEIGHT_BOLD等）
    //第4引数：フォントスタイル（DWRITE_FONT_STYLE_NORMAL, DWRITE_FONT_STYLE_OBLIQUE, DWRITE_FONT_STYLE_ITALIC）
    //第5引数：フォントの幅（DWRITE_FONT_STRETCH_NORMAL,DWRITE_FONT_STRETCH_EXTRA_EXPANDED等）
    //第6引数：フォントサイズ（20, 30等）
    //第7引数：ロケール名（L""）
    //第8引数：テキストフォーマット（&g_pTextFormat）
    hr = DWriteFactory->CreateTextFormat(L"メイリオ", nullptr, DWRITE_FONT_WEIGHT_NORMAL, DWRITE_FONT_STYLE_NORMAL, DWRITE_FONT_STRETCH_NORMAL, 20, L"", &m_DWriteTextFormat);//&m_DWriteTextFormat 初期化
    if (FAILED(hr))
        return hr;

    //関数SetTextAlignment()
    //第1引数：テキストの配置（DWRITE_TEXT_ALIGNMENT_LEADING：前, DWRITE_TEXT_ALIGNMENT_TRAILING：後, DWRITE_TEXT_ALIGNMENT_CENTER：中央）
    hr = m_DWriteTextFormat->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_LEADING);
    if (FAILED(hr))
        return hr;

    //関数CreateSolidColorBrush()
    //第1引数：フォント色（D2D1::ColorF(D2D1::ColorF::Black)：黒, D2D1::ColorF(D2D1::ColorF(0.0f, 0.2f, 0.9f, 1.0f))：RGBA指定）
    hr = m_D2DDeviceContext->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Black), &m_D2DSolidBrush);//&m_D2DSolidBrush 初期化
    if (FAILED(hr))
        return hr;

    //--------------------------★変更↓--------------------------

    //--------------------------★変更↑--------------------------

    return S_OK;
}

//--------------------------------------------------------------------------------------
// DirectX11::Render()：DirectX関係の描画
//--------------------------------------------------------------------------------------
void DirectX11::Render()
{
    m_D3DDeviceContext->ClearRenderTargetView(m_D3DRenderTargetView.Get(), DirectX::Colors::Aquamarine);//m_D3DRenderTargetViewではなくm_D3DRenderTargetView.Get()

    //--------------------------★変更↓--------------------------

    //------------------------------------------------------------
    // キー入力関係
    //------------------------------------------------------------
    //マウス入力
    int iMouseL = 0, iMouseR = 0;
    if (GetAsyncKeyState(VK_LBUTTON) & 0x8000) iMouseL = 1;
    if (GetAsyncKeyState(VK_RBUTTON) & 0x8000) iMouseR = 1;

    POINT mousepoint_a;
    static FLOAT fMouseX_a, fMouseY_a;

    POINT mousepoint_b;
    static FLOAT fMouseX_b, fMouseY_b;

    //------------------------------------------------------------
    // 文字操作
    //------------------------------------------------------------
    //FPS表示用
    WCHAR wcText1[256] = { 0 };
    swprintf(wcText1, 256, L"FPS=%lf", Window::GetFps());

    //------------------------------------------------------------
    // 2D描画（作業用ビットマップに描画）
    //------------------------------------------------------------
    static int iDrawStart = 1;
    if (iMouseL)//マウス左ボタンが押されているとき
    {
        if (iDrawStart)//マウス左ボタンが押された時のマウス座標取得
        {
            GetCursorPos(&mousepoint_b);//マウスのスクリーン座標取得
            ScreenToClient(Window::GethWnd(), &mousepoint_b);//スクリーン座標をクライアント座標（アプリケーションの左上を(0, 0)）に変換
            fMouseX_b = static_cast<FLOAT>(mousepoint_b.x);
            fMouseY_b = static_cast<FLOAT>(mousepoint_b.y);

            iDrawStart = 0;
        }
        GetCursorPos(&mousepoint_a);//マウスのスクリーン座標取得
        ScreenToClient(Window::GethWnd(), &mousepoint_a);//スクリーン座標をクライアント座標（アプリケーションの左上を(0, 0)）に変換
        fMouseX_a = static_cast<FLOAT>(mousepoint_a.x);
        fMouseY_a = static_cast<FLOAT>(mousepoint_a.y);

        if (fMouseX_a != fMouseX_b || fMouseY_a != fMouseY_b)//マウス座標に変更があった場合、線を描画
        {
            m_D2DDeviceContext->SetTarget(m_D2DBackBitmap1.Get());//作業用ビットマップに描画ターゲットとを移す
            m_D2DDeviceContext->BeginDraw();
            m_D2DSolidBrush->SetColor(D2D1::ColorF(0.0f, 0.0f, 1.0f, 1.0f));//RGBA
            m_D2DDeviceContext->DrawLine(D2D1::Point2F(fMouseX_b, fMouseY_b), D2D1::Point2F(fMouseX_a, fMouseY_a), m_D2DSolidBrush.Get(), 1);//線を描画
            m_D2DDeviceContext->EndDraw();

            fMouseX_b = fMouseX_a;
            fMouseY_b = fMouseY_a;
        }
    }
    else//マウス左ボタンが押されていないとき
    {
        iDrawStart = 1;
    }

    if (iMouseL == 0 && iMouseR == 1)//マウス左ボタンが押されていない　かつ　マウス右ボタンが押されているとき作業用ビットマップをクリアする
    {
        m_D2DDeviceContext->SetTarget(m_D2DBackBitmap1.Get());//作業用ビットマップに描画ターゲットとを移す
        m_D2DDeviceContext->BeginDraw();
        m_D2DDeviceContext->Clear(D2D1::ColorF(0, 0, 0, 0));//クリア
        m_D2DDeviceContext->EndDraw();
    }

    //------------------------------------------------------------
    // 2D描画（ウィンドウに描画）
    //------------------------------------------------------------
    m_D2DDeviceContext->SetTarget(m_D2DBitmap1.Get());//ウィンドウ表示用ビットマップに描画ターゲットとを移す
    m_D2DDeviceContext->BeginDraw();
    m_D2DDeviceContext->DrawBitmap(m_D2DBackBitmap1.Get());//作業用ビットマップを描画
    //関数DrawBitmap()
    //第1引数：レンダリングするビットマップ（m_D2DBitmapではなくm_D2DBitmap.Get()）
    //第2引数：ビットマップの位置の座標（D2D1::RectF(左, 上, 右, 下)）
    //第3引数：不透明度（0.0f～1.0f）
    //第4引数：ビットマップが拡大縮小または回転される場合に使用する補間モード（D2D1_BITMAP_INTERPOLATION_MODE_NEAREST_NEIGHBOR：ドット絵風[ギザギザ]
    //                                                                         D2D1_BITMAP_INTERPOLATION_MODE_LINEAR：写真風[なめらか]）
    //第5引数：トリミング（D2D1::RectF(左, 上, 右, 下), nullptr：イメージ全体の場合）
    //m_D2DDeviceContext->DrawBitmap(m_D2DBitmap.Get(), D2D1::RectF(fPosX, fPosY, fPosX + 512, fPosY + 512), 1.0f, D2D1_BITMAP_INTERPOLATION_MODE_NEAREST_NEIGHBOR, D2D1::RectF(0, 0, iPixSize, iPixSize));
    m_D2DSolidBrush->SetColor(D2D1::ColorF(0, 0, 0, 1));//RGBA
    m_D2DDeviceContext->DrawText(wcText1, ARRAYSIZE(wcText1) - 1, m_DWriteTextFormat.Get(), D2D1::RectF(0, 0, 800, 20), m_D2DSolidBrush.Get(), D2D1_DRAW_TEXT_OPTIONS_NONE);//m_DWriteTextFormatではなくm_DWriteTextFormat.Get()
    m_D2DDeviceContext->EndDraw();
    //--------------------------★変更↑--------------------------

    m_DXGISwapChain1->Present(0, 0);
}