#include "Pch.h"
#include "Model.h"
#include "DxRenderer.h"
#include "DxShader.h"
#include "Application.h"
#include "GltfLoader.h"

Rove::Object::Object(DxRenderer* renderer, DxShader* shader) : m_DxRenderer(renderer), m_DxShader(shader)
{
}

void Rove::Object::LoadFile(const std::filesystem::path& path)
{
	// Clear old data
	for (Model* model : m_Models)
	{
		delete model;
	}

	m_Models.clear();

	// Load new data
	GltfLoader loader(m_DxRenderer, m_DxShader);
	std::vector<Model*> models = loader.Load(path);
	m_Models = loader.Load(path);

	// Set filename
	Filename = path.filename().string();
}

void Rove::Object::Render()
{
	for (auto& model : m_Models)
	{
		model->Render();
	}
}

std::vector<Rove::Material*> Rove::Object::GetMaterials()
{
	std::vector<Material*> materials;
	for (auto& model : m_Models)
	{
		materials.push_back(&model->Material);
	}

	return materials;
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

	// Apply materials
	Rove::MaterialBuffer material_buffer = {};
	material_buffer.diffuse_texture = static_cast<int>(Material.diffuse_texture);
	material_buffer.normal_texture = static_cast<int>(Material.normal_texture);
	material_buffer.metallicFactor = Material.metallicFactor;
	material_buffer.roughnessFactor = Material.roughnessFactor;
	m_DxShader->UpdateMaterialBuffer(material_buffer);

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
