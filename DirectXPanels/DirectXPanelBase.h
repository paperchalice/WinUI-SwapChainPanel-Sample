#pragma once
#include "DirectXPanelBase.g.h"

#include <d2d1_2.h>
#include <d3d11_1.h>
#include <dxgi1_3.h>
#include <mutex>

namespace winrt::DirectXPanels::implementation
{
struct DirectXPanelBase : DirectXPanelBaseT<DirectXPanelBase>
{
    DirectXPanelBase();

  protected:
    void RegisterEventHandlers();
    virtual void CreateDeviceIndependentResources();
    virtual void CreateDeviceResources();
    virtual void CreateSizeDependentResources();

    virtual void OnDeviceLost();
    virtual void OnSizeChanged(const winrt::Windows::Foundation::IInspectable &sender,
                               const winrt::Microsoft::UI::Xaml::SizeChangedEventArgs &e);
    virtual void OnCompositionScaleChanged(const winrt::Microsoft::UI::Xaml::Controls::SwapChainPanel &sender,
                                           const winrt::Windows::Foundation::IInspectable &args);

    virtual void Render() {};
    virtual void Present();

  protected:
    winrt::com_ptr<ID3D11Device1> m_d3dDevice;
    winrt::com_ptr<ID3D11DeviceContext1> m_d3dContext;
    winrt::com_ptr<IDXGISwapChain2> m_swapChain;

    winrt::com_ptr<ID2D1Factory2> m_d2dFactory;
    winrt::com_ptr<ID2D1Device> m_d2dDevice;
    winrt::com_ptr<ID2D1DeviceContext> m_d2dContext;
    winrt::com_ptr<ID2D1Bitmap1> m_d2dTargetBitmap;

    D2D1_COLOR_F m_backgroundColor;
    DXGI_ALPHA_MODE m_alphaMode;

    bool m_loadingComplete;

    std::mutex m_mutex;

    float m_renderTargetHeight;
    float m_renderTargetWidth;

    float m_compositionScaleX;
    float m_compositionScaleY;

    float m_height;
    float m_width;
};
} // namespace winrt::DirectXPanels::implementation
namespace winrt::DirectXPanels::factory_implementation
{
struct DirectXPanelBase : DirectXPanelBaseT<DirectXPanelBase, implementation::DirectXPanelBase>
{
};
} // namespace winrt::DirectXPanels::factory_implementation
