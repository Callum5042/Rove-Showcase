#pragma once

#include "Pch.h"
#include "Windows\Window.h"
#include "Rendering\DxRenderer.h"
#include "Rendering\DxShader.h"

#include "Model.h"
#include "Camera.h"
#include "PointLight.h"

// Components
#include "Components\ViewportComponent.h"
#include "Components\InfoComponent.h"

namespace Rove
{
	// Convert std::string to std::wstring
	std::wstring ConvertToWideString(std::string str);

	// Convert std::wstring to std::string
	std::string ConvertToString(std::wstring str);

	// Main application
	class Application
	{
	public:
		Application();
		virtual ~Application();

		int Run();

		Window* GetWindow() { return m_Window.get(); }
		DxRenderer* GetRenderer() { return m_DxRenderer.get(); }

		///////////
		// Events

		void OnResize(int width, int height);
		void OnMouseWheel(int scroll);
		void OnMouseMove(int mouse_x, int mouse_y, int key_modifier);
		void OnMousePressed(int mouse_x, int mouse_y, int key_modifier);
		void OnMouseReleased(int mouse_x, int mouse_y, int key_modifier);

	private:
		std::unique_ptr<Window> m_Window = nullptr;
		std::unique_ptr<DxRenderer> m_DxRenderer = nullptr;
		std::unique_ptr<DxShader> m_DxShader = nullptr;
		std::unique_ptr<Camera> m_Camera = nullptr;
		std::unique_ptr<Model> m_Model = nullptr;
		std::unique_ptr<PointLight> m_PointLight = nullptr;

		void SetupDearImGui();
		void UpdateCamera();

		void Create();
		void MenuItem_Load();

		// Show camera GUI
		bool m_ShowCameraDetails = true;
		bool m_ShowDebugDetails = true;
		bool m_ShowModelDetails = true;
		bool m_ShowEnvironmentDetails = true;

		int m_MousePressedX = 0;
		int m_MousePressedY = 0;

		// Background colour
		float m_BackgroundColour[3];

		// Model wireframe
		bool m_RenderWireframe = false;
	};
}