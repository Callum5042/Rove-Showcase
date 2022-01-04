#pragma once

#include "Pch.h"

namespace Rove
{
	class Application;

	class Window
	{
	public:
		Window(Application* application);
		virtual ~Window() = default;

		void Create(std::wstring&& title);

		// Get Win32 HWND
		constexpr HWND GetHwnd() { return m_Hwnd; }

		// Get window size
		void GetSize(int* width, int* height);

		// Get window title
		std::wstring GetTitle();

		// Get application
		constexpr Application* GetApplication() { return m_Application; }

	private:
		Application* m_Application = nullptr;

		HWND m_Hwnd = NULL;
	};
}