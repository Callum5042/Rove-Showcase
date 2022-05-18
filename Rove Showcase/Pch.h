#pragma once

// Windows
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <windowsx.h>
#include <crtdbg.h>

// Used for Win32 file dialog
#include <shobjidl.h>

// DirectX
#include <d3d11_1.h>
#include <DirectXColors.h>
#include <DirectXMath.h>

// This include is requires for using DirectX smart pointers (ComPtr)
#include <wrl\client.h>
using Microsoft::WRL::ComPtr;

// Required to use RoInitializeWrapper to load the COM library
#include <wrl\wrappers\corewrappers.h>

// Dear ImGui
#include "ImGui\imgui.h"
#include "ImGui\imgui_impl_win32.h"
#include "ImGui\imgui_impl_dx11.h"
#include "ImGui\imgui_stdlib.h"
#include "ImGui\implot.h"

// C++
#include <string>
#include <memory>
#include <vector>
#include <fstream>
#include <filesystem>
#include <exception>
#include <thread>
#include <map>

#include <locale>
#include <codecvt>

#undef min
#undef max