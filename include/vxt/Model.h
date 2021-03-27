/******************

Parts of this code are heavily borrowed and modified from Sascha Willem's Vulkan glTF-PBR library (MIT License)
https://github.com/SaschaWillems/Vulkan-glTF-PBR

********************/


#pragma once
#include <vxt/VXT_EXPORT.h>
#include <string>
#include <vxt/LinearAlgebra.h>
#include <vkl/Common.h>
#include <vkl/TextureBuffer.h>
#include <memory>
#include <vkl/DrawCall.h>
#include <vkl/Reflect.h>

namespace vxt
{
	class Model : public reflect::object
	{
		REFLECTED_TYPE_ABSTRACT(Model, reflect::object, reflect::reflection_data_base)
		static void populateReflection(reflect::reflection_data_base& reflection) {}
	public:

		struct Material {
			enum AlphaMode { ALPHAMODE_OPAQUE, ALPHAMODE_MASK, ALPHAMODE_BLEND };
			AlphaMode alphaMode = ALPHAMODE_OPAQUE;
			float alphaCutoff = 1.0f;
			float metallicFactor = 1.0f;
			float roughnessFactor = 1.0f;
			glm::vec4 baseColorFactor = glm::vec4(1.0f);
			glm::vec4 emissiveFactor = glm::vec4(1.0f);
			std::shared_ptr<const vkl::TextureBuffer> baseColorTexture;
			std::shared_ptr<const vkl::TextureBuffer> metallicRoughnessTexture;
			std::shared_ptr<const vkl::TextureBuffer> normalTexture;
			std::shared_ptr<const vkl::TextureBuffer> occlusionTexture;
			std::shared_ptr<const vkl::TextureBuffer> emissiveTexture;
			struct TexCoordSets {
				uint8_t baseColor = 0;
				uint8_t metallicRoughness = 0;
				uint8_t specularGlossiness = 0;
				uint8_t normal = 0;
				uint8_t occlusion = 0;
				uint8_t emissive = 0;
			} texCoordSets;
			struct Extension {
				std::shared_ptr<const vkl::TextureBuffer> specularGlossinessTexture;
				std::shared_ptr<const vkl::TextureBuffer> diffuseTexture;
				glm::vec4 diffuseFactor = glm::vec4(1.0f);
				glm::vec3 specularFactor = glm::vec3(0.0f);
			} extension;
			struct PbrWorkflows {
				bool metallicRoughness = true;
				bool specularGlossiness = false;
			} pbrWorkflows;
		};

		struct Vertex {
			glm::vec3 pos;
			glm::vec3 normal;
			glm::vec2 uv0;
			glm::vec2 uv1;
			glm::vec4 joint0;
			glm::vec4 weight0;
		};

		struct Primitive
		{
			std::shared_ptr<const vkl::DrawCall> draw;
			glm::mat4 transform{ glm::identity<glm::mat4>() };
			int material{ -1 };
		};

		Model() = default;
		virtual ~Model() = default;
		Model(const Model&) = delete;
		Model(Model&&) noexcept = default;
		Model& operator=(Model&&) noexcept = default;
		Model& operator=(const Model&) = delete;

		virtual void init(const std::string& file, const vkl::Device& device, const vkl::SwapChain& swapChain, vkl::BufferManager& bufferManager) = 0;

		virtual std::span<const Vertex> getVerts() const = 0;
		virtual std::shared_ptr<const vkl::VertexBuffer> getVertexBuffer() const = 0;

		virtual std::span<const uint32_t> getIndices() const = 0;
		virtual std::shared_ptr<const vkl::IndexBuffer> getIndexBuffer() const = 0;

		virtual std::span<const Primitive> getPrimitives() const = 0;
		virtual std::span<const Material> getMaterials() const = 0;
	};
}