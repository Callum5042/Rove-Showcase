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
	};
}