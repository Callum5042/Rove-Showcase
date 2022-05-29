#pragma once

#include "Pch.h"
#include "simdjson\simdjson.h"

namespace Rove
{
	class Model;
	class DxRenderer;
	class DxShader;

	enum class ComponentDataType
	{
		SIGNED_BYTE = 5120,
		UNSIGNED_BYTE = 5121,
		SIGNED_SHORT = 5122,
		UNSIGNED_SHORT = 5123,
		UNSIGNED_INT = 5125,
		FLOAT = 5126
	};

	enum class AccessorDataType
	{
		UNKNOWN,
		SCALAR,
		VEC2,
		VEC3,
		VEC4
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

	class GltfLoader
	{
	private: 
		// Dependencies
		DxRenderer* m_DxRenderer = nullptr;
		DxShader* m_DxShader = nullptr;

	public:
		GltfLoader(DxRenderer* renderer, DxShader* shader);
		virtual ~GltfLoader() = default;

		std::vector<Model*> Load(const std::filesystem::path& path);

	private:
		std::filesystem::path m_Path;

		DirectX::XMMATRIX ApplyWorldTransformation(simdjson::dom::element& node);

		void LoadVertices(simdjson::dom::element& document, simdjson::dom::element& attribute, Model* model);
		void LoadIndices(simdjson::dom::element& document, simdjson::dom::element& accessor, Model* model);
		char* BufferAccessor(simdjson::dom::element& document, simdjson::dom::element& accessor, ComponentDataType* componentDataType, AccessorDataType* accessorDataType, int64_t* count);
	};
}
