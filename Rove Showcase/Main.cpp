
#include "Pch.h"
#include "Application.h"

namespace
{
	std::wstring ConvertToWideString(std::string str) 
	{
		int wchars_num = MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1, NULL, 0);

		std::wstring wide_str;
		wide_str.resize(wchars_num);

		MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1, wide_str.data(), wchars_num);
		return wide_str;
	}
}

int main(int argc, char** argv)
{
	// Detect memory leaks during debugging
#ifdef _DEBUG
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif

	try
	{
		auto application = std::make_unique<Rove::Application>();
		return application->Run();
	}
	catch (const std::exception& ex)
	{
		std::wstring error = ConvertToWideString(ex.what());
		MessageBox(NULL, error.c_str(), L"Error", MB_OK | MB_ICONERROR);
		return 0;
	}
	catch (...) 
	{
		MessageBox(NULL, L"Exception has occurred", L"Error", MB_OK | MB_ICONERROR);
		return 0;
	}
}
