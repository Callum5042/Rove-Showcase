#pragma once

#include "Pch.h"

namespace Rove
{
	class PointLight
	{
	public:
		PointLight() = default;
		virtual ~PointLight() = default;

		// Camera position
		DirectX::XMFLOAT3 Position = DirectX::XMFLOAT3(0.0f, 0.0f, 0.0f);

		// Diffuse light
		DirectX::XMFLOAT4 DiffuseColour = DirectX::XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);

		// Ambient light
		DirectX::XMFLOAT4 AmbientColour = DirectX::XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);

		// Specular light
		DirectX::XMFLOAT4 SpecularColour = DirectX::XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);

	private:

	};
}