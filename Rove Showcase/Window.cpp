#include "Pch.h"
#include "Window.h"

static LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
	case WM_CREATE:
		return 0;

	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;

	case WM_SIZE:
		return 0;

	default:
		return DefWindowProc(hwnd, uMsg, wParam, lParam);
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
	m_Hwnd = CreateWindowEx(0, L"RoveShowcase", title.c_str(), WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, NULL, NULL, hInstance, NULL);

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
