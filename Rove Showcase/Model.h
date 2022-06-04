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
	};

	// Material
	struct Material
	{
		bool diffuse_texture = false;
		bool normal_texture = false;
		float metallicFactor = 0.0f;
		float roughnessFactor = 0.5f;
	};

	// Rendering Model
	class Model
	{
		DxRenderer* m_DxRenderer = nullptr;
		DxShader* m_DxShader = nullptr;

	public: 
		Model(DxRenderer* renderer, DxShader* shader);
		virtual ~Model() = default;

		void Render(const DirectX::XMFLOAT3& position, const DirectX::XMFLOAT3& rotation, const DirectX::XMFLOAT3& scale);

		// World transformation
		DirectX::XMMATRIX World = DirectX::XMMatrixIdentity();

		// Model name
		std::string Name;

		// Material
		Material Material;

		// Number of indices to draw
		UINT m_IndexCount = 0;

		// Vertex buffer
		ComPtr<ID3D11Buffer> m_VertexBuffer = nullptr;
		void CreateVertexBuffer(const std::vector<Vertex>& vertices);

		// Index buffer
		ComPtr<ID3D11Buffer> m_IndexBuffer = nullptr;
		void CreateIndexBuffer(void* indices, UINT count, int64_t size, DXGI_FORMAT format);

		// Texture
		ComPtr<ID3D11ShaderResourceView> m_DiffuseTexture = nullptr;
		ComPtr<ID3D11ShaderResourceView> m_NormalTexture = nullptr;

	private:
		DXGI_FORMAT m_IndexBufferFormat;
	};

	// Object
	class Object
	{
		DxRenderer* m_DxRenderer = nullptr;
		DxShader* m_DxShader = nullptr;

	public:
		Object(DxRenderer* renderer, DxShader* shader);
		virtual ~Object() = default;

		// Loads a GLTF file
		void LoadFile(const std::filesystem::path& path);

		// Renders the object
		void Render();

		// World 
		DirectX::XMFLOAT3 Position;
		DirectX::XMFLOAT3 Rotation;
		DirectX::XMFLOAT3 Scale = DirectX::XMFLOAT3(1.0f, 1.0f, 1.0f);

		// Materials
		std::vector<Material*> GetMaterials();

		// Models
		const std::vector<std::unique_ptr<Model>>& GetModels() { return m_Models; }

		// Object name
		std::string Filename;


	private:
		// Models
		std::vector<std::unique_ptr<Model>> m_Models;
	};
}