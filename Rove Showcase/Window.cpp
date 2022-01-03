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

Rove::Window::Window(HINSTANCE hInstance, int nCmdShow) : m_hInstance(hInstance), m_CmdShow(nCmdShow)
{
}

void Rove::Window::Create(std::wstring&& title)
{
	// Register class
	WNDCLASSEX wndClass = {};
	wndClass.cbSize = sizeof(WNDCLASSEX);
	wndClass.lpfnWndProc = WindowProc;
	wndClass.hInstance = m_hInstance;
	wndClass.lpszClassName = L"RoveShowcase";
	wndClass.hCursor = LoadCursor(NULL, IDC_ARROW);

	if (!RegisterClassEx(&wndClass))
	{
		throw std::exception("RegisterClassEx failed");
	}

	// Create window
	HWND hwnd = CreateWindowEx(0, L"RoveShowcase", title.c_str(), WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, NULL, NULL, m_hInstance, NULL);

	if (hwnd == NULL) 
	{
		throw std::exception("CreateWindowEx failed");
	}

	ShowWindow(hwnd, m_CmdShow);
}
