#include "Pch.h"
#include "Viewport.h"
#include "Application.h"

Rove::Viewport::Viewport(Application* application) : m_Application(application)
{
}

void Rove::Viewport::OnCreate()
{
	// Texture
	auto device =  m_Application->GetRenderer()->GetDevice();
	auto context = m_Application->GetRenderer()->GetDeviceContext();

	int width, height;
	m_Application->GetWindow()->GetSize(&width, &height);

	// Render targets
	{
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

		DX::Check(device->CreateTexture2D(&texture_desc, 0, m_Texture.ReleaseAndGetAddressOf()));

		// Create the render target view.
		D3D11_RENDER_TARGET_VIEW_DESC target_view_desc = {};
		target_view_desc.Format = texture_desc.Format;
		target_view_desc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
		target_view_desc.Texture2D.MipSlice = 0;


		DX::Check(device->CreateRenderTargetView(m_Texture.Get(), &target_view_desc, m_TextureRenderTargetView.ReleaseAndGetAddressOf()));
	}

	// Depth stencil
	{
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

	// Shader resource
	D3D11_SHADER_RESOURCE_VIEW_DESC view_desc = {};
	view_desc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	view_desc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	view_desc.Texture2D.MostDetailedMip = 0;
	view_desc.Texture2D.MipLevels = 1;

	DX::Check(device->CreateShaderResourceView(m_Texture.Get(), &view_desc, m_RenderedTexture.ReleaseAndGetAddressOf()));
}

void Rove::Viewport::Set()
{
	auto context = m_Application->GetRenderer()->GetDeviceContext();

	// Clear the render target view to the chosen colour
	context->ClearRenderTargetView(m_TextureRenderTargetView.Get(), reinterpret_cast<const float*>(&DirectX::Colors::DarkGreen));
	context->ClearDepthStencilView(m_TextureDepthStencilView.Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

	// Bind the render target view to the pipeline's output merger stage
	context->OMSetRenderTargets(1, m_TextureRenderTargetView.GetAddressOf(), m_TextureDepthStencilView.Get());
}
