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

	// Viewport
	m_Viewport = std::make_unique<Rove::Viewport>(this);
	m_Viewport->OnCreate();

	// Dear ImGui
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();

	io.Fonts->AddFontFromFileTTF("ImGui\\fonts\\DroidSans.ttf", 13);

	ImGui::StyleColorsDark();
	ImGui_ImplWin32_Init(m_Window->GetHwnd());
	ImGui_ImplDX11_Init(m_DxRenderer->GetDevice(), m_DxRenderer->GetDeviceContext());

	

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
			m_Viewport->Set();



			// Clear backbuffer
			m_DxRenderer->Clear();



			// Dear ImGui
			ImGui_ImplDX11_NewFrame();
			ImGui_ImplWin32_NewFrame();
			ImGui::NewFrame();

			/*bool show_demo_window = true;
			ImGui::ShowDemoWindow(&show_demo_window);*/

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

				auto viewport_texture = m_Viewport->GetViewportTexture();
				ImGui::GetWindowDrawList()->AddImage(reinterpret_cast<void*>(viewport_texture), pos, ImVec2(pos.x + width, pos.y + height));

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
