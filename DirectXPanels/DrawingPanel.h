#pragma once

#include <Windows.Storage.Streams.h>
#include <Windows.System.Threading.h>
#include <Windows.UI.Input.Inking.h>

// clang-format off
#include "DrawingPanel.g.h"
#include "DirectXPanelBase.h"
// clang-format on

namespace winrt::DirectXPanels::implementation
{
struct DrawingPanel : DrawingPanelT<DrawingPanel, DirectXPanels::implementation::DirectXPanelBase>
{
    DrawingPanel();

    static void RegisterDependencyProperties();

    static winrt::Microsoft::UI::Xaml::DependencyProperty BrushColorProperty();
    static winrt::Microsoft::UI::Xaml::DependencyProperty BrushFitsToCurveProperty();
    static winrt::Microsoft::UI::Xaml::DependencyProperty BrushSizeProperty();
    static winrt::Microsoft::UI::Xaml::DependencyProperty BrushIsEraserProperty();
    winrt::Windows::UI::Color BrushColor();
    void BrushColor(winrt::Windows::UI::Color const &value);
    bool BrushFitsToCurve();
    void BrushFitsToCurve(bool value);
    winrt::Windows::Foundation::Size BrushSize();
    void BrushSize(winrt::Windows::Foundation::Size const &value);
    bool BrushIsEraser();
    void BrushIsEraser(bool value);
    bool HasContent();
    winrt::event_token RecognitionResultsUpdated(
        winrt::DirectXPanels::RecognitionResultUpdatedEventHandler const &handler);
    void RecognitionResultsUpdated(winrt::event_token const &token) noexcept;

  public:
    void StartProcessingInput();
    void StopProcessingInput();

    void Update();

    bool SaveStrokesToStreamAsync(winrt::Windows::Storage::Streams::IRandomAccessStream const &stream);
    bool LoadStrokesFromStreamAsync(winrt::Windows::Storage::Streams::IRandomAccessStream const &stream);
    void BeginStrokesReplayFromStream(winrt::Windows::Storage::Streams::IRandomAccessStream const &stream,
                                      std::int32_t intervalInMilliseconds);
    void StopStrokesReplay();

  private:
    enum class DrawingState
    {
        Uninitialized = 0,
        None,
        Inking,
        Erasing,
        Replaying
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

    void RenderActiveStroke(const Microsoft::UI::Input::PointerPoint &newPoint);
    void RenderCompletedStrokes(unsigned int strokeCount);
    inline void RenderCompletedStrokes()
    {
        // RenderCompletedStrokes(m_inkManager.GetStrokes().Size());
    }

    void ConvertStrokeToGeometry(Windows::UI::Input::Inking::InkStroke &stroke, unsigned int segmentCount,
                                 ID2D1PathGeometry **geometry);
    inline void ConvertStrokeToGeometry(Windows::UI::Input::Inking::InkStroke stroke, ID2D1PathGeometry **geometry)
    {
        ConvertStrokeToGeometry(stroke, stroke.GetRenderingSegments().Size(), geometry);
    }

    // DependencyProperties
    static void BrushColorValueChanged(const Microsoft::UI::Xaml::DependencyObject &sender,
                                       const Microsoft::UI::Xaml::DependencyPropertyChangedEventArgs &e);
    static void BrushFitsToCurveValueChanged(const Microsoft::UI::Xaml::DependencyObject &sender,
                                             const Microsoft::UI::Xaml::DependencyPropertyChangedEventArgs &e);
    static void BrushSizeValueChanged(const Microsoft::UI::Xaml::DependencyObject &sender,
                                      const Microsoft::UI::Xaml::DependencyPropertyChangedEventArgs &e);
    static void BrushIsEraserValueChanged(const Microsoft::UI::Xaml::DependencyObject &sender,
                                          const Microsoft::UI::Xaml::DependencyPropertyChangedEventArgs &e);

    static Microsoft::UI::Xaml::DependencyProperty m_brushColorProperty;
    static Microsoft::UI::Xaml::DependencyProperty m_brushFitsToCurveProperty;
    static Microsoft::UI::Xaml::DependencyProperty m_brushSizeProperty;
    Windows::Foundation::Size m_brushSize;
    static Microsoft::UI::Xaml::DependencyProperty m_brushIsEraserProperty;
    bool m_brushIsEraser;
    bool m_brushFitsToCurve;
    Windows::UI::Color m_brushColor;

    DrawingState m_drawingState;

    Microsoft::UI::Input::InputPointerSource m_coreInput;
    Windows::Foundation::IAsyncAction m_inputLoopWorker;

    winrt::com_ptr<ID3D11Texture2D> m_currentBuffer;
    winrt::com_ptr<ID3D11Texture2D> m_previousBuffer;

    winrt::com_ptr<ID2D1StrokeStyle> m_inkStrokeStyle;
    winrt::com_ptr<ID2D1SolidColorBrush> m_strokeBrush;

    // Windows::UI::Input::Inking::InkManager m_inkManager;
    // Windows::UI::Input::Inking::InkDrawingAttributes m_inkDrawingAttributes;
    Windows::Foundation::Point m_previousPoint;
    unsigned int m_activePointerId;

    std::optional<Windows::System::Threading::ThreadPoolTimer> m_replayTimer;
    unsigned int m_currentStrokeIndex;
    unsigned int m_currentStrokeSegmentIndex;

    winrt::event<winrt::DirectXPanels::RecognitionResultUpdatedEventHandler> m_recognitionResultUpdateEvent;
};
} // namespace winrt::DirectXPanels::implementation
namespace winrt::DirectXPanels::factory_implementation
{
struct DrawingPanel : DrawingPanelT<DrawingPanel, implementation::DrawingPanel>
{
};
} // namespace winrt::DirectXPanels::factory_implementation
