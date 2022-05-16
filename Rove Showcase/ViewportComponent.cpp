#include "Pch.h"
#include "ViewportComponent.h"
#include "Application.h"

Rove::ViewportComponent::ViewportComponent(Application* application) : m_Application(application)
{
}

void Rove::ViewportComponent::OnCreate()
{
	int width, height;
	m_Application->GetWindow()->GetSize(&width, &height);

	m_WindowWidth = width;
	m_WindowHeight = height;

	CreateSharedTexture(width, height);
	CreateRenderTargetView();
	CreateDepthStencilView(width, height);
	CreateShaderResourceView();
	SetViewport(width, height);
}

void Rove::ViewportComponent::OnRender()
{
	ImGui::SetNextWindowSize(ImVec2(800, 600), ImGuiCond_Once);
	ImGui::Begin("Viewport", nullptr);

	// Get window details
	ImVec2 pos = ImGui::GetCursorScreenPos();
	ImVec2 size = ImGui::GetContentRegionAvail();
	int width = static_cast<int>(size.x);
	int height = static_cast<int>(size.y);

	// Detect ImGui window is resizing
	if (m_WindowWidth != width && m_WindowHeight != height)
	{
		if (ImGui::IsMouseReleased(ImGuiMouseButton_Left))
		{
			m_WindowWidth = width;
			m_WindowHeight = height;

			Resize(width, height);
		}
	}

	// Render Direct3D11 texture to ImGui viewport
	ImGui::GetWindowDrawList()->AddImage(reinterpret_cast<void*>(m_RenderedTexture.Get()), pos, ImVec2(pos.x + width, pos.y + height));

	ImGui::End();
}

void Rove::ViewportComponent::Set()
{
	auto context = m_Application->GetRenderer()->GetDeviceContext();

	// Clear the render target view to the chosen colour
	context->ClearRenderTargetView(m_TextureRenderTargetView.Get(), reinterpret_cast<const float*>(&DirectX::Colors::DarkGreen));
	context->ClearDepthStencilView(m_TextureDepthStencilView.Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

	// Bind the render target view to the pipeline's output merger stage
	context->OMSetRenderTargets(1, m_TextureRenderTargetView.GetAddressOf(), m_TextureDepthStencilView.Get());

	// Set viewport
	SetViewport(m_WindowWidth, m_WindowHeight);
}

void Rove::ViewportComponent::CreateSharedTexture(int width, int height)
{
	auto device = m_Application->GetRenderer()->GetDevice();

	// Texture
	D3D11_TEXTURE2D_DESC texture_desc = {};
	texture_desc.Width = width;
	texture_desc.Height = height;
	texture_desc.MipLevels = 1;
	texture_desc.ArraySize = 1;
	texture_desc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	texture_desc.SampleDesc.Count = 1;
	texture_desc.SampleDesc.Quality = 0;
	texture_desc.Usage = D3D11_USAGE_DEFAULT;
	texture_desc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
	texture_desc.CPUAccessFlags = 0;
	texture_desc.MiscFlags = 0;

	DX::Check(device->CreateTexture2D(&texture_desc, 0, m_SharedTexture.ReleaseAndGetAddressOf()));
}

void Rove::ViewportComponent::CreateShaderResourceView()
{
	auto device = m_Application->GetRenderer()->GetDevice();

	D3D11_SHADER_RESOURCE_VIEW_DESC view_desc = {};
	view_desc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	view_desc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	view_desc.Texture2D.MostDetailedMip = 0;
	view_desc.Texture2D.MipLevels = 1;

	DX::Check(device->CreateShaderResourceView(m_SharedTexture.Get(), &view_desc, m_RenderedTexture.ReleaseAndGetAddressOf()));
}

void Rove::ViewportComponent::CreateRenderTargetView()
{
	auto device = m_Application->GetRenderer()->GetDevice();
	auto context = m_Application->GetRenderer()->GetDeviceContext();

	// Create the render target view.
	D3D11_RENDER_TARGET_VIEW_DESC target_view_desc = {};
	target_view_desc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	target_view_desc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
	target_view_desc.Texture2D.MipSlice = 0;

	DX::Check(device->CreateRenderTargetView(m_SharedTexture.Get(), &target_view_desc, m_TextureRenderTargetView.ReleaseAndGetAddressOf()));
}

void Rove::ViewportComponent::CreateDepthStencilView(int width, int height)
{
	auto device = m_Application->GetRenderer()->GetDevice();
	auto context = m_Application->GetRenderer()->GetDeviceContext();

	// Create depth stencil
	D3D11_TEXTURE2D_DESC texture_desc = {};
	texture_desc.Width = width;
	texture_desc.Height = height;
	texture_desc.MipLevels = 1;
	texture_desc.ArraySize = 1;
	texture_desc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	texture_desc.SampleDesc.Count = 1;
	texture_desc.SampleDesc.Quality = 0;
	texture_desc.Usage = D3D11_USAGE_DEFAULT;
	texture_desc.BindFlags = D3D11_BIND_DEPTH_STENCIL;

	ComPtr<ID3D11Texture2D> texture = nullptr;
	DX::Check(device->CreateTexture2D(&texture_desc, nullptr, texture.ReleaseAndGetAddressOf()));
	DX::Check(device->CreateDepthStencilView(texture.Get(), nullptr, m_TextureDepthStencilView.ReleaseAndGetAddressOf()));
}

void Rove::ViewportComponent::Resize(int width, int height)
{
	CreateSharedTexture(width, height);
	CreateRenderTargetView();
	CreateDepthStencilView(width, height);
	CreateShaderResourceView();
	SetViewport(width, height);
}

void Rove::ViewportComponent::SetViewport(int width, int height)
{
	auto context = m_Application->GetRenderer()->GetDeviceContext();

	D3D11_VIEWPORT vp = {};
	vp.Width = static_cast<FLOAT>(width);
	vp.Height = static_cast<FLOAT>(height);
	vp.MinDepth = 0.0f;
	vp.MaxDepth = 1.0f;
	vp.TopLeftX = 0;
	vp.TopLeftY = 0;

	context->RSSetViewports(1, &vp);
}
