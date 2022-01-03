#include "Pch.h"
#include "Application.h"

int Rove::Application::Run()
{
	// Create window
	m_Window = std::make_unique<Rove::Window>();
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
