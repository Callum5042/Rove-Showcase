#pragma once

#include "Pch.h"

namespace Rove
{
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

	struct MeshDetails
	{
		UINT indices_count = 0;
		UINT indices_start = 0;
		int vertex_start = 0;
	};

	class DxRenderer;

	class Model
	{
	public:
		Model(DxRenderer* renderer);
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

		// Number of indices to draw
		UINT m_IndexCount = 0;

		// Number of vertices
		UINT m_VertexCount = 0;

		// Details on how to draw the vertices/indices
		std::vector<MeshDetails> m_MeshDetails;

		// Vertex buffer
		ComPtr<ID3D11Buffer> m_VertexBuffer = nullptr;
		void CreateVertexBuffer(const std::vector<Vertex>& vertices);

		// Index buffer
		ComPtr<ID3D11Buffer> m_IndexBuffer = nullptr;
		void CreateIndexBuffer(const std::vector<UINT>& indices);
	};
}