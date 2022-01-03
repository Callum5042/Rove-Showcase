#include "Pch.h"
#include "Application.h"

int Rove::Application::Run(HINSTANCE hInstance, int nCmdShow)
{
	// Create window
	m_Window = std::make_unique<Rove::Window>(hInstance, nCmdShow);
	m_Window->Create(L"Rove Showcase");

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
