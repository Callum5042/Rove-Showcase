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

			// Camera details
			{
				ImGui::Begin("Camera");
				ImGui::Text("Camera");
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
