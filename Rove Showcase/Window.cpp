#include "Pch.h"
#include "Window.h"
#include "Application.h"
#include "resource.h"

// Forward declare message handler from imgui_impl_win32.cpp
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

namespace
{
	static Rove::Window* GetWindow(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
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
		if (ImGui_ImplWin32_WndProcHandler(hwnd, uMsg, wParam, lParam))
			return true;

		// Get pointer
		Rove::Window* window = GetWindow(hwnd, uMsg, wParam, lParam);
		if (window != nullptr)
		{
			return window->HandleMessage(hwnd, uMsg, wParam, lParam);
		}
		else
		{
			return DefWindowProc(hwnd, uMsg, wParam, lParam);
		}
	}
}

Rove::Window::Window(Application* application) : m_Application(application)
{
}

Rove::Window::~Window() 
{
	DestroyWindow(m_Hwnd);
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
	wndClass.hIcon = static_cast<HICON>(LoadImage(hInstance, MAKEINTRESOURCE(IDI_APPICON), IMAGE_ICON, GetSystemMetrics(SM_CXICON), GetSystemMetrics(SM_CYICON), 0));
	wndClass.hIconSm = static_cast<HICON>(LoadImage(hInstance, MAKEINTRESOURCE(IDI_APPICON), IMAGE_ICON, GetSystemMetrics(SM_CXSMICON), GetSystemMetrics(SM_CYSMICON), 0));

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
}

void Rove::Window::Show()
{
	ShowWindow(m_Hwnd, SW_SHOWMAXIMIZED);
}

LRESULT Rove::Window::HandleMessage(HWND hwnd, INT uMsg, WPARAM wParam, LPARAM lParam)
{
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

	case WM_GETMINMAXINFO:
		reinterpret_cast<MINMAXINFO*>(lParam)->ptMinTrackSize.x = Rove::WINDOW_MIN_WIDTH;
		reinterpret_cast<MINMAXINFO*>(lParam)->ptMinTrackSize.y = Rove::WINDOW_MIN_HEIGHT;
		return 0;

	case WM_SIZE:
		WindowResizing(hwnd, uMsg, wParam, lParam);
		return 0;

	case WM_MOUSEWHEEL:
		MouseWheel(hwnd, uMsg, wParam, lParam);
		return 0;

	case WM_MOUSEMOVE:
		MouseMove(hwnd, uMsg, wParam, lParam);
		return 0;

	case WM_LBUTTONDOWN:
	case WM_RBUTTONDOWN:
	case WM_MBUTTONDOWN:
	case WM_XBUTTONDOWN:
		MouseDown(hwnd, uMsg, wParam, lParam);
		return 0;

	case WM_LBUTTONUP:
	case WM_RBUTTONUP:
	case WM_MBUTTONUP:
	case WM_XBUTTONUP:
		MouseUp(hwnd, uMsg, wParam, lParam);
		return 0;

	default:
		return DefWindowProc(hwnd, uMsg, wParam, lParam);
	}
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

void Rove::Window::SetTitle(const std::wstring& title)
{
	SetWindowText(m_Hwnd, title.c_str());
}

void Rove::Window::WindowResizing(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	// Don't call onresize if the window has not yet been created
	if (m_Hwnd == NULL)
		return;

	// Get window size
	int window_width = LOWORD(lParam);
	int window_height = HIWORD(lParam);

	// Don't resize on minimized
	if (wParam != SIZE_MINIMIZED)
	{
		GetApplication()->OnResize(window_width, window_height);
	}
}

void Rove::Window::MouseWheel(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	// https://docs.microsoft.com/en-us/windows/win32/inputdev/wm-mousewheel
	int scroll_delta = GET_WHEEL_DELTA_WPARAM(wParam) / WHEEL_DELTA;
	//int key_modifier = GET_KEYSTATE_WPARAM(wParam);

	//int mouse_x = GET_X_LPARAM(lParam);
	//int mouse_y = GET_Y_LPARAM(lParam);

	m_Application->OnMouseWheel(scroll_delta);
}

void Rove::Window::MouseMove(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	// https://docs.microsoft.com/en-us/windows/win32/inputdev/wm-mousemove
	int mouse_x = GET_X_LPARAM(lParam);
	int mouse_y = GET_Y_LPARAM(lParam);

	m_Application->OnMouseMove(mouse_x, mouse_y, static_cast<int>(wParam));
}

void Rove::Window::MouseDown(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	// https://docs.microsoft.com/en-us/windows/win32/inputdev/wm-lbuttondown
	int mouse_x = GET_X_LPARAM(lParam);
	int mouse_y = GET_Y_LPARAM(lParam);

	m_Application->OnMousePressed(mouse_x, mouse_y, static_cast<int>(wParam));
}

void Rove::Window::MouseUp(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	// https://docs.microsoft.com/en-us/windows/win32/inputdev/wm-lbuttondown
	int mouse_x = GET_X_LPARAM(lParam);
	int mouse_y = GET_Y_LPARAM(lParam);

	m_Application->OnMouseReleased(mouse_x, mouse_y, static_cast<int>(wParam));
}

void Rove::ShowMessage(const std::wstring& text)
{
	MessageBox(NULL, text.c_str(), L"Info", MB_OK | MB_ICONINFORMATION);
}
