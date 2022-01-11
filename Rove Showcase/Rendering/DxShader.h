#pragma once

#include "Pch.h"

namespace Rove
{
	struct CameraBuffer
	{
		DirectX::XMMATRIX view;
		DirectX::XMMATRIX projection;
		DirectX::XMFLOAT3 cameraPosition;
		float padding;
	};

	struct WorldBuffer
	{
		DirectX::XMMATRIX world;
		DirectX::XMMATRIX worldInverse;
	};

	struct PointLightBuffer
	{
		DirectX::XMFLOAT3 position;
		float padding;

		DirectX::XMFLOAT4 diffuse;
		DirectX::XMFLOAT4 ambient;
		DirectX::XMFLOAT4 specular;
	};

	class DxRenderer;

	class DxShader
	{
	public:
		DxShader(DxRenderer* renderer);
		virtual ~DxShader() = default;

		void Load();
		void Apply();

		// Update camera buffer
		void UpdateCameraBuffer(const CameraBuffer& buffer);

		// Set world constant buffer from camera
		void UpdateWorldConstantBuffer(const WorldBuffer& worldBuffer);

		// Update camera buffer
		void UpdatePointLightBuffer(const PointLightBuffer& buffer);
		
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


		// Camera constant buffer
		ComPtr<ID3D11Buffer> m_CameraConstantBuffer = nullptr;
		void CreateCameraConstantBuffer();

		// World constant buffer
		ComPtr<ID3D11Buffer> m_WorldConstantBuffer = nullptr;
		void CreateWorldConstantBuffer();

		// Point light constant buffer
		ComPtr<ID3D11Buffer> m_PointLightConstantBuffer = nullptr;
		void CreatePointLightConstantBuffer();
	};
}