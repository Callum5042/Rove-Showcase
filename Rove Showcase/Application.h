#pragma once

#include "Pch.h"
#include "Windows\Window.h"
#include "Rendering\DxRenderer.h"
#include "Rendering\DxShader.h"

#include "Model.h"
#include "Camera.h"

// Components
#include "Components\ViewportComponent.h"
#include "Components\InfoComponent.h"

namespace Rove
{
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

		void SetupDearImGui();
		void UpdateCamera();

		// Components
		std::unique_ptr<ViewportComponent> m_ViewportComponent = nullptr;
		std::unique_ptr<InfoComponent> m_InfoComponent = nullptr;

		void Create();
		void MenuItem_Load();

		// Show camera GUI
		bool m_ShowCameraProperties = true;
		bool m_ShowDebugDetails = true;

		int m_MousePressedX = 0;
		int m_MousePressedY = 0;
	};
}