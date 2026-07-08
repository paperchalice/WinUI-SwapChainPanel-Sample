#pragma once

// clang-format off
#include "D2DPanel.g.h"
#include "DirectXPanelBase.h"
// clang-format on

#include <d2d1.h>

namespace winrt::DirectXPanels::implementation
{
struct D2DPanel : D2DPanelT<D2DPanel, DirectXPanels::implementation::DirectXPanelBase>
{
    D2DPanel();

  private:
    void Render() override;
    void CreateDeviceResources() override;

    void OnDeviceLost() override;
    void OnSizeChanged(const winrt::Windows::Foundation::IInspectable &sender,
                       const winrt::Microsoft::UI::Xaml::SizeChangedEventArgs &e) override;
    void OnCompositionScaleChanged(const winrt::Microsoft::UI::Xaml::Controls::SwapChainPanel &sender,
                                   const winrt::Windows::Foundation::IInspectable &args) override;

    winrt::com_ptr<ID2D1SolidColorBrush> m_strokeBrush;
};
} // namespace winrt::DirectXPanels::implementation
namespace winrt::DirectXPanels::factory_implementation
{
struct D2DPanel : D2DPanelT<D2DPanel, implementation::D2DPanel>
{
};
} // namespace winrt::DirectXPanels::factory_implementation
