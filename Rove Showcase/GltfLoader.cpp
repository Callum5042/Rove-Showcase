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

		if (dataTypes.find(type) != dataTypes.end())
		{
			return dataTypes[type];
		}

		return Rove::AccessorDataType::UNKNOWN;
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

std::vector<Rove::Model*> Rove::GltfLoader::Load(const std::filesystem::path& path)
{
	m_Path = path;

	// Load file
	parser parser;
	simdjson_result<element> document = parser.load(m_Path.string());

	// Populate models
	std::vector<Model*> models;
	for (element node : document[Json::Nodes])
	{ 
		Model* model = new Model(m_DxRenderer, m_DxShader);

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
		LoadVertices(document.value(), attribute.value(), model);

		// Indices
		simdjson_result<int64_t> indices_index = primitive[Json::Indices].get_int64();
		simdjson_result<element> index_accessor = document[Json::Accessors].at(indices_index.value());
		LoadIndices(document.value(), index_accessor.value(), model);

		// Material
		HRESULT hr = CoInitializeEx(nullptr, COINIT_MULTITHREADED);

		simdjson_result<int64_t> material_index = primitive[Json::Material].get_int64();
		simdjson_result<element> material = document[Json::Materials].at(material_index.value());

		// Properties
		model->Material.metallicFactor = static_cast<float>(material[Json::PbrMetallicRoughness][Json::MetallicFactor].get_double());
		model->Material.roughnessFactor = static_cast<float>(material[Json::PbrMetallicRoughness][Json::RoughnessFactor].get_double());

		// Diffuse texture
		LoadDiffuseTexture(document.value(), material.value(), model);

		// Normal texture
		LoadNormalTexture(document.value(), material.value(), model);

		// Assign model
		model->LocalWorld = world;
		model->Name = name.value();
		models.push_back(model);
	}

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
		simdjson_result<element> accessor = document[Json::Accessors].at(accessor_index.value());

		ComponentDataType component_data_type;
		AccessorDataType accessor_data_type;
		int64_t count = 0;
		char* buffer = BufferAccessor(document, accessor.value(), &component_data_type, &accessor_data_type, &count);
		Vec3<float>* positions = reinterpret_cast<Vec3<float>*>(buffer);

		vertices.resize(count);
		for (int64_t i = 0; i < count; ++i)
		{
			Vec3<float> position = positions[i];
			vertices[i].x = position.x;
			vertices[i].y = position.y;
			vertices[i].z = position.z;
		}
	}

	// Normal
	{
		simdjson_result<int64_t> accessor_index = attribute[Json::Normal].get_int64();
		simdjson_result<element> accessor = document[Json::Accessors].at(accessor_index.value());

		ComponentDataType component_data_type;
		AccessorDataType accessor_data_type;
		int64_t count = 0;
		char* buffer = BufferAccessor(document, accessor.value(), &component_data_type, &accessor_data_type, &count);
		Vec3<float>* normals = reinterpret_cast<Vec3<float>*>(buffer);

		for (int64_t i = 0; i < count; ++i)
		{
			Vec3<float> normal = normals[i];
			vertices[i].normal_x = normal.x;
			vertices[i].normal_y = normal.y;
			vertices[i].normal_z = normal.z;
		}
	}

	// Tangent
	{
		simdjson_result<int64_t> accessor_index = attribute[Json::Tangent].get_int64();
		simdjson_result<element> accessor = document[Json::Accessors].at(accessor_index.value());

		ComponentDataType component_data_type;
		AccessorDataType accessor_data_type;
		int64_t count = 0;
		char* buffer = BufferAccessor(document, accessor.value(), &component_data_type, &accessor_data_type, &count);
		Vec4<float>* data = reinterpret_cast<Vec4<float>*>(buffer);

		for (int64_t i = 0; i < count; ++i)
		{
			Vec4<float> tangent = data[i];
			vertices[i].tangent_x = tangent.x;
			vertices[i].tangent_y = tangent.y;
			vertices[i].tangent_z = tangent.z;
		}
	}

	// Texcoord
	{
		simdjson_result<int64_t> accessor_index = attribute[Json::Texcoord0].get_int64();
		simdjson_result<element> accessor = document[Json::Accessors].at(accessor_index.value());

		ComponentDataType component_data_type;
		AccessorDataType accessor_data_type;
		int64_t count = 0;
		char* buffer = BufferAccessor(document, accessor.value(), &component_data_type, &accessor_data_type, &count);
		Vec2<float>* data = reinterpret_cast<Vec2<float>*>(buffer);

		for (int64_t i = 0; i < count; ++i)
		{
			Vec2<float> texcoord = data[i];
			vertices[i].texture_u = texcoord.x;
			vertices[i].texture_v = texcoord.y;
		}
	}

	model->CreateVertexBuffer(vertices);
}

void Rove::GltfLoader::LoadIndices(simdjson::dom::element& document, simdjson::dom::element& accessor, Model* model)
{
	ComponentDataType component_data_type;
	AccessorDataType accessor_data_type;
	int64_t count = 0;
	char* indices_buffer = BufferAccessor(document, accessor, &component_data_type, &accessor_data_type, &count);

	std::vector<short> indices_data1;
	short* indices_data = reinterpret_cast<short*>(indices_buffer);
	indices_data1.assign(indices_data, indices_data + count);

	std::vector<UINT> indices;
	indices.assign(indices_data1.begin(), indices_data1.end());
	model->CreateIndexBuffer(indices);
}

void Rove::GltfLoader::LoadDiffuseTexture(simdjson::dom::element& document, simdjson::dom::element& node, Model* model)
{
	simdjson_result<int64_t> texture_index = node[Json::PbrMetallicRoughness][Json::BaseColorTexture][Json::Index].get_int64();
	simdjson_result<int64_t> image_index = document[Json::Textures].at(texture_index.value())[Json::Source].get_int64();
	simdjson_result<element> image = document[Json::Images].at(image_index.value());
	std::string_view uri = image[Json::Uri].get_string().value();

	std::filesystem::path texture_path = m_Path.parent_path();
	texture_path.append(uri);

	ComPtr<ID3D11Resource> resource = nullptr;
	DirectX::CreateWICTextureFromFile(m_DxRenderer->GetDevice(), texture_path.wstring().c_str(), resource.ReleaseAndGetAddressOf(), model->m_DiffuseTexture.ReleaseAndGetAddressOf());
	model->Material.diffuse_texture = true;
}

void Rove::GltfLoader::LoadNormalTexture(simdjson::dom::element& document, simdjson::dom::element& node, Model* model)
{
	simdjson_result<int64_t> texture_index = node[Json::NormalTexture][Json::Index].get_int64();
	simdjson_result<int64_t> image_index = document[Json::Textures].at(texture_index.value())[Json::Source].get_int64();
	simdjson_result<element> image = document[Json::Images].at(image_index.value());
	std::string_view uri = image[Json::Uri].get_string().value();

	std::filesystem::path texture_path = m_Path.parent_path();
	texture_path.append(uri);

	ComPtr<ID3D11Resource> resource = nullptr;
	DirectX::CreateWICTextureFromFile(m_DxRenderer->GetDevice(), texture_path.wstring().c_str(), resource.ReleaseAndGetAddressOf(), model->m_NormalTexture.ReleaseAndGetAddressOf());
	model->Material.normal_texture = true;
}

char* Rove::GltfLoader::BufferAccessor(simdjson::dom::element& document, simdjson::dom::element& accessor, ComponentDataType* componentDataType, AccessorDataType* accessorDataType, int64_t* count)
{
	// Accessor
	*count = accessor[Json::Count].get_int64();
	*componentDataType = static_cast<ComponentDataType>(accessor[Json::ComponentType].get_int64().value());
	*accessorDataType = GetAccessorType(accessor[Json::Type].get_string());
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

	char* data = new char[byte_length];
	file.read(data, byte_length);

	return data;
}


