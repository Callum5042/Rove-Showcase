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

		// File type filters
		COMDLG_FILTERSPEC file_filters[] =
		{
			{ L"glTF JSON", L"*.gltf"},
		};

		fileOpen->SetFileTypes(1, file_filters);
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

std::wstring Rove::ConvertToWideString(std::string str)
{
	int wchars_num = MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1, NULL, 0);

	std::wstring wide_str;
	wide_str.resize(wchars_num);

	MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1, wide_str.data(), wchars_num);
	return wide_str;
}

std::string Rove::ConvertToString(std::wstring wide_str)
{
	int char_num = WideCharToMultiByte(CP_UTF8, 0, wide_str.c_str(), -1, NULL, 0, NULL, NULL);

	std::string str;
	str.resize(char_num);
	WideCharToMultiByte(CP_UTF8, 0, wide_str.c_str(), -1, str.data(), char_num, NULL, NULL);

	return str;
}

Rove::Application::Application()
{
	auto folder = std::filesystem::current_path();

	// Initialise data
	m_Window = std::make_unique<Rove::Window>(this);
	m_DxRenderer = std::make_unique<Rove::DxRenderer>(m_Window.get());
	m_DxShader = std::make_unique<Rove::DxShader>(m_DxRenderer.get());
	
	// Default light
	auto light = std::make_unique<Rove::PointLight>();
	light->Position = DirectX::XMFLOAT3(5.0f, 8.0f, -10.0f);
	light->DiffuseColour = DirectX::XMFLOAT4(0.785f, 0.785f, 0.785f, 1.0f);
	light->AmbientColour = DirectX::XMFLOAT4(0.3925f, 0.3925f, 0.3925f, 1.0f);
	light->SpecularColour = DirectX::XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	m_PointLights.push_back(std::move(light));

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
	ImPlot::DestroyContext();
	ImGui::DestroyContext();
}

int Rove::Application::Run()
{
	Create();
	m_Window->Show();

	// Starts the timer
	m_Timer = std::make_unique<Rove::Timer>();
	m_Timer->Start();

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
			m_Timer->Tick();
			CalculateFramesPerSecond();

			// Start rendering into Dear ImGui
			ImGui_ImplDX11_NewFrame();
			ImGui_ImplWin32_NewFrame();
			ImGui::NewFrame();

			// Clear backbuffer
			m_DxRenderer->Clear(m_BackgroundColour);

			// Render MSAA?
			if (m_EnableMsaa)
			{
				m_DxRenderer->SetRenderToMsaa();
			}
			else
			{
				m_DxRenderer->SetRenderToBackBuffer();
			}

			// Render model into viewport
			m_DxShader->Apply();

			// Enable wireframe rendering
			if (m_RenderWireframe)
			{
				m_DxRenderer->SetWireframeRasterState();
			}

			// Render model
			m_Object->Render();

			// Enable solid rendering
			m_DxRenderer->SetSolidRasterState();

			// Render ImGui windows
			RenderGui();

			// Render Dear ImGui
			ImGui::Render();
			ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

			if (m_EnableMsaa)
			{
				m_DxRenderer->CopyMsaaRenderTargetBackBuffer();
			}

			// Present backbuffer to screen
			m_DxRenderer->Present(m_EnableVSync);
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
	ImPlot::CreateContext();
	ImGuiIO& io = ImGui::GetIO();

	ImPlot::GetStyle().AntiAliasedLines = true;
	io.Fonts->AddFontFromFileTTF("fonts\\DroidSans.ttf", 13);

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

	// Update camera buffer
	Rove::CameraBuffer camera_buffer = {};
	camera_buffer.view = DirectX::XMMatrixTranspose(m_Camera->GetView());
	camera_buffer.projection = DirectX::XMMatrixTranspose(m_Camera->GetProjection());
	camera_buffer.cameraPosition = m_Camera->GetPosition();
	m_DxShader->UpdateCameraBuffer(camera_buffer);

	// Update world buffer
	Rove::WorldBuffer world_buffer = {};
	world_buffer.world = DirectX::XMMatrixTranspose(m_Object->World);
	world_buffer.worldInverse = DirectX::XMMatrixInverse(nullptr, m_Object->World);
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

	// Model
	m_Object = std::make_unique<Rove::Object>(m_DxRenderer.get(), m_DxShader.get());

	UpdateCamera();

	// Dear ImGui
	SetupDearImGui();

	// Update buffers
	UpdateLightBuffer();
}

void Rove::Application::MenuItem_Load()
{
	std::wstring filepath;
	if (OpenFileDialog(filepath, m_Window->GetHwnd()))
	{
		try
		{
			std::string path = ConvertToString(filepath);
			m_Object->LoadFile(path);
		}
		catch (std::exception& e)
		{
			CoUninitialize();
			std::wstring error = Rove::ConvertToWideString(e.what());
			MessageBox(NULL, error.c_str(), L"Error", MB_OK | MB_ICONERROR);
		}
	}
}

void Rove::Application::UpdateLightBuffer()
{
	// Update light buffer
	Rove::PointLightBuffer light_buffer = {};

	light_buffer.lightCount = static_cast<int>(m_PointLights.size());
	for (int i = 0; i < m_PointLights.size(); ++i)
	{
		light_buffer.pointLight[i] = Rove::PointLightStruct();
		light_buffer.pointLight[i].position = m_PointLights[i]->Position;
		light_buffer.pointLight[i].diffuse = m_PointLights[i]->DiffuseColour;
		light_buffer.pointLight[i].ambient = m_PointLights[i]->AmbientColour;
		light_buffer.pointLight[i].specular = m_PointLights[i]->SpecularColour;
	}

	m_DxShader->UpdatePointLightBuffer(light_buffer);
}

void Rove::Application::CalculateFramesPerSecond()
{
	// Consider using a third-party library such as ImPlot: https://github.com/epezent/implot
	// (see others https://github.com/ocornut/imgui/wiki/Useful-Extensions)

	static double time = 0;
	static int frameCount = 0;

	frameCount++;
	time += m_Timer->DeltaTime();
	if (time > 1.0f)
	{
		auto fps = frameCount;
		time = 0.0f;
		frameCount = 0;

		m_FramesPerSecond = fps;
	}
}

void Rove::Application::RenderGui()
{
	//ImGui::ShowDemoWindow(nullptr);
	//ImPlot::ShowDemoWindow();

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

	// Renderer details
	if (m_ShowRendererDetails)
	{
		if (ImGui::Begin("Renderer", &m_ShowRendererDetails))
		{
			ImGui::Text(m_DxRenderer->GetGpuName().c_str());

			std::string vram = "VRAM: ";
			vram += std::to_string(m_DxRenderer->GetGpuVramMB()) + " MB";
			ImGui::Text(vram.c_str());

			std::string fps = "FPS: " + std::to_string(m_FramesPerSecond);
			ImGui::Text(fps.c_str());

			ImGui::Checkbox("MSAA", &m_EnableMsaa);
			ImGui::Checkbox("V-Sync", &m_EnableVSync);
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
			// ImGui::Text("Vertices: %i", m_Model->GetVertices());
			// ImGui::Text("Indices: %i", m_Model->GetIndices());
			ImGui::Checkbox("Enable Wireframe", &m_RenderWireframe);
		}

		// Show materials
		std::vector<Rove::Material*> materials = m_Object->GetMaterials();
		for (size_t i = 0; i < materials.size(); ++i)
		{
			ImGui::Separator();

			std::string name = "Materials: " + std::to_string(i);
			ImGui::Text(name.c_str());

			std::string roughness_label = "Roughness## " + std::to_string(i);
			ImGui::SliderFloat(roughness_label.c_str(), &materials[i]->roughnessFactor, 0.0f, 1.0f);
		}

		ImGui::End();
	}

	// Environment details
	if (m_ShowEnvironmentDetails)
	{
		if (ImGui::Begin("Environment", &m_ShowEnvironmentDetails, ImGuiWindowFlags_AlwaysAutoResize))
		{
			ImGui::ColorEdit3("Background Colour", m_BackgroundColour, ImGuiColorEditFlags_NoOptions);
			ImGui::Separator();
			ImGui::Text("Lights");

			/*static bool cast_shadows = false;
			ImGui::Checkbox("Cast Shadows", &cast_shadows);*/

			if (ImGui::Button("Add light"))
			{
				m_PointLights.push_back(std::make_unique<Rove::PointLight>());
			}

			for (int i = 0; i < m_PointLights.size(); ++i)
			{
				const auto& point_light = m_PointLights[i];

				float* light_position = reinterpret_cast<float*>(&point_light->Position);

				if (ImGui::DragFloat3(("Position##" + std::to_string(i)).c_str(), light_position))
				{
					UpdateLightBuffer();
				}

				float* light_diffuse = reinterpret_cast<float*>(&point_light->DiffuseColour);
				if (ImGui::ColorEdit3(("Diffuse##" + std::to_string(i)).c_str(), light_diffuse))
				{
					UpdateLightBuffer();
				}

				float* light_ambient = reinterpret_cast<float*>(&point_light->AmbientColour);
				if (ImGui::ColorEdit3(("Ambient##" + std::to_string(i)).c_str(), light_ambient))
				{
					UpdateLightBuffer();
				}

				float* light_specular = reinterpret_cast<float*>(&point_light->SpecularColour);
				if (ImGui::ColorEdit3(("Specular##" + std::to_string(i)).c_str(), light_specular))
				{
					UpdateLightBuffer();
				}

				// Add line seperate between each light section
				if (i != m_PointLights.size() - 1)
				{
					ImGui::Separator();
				}
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
			ImGui::MenuItem("Renderer", nullptr, &m_ShowRendererDetails);
			ImGui::MenuItem("Camera", nullptr, &m_ShowCameraDetails);
			ImGui::MenuItem("Model", nullptr, &m_ShowModelDetails);
			ImGui::MenuItem("Environment", nullptr, &m_ShowEnvironmentDetails);
			ImGui::EndMenu();
		}

		ImGui::EndMainMenuBar();
	}
}
