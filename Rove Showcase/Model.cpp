#include "Pch.h"
#include "Model.h"
#include "DxRenderer.h"
#include "DxShader.h"
#include "Application.h"
#include "simdjson\simdjson.h"

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
struct Vec3
{
	TDataType x;
	TDataType y;
	TDataType z;
};

Rove::Object::Object(DxRenderer* renderer, DxShader* shader) : m_DxRenderer(renderer), m_DxShader(shader)
{
}

void Rove::Object::LoadFile(const std::string& path)
{
	// Clear old data
	m_Models.clear();

	// Load file
	simdjson::dom::parser parser;
	auto json = parser.load(path);

	// Nodes
	for (auto node : json["nodes"])
	{
		auto meshIndex = node["mesh"].get_int64();

		if (meshIndex.error() == simdjson::SUCCESS)
		{
			DirectX::XMMATRIX LocalWorld = DirectX::XMMatrixIdentity();
			std::vector<Vertex> vertices;
			std::vector<UINT> indices;
			ModelV2* model = new ModelV2(m_DxRenderer, m_DxShader);

			// Apply translation
			auto translation = node["translation"].get_array();
			if (translation.error() == simdjson::SUCCESS)
			{
				float x = static_cast<float>(translation.at(0).get_double().value());
				float y = static_cast<float>(translation.at(1).get_double().value());
				float z = static_cast<float>(translation.at(2).get_double().value());
				LocalWorld *= DirectX::XMMatrixTranslation(x, y, z);
			}

			auto rotation = node["rotation"].get_array();
			if (rotation.error() == simdjson::SUCCESS)
			{
				float x = static_cast<float>(rotation.at(0).get_double().value());
				float y = static_cast<float>(rotation.at(1).get_double().value());
				float z = static_cast<float>(rotation.at(2).get_double().value());
				float w = static_cast<float>(rotation.at(3).get_double().value());
				LocalWorld *= DirectX::XMMatrixRotationQuaternion(DirectX::XMVectorSet(x, y, z, w));
			}

			// Load
			auto jsonMesh = json["meshes"].at(meshIndex.value());
			for (auto jsonPrimitive : jsonMesh["primitives"])
			{
				// Load indices
				{
					auto indicesIndex = jsonPrimitive["indices"].get_int64().value();

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

					std::vector<short> _indices(indexCount);
					file.read(reinterpret_cast<char*>(_indices.data()), byteLength);

					indices.resize(indexCount);
					indices.assign(_indices.begin(), _indices.end());
				}

				// Load vertices
				{
					auto positionIndex = jsonPrimitive["attributes"]["POSITION"].get_int64().value();

					// Accessor
					auto indexAccessor = json["accessors"].at(positionIndex);
					auto bufferViewIndex = indexAccessor["bufferView"].get_int64().value();
					AccessorDataType componentType = static_cast<AccessorDataType>(indexAccessor["componentType"].get_int64().value());
					auto vertexCount = indexAccessor["count"].get_int64().value();

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

					std::vector<Vec3<float>> _vertices(vertexCount);
					file.read(reinterpret_cast<char*>(_vertices.data()), byteLength);

					for (auto& v : _vertices)
					{
						Vertex v1;
						v1.x = v.x;
						v1.y = v.y;
						v1.z = v.z;

						vertices.push_back(v1);
					}
				}

				// Apply normals
				{
					auto positionIndex = jsonPrimitive["attributes"]["NORMAL"].get_int64().value();

					// Accessor
					auto indexAccessor = json["accessors"].at(positionIndex);
					auto bufferViewIndex = indexAccessor["bufferView"].get_int64().value();
					AccessorDataType componentType = static_cast<AccessorDataType>(indexAccessor["componentType"].get_int64().value());
					auto vertexCount = indexAccessor["count"].get_int64().value();

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

					std::vector<Vec3<float>> _vertices(vertexCount);
					file.read(reinterpret_cast<char*>(_vertices.data()), byteLength);

					for (size_t i = 0; i < _vertices.size(); ++i)
					{
						vertices[i].normal_x = _vertices[i].x;
						vertices[i].normal_y = _vertices[i].y;
						vertices[i].normal_z = _vertices[i].z;
					}
				}
			}

			// Build model
			model->LocalWorld = LocalWorld;
			model->CreateVertexBuffer(vertices);
			model->CreateIndexBuffer(indices);
			m_Models.push_back(model);
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

Rove::ModelV2::ModelV2(DxRenderer* renderer, DxShader* shader) : m_DxRenderer(renderer), m_DxShader(shader)
{
}

void Rove::ModelV2::Render()
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

	// Apply local transformations
	Rove::LocalWorldBuffer world_buffer = {};
	world_buffer.world = DirectX::XMMatrixTranspose(LocalWorld);
	m_DxShader->UpdateLocalWorldConstantBuffer(world_buffer);

	// Render geometry
	d3dDeviceContext->DrawIndexed(m_IndexCount, 0, 0);
}

void Rove::ModelV2::CreateVertexBuffer(const std::vector<Vertex>& vertices)
{
	auto d3dDevice = m_DxRenderer->GetDevice();

	// m_VertexCount = static_cast<UINT>(vertices.size());

	// Create vertex buffer
	D3D11_BUFFER_DESC vertex_buffer_desc = {};
	vertex_buffer_desc.Usage = D3D11_USAGE_DEFAULT;
	vertex_buffer_desc.ByteWidth = static_cast<UINT>(sizeof(Vertex) * vertices.size());
	vertex_buffer_desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;

	D3D11_SUBRESOURCE_DATA vertex_subdata = {};
	vertex_subdata.pSysMem = vertices.data();

	DX::Check(d3dDevice->CreateBuffer(&vertex_buffer_desc, &vertex_subdata, m_VertexBuffer.ReleaseAndGetAddressOf()));
}

void Rove::ModelV2::CreateIndexBuffer(const std::vector<UINT>& indices)
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
