#pragma once

#include "Pch.h"

namespace Rove
{
	class Application;

	// Window minimum width when resizing
	constexpr int WINDOW_MIN_WIDTH = 200;

	// Window minimum height when resizing
	constexpr int WINDOW_MIN_HEIGHT = 200;

	// Window
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

		// Win32 HWND
		HWND m_Hwnd = NULL;
	};
}