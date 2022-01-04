#pragma once

#include "Pch.h"
#include "Window.h"
#include "DxRenderer.h"

namespace Rove
{
	class Application
	{
	public:
		Application() = default;
		virtual ~Application() = default;

		int Run();

	private:
		std::unique_ptr<Window> m_Window = nullptr;
		std::unique_ptr<DxRenderer> m_DxRenderer = nullptr;
	};
}