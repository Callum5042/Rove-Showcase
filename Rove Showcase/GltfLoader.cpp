#include "Pch.h"
#include "GltfLoader.h"
#include "Model.h"
#include "TextureLoader\WICTextureLoader.h"
#include "DxRenderer.h"
using namespace simdjson;
using namespace simdjson::dom;

namespace
{
	Rove::AccessorDataType GetAccessorType(std::string_view type)
	{
		std::map<std::string_view, Rove::AccessorDataType> dataTypes;
		dataTypes["SCALAR"] = Rove::AccessorDataType::SCALAR;
		dataTypes["VEC2"] = Rove::AccessorDataType::VEC2;
		dataTypes["VEC3"] = Rove::AccessorDataType::VEC3;
		dataTypes["VEC4"] = Rove::AccessorDataType::VEC4;

		if (dataTypes.find(type) == dataTypes.end())
		{
			return Rove::AccessorDataType::UNKNOWN;
		}

		return dataTypes[type];
	}

	template <typename TDataType>
	std::vector<TDataType> ReinterpretBuffer(std::vector<char> buffer, int64_t count)
	{
		TDataType* data = reinterpret_cast<TDataType*>(buffer.data());
		return std::vector<TDataType>(data, data + count);
	}
}

namespace Json
{
	constexpr std::string_view Nodes = "nodes";
	constexpr std::string_view Mesh = "mesh";
	constexpr std::string_view Translation = "translation";
	constexpr std::string_view Rotation = "rotation";
	constexpr std::string_view Meshes = "meshes";
	constexpr std::string_view Primitives = "primitives";
	constexpr std::string_view Attributes = "attributes";
	constexpr std::string_view Name = "name";
	constexpr std::string_view Position = "POSITION";
	constexpr std::string_view Normal = "NORMAL";
	constexpr std::string_view Tangent = "TANGENT";
	constexpr std::string_view Texcoord0 = "TEXCOORD_0";
	constexpr std::string_view Indices = "indices";
	constexpr std::string_view Accessors = "accessors";
	constexpr std::string_view BufferView = "bufferView";
	constexpr std::string_view BufferViews = "bufferViews";
	constexpr std::string_view Count = "count";
	constexpr std::string_view ComponentType = "componentType";
	constexpr std::string_view Buffers = "buffers";
	constexpr std::string_view Buffer = "buffer";
	constexpr std::string_view ByteLength = "byteLength";
	constexpr std::string_view ByteOffset = "byteOffset";
	constexpr std::string_view Uri = "uri";
	constexpr std::string_view Type = "type";
	constexpr std::string_view Material = "material";
	constexpr std::string_view Materials = "materials";
	constexpr std::string_view PbrMetallicRoughness = "pbrMetallicRoughness";
	constexpr std::string_view MetallicFactor = "metallicFactor";
	constexpr std::string_view RoughnessFactor = "roughnessFactor";
	constexpr std::string_view BaseColorTexture = "baseColorTexture";
	constexpr std::string_view NormalTexture = "normalTexture";
	constexpr std::string_view Index = "index";
	constexpr std::string_view TexCoord = "texCoord";
	constexpr std::string_view Textures = "textures";
	constexpr std::string_view Images = "images";
	constexpr std::string_view Source = "source";
}

Rove::GltfLoader::GltfLoader(DxRenderer* renderer, DxShader* shader) : m_DxRenderer(renderer), m_DxShader(shader)
{
}

std::vector<std::unique_ptr<Rove::Model>> Rove::GltfLoader::Load(const std::filesystem::path& path)
{
	HRESULT hr = CoInitializeEx(nullptr, COINIT_MULTITHREADED);

	m_Path = path;

	// Load file
	parser parser;
	simdjson_result<element> document = parser.load(m_Path.string());

	// Populate models
	std::vector<std::unique_ptr<Model>> models;
	for (element node : document[Json::Nodes])
	{
		// Model* model = new Model(m_DxRenderer, m_DxShader);
		std::unique_ptr<Model> model = std::make_unique<Model>(m_DxRenderer, m_DxShader);

		// Check if node is valid
		simdjson_result<int64_t> mesh_index = node[Json::Mesh].get_int64();
		if (mesh_index.error() != simdjson::SUCCESS)
		{
			continue;
		}

		// World transformation
		DirectX::XMMATRIX world = ApplyWorldTransformation(node);

		// Mesh data
		simdjson_result<element> mesh = document[Json::Meshes].at(mesh_index.value());
		if (mesh.error() != simdjson::SUCCESS)
		{
			continue;
		}

		// Name of mesh
		simdjson_result<std::string_view> name = mesh[Json::Name].get_string();

		// We only care about the first primitive as we don't care about trying to render other primitives and we assume its a triangle list
		simdjson_result<element> primitive = mesh[Json::Primitives].at(0);
		simdjson_result<element> attribute = primitive[Json::Attributes];

		// Position vertices
		LoadVertices(document.value(), attribute.value(), model.get());

		// Indices
		simdjson_result<int64_t> indices_index = primitive[Json::Indices].get_int64();
		simdjson_result<element> index_accessor = document[Json::Accessors].at(indices_index.value());
		LoadIndices(document.value(), index_accessor.value(), model.get());

		// Material
		simdjson_result<int64_t> material_index = primitive[Json::Material].get_int64();
		if (material_index.error() == simdjson::SUCCESS)
		{
			simdjson_result<element> material = document[Json::Materials].at(material_index.value());

			// Properties
			model->Material.metallicFactor = static_cast<float>(material[Json::PbrMetallicRoughness][Json::MetallicFactor].get_double());
			model->Material.roughnessFactor = static_cast<float>(material[Json::PbrMetallicRoughness][Json::RoughnessFactor].get_double());

			// Diffuse texture
			LoadDiffuseTexture(document.value(), material.value(), model.get());

			// Normal texture
			LoadNormalTexture(document.value(), material.value(), model.get());
		}

		// Assign model
		model->World = world;
		model->Name = name.value();
		models.push_back(std::move(model));
	}

	CoUninitialize();
	return models;
}

DirectX::XMMATRIX Rove::GltfLoader::ApplyWorldTransformation(simdjson::dom::element& node)
{
	DirectX::XMMATRIX world = DirectX::XMMatrixIdentity();

	// Rotation - this must be done before position otherwise it will skew the maths
	simdjson_result<array> rotation = node[Json::Rotation].get_array();
	if (rotation.error() == simdjson::SUCCESS)
	{
		float x = static_cast<float>(rotation.at(0).get_double().value());
		float y = static_cast<float>(rotation.at(1).get_double().value());
		float z = static_cast<float>(rotation.at(2).get_double().value());
		float w = static_cast<float>(rotation.at(3).get_double().value());
		world *= DirectX::XMMatrixRotationQuaternion(DirectX::XMVectorSet(x, y, z, w));
	}

	// Position
	simdjson_result<array> translation = node[Json::Translation].get_array();
	if (translation.error() == simdjson::SUCCESS)
	{
		float x = static_cast<float>(translation.at(0).get_double().value());
		float y = static_cast<float>(translation.at(1).get_double().value());
		float z = static_cast<float>(translation.at(2).get_double().value());
		world *= DirectX::XMMatrixTranslation(x, y, z);
	}

	return world;
}

void Rove::GltfLoader::LoadVertices(simdjson::dom::element& document, simdjson::dom::element& attribute, Model* model)
{
	std::vector<Vertex> vertices;

	// Position
	{
		simdjson_result<int64_t> accessor_index = attribute[Json::Position].get_int64();
		if (accessor_index.error() != simdjson::SUCCESS)
		{
			throw std::exception("Could not detect vertex position data");
		}

		simdjson_result<element> accessor = document[Json::Accessors].at(accessor_index.value());

		int64_t count = 0;
		std::vector<char> buffer = BufferAccessor(document, accessor.value(), nullptr, nullptr, &count);
		std::vector<Vec3<float>> data = ReinterpretBuffer<Vec3<float>>(buffer, count);
		vertices.resize(count);

		for (int64_t i = 0; i < count; ++i)
		{
			Vec3<float> position = data[i];
			vertices[i].x = position.x;
			vertices[i].y = position.y;
			vertices[i].z = position.z;
		}
	}

	// Normal
	{
		simdjson_result<int64_t> accessor_index = attribute[Json::Normal].get_int64();
		if (accessor_index.error() == simdjson::SUCCESS)
		{
			simdjson_result<element> accessor = document[Json::Accessors].at(accessor_index.value());
			int64_t count = 0;
			std::vector<char> buffer = BufferAccessor(document, accessor.value(), nullptr, nullptr, &count);
			std::vector<Vec3<float>> data = ReinterpretBuffer<Vec3<float>>(buffer, count);

			for (int64_t i = 0; i < count; ++i)
			{
				Vec3<float> normal = data[i];
				vertices[i].normal_x = normal.x;
				vertices[i].normal_y = normal.y;
				vertices[i].normal_z = normal.z;
			}
		}
	}

	// Tangent
	{
		simdjson_result<int64_t> accessor_index = attribute[Json::Tangent].get_int64();
		if (accessor_index.error() == simdjson::SUCCESS)
		{
			simdjson_result<element> accessor = document[Json::Accessors].at(accessor_index.value());

			int64_t count = 0;
			std::vector<char> buffer = BufferAccessor(document, accessor.value(), nullptr, nullptr, &count);
			std::vector<Vec4<float>> data = ReinterpretBuffer<Vec4<float>>(buffer, count);

			for (int64_t i = 0; i < count; ++i)
			{
				Vec4<float> tangent = data[i];
				vertices[i].tangent_x = tangent.x;
				vertices[i].tangent_y = tangent.y;
				vertices[i].tangent_z = tangent.z;
			}
		}
	}

	// Texcoord
	{
		simdjson_result<int64_t> accessor_index = attribute[Json::Texcoord0].get_int64();
		if (accessor_index.error() == simdjson::SUCCESS)
		{
			simdjson_result<element> accessor = document[Json::Accessors].at(accessor_index.value());

			int64_t count = 0;
			std::vector<char> buffer = BufferAccessor(document, accessor.value(), nullptr, nullptr, &count);
			std::vector<Vec2<float>> data = ReinterpretBuffer<Vec2<float>>(buffer, count);

			for (int64_t i = 0; i < count; ++i)
			{
				Vec2<float> texcoord = data[i];
				vertices[i].texture_u = texcoord.x;
				vertices[i].texture_v = texcoord.y;
			}
		}
	}

	model->CreateVertexBuffer(vertices);
}

void Rove::GltfLoader::LoadIndices(simdjson::dom::element& document, simdjson::dom::element& accessor, Model* model)
{
	int64_t count = 0;
	ComponentDataType component_data_type = ComponentDataType::UNKNOWN;
	std::vector<char> indices_buffer = BufferAccessor(document, accessor, &component_data_type, nullptr, &count);

	if (component_data_type == ComponentDataType::UNSIGNED_SHORT)
	{
		USHORT* data = reinterpret_cast<USHORT*>(indices_buffer.data());
		model->CreateIndexBuffer(data, static_cast<UINT>(count), sizeof(USHORT), DXGI_FORMAT_R16_UINT);
	}
	else if (component_data_type == ComponentDataType::UNSIGNED_INT)
	{
		UINT* data = reinterpret_cast<UINT*>(indices_buffer.data());
		model->CreateIndexBuffer(data, static_cast<UINT>(count), sizeof(UINT), DXGI_FORMAT_R32_UINT);
	}
}

void Rove::GltfLoader::LoadDiffuseTexture(simdjson::dom::element& document, simdjson::dom::element& node, Model* model)
{
	simdjson_result<int64_t> texture_index = node[Json::PbrMetallicRoughness][Json::BaseColorTexture][Json::Index].get_int64();
	if (texture_index.error() != simdjson::SUCCESS)
	{
		// No diffuse texture detected
		return;
	}

	simdjson_result<int64_t> image_index = document[Json::Textures].at(texture_index.value())[Json::Source].get_int64();
	simdjson_result<element> image = document[Json::Images].at(image_index.value());
	std::string_view uri = image[Json::Uri].get_string().value();

	std::filesystem::path texture_path = m_Path.parent_path();
	texture_path.append(uri);

	ComPtr<ID3D11Resource> resource = nullptr;
	DX::Check(DirectX::CreateWICTextureFromFile(m_DxRenderer->GetDevice(), m_DxRenderer->GetDeviceContext(), texture_path.wstring().c_str(), resource.ReleaseAndGetAddressOf(), model->m_DiffuseTexture.ReleaseAndGetAddressOf()));
	model->Material.diffuse_texture = true;
}

void Rove::GltfLoader::LoadNormalTexture(simdjson::dom::element& document, simdjson::dom::element& node, Model* model)
{
	simdjson_result<int64_t> texture_index = node[Json::NormalTexture][Json::Index].get_int64();
	if (texture_index.error() != simdjson::SUCCESS)
	{
		// No normal texture detected
		return;
	}

	simdjson_result<int64_t> image_index = document[Json::Textures].at(texture_index.value())[Json::Source].get_int64();
	simdjson_result<element> image = document[Json::Images].at(image_index.value());
	std::string_view uri = image[Json::Uri].get_string().value();

	std::filesystem::path texture_path = m_Path.parent_path();
	texture_path.append(uri);

	ComPtr<ID3D11Resource> resource = nullptr;
	DX::Check(DirectX::CreateWICTextureFromFile(m_DxRenderer->GetDevice(), m_DxRenderer->GetDeviceContext(), texture_path.wstring().c_str(), resource.ReleaseAndGetAddressOf(), model->m_NormalTexture.ReleaseAndGetAddressOf()));
	model->Material.normal_texture = true;
}

std::vector<char> Rove::GltfLoader::BufferAccessor(simdjson::dom::element& document, simdjson::dom::element& accessor, ComponentDataType* componentDataType, AccessorDataType* accessorDataType, int64_t* count)
{
	// Accessor
	*count = accessor[Json::Count].get_int64();
	if (componentDataType != nullptr)
	{
		*componentDataType = static_cast<ComponentDataType>(accessor[Json::ComponentType].get_int64().value());
	}

	if (accessorDataType != nullptr)
	{
		*accessorDataType = GetAccessorType(accessor[Json::Type].get_string());
	}

	int64_t buffer_view_index = accessor[Json::BufferView].get_int64();

	// View
	simdjson_result<element> view_buffer = document[Json::BufferViews].at(buffer_view_index);
	int64_t buffer_index = view_buffer[Json::Buffer].get_int64().value();
	int64_t byte_length = view_buffer[Json::ByteLength].get_int64().value();
	int64_t byte_offset = view_buffer[Json::ByteOffset].get_int64().value();

	// Buffer
	simdjson_result<element> buffer = document[Json::Buffers].at(buffer_index);
	int64_t buffer_byte_length = buffer[Json::ByteLength].get_int64().value();
	std::string_view buffer_uri = buffer[Json::Uri].get_string();

	// Load buffer
	std::filesystem::path binary_path = m_Path.parent_path();
	binary_path.append(buffer_uri);

	std::ifstream file(binary_path.string(), std::fstream::in | std::fstream::binary);
	file.seekg(byte_offset);

	std::vector<char> data;
	data.resize(byte_length);
	file.read(data.data(), byte_length);

	return data;
}
