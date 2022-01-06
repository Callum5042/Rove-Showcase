#pragma once

#include "Pch.h"
#include "Windows/Window.h"
#include "Rendering/DxRenderer.h"

// Components
#include "Components/Viewport.h"

namespace Rove
{
	class Application
	{
	public:
		Application() = default;
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


		// Components
		std::unique_ptr<Viewport> m_Viewport = nullptr;
	};
}