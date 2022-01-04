#include "Pch.h"
#include "Window.h"

namespace
{
	Rove::Window* GetWindow(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
		Rove::Window* window = nullptr;
		if (uMsg == WM_NCCREATE)
		{
			CREATESTRUCT* pCreate = reinterpret_cast<CREATESTRUCT*>(lParam);
			window = reinterpret_cast<Rove::Window*>(pCreate->lpCreateParams);
			SetWindowLongPtr(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(window));
		}
		else
		{
			window = reinterpret_cast<Rove::Window*>(GetWindowLongPtr(hwnd, GWLP_USERDATA));
		}

		return window;
	}

	static LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
		// Get pointer
		Rove::Window* window = GetWindow(hwnd, uMsg, wParam, lParam);

		// Message handling
		switch (uMsg)
		{
		case WM_CREATE:
			return 0;

		/*case WM_CLOSE:
			DestroyWindow(hwnd);
			return 0;*/

		case WM_DESTROY:
			PostQuitMessage(0);
			return 0;

		case WM_SIZE:
			return 0;

		default:
			return DefWindowProc(hwnd, uMsg, wParam, lParam);
		}
	}
}

void Rove::Window::Create(std::wstring&& title)
{
	HINSTANCE hInstance = GetModuleHandle(NULL);

	// Register class
	WNDCLASSEX wndClass = {};
	wndClass.cbSize = sizeof(WNDCLASSEX);
	wndClass.lpfnWndProc = WindowProc;
	wndClass.hInstance = hInstance;
	wndClass.lpszClassName = L"RoveShowcase";
	wndClass.hCursor = LoadCursor(NULL, IDC_ARROW);

	if (!RegisterClassEx(&wndClass))
	{
		throw std::exception("RegisterClassEx failed");
	}

	// Create window
	m_Hwnd = CreateWindowEx(0, L"RoveShowcase", title.c_str(), WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, NULL, NULL, hInstance, this);

	if (m_Hwnd == NULL)
	{
		throw std::exception("CreateWindowEx failed");
	}

	ShowWindow(m_Hwnd, SW_NORMAL);
}

void Rove::Window::GetSize(int* width, int* height)
{
	RECT rect;
	GetClientRect(m_Hwnd, &rect);

	*width = rect.right - rect.left;
	*height = rect.bottom - rect.top;
}

std::wstring Rove::Window::GetTitle()
{
	int length = GetWindowTextLength(m_Hwnd);

	std::wstring title;
	title.resize(length);

	GetWindowText(m_Hwnd, title.data(), length + 1);
	return title;
}
