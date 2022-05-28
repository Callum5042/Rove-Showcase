#include "Pch.h"
#include "DxRenderer.h"
#include "Window.h"
#include "Application.h"
using Rove::DX::Check;

Rove::DxRenderer::DxRenderer(Window* window) : m_Window(window)
{
}

void Rove::DxRenderer::Create()
{
	int window_width = 0;
	int window_height = 0;
	m_Window->GetSize(&window_width, &window_height);

	CreateDeviceAndContext();
	CreateSwapChain(window_width, window_height);
	CreateRenderTargetAndDepthStencilView(window_width, window_height);
	SetViewport(window_width, window_height);

	CreateWireframeRasterState();
	CreateSolidRasterState();

	// MSAA //

	// Check antialising levels
	DX::Check(m_Device->CheckMultisampleQualityLevels(DXGI_FORMAT_R8G8B8A8_UNORM, 4, &m_4xMsaaQuality));

	// Render to texture
	CreateMsaaRenderTargetView(window_width, window_height);
	CreateMsaaDepthStencilView(window_width, window_height);

	CreateAnisotropicFiltering();
}

void Rove::DxRenderer::Clear(const float* colour)
{
	// Clear the render target view to the chosen colour
	m_DeviceContext->ClearRenderTargetView(m_RenderTargetView.Get(), colour);
	m_DeviceContext->ClearDepthStencilView(m_DepthStencilView.Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

	m_DeviceContext->ClearRenderTargetView(m_MsaaRenderTargetView.Get(), colour);
	m_DeviceContext->ClearDepthStencilView(m_MsaaDepthStencilView.Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

	int window_width = 0;
	int window_height = 0;
	m_Window->GetSize(&window_width, &window_height);
	SetViewport(window_width, window_height);
}

void Rove::DxRenderer::Present(bool enable_vsync)
{
	DXGI_PRESENT_PARAMETERS presentParameters = {};
	DX::Check(m_SwapChain->Present1(enable_vsync ? 1 : 0, enable_vsync ? 0 : DXGI_PRESENT_ALLOW_TEARING, &presentParameters));
}

void Rove::DxRenderer::Resize(int width, int height)
{
	// Resize is called when window is first created, so exit early if we are not ready to handle resizing
	if (m_Device == nullptr)
		return;

	m_BackBuffer.ReleaseAndGetAddressOf();

	// Releases the current render target and depth stencil view
	m_DepthStencilView.ReleaseAndGetAddressOf();
	m_RenderTargetView.ReleaseAndGetAddressOf();

	// Resize the swapchain
	DX::Check(m_SwapChain->ResizeBuffers(2, width, height, DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING));

	// Creates a new render target and depth stencil view with the new window size
	CreateRenderTargetAndDepthStencilView(width, height);

	// Render to texture
	CreateMsaaRenderTargetView(width, height);
	CreateMsaaDepthStencilView(width, height);

	// Sets a new viewport with the new window size
	SetViewport(width, height);
}

void Rove::DxRenderer::SetWireframeRasterState()
{
	m_DeviceContext->RSSetState(m_RasterStateWireframe.Get());
}

void Rove::DxRenderer::SetSolidRasterState()
{
	m_DeviceContext->RSSetState(m_RasterStateSolid.Get());
}

void Rove::DxRenderer::CopyMsaaRenderTargetBackBuffer()
{
	m_DeviceContext->ResolveSubresource(m_BackBuffer.Get(), 0, m_MsaaTexture.Get(), 0, DXGI_FORMAT_R8G8B8A8_UNORM);
}

void Rove::DxRenderer::SetRenderToMsaa()
{
	m_DeviceContext->OMSetRenderTargets(1, m_MsaaRenderTargetView.GetAddressOf(), m_MsaaDepthStencilView.Get());
}

void Rove::DxRenderer::SetRenderToBackBuffer()
{
	m_DeviceContext->OMSetRenderTargets(1, m_RenderTargetView.GetAddressOf(), m_DepthStencilView.Get());
}

void Rove::DxRenderer::CreateDeviceAndContext()
{
	// Look for Direct3D 11 feature
	D3D_FEATURE_LEVEL featureLevels[] =
	{
		D3D_FEATURE_LEVEL_11_1
	};

	UINT numFeatureLevels = ARRAYSIZE(featureLevels);

	// Add debug flag if in debug mode
	D3D11_CREATE_DEVICE_FLAG deviceFlag = (D3D11_CREATE_DEVICE_FLAG)0;
#ifdef _DEBUG
	deviceFlag = D3D11_CREATE_DEVICE_DEBUG;
#endif

	// Create device and device context
	D3D_FEATURE_LEVEL featureLevel;
	DX::Check(D3D11CreateDevice(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, deviceFlag, featureLevels, numFeatureLevels,
			  D3D11_SDK_VERSION, m_Device.ReleaseAndGetAddressOf(), &featureLevel, m_DeviceContext.ReleaseAndGetAddressOf()));

	// Check if Direct3D 11.1 is supported, if not fall back to Direct3D 11
	if (featureLevel != D3D_FEATURE_LEVEL_11_1)
	{
		throw std::exception("D3D_FEATURE_LEVEL_11_1 is not supported");
	}
}

void Rove::DxRenderer::CreateSwapChain(int width, int height)
{
	// Query the device until we get the DXGIFactory
	ComPtr<IDXGIDevice> dxgiDevice = nullptr;
	DX::Check(m_Device.As(&dxgiDevice));

	//ComPtr<IDXGIAdapter> adapter = nullptr;
	DX::Check(dxgiDevice->GetAdapter(m_Adapter.GetAddressOf()));

	ComPtr<IDXGIFactory> dxgiFactory = nullptr;
	DX::Check(m_Adapter->GetParent(__uuidof(IDXGIFactory), reinterpret_cast<void**>(dxgiFactory.GetAddressOf())));

	// Query IDXGIFactory to try to get IDXGIFactory2
	ComPtr<IDXGIFactory2> dxgiFactory2 = nullptr;
	DX::Check(dxgiFactory.As(&dxgiFactory2));

	// DirectX 11.1
	DXGI_SWAP_CHAIN_DESC1 swapchain_desc = {};
	swapchain_desc.Width = width;
	swapchain_desc.Height = height;
	swapchain_desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	swapchain_desc.SampleDesc.Count = 1;
	swapchain_desc.SampleDesc.Quality = 0;
	swapchain_desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapchain_desc.BufferCount = 2;
	swapchain_desc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	swapchain_desc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING;

	// CreateSwapChainForHwnd is the prefered way of creating the swap chain
	DX::Check(dxgiFactory2->CreateSwapChainForHwnd(m_Device.Get(), m_Window->GetHwnd(), &swapchain_desc, nullptr, nullptr, &m_SwapChain));

	// Get adapter details
	DXGI_ADAPTER_DESC desc;
	m_Adapter->GetDesc(&desc);

	m_GpuName = Rove::ConvertToString(std::wstring(desc.Description));
	m_GpuVramMb = static_cast<int>(desc.DedicatedVideoMemory / 1024 / 1024);

	// Check tearing support
	ComPtr<IDXGIFactory4> dxgiFactory4 = nullptr;
	DX::Check(dxgiFactory.As(&dxgiFactory4));

	ComPtr<IDXGIFactory5> dxgiFactory5;
	DX::Check(dxgiFactory4.As(&dxgiFactory5));

	BOOL allowTearing = FALSE;
	dxgiFactory5->CheckFeatureSupport(DXGI_FEATURE_PRESENT_ALLOW_TEARING, &allowTearing, sizeof(allowTearing));

	if (allowTearing == false)
	{
		throw std::exception("DXGI_FEATURE_PRESENT_ALLOW_TEARING is not supported");
	}
}

void Rove::DxRenderer::CreateRenderTargetAndDepthStencilView(int width, int height)
{
	// Create the render target view
	DX::Check(m_SwapChain->GetBuffer(0, __uuidof(ID3D11Resource), reinterpret_cast<void**>(m_BackBuffer.GetAddressOf())));
	DX::Check(m_Device->CreateRenderTargetView(m_BackBuffer.Get(), nullptr, m_RenderTargetView.GetAddressOf()));

	// Describe the depth stencil view
	D3D11_TEXTURE2D_DESC depth_desc = {};
	depth_desc.Width = width;
	depth_desc.Height = height;
	depth_desc.MipLevels = 1;
	depth_desc.ArraySize = 1;
	depth_desc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	depth_desc.SampleDesc.Count = 1;
	depth_desc.SampleDesc.Quality = 0;
	depth_desc.Usage = D3D11_USAGE_DEFAULT;
	depth_desc.BindFlags = D3D11_BIND_DEPTH_STENCIL;

	// Create the depth stencil view
	ComPtr<ID3D11Texture2D> depth_stencil = nullptr;
	DX::Check(m_Device->CreateTexture2D(&depth_desc, nullptr, &depth_stencil));
	DX::Check(m_Device->CreateDepthStencilView(depth_stencil.Get(), nullptr, m_DepthStencilView.GetAddressOf()));
}

void Rove::DxRenderer::SetViewport(int width, int height)
{
	// Describe the viewport
	D3D11_VIEWPORT viewport = {};
	viewport.Width = static_cast<float>(width);
	viewport.Height = static_cast<float>(height);
	viewport.MinDepth = 0.0f;
	viewport.MaxDepth = 1.0f;
	viewport.TopLeftX = 0;
	viewport.TopLeftY = 0;

	// Bind viewport to the pipline's rasterization stage
	m_DeviceContext->RSSetViewports(1, &viewport);
}

void Rove::DxRenderer::CreateSolidRasterState()
{
	D3D11_RASTERIZER_DESC rasterizerState = {};
	rasterizerState.AntialiasedLineEnable = true;
	rasterizerState.CullMode = D3D11_CULL_FRONT;
	rasterizerState.FillMode = D3D11_FILL_SOLID;
	rasterizerState.DepthClipEnable = true;
	rasterizerState.FrontCounterClockwise = true;
	rasterizerState.MultisampleEnable = true;

	rasterizerState.DepthBias = 0;
	rasterizerState.DepthBiasClamp = 1.0f;
	rasterizerState.SlopeScaledDepthBias = 1.0f;

	DX::Check(m_Device->CreateRasterizerState(&rasterizerState, m_RasterStateSolid.ReleaseAndGetAddressOf()));
}

void Rove::DxRenderer::CreateMsaaRenderTargetView(int width, int height)
{
	// Render targets
	D3D11_TEXTURE2D_DESC texture_desc = {};
	texture_desc.Width = width;
	texture_desc.Height = height;
	texture_desc.MipLevels = 1;
	texture_desc.ArraySize = 1;
	texture_desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	texture_desc.SampleDesc.Count = 4;
	texture_desc.SampleDesc.Quality = m_4xMsaaQuality - 1;
	texture_desc.Usage = D3D11_USAGE_DEFAULT;
	texture_desc.BindFlags = D3D11_BIND_RENDER_TARGET;
	texture_desc.CPUAccessFlags = 0;
	texture_desc.MiscFlags = 0;

	DX::Check(m_Device->CreateTexture2D(&texture_desc, 0, m_MsaaTexture.ReleaseAndGetAddressOf()));

	// Create the render target view.
	D3D11_RENDER_TARGET_VIEW_DESC target_view_desc = {};
	target_view_desc.Format = texture_desc.Format;
	target_view_desc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2DMS;
	target_view_desc.Texture2D.MipSlice = 0;

	DX::Check(m_Device->CreateRenderTargetView(m_MsaaTexture.Get(), &target_view_desc, m_MsaaRenderTargetView.ReleaseAndGetAddressOf()));
}

void Rove::DxRenderer::CreateMsaaDepthStencilView(int width, int height)
{
	D3D11_TEXTURE2D_DESC texture_desc = {};
	texture_desc.Width = width;
	texture_desc.Height = height;
	texture_desc.MipLevels = 1;
	texture_desc.ArraySize = 1;
	texture_desc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	texture_desc.SampleDesc.Count = 4;
	texture_desc.SampleDesc.Quality = m_4xMsaaQuality - 1;
	texture_desc.Usage = D3D11_USAGE_DEFAULT;
	texture_desc.BindFlags = D3D11_BIND_DEPTH_STENCIL;

	ComPtr<ID3D11Texture2D> texture = nullptr;
	DX::Check(m_Device->CreateTexture2D(&texture_desc, nullptr, texture.ReleaseAndGetAddressOf()));
	DX::Check(m_Device->CreateDepthStencilView(texture.Get(), nullptr, m_MsaaDepthStencilView.ReleaseAndGetAddressOf()));
}

void Rove::DxRenderer::CreateAnisotropicFiltering()
{
	D3D11_SAMPLER_DESC samplerDesc = {};
	samplerDesc.Filter = D3D11_FILTER_ANISOTROPIC;
	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.MipLODBias = 0;
	samplerDesc.MaxAnisotropy = D3D11_REQ_MAXANISOTROPY;
	samplerDesc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
	samplerDesc.MinLOD = 0;
	samplerDesc.MaxLOD = 1000.0f;

	DX::Check(m_Device->CreateSamplerState(&samplerDesc, &m_AnisotropicSampler));

	// Bind to pipeline
	m_DeviceContext->PSSetSamplers(0, 1, m_AnisotropicSampler.GetAddressOf());
}

void Rove::DxRenderer::CreateWireframeRasterState()
{
	D3D11_RASTERIZER_DESC rasterizerState = {};
	rasterizerState.AntialiasedLineEnable = true;
	rasterizerState.CullMode = D3D11_CULL_NONE;
	rasterizerState.FillMode = D3D11_FILL_WIREFRAME;
	rasterizerState.DepthClipEnable = true;
	rasterizerState.FrontCounterClockwise = true;
	rasterizerState.MultisampleEnable = true;

	rasterizerState.DepthBias = 0;
	rasterizerState.DepthBiasClamp = 1.0f;
	rasterizerState.SlopeScaledDepthBias = 1.0f;

	DX::Check(m_Device->CreateRasterizerState(&rasterizerState, m_RasterStateWireframe.ReleaseAndGetAddressOf()));
}
