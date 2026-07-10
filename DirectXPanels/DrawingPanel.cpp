#include "pch.h"
#include <d2d1helper.h>

#define WINRT_IMPORT_MODULE
import winrt.DirectXPanels;
import winrt.Windows.Foundation;
import winrt.Microsoft.UI.Composition;
import winrt.Microsoft.UI.Dispatching;
import winrt.Microsoft.UI.Xaml;
import winrt.Microsoft.UI.Xaml.Controls;
import winrt.Windows.Storage.Streams;
import winrt.Windows.UI.Input.Inking;
import winrt.Windows.System.Threading;
import DX;

// clang-format off
#include "DrawingPanel.h"
#include "DrawingPanel.g.cpp"
// clang-format on

using namespace winrt;
using namespace Windows::Foundation;
using namespace Microsoft::UI::Xaml;
using namespace Windows::UI;
using namespace Windows::UI::Input::Inking;
using namespace Windows::UI::Core;
using namespace Windows::System::Threading;
using namespace Microsoft::UI::Input;
using namespace Microsoft::UI::Dispatching;
using namespace D2D1;
using namespace DX;

namespace winrt::DirectXPanels::implementation
{
// Initialize dependency properties to null.  The app must call RegisterDependencyProperties() before using them.
DependencyProperty DrawingPanel::m_brushColorProperty = nullptr;
DependencyProperty DrawingPanel::m_brushFitsToCurveProperty = nullptr;
DependencyProperty DrawingPanel::m_brushSizeProperty = nullptr;
DependencyProperty DrawingPanel::m_brushIsEraserProperty = nullptr;

DrawingPanel::DrawingPanel()
    : m_drawingState(DrawingState::Uninitialized),
      m_coreInput(CreateCoreIndependentInputSource(InputPointerSourceDeviceKinds::Mouse |
                                                   InputPointerSourceDeviceKinds::Pen |
                                                   InputPointerSourceDeviceKinds::Touch)),
      m_currentStrokeIndex(0), m_currentStrokeSegmentIndex(0), m_activePointerId(0)
{
    std::scoped_lock lock{m_mutex};
    CreateDeviceIndependentResources();
    CreateDeviceResources();
    CreateSizeDependentResources();
}

// Static method to register custom DependencyProperties of the DrawingPanel class.  This should be called from the App
// object constructor of any app that uses this class to ensure the DependencyProperties are correctly registered before
// an instance of DrawingPanel is created.
void DrawingPanel::RegisterDependencyProperties()
{
    // Ensure properties are only registered once.
    if (m_brushColorProperty == nullptr)
    {
        m_brushColorProperty = DependencyProperty::Register(
            L"BrushColor", xaml_typename<Windows::UI::Color>(), xaml_typename<DirectXPanels::DrawingPanel>(),
            PropertyMetadata(nullptr, PropertyChangedCallback(&DrawingPanel::BrushColorValueChanged)));
    }

    if (m_brushFitsToCurveProperty == nullptr)
    {
        m_brushFitsToCurveProperty = DependencyProperty::Register(
            L"BrushFitsToCurve", xaml_typename<bool>(), xaml_typename<DirectXPanels::DrawingPanel>(),
            PropertyMetadata(nullptr, PropertyChangedCallback(&DrawingPanel::BrushFitsToCurveValueChanged)));
    }

    if (m_brushSizeProperty == nullptr)
    {
        m_brushSizeProperty = DependencyProperty::Register(
            L"BrushSize", xaml_typename<Windows::Foundation::Size>(), xaml_typename<DirectXPanels::DrawingPanel>(),
            PropertyMetadata(nullptr, PropertyChangedCallback(&DrawingPanel::BrushSizeValueChanged)));
    }

    if (m_brushIsEraserProperty == nullptr)
    {
        m_brushIsEraserProperty = DependencyProperty::Register(
            L"BrushIsEraser", xaml_typename<bool>(), xaml_typename<DirectXPanels::DrawingPanel>(),
            PropertyMetadata(nullptr, PropertyChangedCallback(&DrawingPanel::BrushIsEraserValueChanged)));
    }
}

winrt::Microsoft::UI::Xaml::DependencyProperty DrawingPanel::BrushColorProperty()
{
    return m_brushColorProperty;
}
winrt::Microsoft::UI::Xaml::DependencyProperty DrawingPanel::BrushFitsToCurveProperty()
{
    return m_brushFitsToCurveProperty;
}
winrt::Microsoft::UI::Xaml::DependencyProperty DrawingPanel::BrushSizeProperty()
{
    return m_brushSizeProperty;
}
winrt::Microsoft::UI::Xaml::DependencyProperty DrawingPanel::BrushIsEraserProperty()
{
    return m_brushIsEraserProperty;
}
winrt::Windows::UI::Color DrawingPanel::BrushColor()
{
    return m_brushColor;
}
void DrawingPanel::BrushColor(winrt::Windows::UI::Color const &value)
{
    m_brushColor = value;
    SetValue(BrushColorProperty(), box_value(value));
}
bool DrawingPanel::BrushFitsToCurve()
{
    return unbox_value<bool>(BrushFitsToCurveProperty());
}
void DrawingPanel::BrushFitsToCurve(bool value)
{
    SetValue(BrushFitsToCurveProperty(), box_value(value));
}
winrt::Windows::Foundation::Size DrawingPanel::BrushSize()
{
    return m_brushSize;
}
void DrawingPanel::BrushSize(winrt::Windows::Foundation::Size const &value)
{
    m_brushSize = value;
    SetValue(BrushSizeProperty(), box_value(value));
}
bool DrawingPanel::BrushIsEraser()
{
    return m_brushIsEraser;
}
void DrawingPanel::BrushIsEraser(bool value)
{
    m_brushIsEraser = value;
    SetValue(BrushIsEraserProperty(), box_value(value));
}
bool DrawingPanel::HasContent()
{
    // return m_inkManager.GetStrokes().Size() > 0;
    return false;
}
winrt::event_token DrawingPanel::RecognitionResultsUpdated(
    winrt::DirectXPanels::RecognitionResultUpdatedEventHandler const &handler)
{
    return m_recognitionResultUpdateEvent.add(handler);
}
void DrawingPanel::RecognitionResultsUpdated(winrt::event_token const &token) noexcept
{
    m_recognitionResultUpdateEvent.remove(token);
}
void DrawingPanel::StartProcessingInput()
{
    // Initialize the rendering surface and prepare it to receive input.

    // Create a task to register for independent input and begin processing input messages.
    auto workItemHandler = WorkItemHandler([this](IAsyncAction) {
        // Create ink manager and set drawing attributes.
        // m_inkManager = Windows::UI::Input::Inking::InkManager();
        // m_inkDrawingAttributes = Windows::UI::Input::Inking::InkDrawingAttributes();

        // m_inkDrawingAttributes.Color(m_brushColor);
        // m_inkDrawingAttributes.FitToCurve(m_brushFitsToCurve);
        // m_inkDrawingAttributes.Size(m_brushSize);
        // m_inkManager.SetDefaultDrawingAttributes(m_inkDrawingAttributes);

        m_drawingState = DrawingState::None;

        // The CoreIndependentInputSource will raise pointer events for the specified device types on whichever thread
        // it's created on.
        m_coreInput =
            CreateCoreIndependentInputSource(InputPointerSourceDeviceKinds::Mouse | InputPointerSourceDeviceKinds::Pen |
                                             InputPointerSourceDeviceKinds::Touch);

        // Register for pointer events, which will be raised on the background thread.
        m_coreInput.PointerPressed({this, &DrawingPanel::OnPointerPressed});
        m_coreInput.PointerMoved({this, &DrawingPanel::OnPointerMoved});
        m_coreInput.PointerReleased({this, &DrawingPanel::OnPointerReleased});

        // Begin processing input messages as they're delivered.
        m_coreInput.DispatcherQueue().RunEventLoop();
    });

    // Run task on a dedicated high priority background thread.
    m_inputLoopWorker = ThreadPool::RunAsync(workItemHandler, WorkItemPriority::High, WorkItemOptions::TimeSliced);
}
void DrawingPanel::StopProcessingInput()
{
    // A call to ProcessEvents() with the ProcessUntilQuit flag will only return by default when the window closes.
    // Calling StopProcessEvents allows ProcessEvents to return even if the window isn't closing so the background
    // thread can exit.
    m_coreInput.DispatcherQueue().EnqueueEventLoopExit();
}
#pragma region DependencyProperty change handlers
void DrawingPanel::BrushColorValueChanged(const Microsoft::UI::Xaml::DependencyObject &sender,
                                          const Microsoft::UI::Xaml::DependencyPropertyChangedEventArgs &e)
{
    com_ptr panel = sender.as<DrawingPanel>();
    auto color = unbox_value<Color>(e.NewValue());

    std::scoped_lock lock(panel->m_mutex);
    panel->m_brushColor = color;
    panel->m_d2dContext->CreateSolidColorBrush(DX::ConvertToColorF(color), panel->m_strokeBrush.put());

    if (panel->m_drawingState != DrawingState::Uninitialized)
    {
        panel->m_coreInput.DispatcherQueue().TryEnqueue([&panel, color]() {
            // panel->m_inkDrawingAttributes.Color(color);
            // panel->m_inkManager.SetDefaultDrawingAttributes(panel->m_inkDrawingAttributes);
        });
    }
}
void DrawingPanel::BrushFitsToCurveValueChanged(const Microsoft::UI::Xaml::DependencyObject &sender,
                                                const Microsoft::UI::Xaml::DependencyPropertyChangedEventArgs &e)
{
    com_ptr panel = sender.as<DrawingPanel>();
    auto value = unbox_value<bool>(e.NewValue());

    panel->m_brushFitsToCurve = value;

    if (panel->m_drawingState != DrawingState::Uninitialized)
    {
        panel->m_coreInput.DispatcherQueue().TryEnqueue([=]() {
            // panel->m_inkDrawingAttributes.FitToCurve(value);
            // panel->m_inkManager.SetDefaultDrawingAttributes(panel->m_inkDrawingAttributes);
        });
    }
}
void DrawingPanel::BrushSizeValueChanged(const Microsoft::UI::Xaml::DependencyObject &sender,
                                         const Microsoft::UI::Xaml::DependencyPropertyChangedEventArgs &e)
{
    com_ptr panel = sender.as<DrawingPanel>();
    auto value = unbox_value<Windows::Foundation::Size>(e.NewValue());

    panel->m_brushSize = value;

    if (panel->m_drawingState != DrawingState::Uninitialized)
    {
        panel->m_coreInput.DispatcherQueue().TryEnqueue([=]() {
            // panel->m_inkDrawingAttributes.Size(value);
            // panel->m_inkManager.SetDefaultDrawingAttributes(panel->m_inkDrawingAttributes);
        });
    }
}
void DrawingPanel::BrushIsEraserValueChanged(const Microsoft::UI::Xaml::DependencyObject &sender,
                                             const Microsoft::UI::Xaml::DependencyPropertyChangedEventArgs &e)
{
    com_ptr panel = sender.as<DrawingPanel>();
    auto value = unbox_value<bool>(e.NewValue());

    panel->m_brushIsEraser = value;
}
#pragma endregion

void DrawingPanel::CreateDeviceResources()
{
    DirectXPanelBase::CreateDeviceResources();

    // Create ink stroke style.
    ThrowIfFailed(m_d2dFactory->CreateStrokeStyle(StrokeStyleProperties(D2D1_CAP_STYLE_ROUND, D2D1_CAP_STYLE_ROUND,
                                                                        D2D1_CAP_STYLE_ROUND, D2D1_LINE_JOIN_ROUND,
                                                                        1.0f, D2D1_DASH_STYLE_SOLID, 0.0f),
                                                  nullptr, 0, m_inkStrokeStyle.put()));

    // Create ink stroke brush.
    ThrowIfFailed(m_d2dContext->CreateSolidColorBrush(ConvertToColorF(BrushColor()), m_strokeBrush.put()));

    m_loadingComplete = true;
}

void DrawingPanel::CreateSizeDependentResources()
{
    m_currentBuffer = nullptr;
    m_previousBuffer = nullptr;

    DirectXPanelBase::CreateSizeDependentResources();

    ThrowIfFailed(m_swapChain->GetBuffer(0, guid_of<ID3D11Texture2D>(), m_currentBuffer.put_void()));

    ThrowIfFailed(m_swapChain->GetBuffer(1, guid_of<ID3D11Texture2D>(), m_previousBuffer.put_void()));
}

void DrawingPanel::OnDeviceLost()
{
    // Handle device lost, then re-render the current frame if resources are initialized and the user isn't drawing or
    // replaying.
    DirectXPanelBase::OnDeviceLost();

    if (m_drawingState == DrawingState::None)
    {
        Render();
    }
}

void DrawingPanel::OnSizeChanged(const winrt::Windows::Foundation::IInspectable &sender,
                                 const Microsoft::UI::Xaml::SizeChangedEventArgs &e)
{
    // Process SizeChanged event, then re-render at the new size if resources are initialized and the user isn't drawing
    // or replaying.
    DirectXPanelBase::OnSizeChanged(sender, e);

    if (m_drawingState == DrawingState::None)
    {
        Update();
    }
}

void DrawingPanel::OnCompositionScaleChanged(const winrt::Microsoft::UI::Xaml::Controls::SwapChainPanel &sender,
                                             const winrt::Windows::Foundation::IInspectable &args)
{
    // Process CompositionScaleChanged event, then re-render at the new scale if resources are initialized and the user
    // isn't drawing or replaying.
    DirectXPanelBase::OnCompositionScaleChanged(sender, args);

    if (m_drawingState == DrawingState::None)
    {
        Update();
    }
}

void DrawingPanel::Render()
{
    if (!m_loadingComplete)
    {
        return;
    }

    // Render and present the DirectX content.

    if (m_drawingState != DrawingState::Uninitialized)
    {
        m_coreInput.DispatcherQueue().TryEnqueue([=]() {
            m_d2dContext->BeginDraw();

            m_d2dContext->Clear(m_backgroundColor);

            RenderCompletedStrokes();

            HRESULT hr = m_d2dContext->EndDraw();
            // We ignore D2DERR_RECREATE_TARGET here. This error indicates that the
            // device is lost. It will be handled during the next call to Present.
            if (hr != D2DERR_RECREATE_TARGET)
            {
                ThrowIfFailed(hr);
            }

            Present();
        });
    }
}

void DrawingPanel::Update()
{
    std::scoped_lock lock(m_mutex);

    Render();
}

// Saves active ink strokes to the specified stream as an image.
bool DrawingPanel::SaveStrokesToStreamAsync(const Windows::Storage::Streams::IRandomAccessStream &stream)
{
    return m_coreInput.DispatcherQueue().TryEnqueue(
        [&stream]() { /*create_task(m_inkManager.SaveAsync(stream)).wait();*/ });
}

// Loads saved strokes from the specified stream.
bool DrawingPanel::LoadStrokesFromStreamAsync(const Windows::Storage::Streams::IRandomAccessStream &stream)
{
    return m_coreInput.DispatcherQueue().TryEnqueue(
        [&stream]() { /* create_task(m_inkManager.LoadAsync(stream)).wait(); */
        });
}

void DrawingPanel::BeginStrokesReplayFromStream(
    [[maybe_unused]] const Windows::Storage::Streams::IRandomAccessStream &stream,
    [[maybe_unused]] std::int32_t intervalInMilliseconds)
{
    m_drawingState = DrawingState::Replaying;
    m_currentStrokeIndex = 0;
    m_currentStrokeSegmentIndex = 0;

    if (m_replayTimer != nullptr)
    {
        m_replayTimer->Cancel();
    }

    // Delegate for the periodic timer that replays strokes.
    // auto timerDelegate = [=](ThreadPoolTimer &timer) {
    //    std::scoped_lock lock(m_mutex);

    //    auto strokes = m_inkManager.GetStrokes();
    //    auto stroke = strokes.GetAt(m_currentStrokeIndex);
    //    m_currentStrokeSegmentIndex++;

    //    if (m_currentStrokeSegmentIndex >= stroke.GetRenderingSegments().Size())
    //    {
    //        // Move to next stroke.
    //        m_currentStrokeIndex++;
    //        m_currentStrokeSegmentIndex = 0;
    //    }

    //    // Convert segments (up to the current segment index) of the current stroke to a single D2D geometry
    //    com_ptr<ID2D1PathGeometry> strokeGeometry;
    //    ConvertStrokeToGeometry(stroke, m_currentStrokeSegmentIndex, strokeGeometry.put());

    //    // Create brush from stroke
    //    com_ptr<ID2D1SolidColorBrush> strokeBrush;
    //    ThrowIfFailed(m_d2dContext->CreateSolidColorBrush(ConvertToColorF(stroke.DrawingAttributes().Color()),
    //                                                      strokeBrush.put()));

    //    m_d2dContext->BeginDraw();

    //    m_d2dContext->Clear(m_backgroundColor);

    //    // Render any strokes that have already been completely replayed.
    //    if (m_currentStrokeIndex > 0)
    //    {
    //        RenderCompletedStrokes(m_currentStrokeIndex);
    //    }

    //    // Render segments of the current stroke.
    //    m_d2dContext->DrawGeometry(strokeGeometry.get(), strokeBrush.get(), stroke.DrawingAttributes().Size().Width,
    //                               m_inkStrokeStyle.get());

    //    HRESULT hr = m_d2dContext->EndDraw();

    //    // We ignore D2DERR_RECREATE_TARGET here. This error indicates that the device
    //    // is lost. It will be handled during the next call to Present.
    //    if (hr != D2DERR_RECREATE_TARGET)
    //    {
    //        ThrowIfFailed(hr);
    //    }

    //    Present();

    //    if (m_currentStrokeIndex >= strokes.Size())
    //    {
    //        // Finished all strokes, so cancel the periodic timer.
    //        m_replayTimer->Cancel();
    //        m_drawingState = DrawingState::None;
    //    }
    //};

    // Load strokes from stream, then start a periodic timer with given interval.
    // concurrency::create_task(m_inkManager.LoadAsync(stream));
    //{
    //    m_inkManager.LoadAsync(stream);
    //    TimeSpan period{intervalInMilliseconds * 10000}; // Duration is in hundreds of ns.
    //    m_replayTimer = ThreadPoolTimer::CreatePeriodicTimer(TimerElapsedHandler(timerDelegate), period);
    //    co_return;
    //}
}

void DrawingPanel::StopStrokesReplay()
{
    if (m_drawingState == DrawingState::Replaying)
    {
        std::scoped_lock lock(m_mutex);

        m_currentStrokeIndex = 0;
        m_currentStrokeSegmentIndex = 0;

        if (m_replayTimer != nullptr)
        {
            m_replayTimer->Cancel();
        }

        m_drawingState = DrawingState::None;

        Render();
    }
}

void DrawingPanel::OnPointerPressed([[maybe_unused]] const winrt::Microsoft::UI::Input::InputPointerSource &sender,
                                    [[maybe_unused]] const winrt::Microsoft::UI::Input::PointerEventArgs &e)
{
    // Handle the PointerPressed event, which will be raised on a background thread.

    if (e.CurrentPoint().Properties().PointerUpdateKind() == PointerUpdateKind::LeftButtonPressed &&
        m_drawingState == DrawingState::None)
    {
        std::scoped_lock lock(m_mutex);

        // Store active pointer ID: only one contact can be inking at a time.
        m_activePointerId = e.CurrentPoint().PointerId();

        if (BrushIsEraser() || e.CurrentPoint().Properties().IsEraser())
        {
            m_drawingState = DrawingState::Erasing;
            // m_inkManager.Mode(InkManipulationMode::Erasing);
        }
        else
        {
            m_drawingState = DrawingState::Inking;
            // m_inkManager.Mode(InkManipulationMode::Inking);
        }

        auto pointerPoint = e.CurrentPoint();
        m_previousPoint = pointerPoint.Position();
        // Pass pointer information to ink manager
        // m_inkManager.ProcessPointerDown(pointerPoint);
    }
}

void DrawingPanel::OnPointerMoved([[maybe_unused]] const winrt::Microsoft::UI::Input::InputPointerSource &sender,
                                  [[maybe_unused]] const winrt::Microsoft::UI::Input::PointerEventArgs &e)
{
    // Handle the PointerMoved event, which will be raised on a background thread.

    if (m_drawingState == DrawingState::Inking && e.CurrentPoint().PointerId() == m_activePointerId)
    {
        std::scoped_lock lock(m_mutex);

        RenderActiveStroke(e.CurrentPoint());

        // Pass pointer information to ink manager.
        // m_inkManager->ProcessPointerUpdate(e->CurrentPoint);
    }
    else if (m_drawingState == DrawingState::Erasing)
    {
        std::scoped_lock lock(m_mutex);

        // Pass pointer information to ink manager.
        // auto invalidatedRect =
        //    unbox_value<Windows::Foundation::Rect>(m_inkManager->ProcessPointerUpdate(e.CurrentPoint()));
        Windows::Foundation::Rect invalidatedRect{};

        // Re-render the current scene if any strokes were erased.
        if (invalidatedRect.Width > 0 && invalidatedRect.Height > 0)
        {
            Render();
        }
    }
}

void DrawingPanel::OnPointerReleased([[maybe_unused]] const winrt::Microsoft::UI::Input::InputPointerSource &sender,
                                     [[maybe_unused]] const winrt::Microsoft::UI::Input::PointerEventArgs &e)
{
    // Handle the PointerReleased event, which will be raised on a background thread.

    if (e.CurrentPoint().Properties().PointerUpdateKind() == PointerUpdateKind::RightButtonReleased)
    {
        // When right-clicks are unhandled on the background thread, the platform can use them for AppBar invocation.
        e.Handled(false);
    }
    else if (m_drawingState == DrawingState::Inking || m_drawingState == DrawingState::Erasing)
    {
        std::scoped_lock lock(m_mutex);

        m_drawingState = DrawingState::None;
        auto pointerPoint = e.CurrentPoint();
        // Pass pointer information to ink manager.
        // m_inkManager->ProcessPointerUp(pointerPoint);

        // Reset active pointer ID.
        m_activePointerId = 0;

        Render();

        m_coreInput.DispatcherQueue().TryEnqueue([=]() {
            if (/* m_inkManager->GetStrokes()->Size > */ 0)
            {
                // The following call to RecognizeAsync may fail for various reasons, most notably if another
                // recognition is in progress.
                try
                {
                    // concurrency::task<IVectorView<InkRecognitionResult>> recognizeTask(
                    //     m_inkManager->RecognizeAsync(InkRecognitionTarget::All));
                    // recognizeTask.then([this](IVectorView<InkRecognitionResult> &recognitionResults) {
                    //     // Get top recognition candidates.
                    //     auto results = com_array<hstring>(recognitionResults.Size());
                    //     for (unsigned int i = 0; i < recognitionResults->Size; i++)
                    //     {
                    //         results[i] = recognitionResults->GetAt(i)->GetTextCandidates()->GetAt(0);
                    //     }

                    //    this->DispatcherQueue().TryEnqueue([=]() {
                    //        RecognitionResultsUpdated(this, RecognitionResultUpdatedEventArgs(results));
                    //    });
                    //});
                }
                catch ([[maybe_unused]] winrt::hresult_error const &ex)
                {
                    // Catch recognition errors so the app doesn't crash. Recognition results will be updated when
                    // the user draws the next stroke.
                }
            }
            else
            {
                this->DispatcherQueue().TryEnqueue(
                    [=]() { m_recognitionResultUpdateEvent(*this, RecognitionResultUpdatedEventArgs{}); });
            }
        });
    }
}

void DrawingPanel::RenderActiveStroke(const Microsoft::UI::Input::PointerPoint &newPoint)
{
    // Must be called on the background thread.

    // While actively inking, copy the last presented buffer to the current buffer since it contains the live ink
    // strokes drawn in response to previous pointer move events, then draw the current stroke segment.
    m_d3dContext->CopyResource(m_currentBuffer.get(), m_previousBuffer.get());

    m_d2dContext->BeginDraw();

    Point newPosition = newPoint.Position();

    m_d2dContext->DrawLine(Point2F(m_previousPoint.X, m_previousPoint.Y), Point2F(newPosition.X, newPosition.Y),
                           m_strokeBrush.get(), BrushSize().Height, m_inkStrokeStyle.get());

    m_previousPoint = newPosition;

    HRESULT hr = m_d2dContext->EndDraw();

    // We ignore D2DERR_RECREATE_TARGET here. This error indicates that the device
    // is lost. It will be handled during the next call to Present.
    if (hr != D2DERR_RECREATE_TARGET)
    {
        ThrowIfFailed(hr);
    }

    Present();
}

void DrawingPanel::RenderCompletedStrokes([[maybe_unused]] unsigned int strokeCount)
{
    // Must be called on the background thread.

    // Get previous ink strokes from the ink manager up to the given count and render them.
    // auto strokes = m_inkManager->GetStrokes();

    // for (unsigned int i = 0; i < strokeCount; i++)
    //{
    //     auto stroke = strokes->GetAt(i);
    //     com_ptr<ID2D1PathGeometry> strokeGeometry;
    //     ConvertStrokeToGeometry(stroke, strokeGeometry.put());

    //    // Create brush from current stroke
    //    com_ptr<ID2D1SolidColorBrush> strokeBrush;
    //    ThrowIfFailed(
    //        m_d2dContext->CreateSolidColorBrush(ConvertToColorF(stroke->DrawingAttributes->Color),
    //        strokeBrush.put()));

    //    m_d2dContext->DrawGeometry(strokeGeometry.get(), strokeBrush.get(), stroke->DrawingAttributes->Size.Width,
    //                               m_inkStrokeStyle.get());
    //}
}

void DrawingPanel::ConvertStrokeToGeometry(Windows::UI::Input::Inking::InkStroke &stroke, unsigned int segmentCount,
                                           ID2D1PathGeometry **geometry)
{
    // Create geometry path.
    ThrowIfFailed(m_d2dFactory->CreatePathGeometry(geometry));

    // Create and initialize geometry sink.
    com_ptr<ID2D1GeometrySink> sink;
    ThrowIfFailed((*geometry)->Open(sink.put()));

    sink->SetSegmentFlags(D2D1_PATH_SEGMENT_FORCE_ROUND_LINE_JOIN);
    sink->SetFillMode(D2D1_FILL_MODE_ALTERNATE);

    auto strokeSegments = stroke.GetRenderingSegments();

    // Set starting point to position of first stroke segment.
    sink->BeginFigure(ConvertToPoint2F(strokeSegments.GetAt(0).Position()), D2D1_FIGURE_BEGIN_FILLED);

    // Add bezier segments for the remaining stroke segments to geometry path sink.
    for (unsigned int i = 1; i < segmentCount; i++)
    {
        auto strokeSegment = strokeSegments.GetAt(i);
        sink->AddBezier(D2D1::BezierSegment(ConvertToPoint2F(strokeSegment.BezierControlPoint1()),
                                            ConvertToPoint2F(strokeSegment.BezierControlPoint2()),
                                            ConvertToPoint2F(strokeSegment.Position())));
    }

    sink->EndFigure(D2D1_FIGURE_END_OPEN);
    ThrowIfFailed(sink->Close());
}

} // namespace winrt::DirectXPanels::implementation
