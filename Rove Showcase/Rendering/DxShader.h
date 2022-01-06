#pragma once

#include "Pch.h"

namespace Rove
{
	class DxRenderer;

	class DxShader
	{
	public:
		DxShader(DxRenderer* renderer);
		virtual ~DxShader() = default;

		void Load();
		void Apply();
		
	private:
		DxRenderer* m_DxRenderer = nullptr;

		// Vertex shader
		ComPtr<ID3D11VertexShader> m_VertexShader = nullptr;
		void LoadVertexShader(std::string&& vertex_shader_path);

		// Vertex shader input layout
		ComPtr<ID3D11InputLayout> m_VertexLayout = nullptr;
		void LoadPixelShader(std::string&& pixel_shader_path);

		// Pixel shader
		ComPtr<ID3D11PixelShader> m_PixelShader = nullptr;

		// World constant buffer
		ComPtr<ID3D11Buffer> m_WorldConstantBuffer = nullptr;
		void CreateWorldConstantBuffer();
	};
}