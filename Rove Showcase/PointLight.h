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



	private:
	};
}