#pragma once

// clang-format off
#include "D3DPanel.g.h"
#include "DirectXPanelBase.h"
// clang-format on

namespace winrt::DirectXPanels::implementation
{
    struct D3DPanel : D3DPanelT<D3DPanel, DirectXPanels::implementation::DirectXPanelBase>
    {
        D3DPanel();
        ~D3DPanel();
        void StartRenderLoop();
        void StopRenderLoop();

      private:
        virtual void Render() override;
        virtual void CreateDeviceResources() override;
        virtual void CreateSizeDependentResources() override;

        winrt::com_ptr<IDXGIOutput> m_dxgiOutput;

        winrt::com_ptr<ID3D11RenderTargetView> m_renderTargetView;
        winrt::com_ptr<ID3D11DepthStencilView> m_depthStencilView;
        winrt::com_ptr<ID3D11VertexShader> m_vertexShader;
        winrt::com_ptr<ID3D11PixelShader> m_pixelShader;
        winrt::com_ptr<ID3D11InputLayout> m_inputLayout;
        winrt::com_ptr<ID3D11Buffer> m_vertexBuffer;
        winrt::com_ptr<ID3D11Buffer> m_indexBuffer;
        winrt::com_ptr<ID3D11Buffer> m_constantBuffer;

        DX::ModelViewProjectionConstantBuffer m_constantBufferData;

        std::uint32_t m_indexCount;

        float m_degreesPerSecond;

        Windows::Foundation::IAsyncAction m_renderLoopWorker;
        // Rendering loop timer.
        DX::StepTimer m_timer;
    };
}
namespace winrt::DirectXPanels::factory_implementation
{
    struct D3DPanel : D3DPanelT<D3DPanel, implementation::D3DPanel>
    {
    };
}
