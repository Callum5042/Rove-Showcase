#pragma once

#include "Pch.h"

namespace Rove
{
	// Forward declarations
	class DxRenderer;
	class DxShader;

	struct Colour
	{
		float r = 0;
		float g = 0;
		float b = 0;
		float a = 0;
	};

	struct Vertex
	{
		// Vertex position
		float x = 0;
		float y = 0;
		float z = 0;

		// Normals
		float normal_x = 0;
		float normal_y = 0;
		float normal_z = 0;

		// Texture UV's
		float texture_u = 0;
		float texture_v = 0;

		// Tangents
		float tangent_x = 0;
		float tangent_y = 0;
		float tangent_z = 0;
		float tangent_w = 0;
	};

	// Rendering Model
	class Model
	{
	public: 
		Model(DxRenderer* renderer, DxShader* shader);
		virtual ~Model() = default;

		void Render();

		// World transformation
		DirectX::XMMATRIX LocalWorld = DirectX::XMMatrixIdentity();

		// Number of indices to draw
		UINT m_IndexCount = 0;

		// Vertex buffer
		ComPtr<ID3D11Buffer> m_VertexBuffer = nullptr;
		void CreateVertexBuffer(const std::vector<Vertex>& vertices);

		// Index buffer
		ComPtr<ID3D11Buffer> m_IndexBuffer = nullptr;
		void CreateIndexBuffer(const std::vector<UINT>& indices);

		// Texture
		ComPtr<ID3D11ShaderResourceView> m_DiffuseTexture = nullptr;
		ComPtr<ID3D11ShaderResourceView> m_NormalTexture = nullptr;

	private:
		DxRenderer* m_DxRenderer = nullptr;
		DxShader* m_DxShader = nullptr;
	};

	// Object
	class Object
	{
	public:
		Object(DxRenderer* renderer, DxShader* shader);
		virtual ~Object() = default;

		// Loads a GLTF file
		void LoadFile(const std::string& path);

		// Renders the object
		void Render();

		// World 
		DirectX::XMMATRIX World = DirectX::XMMatrixIdentity();

	private:
		DxRenderer* m_DxRenderer = nullptr;
		DxShader* m_DxShader = nullptr;

		// Models
		std::vector<Model*> m_Models;
	};
}