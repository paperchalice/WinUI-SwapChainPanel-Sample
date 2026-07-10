#include "pch.h"

#define WINRT_IMPORT_MODULE
import winrt.DirectXPanels;
import winrt.Microsoft.UI.Composition;
import winrt.Microsoft.UI.Dispatching;
import winrt.Microsoft.UI.Input;
import winrt.Microsoft.UI.Xaml;
import winrt.Microsoft.UI.Xaml.Controls;
import winrt.Windows.System.Threading;
import DX;

// clang-format off
#include "HighlighterPanel.h"
#include "HighlighterPanel.g.cpp"
// clang-format on

using namespace winrt;
using namespace Windows::Foundation;
using namespace Microsoft::UI::Xaml;
using namespace Windows::UI;
using namespace Windows::UI::Core;
using namespace Windows::System::Threading;
using namespace Microsoft::UI::Input;
using namespace Microsoft::UI::Dispatching;
using namespace D2D1;
using namespace DX;

namespace winrt::DirectXPanels::implementation
{
HighlighterPanel::HighlighterPanel()
    : m_drawingState(DrawingState::None),
      m_dispatcherQueueController{DispatcherQueueController::CreateOnDedicatedThread()}, m_coreInput(),
      m_activePointerId(0)
{
    // Set alpha mode to premultiplied to enable transparency.
    m_alphaMode = DXGI_ALPHA_MODE_PREMULTIPLIED;
    // Set background color to transparent white.
    m_backgroundColor = ColorF(0.0, 0.0, 0.0, 0.1);

    std::scoped_lock lock(m_mutex);

    CreateDeviceIndependentResources();
    CreateDeviceResources();
    CreateSizeDependentResources();
}

void HighlighterPanel::StartProcessingInput()
{
    // Initialize the rendering surface and prepares it to receive input.

    // Create a task to register for independent input and begin processing input messages.
    auto workItemHandler = [this]{
        // The CoreIndependentInputSource will raise pointer events for the specified device types on whichever
        // thread it's created on.
        m_coreInput = this->CreateCoreIndependentInputSource(InputPointerSourceDeviceKinds::Mouse |
                                                             InputPointerSourceDeviceKinds::Pen |
                                                             InputPointerSourceDeviceKinds::Touch);

        // Register for pointer events, which will be raised on the background thread.
        m_coreInput->PointerPressed({this, &HighlighterPanel::OnPointerPressed});
        m_coreInput->PointerMoved({this, &HighlighterPanel::OnPointerMoved});
        m_coreInput->PointerReleased({this, &HighlighterPanel::OnPointerReleased});
    };

    // Run task on a dedicated high priority background thread.
    m_dispatcherQueueController.DispatcherQueue().TryEnqueue(workItemHandler);
}
void HighlighterPanel::StopProcessingInput()
{
    // A call to ProcessEvents() with the ProcessUntilQuit flag will only return by default when the window closes.
    // Calling StopProcessEvents allows ProcessEvents to return even if the window isn't closing so the background
    // thread can exit.
    // m_coreInput->DispatcherQueue().EnqueueEventLoopExit();
    // m_dispatcherQueueController.DispatcherQueue()
}
void HighlighterPanel::Render()
{
    if (!m_loadingComplete)
    {
        return;
    }

    // Render and present the DirectX content.

    m_d2dContext->BeginDraw();

    // Note that in this simple example, the strokes the user has drawn are not preserved when the panel's size changes
    // or when the device is lost. For an example of preserving strokes, see the DrawingPanel implemention used in
    // Scenario2.

    // Clear the surface with a transparent background color.
    m_d2dContext->Clear(m_backgroundColor);

    HRESULT hr = m_d2dContext->EndDraw();

    // We ignore D2DERR_RECREATE_TARGET here. This error indicates that the device
    // is lost. It will be handled during the next call to Present.
    if (hr != D2DERR_RECREATE_TARGET)
    {
        ThrowIfFailed(hr);
    }
    Present();
}
void HighlighterPanel::CreateDeviceResources()
{
    DirectXPanelBase::CreateDeviceResources();

    // Create stroke style.
    ThrowIfFailed(m_d2dFactory->CreateStrokeStyle(
        D2D1::StrokeStyleProperties(D2D1_CAP_STYLE_ROUND, D2D1_CAP_STYLE_ROUND, D2D1_CAP_STYLE_ROUND,
                                    D2D1_LINE_JOIN_ROUND, 1.0f, D2D1_DASH_STYLE_SOLID, 0.0f),
        nullptr, 0, m_inkStrokeStyle.put()));

    // Create stroke brush.
    ThrowIfFailed(m_d2dContext->CreateSolidColorBrush(ColorF(ColorF::Yellow), m_strokeBrush.put()));

    m_loadingComplete = true;
}
void HighlighterPanel::CreateSizeDependentResources()
{
    m_currentBuffer = nullptr;
    m_previousBuffer = nullptr;

    DirectXPanelBase::CreateSizeDependentResources();

    ThrowIfFailed(m_swapChain->GetBuffer(0, guid_of<decltype(m_currentBuffer)::type>(), m_currentBuffer.put_void()));

    ThrowIfFailed(m_swapChain->GetBuffer(1, guid_of<decltype(m_previousBuffer)::type>(), m_previousBuffer.put_void()));
}
void HighlighterPanel::OnDeviceLost()
{
    // Handle device lost, then re-render.
    DirectXPanelBase::OnDeviceLost();
    Render();
}
void HighlighterPanel::OnSizeChanged(const winrt::Windows::Foundation::IInspectable &sender,
                                     const Microsoft::UI::Xaml::SizeChangedEventArgs &e)
{
    // Process SizeChanged event, then re-render at the new size.
    DirectXPanelBase::OnSizeChanged(sender, e);

    std::scoped_lock lock(m_mutex);
    Render();
}
void HighlighterPanel::OnCompositionScaleChanged(
    [[maybe_unused]] const winrt::Microsoft::UI::Xaml::Controls::SwapChainPanel &sender,
    [[maybe_unused]] const winrt::Windows::Foundation::IInspectable &args)
{
    std::scoped_lock lock(m_mutex);
    Render();
}
void HighlighterPanel::OnPointerPressed([[maybe_unused]] const winrt::Microsoft::UI::Input::InputPointerSource &sender,
                                        const winrt::Microsoft::UI::Input::PointerEventArgs &e)
{
    // Handle the PointerPressed event, which will be raised on a background thread.

    if (e.CurrentPoint().Properties().PointerUpdateKind() == PointerUpdateKind::LeftButtonPressed &&
        m_drawingState == DrawingState::None)
    {
        m_drawingState = DrawingState::Inking;
        m_previousPoint = e.CurrentPoint().Position();
        // Store active pointer ID: only one contact can be inking at a time.
        m_activePointerId = e.CurrentPoint().PointerId();
    }
}
void HighlighterPanel::OnPointerMoved([[maybe_unused]] const winrt::Microsoft::UI::Input::InputPointerSource &sender,
                                      const winrt::Microsoft::UI::Input::PointerEventArgs &e)
{
    if (!m_loadingComplete)
    {
        return;
    }

    // Handle the PointerMoved event, which will be raised on a background thread.

    if (m_drawingState == DrawingState::Inking && e.CurrentPoint().PointerId() == m_activePointerId)
    {
        std::scoped_lock lock(m_mutex);

        auto pointerPosition = e.CurrentPoint().Position();

        // While actively inking, copy the back buffer to the active buffer then draw the current stroke segment.
        m_d3dContext->CopyResource(m_currentBuffer.get(), m_previousBuffer.get());

        m_d2dContext->BeginDraw();

        m_d2dContext->DrawLine(D2D1::Point2F(m_previousPoint.X, m_previousPoint.Y),
                               D2D1::Point2F(pointerPosition.X, pointerPosition.Y), m_strokeBrush.get(), 10.0f,
                               m_inkStrokeStyle.get());

        m_previousPoint = pointerPosition;

        HRESULT hr = m_d2dContext->EndDraw();

        // We ignore D2DERR_RECREATE_TARGET here. This error indicates that the device
        // is lost. It will be handled during the next call to Present.
        if (hr != D2DERR_RECREATE_TARGET)
        {
            ThrowIfFailed(hr);
        }
        Present();
    }
}
void HighlighterPanel::OnPointerReleased([[maybe_unused]] const winrt::Microsoft::UI::Input::InputPointerSource &sender,
                                         const winrt::Microsoft::UI::Input::PointerEventArgs &e)
{
    // Handle the PointerReleased event, which will be raised on a background thread.

    if (e.CurrentPoint().Properties().PointerUpdateKind() == PointerUpdateKind::RightButtonReleased)
    {
        // When right-clicks are unhandled on the background thread, the platform can use them for AppBar invocation.
        e.Handled(false);
    }
    else if (m_drawingState == DrawingState::Inking)
    {
        m_drawingState = DrawingState::None;
        // Reset active pointer ID.
        m_activePointerId = 0;
    }
}
} // namespace winrt::DirectXPanels::implementation
