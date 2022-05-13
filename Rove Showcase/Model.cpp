#include "Pch.h"
#include "Model.h"
#include "Rendering/DxRenderer.h"
#include "Application.h"
#include "simdjson\simdjson.h"

Rove::Model::Model(DxRenderer* renderer) : m_DxRenderer(renderer)
{
	World *= DirectX::XMMatrixTranslation(0.0f, 0.0f, 0.0f);
}

void Rove::Model::CreateVertexBuffer(const std::vector<Vertex>& vertices)
{
	auto d3dDevice = m_DxRenderer->GetDevice();

	m_VertexCount = static_cast<UINT>(vertices.size());

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

	// Render geometry
	d3dDeviceContext->DrawIndexed(m_IndexCount, 0, 0);
	/*for (auto& mesh_details : m_MeshDetails)
	{
	}*/
}

enum class AccessorDataType
{
	SIGNED_BYTE = 5120,
	UNSIGNED_BYTE = 5121,
	SIGNED_SHORT = 5122,
	UNSIGNED_SHORT = 5123,
	UNSIGNED_INT = 5125,
	FLOAT = 5126
};

struct Vec3
{
	float x;
	float y;
	float z;
};

void Rove::Model::LoadFromFile(const std::wstring& filepath)
{
	std::string path = ConvertToString(filepath);

	simdjson::dom::parser parser;
	auto json = parser.load(path);

	std::vector<Vertex> vertices;
	std::vector<UINT> indices;

	// Nodes
	for (auto node : json["nodes"])
	{
		auto meshIndex = node["mesh"].get_int64();

		if (meshIndex.error() == simdjson::SUCCESS)
		{
			// Apply translation
			auto translation = node["translation"].get_array();


			// Load
			auto jsonMesh = json["meshes"].at(meshIndex.value());
			for (auto jsonPrimitive : jsonMesh["primitives"])
			{
				// Load indices
				{
					int indicesIndex = jsonPrimitive["indices"].get_int64();

					// Accessor
					auto indexAccessor = json["accessors"].at(indicesIndex);
					int bufferViewIndex = indexAccessor["bufferView"].get_int64();
					AccessorDataType componentType = static_cast<AccessorDataType>(indexAccessor["componentType"].get_int64().value());
					int indexCount = indexAccessor["count"].get_int64();

					// View
					auto viewBuffer = json["bufferViews"].at(bufferViewIndex);
					int bufferIndex = viewBuffer["buffer"].get_int64();
					int byteLength = viewBuffer["byteLength"].get_int64();
					int byteOffset = viewBuffer["byteOffset"].get_int64();

					// Buffer
					auto buffer = json["buffers"].at(bufferIndex);
					int bufferByteLength = buffer["byteLength"].get_int64();
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
					int positionIndex = jsonPrimitive["attributes"]["POSITION"].get_int64();

					// Accessor
					auto indexAccessor = json["accessors"].at(positionIndex);
					int bufferViewIndex = indexAccessor["bufferView"].get_int64();
					AccessorDataType componentType = static_cast<AccessorDataType>(indexAccessor["componentType"].get_int64().value());
					int vertexCount = indexAccessor["count"].get_int64();

					// View
					auto viewBuffer = json["bufferViews"].at(bufferViewIndex);
					int bufferIndex = viewBuffer["buffer"].get_int64();
					int byteLength = viewBuffer["byteLength"].get_int64();
					int byteOffset = viewBuffer["byteOffset"].get_int64();

					// Buffer
					auto buffer = json["buffers"].at(bufferIndex);
					int bufferByteLength = buffer["byteLength"].get_int64();
					std::string_view bufferUri = buffer["uri"].get_string();

					// Load buffer
					std::string basePath = "C:\\Users\\Callum\\Desktop\\" + std::string(bufferUri);

					std::ifstream file(basePath, std::fstream::in | std::fstream::binary);
					file.seekg(byteOffset);

					std::vector<Vec3> _vertices(vertexCount);
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
					int positionIndex = jsonPrimitive["attributes"]["NORMAL"].get_int64();

					// Accessor
					auto indexAccessor = json["accessors"].at(positionIndex);
					int bufferViewIndex = indexAccessor["bufferView"].get_int64();
					AccessorDataType componentType = static_cast<AccessorDataType>(indexAccessor["componentType"].get_int64().value());
					int vertexCount = indexAccessor["count"].get_int64();

					// View
					auto viewBuffer = json["bufferViews"].at(bufferViewIndex);
					int bufferIndex = viewBuffer["buffer"].get_int64();
					int byteLength = viewBuffer["byteLength"].get_int64();
					int byteOffset = viewBuffer["byteOffset"].get_int64();

					// Buffer
					auto buffer = json["buffers"].at(bufferIndex);
					int bufferByteLength = buffer["byteLength"].get_int64();
					std::string_view bufferUri = buffer["uri"].get_string();

					// Load buffer
					std::string basePath = "C:\\Users\\Callum\\Desktop\\" + std::string(bufferUri);

					std::ifstream file(basePath, std::fstream::in | std::fstream::binary);
					file.seekg(byteOffset);

					std::vector<Vec3> _vertices(vertexCount);
					file.read(reinterpret_cast<char*>(_vertices.data()), byteLength);

					for (size_t i = 0; i < _vertices.size(); ++i)
					{
						vertices[i].normal_x = _vertices[i].x;
						vertices[i].normal_y = _vertices[i].y;
						vertices[i].normal_z = _vertices[i].z;
					}
				}
			}
		}
	}

	// Rebuild Direct3D11 buffers
	CreateVertexBuffer(vertices);
	CreateIndexBuffer(indices);
}
