#pragma once

#include "Pch.h"

namespace Rove
{
	class Application;

	// Window minimum width when resizing
	constexpr int WINDOW_MIN_WIDTH = 200;

	// Window minimum height when resizing
	constexpr int WINDOW_MIN_HEIGHT = 200;

	// Show info message box
	void ShowMessage(const std::wstring& text);

	// Window
	class Window
	{
	public:
		Window(Application* application);
		virtual ~Window();

		// Create the window
		void Create(std::wstring&& title);

		// Show the window
		void Show();

		// Handle Win32 message queue
		LRESULT HandleMessage(HWND hwnd, INT uMsg, WPARAM wParam, LPARAM lParam);

		// Get Win32 HWND
		constexpr HWND GetHwnd() { return m_Hwnd; }

		// Get window size
		void GetSize(int* width, int* height);

		// Get window title
		std::wstring GetTitle();

		// Set window title
		void SetTitle(const std::wstring& title);

		// Get application
		constexpr Application* GetApplication() { return m_Application; }

	private:
		Application* m_Application = nullptr;

		// Win32 HWND
		HWND m_Hwnd = NULL;

		void WindowResizing(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
		void MouseWheel(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
		void MouseMove(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
		void MouseDown(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
		void MouseUp(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	};
}