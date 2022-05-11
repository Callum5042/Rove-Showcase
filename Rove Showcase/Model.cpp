#include "Pch.h"
#include "Model.h"
#include "Rendering/DxRenderer.h"
#include "Application.h"

// TinyGltf
#define TINYGLTF_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "TinyGltf\tiny_gltf.h"

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

struct BasicVertex
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

	// Load
	for (auto jsonMesh : json["meshes"])
	{
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

				std::vector<BasicVertex> _vertices(vertexCount);
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
		}
	}



	////////////////////////
	/// Old ////////////////
	////////////////////////




	// https://www.khronos.org/registry/glTF/specs/2.0/glTF-2.0.html#meshes-overview

	// TODO: Make the transformation matrix apply
	// Might have to split the mesh into a mesh class instead of 1 

	//tinygltf::Model model;
	//tinygltf::TinyGLTF loader;
	//std::string err;
	//std::string warn;

	//// Convert wstring to string
	//std::string path = Rove::ConvertToString(filepath);

	//// Load file
	//if (!loader.LoadBinaryFromFile(&model, &err, &warn, path))
	//{
	//	throw std::exception(err.c_str());
	//}

	//// Parse file
	//std::string name;
	//std::vector<Vertex> vertices;
	//std::vector<UINT> indices;

	//// Constants
	//constexpr int MODE_TRIANGLES = TINYGLTF_MODE_TRIANGLES;

	//// Parse file
	//for (const tinygltf::Node& node : model.nodes)
	//{
	//	if (node.mesh != -1)
	//	{
	//		const tinygltf::Mesh& mesh = model.meshes[node.mesh];

	//		// Transformations


	//		// Primitives
	//		for (const tinygltf::Primitive& primitive : mesh.primitives)
	//		{
	//			// Vertices
	//			const tinygltf::Accessor& vertex_accessor = model.accessors[primitive.attributes.at("POSITION")];
	//			const tinygltf::BufferView& vertex_buffer_view = model.bufferViews[vertex_accessor.bufferView];
	//			const tinygltf::Buffer& vertex_buffer = model.buffers[vertex_buffer_view.buffer];

	//			const size_t vertex_buffer_offset = vertex_buffer_view.byteOffset + vertex_accessor.byteOffset;
	//			const float* vertex_data = reinterpret_cast<const float*>(&vertex_buffer.data[vertex_buffer_offset]);

	//			// Normals
	//			const tinygltf::Accessor& normals_accessor = model.accessors[primitive.attributes.at("NORMAL")];
	//			const tinygltf::BufferView& normals_buffer_view = model.bufferViews[normals_accessor.bufferView];
	//			const tinygltf::Buffer& normals_buffer = model.buffers[normals_buffer_view.buffer];

	//			const size_t normals_buffer_offset = normals_buffer_view.byteOffset + normals_accessor.byteOffset;
	//			const float* normals_data = reinterpret_cast<const float*>(&normals_buffer.data[normals_buffer_offset]);

	//			// Transform into our Vertex struct - Unoptimized, would be better to redesign the pipeline to use multiple channels
	//			for (int i = 0; i < vertex_accessor.count; i++)
	//			{
	//				Vertex vertex;
	//				vertex.x = vertex_data[i * 3 + 0];
	//				vertex.y = vertex_data[i * 3 + 1];
	//				vertex.z = vertex_data[i * 3 + 2];

	//				vertex.normal_x = normals_data[i * 3 + 0];
	//				vertex.normal_y = normals_data[i * 3 + 1];
	//				vertex.normal_z = normals_data[i * 3 + 2];

	//				vertices.push_back(vertex);
	//			}

	//			// Indices
	//			const tinygltf::Accessor& indices_accessor = model.accessors[primitive.indices];
	//			const tinygltf::BufferView& indices_buffer_view = model.bufferViews[indices_accessor.bufferView];
	//			const tinygltf::Buffer& indices_buffer = model.buffers[indices_buffer_view.buffer];

	//			const size_t indices_buffer_offset = indices_buffer_view.byteOffset + indices_accessor.byteOffset;
	//			const short* indices_data = reinterpret_cast<const short*>(&indices_buffer.data[indices_buffer_offset]);

	//			for (size_t i = 0; i < indices_accessor.count; i++)
	//			{
	//				indices.push_back(indices_data[i]);
	//			}
	//		}
	//	}
	//}

	// Rebuild Direct3D11 buffers
	CreateVertexBuffer(vertices);
	CreateIndexBuffer(indices);
}
