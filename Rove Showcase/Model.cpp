#include "Pch.h"
#include "Model.h"
#include "Rendering/DxRenderer.h"
#include "Application.h"

// TinyGltf
#define TINYGLTF_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "TinyGltf\tiny_gltf.h"

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
	for (auto& mesh_details : m_MeshDetails)
	{
		d3dDeviceContext->DrawIndexed(mesh_details.indices_count, mesh_details.indices_start, mesh_details.vertex_start);
	}
}

void Rove::Model::LoadFromFile(const std::wstring& filepath)
{
	// https://www.khronos.org/registry/glTF/specs/2.0/glTF-2.0.html#meshes-overview

	// TODO: Make the transformation matrix apply
	// Might have to split the mesh into a mesh class instead of 1 

	tinygltf::Model model;
	tinygltf::TinyGLTF loader;
	std::string err;
	std::string warn;

	// Convert wstring to string
	std::string path = Rove::ConvertToString(filepath);

	// Load file
	if (!loader.LoadBinaryFromFile(&model, &err, &warn, path))
	{
		throw std::exception(err.c_str());
	}

	// Parse file
	std::string name;
	std::vector<Vertex> vertices;
	std::vector<UINT> indices;

	int i = 0;
	for (auto& mesh : model.meshes)
	{
		float mesh_x = 0; 
		float mesh_y = 0; 
		float mesh_z = 0; 

		if (!model.nodes[i].translation.empty())
		{
			mesh_x = static_cast<float>(model.nodes[i].translation[0]);
			mesh_y = static_cast<float>(model.nodes[i].translation[1]);
			mesh_z = static_cast<float>(model.nodes[i].translation[2]);
		}

		++i;

		MeshDetails mesh_details;
		for (auto& primitive : mesh.primitives)
		{
			{
				mesh_details.vertex_start = vertices.size();

				// Position
				{
					const tinygltf::Accessor& accessor = model.accessors[primitive.attributes["POSITION"]];
					const tinygltf::BufferView& bufferView = model.bufferViews[accessor.bufferView];
					const tinygltf::Buffer& buffer = model.buffers[bufferView.buffer];

					const tinygltf::Accessor& normal_accessor = model.accessors[primitive.attributes["NORMAL"]];
					const tinygltf::BufferView& normal_bufferView = model.bufferViews[normal_accessor.bufferView];
					const tinygltf::Buffer& normal_buffer = model.buffers[normal_bufferView.buffer];

					const float* positions = reinterpret_cast<const float*>(&buffer.data[bufferView.byteOffset + accessor.byteOffset]);
					const float* normals = reinterpret_cast<const float*>(&normal_buffer.data[normal_bufferView.byteOffset + normal_accessor.byteOffset]);

					for (size_t i = 0; i < accessor.count; ++i)
					{
						float x = mesh_x + positions[i * 3 + 0];
						float y = mesh_y + positions[i * 3 + 1];
						float z = mesh_z + positions[i * 3 + 2];

						float nx = normals[i * 3 + 0];
						float ny = normals[i * 3 + 1];
						float nz = normals[i * 3 + 2];

						Vertex vertex;
						vertex.x = x;
						vertex.y = y;
						vertex.z = z;

						vertex.normal_x = nx;
						vertex.normal_y = ny;
						vertex.normal_z = nz;

						vertices.push_back(vertex);
					}
				}

				// Indices
				mesh_details.indices_start = indices.size();

				{
					const auto& accessor = model.accessors[primitive.indices];
					const auto& buffer_view = model.bufferViews[accessor.bufferView];
					const auto& buffer = model.buffers[buffer_view.buffer];

					const short* _indices = reinterpret_cast<const short*>(&buffer.data[buffer_view.byteOffset + accessor.byteOffset]);
					// indices.assign(_indices, _indices + accessor.count);
					for (size_t i = 0; i < accessor.count; i++)
					{
						indices.push_back(_indices[i]);
					}

					mesh_details.indices_count = accessor.count;
				}
			}
		}

		m_MeshDetails.push_back(mesh_details);
	}

	// Rebuild Direct3D11 buffers
	CreateVertexBuffer(vertices);
	CreateIndexBuffer(indices);
}
