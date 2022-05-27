#include "Pch.h"
#include "Model.h"
#include "DxRenderer.h"
#include "DxShader.h"
#include "Application.h"
#include "simdjson\simdjson.h"
#include "TextureLoader\DDSTextureLoader.h"
#include "TextureLoader\WICTextureLoader.h"

namespace
{
	enum class AccessorDataType
	{
		SIGNED_BYTE = 5120,
		UNSIGNED_BYTE = 5121,
		SIGNED_SHORT = 5122,
		UNSIGNED_SHORT = 5123,
		UNSIGNED_INT = 5125,
		FLOAT = 5126
	};

	template <typename TDataType>
	struct Vec4
	{
		TDataType x;
		TDataType y;
		TDataType z;
		TDataType w;
	};

	template <typename TDataType>
	struct Vec3
	{
		TDataType x;
		TDataType y;
		TDataType z;
	};

	template <typename TDataType>
	struct Vec2
	{
		TDataType x;
		TDataType y;
	};

	DirectX::XMMATRIX MeshTranslation(simdjson::dom::element node)
	{
		auto translation = node["translation"].get_array();
		if (translation.error() == simdjson::SUCCESS)
		{
			float x = static_cast<float>(translation.at(0).get_double().value());
			float y = static_cast<float>(translation.at(1).get_double().value());
			float z = static_cast<float>(translation.at(2).get_double().value());
			return DirectX::XMMatrixTranslation(x, y, z);
		}

		return DirectX::XMMatrixIdentity();
	}

	DirectX::XMMATRIX MeshRotation(simdjson::dom::element node)
	{
		auto rotation = node["rotation"].get_array();
		if (rotation.error() == simdjson::SUCCESS)
		{
			float x = static_cast<float>(rotation.at(0).get_double().value());
			float y = static_cast<float>(rotation.at(1).get_double().value());
			float z = static_cast<float>(rotation.at(2).get_double().value());
			float w = static_cast<float>(rotation.at(3).get_double().value());
			return DirectX::XMMatrixRotationQuaternion(DirectX::XMVectorSet(x, y, z, w));
		}

		return DirectX::XMMatrixIdentity();
	}

	template <typename TDataType>
	void LoadMeshIndices(const simdjson::simdjson_result<simdjson::dom::element>& json, const simdjson::dom::element& node, std::vector<TDataType>& data)
	{
		auto indicesIndex = node["indices"].get_int64().value();

		// Accessor
		auto indexAccessor = json["accessors"].at(indicesIndex);
		auto bufferViewIndex = indexAccessor["bufferView"].get_int64().value();
		AccessorDataType componentType = static_cast<AccessorDataType>(indexAccessor["componentType"].get_int64().value());
		auto indexCount = indexAccessor["count"].get_int64().value();

		// View
		auto viewBuffer = json["bufferViews"].at(bufferViewIndex);
		auto bufferIndex = viewBuffer["buffer"].get_int64().value();
		auto byteLength = viewBuffer["byteLength"].get_int64().value();
		auto byteOffset = viewBuffer["byteOffset"].get_int64().value();

		// Buffer
		auto buffer = json["buffers"].at(bufferIndex);
		auto bufferByteLength = buffer["byteLength"].get_int64().value();
		std::string_view bufferUri = buffer["uri"].get_string();

		// Load buffer
		std::string basePath = "C:\\Users\\Callum\\Desktop\\" + std::string(bufferUri);

		std::ifstream file(basePath, std::fstream::in | std::fstream::binary);
		file.seekg(byteOffset);

		data.resize(indexCount);
		file.read(reinterpret_cast<char*>(data.data()), byteLength);
	}

	template <typename TDataType>
	void LoadMeshVerticesAttribute(const simdjson::simdjson_result<simdjson::dom::element>& json, simdjson::simdjson_result<int64_t> attribute, std::vector<TDataType>& data)
	{
		// Accessor
		auto indexAccessor = json["accessors"].at(attribute.value());
		auto bufferViewIndex = indexAccessor["bufferView"].get_int64().value();
		AccessorDataType componentType = static_cast<AccessorDataType>(indexAccessor["componentType"].get_int64().value());
		int64_t vertexCount = indexAccessor["count"].get_int64().value();

		// View
		auto viewBuffer = json["bufferViews"].at(bufferViewIndex);
		auto bufferIndex = viewBuffer["buffer"].get_int64().value();
		auto byteLength = viewBuffer["byteLength"].get_int64().value();
		auto byteOffset = viewBuffer["byteOffset"].get_int64().value();

		// Buffer
		auto buffer = json["buffers"].at(bufferIndex);
		auto bufferByteLength = buffer["byteLength"].get_int64().value();
		std::string_view bufferUri = buffer["uri"].get_string();

		// Load buffer
		std::string basePath = "C:\\Users\\Callum\\Desktop\\" + std::string(bufferUri);

		std::ifstream file(basePath, std::fstream::in | std::fstream::binary);
		file.seekg(byteOffset);

		data.resize(vertexCount);
		file.read(reinterpret_cast<char*>(data.data()), byteLength);
	}
}

Rove::Object::Object(DxRenderer* renderer, DxShader* shader) : m_DxRenderer(renderer), m_DxShader(shader)
{
}

void Rove::Object::LoadFile(const std::string& path)
{
	// Clear old data
	m_Models.clear();

	// Load file
	simdjson::dom::parser parser;
	simdjson::simdjson_result<simdjson::dom::element> json = parser.load(path);

	// Nodes
	for (auto node : json["nodes"])
	{
		auto meshIndex = node["mesh"].get_int64();

		if (meshIndex.error() == simdjson::SUCCESS)
		{
			DirectX::XMMATRIX LocalWorld = DirectX::XMMatrixIdentity();
			std::vector<Vertex> vertices;
			std::vector<UINT> indices;
			Model* model = new Model(m_DxRenderer, m_DxShader);

			// Apply translation
			LocalWorld *= MeshTranslation(node);

			// Apply rotation
			LocalWorld *= MeshRotation(node);

			// Load
			auto jsonMesh = json["meshes"].at(meshIndex.value());
			for (auto jsonPrimitive : jsonMesh["primitives"])
			{
				// Load indices
				{
					std::vector<short> data;
					LoadMeshIndices<short>(json, jsonPrimitive, data);
					indices.resize(data.size());
					indices.assign(data.begin(), data.end());
				}

				// Load vertices
				simdjson::simdjson_result<int64_t> position_attribute = jsonPrimitive["attributes"]["POSITION"].get_int64();
				if (position_attribute.error() == simdjson::SUCCESS)
				{
					std::vector<Vec3<float>> data;
					LoadMeshVerticesAttribute(json, position_attribute, data);

					for (auto& v : data)
					{
						Vertex v1;
						v1.x = v.x;
						v1.y = v.y;
						v1.z = v.z;

						vertices.push_back(v1);
					}
				}

				// Apply normals
				simdjson::simdjson_result<int64_t> normal_attribute = jsonPrimitive["attributes"]["NORMAL"].get_int64();
				if (normal_attribute.error() == simdjson::SUCCESS)
				{
					std::vector<Vec3<float>> data;
					LoadMeshVerticesAttribute(json, normal_attribute, data);

					for (size_t i = 0; i < data.size(); ++i)
					{
						vertices[i].normal_x = data[i].x;
						vertices[i].normal_y = data[i].y;
						vertices[i].normal_z = data[i].z;
					}
				}

				// Apply tangent
				simdjson::simdjson_result<int64_t> tangent_attribute = jsonPrimitive["attributes"]["TANGENT"].get_int64();
				if (tangent_attribute.error() == simdjson::SUCCESS)
				{
					std::vector<Vec4<float>> data;
					LoadMeshVerticesAttribute<Vec4<float>>(json, tangent_attribute, data);

					for (size_t i = 0; i < data.size(); ++i)
					{
						vertices[i].tangent_x = data[i].x;
						vertices[i].tangent_y = data[i].y;
						vertices[i].tangent_z = data[i].z;
					}
				}

				// Apply texture coords
				simdjson::simdjson_result<int64_t> texcoord0_attribute = jsonPrimitive["attributes"]["TEXCOORD_0"].get_int64();
				if (texcoord0_attribute.error() == simdjson::SUCCESS)
				{
					std::vector<Vec2<float>> data;
					LoadMeshVerticesAttribute<Vec2<float>>(json, texcoord0_attribute, data);

					for (size_t i = 0; i < data.size(); ++i)
					{
						vertices[i].texture_u = data[i].x;
						vertices[i].texture_v = data[i].y;
					}
				}

				// Apply material
				HRESULT hr = CoInitializeEx(nullptr, COINIT_MULTITHREADED);

				simdjson::simdjson_result<int64_t> material_index = jsonPrimitive["material"].get_int64();
				auto material = json["materials"].at(material_index.value());

				auto name = material["name"].get_string();

				// Diffuse texture
				{
					simdjson::simdjson_result<int64_t> diffuse_texture_index = material["pbrMetallicRoughness"]["baseColorTexture"]["index"].get_int64();
					simdjson::simdjson_result<int64_t> diffuse_image_index = json["textures"].at(diffuse_texture_index.value())["source"].get_int64();
					auto diffuse_image = json["images"].at(diffuse_image_index.value());
					std::string_view uri = diffuse_image["uri"].get_string().value();

					ComPtr<ID3D11Resource> resource = nullptr;

					std::string directory = path.substr(0, path.find_last_of('\\'));
					std::string terxture_path = directory + "\\" + std::string(uri);
					DX::Check(DirectX::CreateWICTextureFromFile(m_DxRenderer->GetDevice(), ConvertToWideString(terxture_path).c_str(), resource.ReleaseAndGetAddressOf(), model->m_DiffuseTexture.ReleaseAndGetAddressOf()));
				}

				// Normal texture
				{
					simdjson::simdjson_result<int64_t> normal_texture_index = material["normalTexture"]["index"].get_int64();
					simdjson::simdjson_result<int64_t> normal_image_index = json["textures"].at(normal_texture_index.value())["source"].get_int64();
					auto diffuse_image = json["images"].at(normal_image_index.value());
					std::string_view uri = diffuse_image["uri"].get_string().value();

					ComPtr<ID3D11Resource> resource = nullptr;

					std::string directory = path.substr(0, path.find_last_of('\\'));
					std::string terxture_path = directory + "\\" + std::string(uri);
					DX::Check(DirectX::CreateWICTextureFromFile(m_DxRenderer->GetDevice(), ConvertToWideString(terxture_path).c_str(), resource.ReleaseAndGetAddressOf(), model->m_NormalTexture.ReleaseAndGetAddressOf()));
				}

				CoUninitialize();
			}

			// Build model
			model->LocalWorld = LocalWorld;
			model->CreateVertexBuffer(vertices);
			model->CreateIndexBuffer(indices);
			m_Models.push_back(model);


			// Read material



			// Load texture
			/*HRESULT hr = CoInitializeEx(nullptr, COINIT_MULTITHREADED);

			ComPtr<ID3D11Resource> resource = nullptr;
			DX::Check(DirectX::CreateWICTextureFromFile(m_DxRenderer->GetDevice(), L"C:\\Users\\Callum\\Desktop\\crate_diffuse.png", resource.ReleaseAndGetAddressOf(), model->m_DiffuseTexture.ReleaseAndGetAddressOf()));
			DX::Check(DirectX::CreateWICTextureFromFile(m_DxRenderer->GetDevice(), L"C:\\Users\\Callum\\Desktop\\crate_normal.png", resource.ReleaseAndGetAddressOf(), model->m_NormalTexture.ReleaseAndGetAddressOf()));

			CoUninitialize();*/
		}
	}
}

void Rove::Object::Render()
{
	for (auto& model : m_Models)
	{
		model->Render();
	}
}

Rove::Model::Model(DxRenderer* renderer, DxShader* shader) : m_DxRenderer(renderer), m_DxShader(shader)
{
}

void Rove::Model::Render()
{
	auto d3dDeviceContext = m_DxRenderer->GetDeviceContext();

	// We need the stride and offset for the vertex
	UINT vertex_stride = sizeof(Vertex);
	auto vertex_offset = 0u;

	// Bind the vertex buffer to the pipeline's Input Assembler stage
	d3dDeviceContext->IASetVertexBuffers(0, 1, m_VertexBuffer.GetAddressOf(), &vertex_stride, &vertex_offset);

	// Bind the index buffer to the pipeline's Input Assembler stage
	d3dDeviceContext->IASetIndexBuffer(m_IndexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);

	// Bind the geometry topology to the pipeline's Input Assembler stage
	d3dDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	// Bind texture to the pixel shader
	m_DxRenderer->GetDeviceContext()->PSSetShaderResources(0, 1, m_DiffuseTexture.GetAddressOf());
	m_DxRenderer->GetDeviceContext()->PSSetShaderResources(1, 1, m_NormalTexture.GetAddressOf());

	// Apply local transformations
	Rove::LocalWorldBuffer world_buffer = {};
	world_buffer.world = DirectX::XMMatrixTranspose(LocalWorld);
	m_DxShader->UpdateLocalWorldConstantBuffer(world_buffer);

	// Render geometry
	d3dDeviceContext->DrawIndexed(m_IndexCount, 0, 0);
}

void Rove::Model::CreateVertexBuffer(const std::vector<Vertex>& vertices)
{
	auto d3dDevice = m_DxRenderer->GetDevice();

	// Create vertex buffer
	D3D11_BUFFER_DESC vertex_buffer_desc = {};
	vertex_buffer_desc.Usage = D3D11_USAGE_DEFAULT;
	vertex_buffer_desc.ByteWidth = static_cast<UINT>(sizeof(Vertex) * vertices.size());
	vertex_buffer_desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;

	D3D11_SUBRESOURCE_DATA vertex_subdata = {};
	vertex_subdata.pSysMem = vertices.data();

	DX::Check(d3dDevice->CreateBuffer(&vertex_buffer_desc, &vertex_subdata, m_VertexBuffer.ReleaseAndGetAddressOf()));
}

void Rove::Model::CreateIndexBuffer(const std::vector<UINT>& indices)
{
	auto d3dDevice = m_DxRenderer->GetDevice();

	m_IndexCount = static_cast<UINT>(indices.size());

	// Create index buffer
	D3D11_BUFFER_DESC index_buffer_desc = {};
	index_buffer_desc.Usage = D3D11_USAGE_DEFAULT;
	index_buffer_desc.ByteWidth = static_cast<UINT>(sizeof(UINT) * indices.size());
	index_buffer_desc.BindFlags = D3D11_BIND_INDEX_BUFFER;

	D3D11_SUBRESOURCE_DATA index_subdata = {};
	index_subdata.pSysMem = indices.data();

	DX::Check(d3dDevice->CreateBuffer(&index_buffer_desc, &index_subdata, m_IndexBuffer.ReleaseAndGetAddressOf()));
}
