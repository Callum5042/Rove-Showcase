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
	};

	// Rendering Model
	class ModelV2
	{
	public:
		ModelV2(DxRenderer* renderer, DxShader* shader);
		virtual ~ModelV2() = default;

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
		std::vector<ModelV2*> m_Models;
	};

	// Main model class
	class Model
	{
	public:
		Model(DxRenderer* renderer, DxShader* shader);
		virtual ~Model() = default;

		// Render the model
		void Render();

		// World 
		DirectX::XMMATRIX World = DirectX::XMMatrixIdentity();

		// Load model
		void LoadFromFile(const std::wstring& filepath);

		// Get vertices
		constexpr UINT GetVertices() { return m_VertexCount; }

		// Get indices
		constexpr UINT GetIndices() { return m_IndexCount; }

	private:
		DxRenderer* m_DxRenderer = nullptr;
		DxShader* m_DxShader = nullptr;

		// Number of indices to draw
		UINT m_IndexCount = 0;

		// Number of vertices
		UINT m_VertexCount = 0;

		// Vertex buffer
		ComPtr<ID3D11Buffer> m_VertexBuffer = nullptr;
		void CreateVertexBuffer(const std::vector<Vertex>& vertices);

		// Index buffer
		ComPtr<ID3D11Buffer> m_IndexBuffer = nullptr;
		void CreateIndexBuffer(const std::vector<UINT>& indices);

		// Local translation
		DirectX::XMMATRIX m_LocalWorld = DirectX::XMMatrixIdentity();
	};
}