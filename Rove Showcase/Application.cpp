#include "Pch.h"
#include "Application.h"

int Rove::Application::Run()
{
	// Initialise data
	m_Window = std::make_unique<Rove::Window>(this);
	m_DxRenderer = std::make_unique<Rove::DxRenderer>(m_Window.get());

	// Create window
	m_Window->Create(L"Rove Showcase");

	// Create renderer
	m_DxRenderer->Create();

	// Dear ImGui
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();

	io.Fonts->AddFontFromFileTTF("ImGui\\fonts\\DroidSans.ttf", 13);

	ImGui::StyleColorsDark();
	ImGui_ImplWin32_Init(m_Window->GetHwnd());
	ImGui_ImplDX11_Init(m_DxRenderer->GetDevice(), m_DxRenderer->GetDeviceContext());

	// Texture
	auto device = m_DxRenderer->GetDevice();
	auto context = m_DxRenderer->GetDeviceContext();

	ComPtr<ID3D11ShaderResourceView> m_RenderedTexture = nullptr;
	ComPtr<ID3D11Texture2D> m_Texture = nullptr;

	ComPtr<ID3D11RenderTargetView> m_TextureRenderTargetView = nullptr;
	ComPtr<ID3D11DepthStencilView> m_TextureDepthStencilView = nullptr;

	int width, height;
	m_Window->GetSize(&width, &height);

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

	// Main loop
	MSG msg = {};
	while (msg.message != WM_QUIT) 
	{
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) 
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		else
		{
			// Clear the render target view to the chosen colour
			context->ClearRenderTargetView(m_TextureRenderTargetView.Get(), reinterpret_cast<const float*>(&DirectX::Colors::DarkGreen));
			context->ClearDepthStencilView(m_TextureDepthStencilView.Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

			// Bind the render target view to the pipeline's output merger stage
			context->OMSetRenderTargets(1, m_TextureRenderTargetView.GetAddressOf(), m_TextureDepthStencilView.Get());





			// Clear backbuffer
			m_DxRenderer->Clear();



			// Dear ImGui
			ImGui_ImplDX11_NewFrame();
			ImGui_ImplWin32_NewFrame();
			ImGui::NewFrame();

			bool show_demo_window = true;
			ImGui::ShowDemoWindow(&show_demo_window);

			// Info window
			{
				ImGui::SetNextWindowPos(ImVec2(20, 20), ImGuiCond_Once);
				ImGui::SetNextWindowSize(ImVec2(200, 60), ImGuiCond_Once);
				ImGui::Begin("InfoWindow", nullptr, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoTitleBar);
				ImGui::Text("Rove Showroom - FPS: %d", 10);
				ImGui::End();
			}

			// Viewport
			{
				ImGui::SetNextWindowSize(ImVec2(640, 480), ImGuiCond_Once);
				ImGui::Begin("Viewport", nullptr);

				ImVec2 pos = ImGui::GetCursorScreenPos();
				ImVec2 size = ImGui::GetContentRegionAvail();
				int width = (int)size.x;
				int height = (int)size.y;

				ImGui::GetWindowDrawList()->AddImage(reinterpret_cast<void*>(m_RenderedTexture.Get()), pos, ImVec2(pos.x + width, pos.y + height));

				ImGui::End();
			}

			ImGui::Render();
			ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

			// Present backbuffer to screen
			m_DxRenderer->Present();
		}
	}

	// Clean up ImGui
	ImGui_ImplDX11_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();

	return 0;
}

void Rove::Application::OnResize(int width, int height)
{
	m_DxRenderer->Resize(width, height);
}
