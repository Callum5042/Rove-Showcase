#include "Pch.h"
#include "Application.h"
#include <wrl/wrappers/corewrappers.h>

int main(int argc, char** argv)
{
	// Detect memory leaks during debugging
#ifdef _DEBUG
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif

	// Load the COM library
#if (_WIN32_WINNT >= _WIN32_WINNT_WIN10)
#pragma comment(lib, "runtimeobject.lib")
	Microsoft::WRL::Wrappers::RoInitializeWrapper initialize(RO_INIT_MULTITHREADED);
#else
	HRESULT hr = CoInitializeEx(nullptr, COINIT_MULTITHREADED);
#endif

	try
	{
		auto application = std::make_unique<Rove::Application>();
		return application->Run();
	}
	catch (const std::exception& ex)
	{
		std::wstring error = Rove::ConvertToWideString(ex.what());
		MessageBox(NULL, error.c_str(), L"Error", MB_DEFAULT_DESKTOP_ONLY | MB_ICONERROR);
		return -1;
	}
	catch (...) 
	{
		MessageBox(NULL, L"Unknown error has occurred", L"Error", MB_DEFAULT_DESKTOP_ONLY | MB_ICONERROR);
		return -1;
	}
}
