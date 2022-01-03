#pragma once

#include "Pch.h"

namespace Rove
{
	class Window
	{
	public:
		Window(HINSTANCE hInstance, int nCmdShow);
		virtual ~Window() = default;

		void Create(std::wstring&& title);

	private:
		HINSTANCE m_hInstance = NULL;
		int m_CmdShow = 0;
	};
}