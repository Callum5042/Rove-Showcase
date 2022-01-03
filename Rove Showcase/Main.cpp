
#include "Pch.h"
#include "Application.h"

namespace
{
	std::wstring&& ConvertToWideString(std::string str) 
	{
		int wchars_num = MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1, NULL, 0);

		wchar_t* wstr = new wchar_t[wchars_num];
		MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1, wstr, wchars_num);

		std::wstring wide_str(wstr);
		delete[] wstr;

		return std::move(wide_str);
	}
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	try
	{
		auto application = std::make_unique<Rove::Application>();
		return application->Run(hInstance, nCmdShow);
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
