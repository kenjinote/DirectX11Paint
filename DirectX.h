#pragma once

#pragma comment(lib,"d3d11.lib")
#pragma comment(lib,"d2d1.lib")
#pragma comment(lib,"dwrite.lib")
#include <d3d11_1.h>
#include <directxcolors.h>
#include <d2d1_1.h>
#include <dwrite.h>
#include <wchar.h>
#include <wrl/client.h>
#include <random>

//--------------------------------------------------------------------------------------
// DirectX11クラス：DirectX関係
//--------------------------------------------------------------------------------------
class DirectX11
{
public:
    DirectX11();
    ~DirectX11();
    HRESULT InitDevice();
    void Render();
private:
    //------------------------------------------------------------
    // DirectX11とDirect2D 1.1の初期化
    //------------------------------------------------------------
    Microsoft::WRL::ComPtr<ID3D11DeviceContext> m_D3DDeviceContext;
    Microsoft::WRL::ComPtr<ID2D1DeviceContext> m_D2DDeviceContext;
    Microsoft::WRL::ComPtr<IDXGISwapChain1> m_DXGISwapChain1;
    Microsoft::WRL::ComPtr<ID2D1Bitmap1> m_D2DBitmap1;
    Microsoft::WRL::ComPtr<ID2D1Bitmap1> m_D2DBackBitmap1;//---★追加---
    Microsoft::WRL::ComPtr<ID3D11RenderTargetView> m_D3DRenderTargetView;

    //------------------------------------------------------------
    // DirectWriteの初期化
    //------------------------------------------------------------
    Microsoft::WRL::ComPtr<IDWriteTextFormat> m_DWriteTextFormat;
    Microsoft::WRL::ComPtr<ID2D1SolidColorBrush> m_D2DSolidBrush;

    //------------------------------------------------------------
    // ビットマップ作成
    //------------------------------------------------------------
    Microsoft::WRL::ComPtr<ID2D1Bitmap> m_D2DBitmap;
};