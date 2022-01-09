#include "Pch.h"
#include "Application.h"

Rove::Application::Application()
{
	// Initialise data
	m_Window = std::make_unique<Rove::Window>(this);
	m_DxRenderer = std::make_unique<Rove::DxRenderer>(m_Window.get());
	m_DxShader = std::make_unique<Rove::DxShader>(m_DxRenderer.get());

	HRESULT hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);
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
			m_DxRenderer->Clear();

			// Render model into viewport
			m_DxShader->Apply();

			// Render model
			m_Model->Render();

			//
			// Render ImGui windows
			// 
			//ImGui::ShowDemoWindow(nullptr);

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
			if (m_ShowCameraProperties)
			{
				if (ImGui::Begin("Camera", &m_ShowCameraProperties, ImGuiWindowFlags_AlwaysAutoResize))
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
					ImGui::MenuItem("Camera", nullptr, &m_ShowCameraProperties);
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


		/*static int previous_mouse_x = mouse_x;
		static int previous_mouse_y = mouse_y;

		float yaw = static_cast<float>(mouse_x - previous_mouse_x) * 0.005f;
		float pitch = static_cast<float>(mouse_y - previous_mouse_y) * 0.005f;

		pitch = std::clamp<float>(pitch, -89, 89);

		m_Camera->Rotate(pitch, yaw);
		UpdateCamera();

		previous_mouse_x = mouse_x;
		previous_mouse_y = mouse_y;*/
	}
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
	// https://docs.microsoft.com/en-us/windows/win32/learnwin32/example--the-open-dialog-box
	IFileOpenDialog* fileOpen = nullptr;
	HRESULT hr = CoCreateInstance(CLSID_FileOpenDialog, NULL, CLSCTX_ALL, IID_IFileOpenDialog, reinterpret_cast<void**>(&fileOpen));

	hr = fileOpen->Show(NULL);
}
