
#include "Pch.h"

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

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	// Register class
	WNDCLASSEX wndClass = {};
	wndClass.cbSize = sizeof(WNDCLASSEX);
	wndClass.lpfnWndProc = WindowProc;
	wndClass.hInstance = hInstance;
	wndClass.lpszClassName = L"RoveShowcase";
	wndClass.hCursor = LoadCursor(NULL, IDC_ARROW);

	if (!RegisterClassEx(&wndClass))
	{
		MessageBox(nullptr, L"RegisterClassEx failed!", L"Error", MB_OK | MB_ICONERROR);
		return false;
	}

	// Create window
	HWND hwnd = CreateWindowEx(
		0,                              // Optional window styles.
		L"RoveShowcase",                     // Window class
		L"Rove Showcase",			// Window text
		WS_OVERLAPPEDWINDOW,            // Window style

		// Size and position
		CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,

		NULL,       // Parent window    
		NULL,       // Menu
		hInstance,  // Instance handle
		NULL        // Additional application data
	);

	if (hwnd == NULL) 
	{
		MessageBox(nullptr, L"CreateWindowEx failed!", L"Error", MB_OK | MB_ICONERROR);
		return false;
	}

	ShowWindow(hwnd, nCmdShow);


	// Main loop
	MSG msg = {};
	while (msg.message != WM_QUIT) 
	{
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	return 0;
}
