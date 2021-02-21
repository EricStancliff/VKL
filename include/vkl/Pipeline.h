#pragma once

#include <Common.h>

#include <filesystem>

namespace vkl
{
	class ShaderData;

	class PipelineDescription
	{
	public:

		struct ShaderDescription {
			VkShaderStageFlagBits stage;
			std::shared_ptr<const ShaderData> shader;
		};

		struct VertexAttributeDescription
		{
			uint32_t binding{ 0 };
			uint32_t location{ 0 };
			VkFormat format;
			size_t size{ 0 };
			size_t offset{ 0 };
		};

		struct UniformDescription
		{
			uint32_t binding{ 0 };
			size_t size{ 0 };
		};

		struct PushConstantDescription
		{
			size_t size{ 0 };
			bool hasPushConstant{ false };
		};

		struct TextureDescription
		{
			uint32_t binding{ 0 };
		};

		PipelineDescription();
		~PipelineDescription();

		void addShaderGLSL(VkShaderStageFlagBits stage, const char* shader);
		void addShaderGLSL(VkShaderStageFlagBits stage, const std::filesystem::path& path);

		void addShaderSPV(VkShaderStageFlagBits stage, const char* shader);
		void addShaderSPV(VkShaderStageFlagBits stage, const std::filesystem::path& path);

		void declareVertexAttribute(uint32_t binding, uint32_t location, VkFormat format, size_t bindingSize, size_t locationOffset);

		void declareUniform(uint32_t binding, size_t size);
		void declarePushConstant(size_t size);

		void declareTexture(uint32_t binding);


		//for use by pipeline
		std::span<const ShaderDescription> shaders() const;
		std::span<const VertexAttributeDescription> attributes() const;
		std::span<const UniformDescription> uniforms() const;
		const PushConstantDescription& pushConstant() const;
		std::span<const TextureDescription> textures() const;

	private:
		std::vector< ShaderDescription> _shaders;
		std::vector<VertexAttributeDescription> _attributes;
		std::vector<UniformDescription> _uniforms;
		PushConstantDescription _pushConstant;
		std::vector<TextureDescription> _textures;
	};

	class Pipeline
	{
	public:
		Pipeline() = delete;
		Pipeline(const Device& device, const PipelineDescription& description);

		VkPipeline handle() const;
	private:
		VkPipeline _pipeline;
	};
}