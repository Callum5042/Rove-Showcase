#pragma once

#include "Pch.h"

namespace Rove
{
	class Window
	{
	public:
		Window() = default;
		virtual ~Window() = default;

		void Create(std::wstring&& title);

		// Get Win32 HWND
		constexpr HWND GetHwnd() { return m_Hwnd; }

		// Get window size
		void GetSize(int* width, int* height);

	private:
		HWND m_Hwnd = NULL;
	};
}