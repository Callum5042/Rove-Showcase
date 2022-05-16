#pragma once

#include "Pch.h"

namespace Rove
{
	namespace DX
	{
		// DirectX11 error checking
		inline void Check(HRESULT hr)
		{
#ifdef _DEBUG
			if (FAILED(hr))
			{
				throw std::exception();
			}
#endif
		}
	}

	// Forward declarations
	class Window;

	// Direct3D11 renderer
	class DxRenderer
	{
	public:
		DxRenderer(Window* window);
		virtual ~DxRenderer() = default;

		// Create Direct3D11 renderer
		void Create();

		// Clear the buffers
		void Clear(const float* colour);

		// Display the rendered scene
		void Present(bool enable_vsync);

		// Resizing
		void Resize(int width, int height);

		// Get Direct3D11 Device
		ID3D11Device* GetDevice() { return m_Device.Get(); }

		// Get Direct3D11 Device Context
		ID3D11DeviceContext* GetDeviceContext() { return m_DeviceContext.Get(); }

		// Set raster state as wireframe
		void SetWireframeRasterState();

		// Set raster state as solid
		void SetSolidRasterState();

		// GPU name
		constexpr std::string& GetGpuName() { return m_GpuName; }

		// GPU VRAM
		constexpr int GetGpuVramMB() { return m_GpuVramMb; }

		// Copy MSAA to back buffer
		void CopyMsaaRenderTargetBackBuffer();

		// Set render to MSAA
		void SetRenderToMsaa();

		// Set render to back buffer
		void SetRenderToBackBuffer();

	private:
		Window* m_Window = nullptr;

		// Device and device context
		ComPtr<ID3D11Device> m_Device = nullptr;
		ComPtr<ID3D11DeviceContext> m_DeviceContext = nullptr;
		void CreateDeviceAndContext();

		// Swapchain
		ComPtr<ID3D11Texture2D> m_BackBuffer = nullptr;
		ComPtr<IDXGISwapChain1> m_SwapChain = nullptr;
		void CreateSwapChain(int width, int height);

		// Render target and depth stencil view
		ComPtr<ID3D11RenderTargetView> m_RenderTargetView = nullptr;
		ComPtr<ID3D11DepthStencilView> m_DepthStencilView = nullptr;
		void CreateRenderTargetAndDepthStencilView(int width, int height);

		// Viewport
		void SetViewport(int width, int height);

		// Wireframe raster state
		ComPtr<ID3D11RasterizerState> m_RasterStateWireframe = nullptr;
		void CreateWireframeRasterState();

		// Solid raster state
		ComPtr<ID3D11RasterizerState> m_RasterStateSolid = nullptr;
		void CreateSolidRasterState();

		// Adapter info
		ComPtr<IDXGIAdapter> m_Adapter = nullptr;
		std::string m_GpuName;
		int m_GpuVramMb = 0;

		// Multisample anti-aliasing
		UINT m_4xMsaaQuality = 0;

		void CreateMsaaRenderTargetView(int width, int height);
		ComPtr<ID3D11Texture2D> m_MsaaTexture = nullptr;
		ComPtr<ID3D11RenderTargetView> m_MsaaRenderTargetView = nullptr;

		void CreateMsaaDepthStencilView(int width, int height);
		ComPtr<ID3D11DepthStencilView> m_MsaaDepthStencilView = nullptr;
	};
}