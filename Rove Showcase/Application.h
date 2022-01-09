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
		virtual ~Application() = default;

		int Run();

		Window* GetWindow() { return m_Window.get(); }
		DxRenderer* GetRenderer() { return m_DxRenderer.get(); }

		///////////
		// Events

		void OnResize(int width, int height);

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
	};
}