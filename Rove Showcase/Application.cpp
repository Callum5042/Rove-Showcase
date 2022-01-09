#include "Pch.h"
#include "Application.h"

namespace
{
	static bool OpenFileDialog(std::wstring& title, HWND owner)
	{
		// https://docs.microsoft.com/en-us/windows/win32/learnwin32/example--the-open-dialog-box

		HRESULT hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);
		if (hr != S_OK)
		{
			throw std::exception("CoInitializeEx failed");
		}

		// Show dialog box
		IFileOpenDialog* fileOpen = nullptr;
		hr = CoCreateInstance(CLSID_FileOpenDialog, NULL, CLSCTX_ALL, IID_IFileOpenDialog, reinterpret_cast<void**>(&fileOpen));
		if (hr != S_OK)
		{
			throw std::exception("CoCreateInstance failed");
		}

		hr = fileOpen->Show(owner);

		// Get the file name from the dialog box.
		if (hr == S_OK)
		{
			IShellItem* pItem = nullptr;
			hr = fileOpen->GetResult(&pItem);

			PWSTR pszFilePath;
			hr = pItem->GetDisplayName(SIGDN_FILESYSPATH, &pszFilePath);
			title = pszFilePath;

			CoTaskMemFree(pszFilePath);
			pItem->Release();

			CoUninitialize();
			return true;
		}

		CoUninitialize();
		return false;
	}
}

Rove::Application::Application()
{
	// Initialise data
	m_Window = std::make_unique<Rove::Window>(this);
	m_DxRenderer = std::make_unique<Rove::DxRenderer>(m_Window.get());
	m_DxShader = std::make_unique<Rove::DxShader>(m_DxRenderer.get());

	// Set colour
	auto colour = DirectX::Colors::SteelBlue;
	m_BackgroundColour[0] = colour[0];
	m_BackgroundColour[1] = colour[1];
	m_BackgroundColour[2] = colour[2];
}

Rove::Application::~Application()
{
	// Clean up ImGui
	ImGui_ImplDX11_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();
}

int Rove::Application::Run()
{
	Create();
	m_Window->Show();

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
			// Start rendering into Dear ImGui
			ImGui_ImplDX11_NewFrame();
			ImGui_ImplWin32_NewFrame();
			ImGui::NewFrame();

			// Clear backbuffer
			m_DxRenderer->Clear(m_BackgroundColour);

			// Render model into viewport
			m_DxShader->Apply();

			// Enable wireframe rendering
			if (m_RenderWireframe)
			{
				m_DxRenderer->SetWireframeRasterState();
			}

			// Render model
			m_Model->Render();

			// Enable solid rendering
			m_DxRenderer->SetSolidRasterState();

			//
			// Render ImGui windows
			// 
			// ImGui::ShowDemoWindow(nullptr);

			// Debug details
			if (m_ShowDebugDetails) 
			{
				if (ImGui::Begin("Debug", &m_ShowDebugDetails, ImGuiWindowFlags_AlwaysAutoResize))
				{
					// Position
					POINT mouse_position;
					GetCursorPos(&mouse_position);
					ScreenToClient(m_Window->GetHwnd(), &mouse_position);

					ImGui::Text("Mouse (%i, %i)", mouse_position.x, mouse_position.y);
				}

				ImGui::End();
			}

			// Camera details
			if (m_ShowCameraDetails)
			{
				if (ImGui::Begin("Camera", &m_ShowCameraDetails, ImGuiWindowFlags_AlwaysAutoResize))
				{
					// Position
					DirectX::XMFLOAT3 position = m_Camera->GetPosition();
					ImGui::Text("X: %f - Y: %f - Z: %f", position.x, position.y, position.z);

					// Field of view
					float fov_degrees = m_Camera->GetFieldOfView();
					if (ImGui::SliderFloat("Field of view", &fov_degrees, 0.1f, 179.9f))
					{
						m_Camera->SetFov(fov_degrees);
						UpdateCamera();
					}
				}

				ImGui::End();
			}

			// Model details
			if (m_ShowModelDetails) 
			{
				if (ImGui::Begin("Model", &m_ShowModelDetails, ImGuiWindowFlags_AlwaysAutoResize))
				{
					ImGui::Checkbox("Enable Wireframe", &m_RenderWireframe);
				}

				ImGui::End();
			}

			// Environment details
			if (m_ShowEnvironmentDetails) 
			{
				if (ImGui::Begin("Environment", &m_ShowEnvironmentDetails, ImGuiWindowFlags_AlwaysAutoResize))
				{
					ImGui::ColorEdit3("Background Colour", m_BackgroundColour, ImGuiColorEditFlags_NoOptions);
				}

				ImGui::End();
			}

			// Menu
			{
				ImGui::BeginMainMenuBar();

				if (ImGui::BeginMenu("File"))
				{
					if (ImGui::MenuItem("Open"))
					{
						MenuItem_Load();
					}

					ImGui::EndMenu();
				}

				if (ImGui::BeginMenu("View"))
				{
					ImGui::MenuItem("Debug", nullptr, &m_ShowDebugDetails);
					ImGui::MenuItem("Camera", nullptr, &m_ShowCameraDetails);
					ImGui::MenuItem("Model", nullptr, &m_ShowModelDetails);
					ImGui::MenuItem("Environment", nullptr, &m_ShowEnvironmentDetails);
					ImGui::EndMenu();
				}

				ImGui::EndMainMenuBar();
			}

			// Render Dear ImGui
			ImGui::Render();
			ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

			// Present backbuffer to screen
			m_DxRenderer->Present();
		}
	}

	return 0;
}

void Rove::Application::OnResize(int width, int height)
{
	m_DxRenderer->Resize(width, height);
	UpdateCamera();
}

void Rove::Application::OnMouseWheel(int scroll)
{
	m_Camera->UpdateFov(static_cast<float>(scroll * -1.0f));
	UpdateCamera();
}

void Rove::Application::OnMouseMove(int mouse_x, int mouse_y, int key_modifier)
{
	if (key_modifier & MK_MBUTTON)
	{
		float yaw = static_cast<float>(mouse_x - m_MousePressedX) * 0.005f;
		float pitch = static_cast<float>(mouse_y - m_MousePressedY) * 0.005f;

		pitch = std::clamp<float>(pitch, -89, 89);

		m_Camera->Rotate(pitch, yaw);
		UpdateCamera();

		m_MousePressedX = mouse_x;
		m_MousePressedY = mouse_y;
	}
}

void Rove::Application::OnMousePressed(int mouse_x, int mouse_y, int key_modifier)
{
	if (key_modifier & MK_MBUTTON)
	{
		m_MousePressedX = mouse_x;
		m_MousePressedY = mouse_y;
	}
}

void Rove::Application::OnMouseReleased(int mouse_x, int mouse_y, int key_modifier)
{
}

void Rove::Application::SetupDearImGui()
{
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();

	io.Fonts->AddFontFromFileTTF("ImGui\\fonts\\DroidSans.ttf", 13);

	ImGui::StyleColorsDark();
	ImGui_ImplWin32_Init(m_Window->GetHwnd());
	ImGui_ImplDX11_Init(m_DxRenderer->GetDevice(), m_DxRenderer->GetDeviceContext());
}

void Rove::Application::UpdateCamera()
{
	if (m_Camera == nullptr)
		return;

	int width, height;
	m_Window->GetSize(&width, &height);

	m_Camera->UpdateAspectRatio(width, height);

	Rove::WorldBuffer world_buffer = {};
	world_buffer.world = DirectX::XMMatrixTranspose(m_Model->World);
	world_buffer.view = DirectX::XMMatrixTranspose(m_Camera->GetView());
	world_buffer.projection = DirectX::XMMatrixTranspose(m_Camera->GetProjection());

	m_DxShader->UpdateWorldConstantBuffer(world_buffer);
}

void Rove::Application::Create()
{
	// Create window
	m_Window->Create(L"Rove Showcase");

	// Create renderer
	m_DxRenderer->Create();
	m_DxShader->Load();

	// Camera
	int width, height;
	m_Window->GetSize(&width, &height);
	m_Camera = std::make_unique<Rove::Camera>(width, height);

	// Viewport
	m_ViewportComponent = std::make_unique<Rove::ViewportComponent>(this);
	m_ViewportComponent->OnCreate();

	// Model
	m_Model = std::make_unique<Rove::Model>(m_DxRenderer.get());
	m_Model->Create();
	UpdateCamera();

	// Dear ImGui
	SetupDearImGui();
}


void Rove::Application::MenuItem_Load()
{
	std::wstring filepath;
	if (OpenFileDialog(filepath, m_Window->GetHwnd()))
	{
		ShowMessage(filepath);
	}
}
