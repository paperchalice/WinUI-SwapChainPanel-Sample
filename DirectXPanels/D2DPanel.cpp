#include "pch.h"

#include <d2d1.h>

#define WINRT_IMPORT_MODULE
import winrt.DirectXPanels;
import winrt.Microsoft.UI.Composition;
import winrt.Microsoft.UI.Xaml;
import winrt.Microsoft.UI.Xaml.Controls;

// clang-format off
#include "D2DPanel.h"
#include "D2DPanel.g.cpp"
// clang-format on

using namespace D2D1;

namespace winrt::DirectXPanels::implementation
{
D2DPanel::D2DPanel()
{
    m_backgroundColor = ColorF(ColorF::AliceBlue);

    CreateDeviceIndependentResources();
    CreateDeviceResources();
    CreateSizeDependentResources();
}

void D2DPanel::Render()
{
    if (!m_loadingComplete)
    {
        return;
    }

    m_d2dContext->BeginDraw();
    m_d2dContext->Clear(m_backgroundColor);

    // Set up simple tic-tac-toe game board.
    float horizontalSpacing = m_renderTargetWidth / 3.0f;
    float verticalSpacing = m_renderTargetHeight / 3.0f;

    // Since the unit mode is set to pixels in CreateDeviceResources(), here we scale the line thickness by the
    // composition scale so that elements are rendered in the same position but larger as you zoom in. Whether or not
    // the composition scale should be factored into the size or position of elements depends on the app's scenario.
    float lineThickness = m_compositionScaleX * 2.0f;
    float strokeThickness = m_compositionScaleX * 4.0f;

    // Draw grid lines.
    m_d2dContext->DrawLine(Point2F(horizontalSpacing, 0), Point2F(horizontalSpacing, m_renderTargetHeight),
                           m_strokeBrush.get(), lineThickness);
    m_d2dContext->DrawLine(Point2F(horizontalSpacing * 2, 0), Point2F(horizontalSpacing * 2, m_renderTargetHeight),
                           m_strokeBrush.get(), lineThickness);
    m_d2dContext->DrawLine(Point2F(0, verticalSpacing), Point2F(m_renderTargetWidth, verticalSpacing),
                           m_strokeBrush.get(), lineThickness);
    m_d2dContext->DrawLine(Point2F(0, verticalSpacing * 2), Point2F(m_renderTargetWidth, verticalSpacing * 2),
                           m_strokeBrush.get(), lineThickness);

    // Draw center circle.
    m_d2dContext->DrawEllipse(Ellipse(Point2F(m_renderTargetWidth / 2.0f, m_renderTargetHeight / 2.0f),
                                      horizontalSpacing / 2.0f - strokeThickness,
                                      verticalSpacing / 2.0f - strokeThickness),
                              m_strokeBrush.get(), strokeThickness);

    // Draw top left X.
    m_d2dContext->DrawLine(Point2F(0, 0), Point2F(horizontalSpacing - lineThickness, verticalSpacing - lineThickness),
                           m_strokeBrush.get(), strokeThickness);
    m_d2dContext->DrawLine(Point2F(horizontalSpacing - lineThickness, 0), Point2F(0, verticalSpacing - lineThickness),
                           m_strokeBrush.get(), strokeThickness);

    m_d2dContext->EndDraw();

    Present();
}

void D2DPanel::CreateDeviceResources()
{
    DirectXPanelBase::CreateDeviceResources();

    m_d2dContext->CreateSolidColorBrush(ColorF(ColorF::Black), m_strokeBrush.put());

    // Set D2D's unit mode to pixels so that drawing operation units are interpreted in pixels rather than DIPS.
    m_d2dContext->SetUnitMode(D2D1_UNIT_MODE::D2D1_UNIT_MODE_PIXELS);

    m_loadingComplete = true;
}

void D2DPanel::OnDeviceLost()
{
    // Handle device lost, then re-render.
    DirectXPanelBase::OnDeviceLost();
    Render();
}

void D2DPanel::OnSizeChanged(const winrt::Windows::Foundation::IInspectable &sender,
                             const winrt::Microsoft::UI::Xaml::SizeChangedEventArgs &e)
{
    // Process SizeChanged event, then re-render at the new size.
    DirectXPanelBase::OnSizeChanged(sender, e);
    Render();
}
void D2DPanel::OnCompositionScaleChanged(const winrt::Microsoft::UI::Xaml::Controls::SwapChainPanel &sender,
                                         const winrt::Windows::Foundation::IInspectable &args)
{
    // Process CompositionScaleChanged event, then re-render at the new scale.
    DirectXPanelBase::OnCompositionScaleChanged(sender, args);
    Render();
}
} // namespace winrt::DirectXPanels::implementation
