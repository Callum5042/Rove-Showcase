#include "Pch.h"
#include "GltfLoader.h"
#include "Model.h"
using namespace simdjson;
using namespace simdjson::dom;

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
	constexpr std::string_view Position = "position";
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
}

Rove::GltfLoader::GltfLoader(DxRenderer* renderer, DxShader* shader) : m_DxRenderer(renderer), m_DxShader(shader)
{
}

std::vector<Rove::Model*> Rove::GltfLoader::Load(const std::filesystem::path& path)
{
	// Load file
	parser parser;
	simdjson_result<element> document = parser.load(path.string());

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
		simdjson_result<int64_t> position_index = attribute[Json::Position].get_int64();

		// Indices
		simdjson_result<int64_t> indices_index = primitive[Json::Indices].get_int64();
		simdjson_result<element> index_accessor = document[Json::Accessors].at(indices_index.value());

		int64_t index_count = index_accessor[Json::Count].get_int64();
		AccessorDataType component_type = static_cast<AccessorDataType>(index_accessor[Json::ComponentType].get_int64().value());
		int64_t buffer_view_index = index_accessor[Json::BufferView].get_int64();

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
		std::filesystem::path binary_path = path.parent_path();
		binary_path.append(buffer_uri);

		std::ifstream file(binary_path.string(), std::fstream::in | std::fstream::binary);
		file.seekg(byte_offset);

		std::vector<short> indices;
		indices.resize(index_count);
		file.read(reinterpret_cast<char*>(indices.data()), byte_length);

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
