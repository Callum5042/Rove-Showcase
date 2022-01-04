#pragma once

// Windows
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <crtdbg.h>

// DirectX
#include <d3d11_1.h>
#include <DirectXColors.h>

// This include is requires for using DirectX smart pointers (ComPtr)
#include <wrl\client.h>
using Microsoft::WRL::ComPtr;

// C++
#include <string>
#include <memory>

#include <locale>
#include <codecvt>