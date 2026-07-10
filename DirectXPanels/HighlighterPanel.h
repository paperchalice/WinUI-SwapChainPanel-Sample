#pragma once

// clang-format off
#include "HighlighterPanel.g.h"
#include "DirectXPanelBase.h"
// clang-format on

namespace winrt::DirectXPanels::implementation
{
struct HighlighterPanel : HighlighterPanelT<HighlighterPanel, DirectXPanels::implementation::DirectXPanelBase>
{
    HighlighterPanel();

    void StartProcessingInput();
    void StopProcessingInput();

  private:
    enum class DrawingState
    {
        None = 0,
        Inking
    };
    void Render() override;
    void CreateDeviceResources() override;
    void CreateSizeDependentResources() override;

    void OnDeviceLost() override;
    void OnSizeChanged(const winrt::Windows::Foundation::IInspectable &sender,
                       const Microsoft::UI::Xaml::SizeChangedEventArgs &e) override;
    void OnCompositionScaleChanged(const winrt::Microsoft::UI::Xaml::Controls::SwapChainPanel &sender,
                                   const winrt::Windows::Foundation::IInspectable &args) override;

    void OnPointerPressed(const winrt::Microsoft::UI::Input::InputPointerSource &sender,
                          const winrt::Microsoft::UI::Input::PointerEventArgs &e);
    void OnPointerMoved(const winrt::Microsoft::UI::Input::InputPointerSource &sender,
                        const winrt::Microsoft::UI::Input::PointerEventArgs &e);
    void OnPointerReleased(const winrt::Microsoft::UI::Input::InputPointerSource &sender,
                           const winrt::Microsoft::UI::Input::PointerEventArgs &e);

  private:
    DrawingState m_drawingState;

    Microsoft::UI::Dispatching::DispatcherQueueController m_dispatcherQueueController; 
    std::optional<Microsoft::UI::Input::InputPointerSource> m_coreInput;
    Windows::Foundation::IAsyncAction m_inputLoopWorker;

    winrt::com_ptr<ID3D11Texture2D> m_currentBuffer;
    winrt::com_ptr<ID3D11Texture2D> m_previousBuffer;

    winrt::com_ptr<ID2D1StrokeStyle> m_inkStrokeStyle;
    winrt::com_ptr<ID2D1SolidColorBrush> m_strokeBrush;

    Windows::Foundation::Point m_previousPoint;
    unsigned int m_activePointerId;
};
} // namespace winrt::DirectXPanels::implementation
namespace winrt::DirectXPanels::factory_implementation
{
struct HighlighterPanel : HighlighterPanelT<HighlighterPanel, implementation::HighlighterPanel>
{
};
} // namespace winrt::DirectXPanels::factory_implementation
