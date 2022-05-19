#include "Pch.h"
#include "DxShader.h"
#include "DxRenderer.h"

Rove::DxShader::DxShader(DxRenderer* renderer) : m_DxRenderer(renderer)
{
}

void Rove::DxShader::Load()
{
	CreateCameraConstantBuffer();
	CreateWorldConstantBuffer();
	CreatePointLightConstantBuffer();
	CreateLocalWorldConstantBuffer();

	LoadVertexShader("VertexShader.cso");
	LoadPixelShader("PixelShader.cso");
}

void Rove::DxShader::Apply()
{
	auto deviceContext = m_DxRenderer->GetDeviceContext();

	// Bind the input layout to the pipeline's Input Assembler stage
	deviceContext->IASetInputLayout(m_VertexLayout.Get());

	// Bind the vertex shader to the pipeline's Vertex Shader stage
	deviceContext->VSSetShader(m_VertexShader.Get(), nullptr, 0);

	// Bind the pixel shader to the pipeline's Pixel Shader stage
	deviceContext->PSSetShader(m_PixelShader.Get(), nullptr, 0);

	// Bind the world constant buffer to the vertex shader
	deviceContext->VSSetConstantBuffers(0, 1, m_CameraConstantBuffer.GetAddressOf());
	deviceContext->VSSetConstantBuffers(1, 1, m_WorldConstantBuffer.GetAddressOf());
	deviceContext->VSSetConstantBuffers(3, 1, m_LocalWorldConstantBuffer.GetAddressOf());

	// Bind the light constant buffer to pixel shader
	deviceContext->PSSetConstantBuffers(0, 1, m_CameraConstantBuffer.GetAddressOf());
	deviceContext->PSSetConstantBuffers(2, 1, m_PointLightConstantBuffer.GetAddressOf());
}

void Rove::DxShader::UpdateCameraBuffer(const CameraBuffer& buffer)
{
	auto deviceContext = m_DxRenderer->GetDeviceContext();
	deviceContext->UpdateSubresource(m_CameraConstantBuffer.Get(), 0, nullptr, &buffer, 0, 0);
}

void Rove::DxShader::UpdateWorldConstantBuffer(const WorldBuffer& worldBuffer)
{
	auto deviceContext = m_DxRenderer->GetDeviceContext();
	deviceContext->UpdateSubresource(m_WorldConstantBuffer.Get(), 0, nullptr, &worldBuffer, 0, 0);
}

void Rove::DxShader::UpdateLocalWorldConstantBuffer(const LocalWorldBuffer& worldBuffer)
{
	auto deviceContext = m_DxRenderer->GetDeviceContext();
	deviceContext->UpdateSubresource(m_LocalWorldConstantBuffer.Get(), 0, nullptr, &worldBuffer, 0, 0);
}

void Rove::DxShader::UpdatePointLightBuffer(const PointLightBuffer& buffer)
{
	auto deviceContext = m_DxRenderer->GetDeviceContext();
	deviceContext->UpdateSubresource(m_PointLightConstantBuffer.Get(), 0, nullptr, &buffer, 0, 0);
}

void Rove::DxShader::LoadVertexShader(std::string&& vertex_shader_path)
{
	auto device = m_DxRenderer->GetDevice();

	// Throw if file doesn't exist
	if (!std::filesystem::exists(vertex_shader_path))
	{
		std::string error = "Could not find file " + vertex_shader_path;
		throw std::exception(error.c_str());
	}

	// Load the binary file into memory
	std::ifstream file(vertex_shader_path, std::fstream::in | std::fstream::binary);
	std::vector<char> data((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());

	// Create the vertex shader
	DX::Check(device->CreateVertexShader(data.data(), data.size(), nullptr, m_VertexShader.ReleaseAndGetAddressOf()));

	// Describe the memory layout
	D3D11_INPUT_ELEMENT_DESC layout[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 32, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};

	UINT numElements = ARRAYSIZE(layout);
	DX::Check(device->CreateInputLayout(layout, numElements, data.data(), data.size(), m_VertexLayout.ReleaseAndGetAddressOf()));
}

void Rove::DxShader::LoadPixelShader(std::string&& pixel_shader_path)
{
	auto device = m_DxRenderer->GetDevice();

	// Throw if file doesn't exist
	if (!std::filesystem::exists(pixel_shader_path))
	{
		std::string error = "Could not find file " + pixel_shader_path;
		throw std::exception(error.c_str());
	}

	// Load the binary file into memory
	std::ifstream file(pixel_shader_path, std::fstream::in | std::fstream::binary);
	std::vector<char> data((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());

	// Create pixel shader
	DX::Check(device->CreatePixelShader(data.data(), data.size(), nullptr, m_PixelShader.ReleaseAndGetAddressOf()));
}

void Rove::DxShader::CreateCameraConstantBuffer()
{
	auto device = m_DxRenderer->GetDevice();

	// Create world constant buffer - Must set D3D11_USAGE_DYNAMIC and D3D11_CPU_ACCESS_WRITE to be able to with Map
	D3D11_BUFFER_DESC bd = {};
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.ByteWidth = sizeof(CameraBuffer);
	bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	//bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

	DX::Check(device->CreateBuffer(&bd, nullptr, m_CameraConstantBuffer.ReleaseAndGetAddressOf()));
}

void Rove::DxShader::CreateWorldConstantBuffer()
{
	auto device = m_DxRenderer->GetDevice();

	// Create world constant buffer
	D3D11_BUFFER_DESC bd = {};
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.ByteWidth = sizeof(WorldBuffer);
	bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;

	DX::Check(device->CreateBuffer(&bd, nullptr, m_WorldConstantBuffer.ReleaseAndGetAddressOf()));
}

void Rove::DxShader::CreatePointLightConstantBuffer()
{
	auto device = m_DxRenderer->GetDevice();

	// Create point light constant buffer
	D3D11_BUFFER_DESC bd = {};
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.ByteWidth = sizeof(PointLightBuffer);
	bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;

	DX::Check(device->CreateBuffer(&bd, nullptr, m_PointLightConstantBuffer.ReleaseAndGetAddressOf()));
}

void Rove::DxShader::CreateLocalWorldConstantBuffer()
{
	auto device = m_DxRenderer->GetDevice();

	// Create point light constant buffer
	D3D11_BUFFER_DESC bd = {};
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.ByteWidth = sizeof(LocalWorldBuffer);
	bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;

	DX::Check(device->CreateBuffer(&bd, nullptr, m_LocalWorldConstantBuffer.ReleaseAndGetAddressOf()));
}
