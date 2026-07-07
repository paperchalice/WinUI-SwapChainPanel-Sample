#include "pch.h"

#include <microsoft.ui.xaml.media.dxinterop.h>

#define WINRT_IMPORT_MODULE
import winrt.DirectXPanels;
import winrt.Windows.UI.Core;
import winrt.Microsoft.UI.Composition;
import winrt.Microsoft.UI.Xaml;
import winrt.Microsoft.UI.Xaml.Controls;

import DirectXHelper;

#include "DirectXPanelBase.g.cpp"
#include "DirectXPanelBase.h"

using namespace DX;
using namespace Concurrency;
using namespace DirectX;
using namespace D2D1;
using namespace winrt;
using namespace winrt::Windows::Foundation;
using namespace winrt::Windows::UI::Core;
using namespace winrt::Microsoft::UI::Xaml;
using namespace winrt::Microsoft::UI::Xaml::Controls;

static constexpr float m_dipsPerInch = 96.0f;

namespace winrt::DirectXPanels::implementation
{
DirectXPanelBase::DirectXPanelBase()
    : m_loadingComplete(false), m_backgroundColor(D2D1::ColorF(D2D1::ColorF::White)), // Default to white background.
      m_alphaMode(DXGI_ALPHA_MODE_UNSPECIFIED), // Default to ignore alpha, which can provide better performance if
                                                // transparency is not required.
      m_compositionScaleX(1.0f), m_compositionScaleY(1.0f), m_height(1.0f), m_width(1.0f)
{
    RegisterEventHandlers();
}

// Ensure virtual function are registred as event handler.
void DirectXPanelBase::RegisterEventHandlers()
{
    SizeChanged({this, &DirectXPanelBase::OnSizeChanged});
    CompositionScaleChanged({this, &DirectXPanelBase::OnCompositionScaleChanged});
    // Application::Current()
}

void DirectXPanelBase::CreateDeviceIndependentResources()
{
    D2D1_FACTORY_OPTIONS options = {};

#if defined(_DEBUG)
    // Enable D2D debugging via SDK Layers when in debug mode.
    options.debugLevel = D2D1_DEBUG_LEVEL_INFORMATION;
#endif

    ThrowIfFailed(D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, __uuidof(ID2D1Factory2), &options,
                                    m_d2dFactory.put_void()));
}

void DirectXPanelBase::CreateDeviceResources()
{
    // This flag adds support for surfaces with a different color channel ordering than the API default.
    // It is recommended usage, and is required for compatibility with Direct2D.
    UINT creationFlags = D3D11_CREATE_DEVICE_BGRA_SUPPORT;

#if defined(_DEBUG)
    // If the project is in a debug build, enable debugging via SDK Layers with this flag.
    creationFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

    // This array defines the set of DirectX hardware feature levels this app will support.
    // Note the ordering should be preserved.
    // Don't forget to declare your application's minimum required feature level in its
    // description.  All applications are assumed to support 9.1 unless otherwise stated.
    D3D_FEATURE_LEVEL featureLevels[] = {D3D_FEATURE_LEVEL_11_1, D3D_FEATURE_LEVEL_11_0, D3D_FEATURE_LEVEL_10_1,
                                         D3D_FEATURE_LEVEL_10_0, D3D_FEATURE_LEVEL_9_3,  D3D_FEATURE_LEVEL_9_2,
                                         D3D_FEATURE_LEVEL_9_1};

    // Create the DX11 API device object, and get a corresponding context.
    com_ptr<ID3D11Device> device;
    com_ptr<ID3D11DeviceContext> context;
    ThrowIfFailed(D3D11CreateDevice(nullptr, // Specify null to use the default adapter.
                                    D3D_DRIVER_TYPE_HARDWARE, 0,
                                    creationFlags, // Optionally set debug and Direct2D compatibility flags.
                                    featureLevels, // List of feature levels this app can support.
                                    ARRAYSIZE(featureLevels),
                                    D3D11_SDK_VERSION, // Always set this to D3D11_SDK_VERSION for Windows Store apps.
                                    device.put(),      // Returns the Direct3D device created.
                                    nullptr,           // Returns feature level of device created.
                                    context.put()      // Returns the device immediate context.
                                    ));

    // Get D3D11.1 device
    device.as(m_d3dDevice);

    // Get D3D11.1 context
    context.as(m_d3dContext);

    // Get underlying DXGI device of D3D device
    auto dxgiDevice = m_d3dDevice.as<IDXGIDevice>();

    // Get D2D device
    ThrowIfFailed(m_d2dFactory->CreateDevice(dxgiDevice.get(), m_d2dDevice.put()));

    // Get D2D context
    ThrowIfFailed(m_d2dDevice->CreateDeviceContext(D2D1_DEVICE_CONTEXT_OPTIONS_NONE, m_d2dContext.put()));

    // Set D2D text anti-alias mode to Grayscale to ensure proper rendering of text on intermediate surfaces.
    m_d2dContext->SetTextAntialiasMode(D2D1_TEXT_ANTIALIAS_MODE_GRAYSCALE);
}

void DirectXPanelBase::CreateSizeDependentResources()
{
    // Ensure dependent objects have been released.
    m_d2dContext->SetTarget(nullptr);
    m_d2dTargetBitmap = nullptr;
    m_d3dContext->OMSetRenderTargets(0, nullptr, nullptr);
    m_d3dContext->Flush();

    // Set render target size to the rendered size of the panel including the composition scale,
    // defaulting to the minimum of 1px if no size was specified.
    m_renderTargetWidth = m_width * m_compositionScaleX;
    m_renderTargetHeight = m_height * m_compositionScaleY;

    // If the swap chain already exists, then resize it.
    if (m_swapChain != nullptr)
    {
        HRESULT hr = m_swapChain->ResizeBuffers(2, static_cast<UINT>(m_renderTargetWidth),
                                                static_cast<UINT>(m_renderTargetHeight), DXGI_FORMAT_B8G8R8A8_UNORM, 0);

        if (hr == DXGI_ERROR_DEVICE_REMOVED || hr == DXGI_ERROR_DEVICE_RESET)
        {
            // If the device was removed for any reason, a new device and swap chain will need to be created.
            OnDeviceLost();
            return;
        }
        else
        {
            ThrowIfFailed(hr);
        }
    }
    else // Otherwise, create a new one.
    {
        DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {0};
        swapChainDesc.Width = static_cast<UINT>(m_renderTargetWidth); // Match the size of the panel.
        swapChainDesc.Height = static_cast<UINT>(m_renderTargetHeight);
        swapChainDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM; // This is the most common swap chain format.
        swapChainDesc.Stereo = false;
        swapChainDesc.SampleDesc.Count = 1; // Don't use multi-sampling.
        swapChainDesc.SampleDesc.Quality = 0;
        swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
        swapChainDesc.BufferCount = 2;                               // Use double buffering to enable flip.
        swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL; // All Windows Store apps must use this SwapEffect.
        swapChainDesc.Flags = 0;
        swapChainDesc.AlphaMode = m_alphaMode;

        // Get underlying DXGI Device from D3D Device.
        auto dxgiDevice = m_d3dDevice.as<IDXGIDevice1>();

        // Get adapter.
        com_ptr<IDXGIAdapter> dxgiAdapter;
        ThrowIfFailed(dxgiDevice->GetAdapter(dxgiAdapter.put()));

        // Get factory.
        com_ptr<IDXGIFactory2> dxgiFactory;
        ThrowIfFailed(dxgiAdapter->GetParent(IID_PPV_ARGS(dxgiFactory.put())));

        com_ptr<IDXGISwapChain1> swapChain;
        // Create swap chain.
        ThrowIfFailed(
            dxgiFactory->CreateSwapChainForComposition(m_d3dDevice.get(), &swapChainDesc, nullptr, swapChain.put()));
        swapChain.as(m_swapChain);

        // Ensure that DXGI does not queue more than one frame at a time. This both reduces
        // latency and ensures that the application will only render after each VSync, minimizing
        // power consumption.
        ThrowIfFailed(dxgiDevice->SetMaximumFrameLatency(1));

        DispatcherQueue().TryEnqueue([=]() {
            // Get backing native interface for SwapChainPanel.
            Windows::Foundation::IUnknown panel = *this;
            auto panelNative = panel.as<ISwapChainPanelNative>();

            // Associate swap chain with SwapChainPanel.  This must be done on the UI thread.
            ThrowIfFailed(panelNative->SetSwapChain(m_swapChain.get()));
        });
    }

    // Ensure the physical pixel size of the swap chain takes into account both the XAML SwapChainPanel's logical layout
    // size and any cumulative composition scale applied due to zooming, render transforms, or the system's current
    // scaling plateau. For example, if a 100x100 SwapChainPanel has a cumulative 2x scale transform applied, we instead
    // create a 200x200 swap chain to avoid artifacts from scaling it up by 2x, then apply an inverse 1/2x transform to
    // the swap chain to cancel out the 2x transform.
    DXGI_MATRIX_3X2_F inverseScale = {};
    inverseScale._11 = 1.0f / m_compositionScaleX;
    inverseScale._22 = 1.0f / m_compositionScaleY;

    m_swapChain->SetMatrixTransform(&inverseScale);

    D2D1_BITMAP_PROPERTIES1 bitmapProperties =
        BitmapProperties1(D2D1_BITMAP_OPTIONS_TARGET | D2D1_BITMAP_OPTIONS_CANNOT_DRAW,
                          PixelFormat(DXGI_FORMAT_B8G8R8A8_UNORM, D2D1_ALPHA_MODE_PREMULTIPLIED),
                          m_dipsPerInch * m_compositionScaleX, m_dipsPerInch * m_compositionScaleY);

    // Direct2D needs the DXGI version of the backbuffer surface pointer.
    com_ptr<IDXGISurface> dxgiBackBuffer;
    ThrowIfFailed(m_swapChain->GetBuffer(0, IID_PPV_ARGS(dxgiBackBuffer.put())));

    // Get a D2D surface from the DXGI back buffer to use as the D2D render target.
    ThrowIfFailed(
        m_d2dContext->CreateBitmapFromDxgiSurface(dxgiBackBuffer.get(), &bitmapProperties, m_d2dTargetBitmap.put()));

    m_d2dContext->SetDpi(m_dipsPerInch * m_compositionScaleX, m_dipsPerInch * m_compositionScaleY);
    m_d2dContext->SetTarget(m_d2dTargetBitmap.get());
}

void DirectXPanelBase::OnDeviceLost()
{
    m_loadingComplete = false;

    m_swapChain = nullptr;

    // Make sure the rendering state has been released.
    m_d3dContext->OMSetRenderTargets(0, nullptr, nullptr);

    m_d2dContext->SetTarget(nullptr);
    m_d2dTargetBitmap = nullptr;

    m_d2dContext = nullptr;
    m_d2dDevice = nullptr;

    m_d3dContext->Flush();

    CreateDeviceResources();
    CreateSizeDependentResources();
}

void DirectXPanelBase::OnSizeChanged([[maybe_unused]] const IInspectable &sender, const SizeChangedEventArgs &e)
{
    if (m_width != e.NewSize().Width || m_height != e.NewSize().Height)
    {
        std::scoped_lock lock(m_mutex);

        // Store values so they can be accessed from a background thread.
        m_width = max(e.NewSize().Width, 1.0f);
        m_height = max(e.NewSize().Height, 1.0f);

        // Recreate size-dependent resources when the panel's size changes.
        CreateSizeDependentResources();
    }
}

void DirectXPanelBase::OnCompositionScaleChanged([[maybe_unused]] const SwapChainPanel &sender,
                                                 [[maybe_unused]] const IInspectable &args)
{
    if (m_compositionScaleX != CompositionScaleX() || m_compositionScaleY != CompositionScaleY())
    {
        std::scoped_lock lock(m_mutex);

        // Store values so they can be accessed from a background thread.
        m_compositionScaleX = CompositionScaleX();
        m_compositionScaleY = CompositionScaleY();

        // Recreate size-dependent resources when the composition scale changes.
        CreateSizeDependentResources();
    }
}

void DirectXPanelBase::Present()
{
    DXGI_PRESENT_PARAMETERS parameters = {};

    HRESULT hr = S_OK;

    hr = m_swapChain->Present1(1, 0, &parameters);

    if (hr == DXGI_ERROR_DEVICE_REMOVED || hr == DXGI_ERROR_DEVICE_RESET)
    {
        OnDeviceLost();
    }
    else
    {
        ThrowIfFailed(hr);
    }
}

} // namespace winrt::DirectXPanels::implementation
