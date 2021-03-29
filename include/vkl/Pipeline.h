#pragma once

#include <vkl/Common.h>

#include <filesystem>
#include <typeindex>

namespace vkl
{
	class ShaderData;

	class VKL_EXPORT PipelineDescription
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

		void setPrimitiveTopology(VkPrimitiveTopology topology);

		//for use by pipeline
		std::span<const ShaderDescription> shaders() const;
		std::span<const VertexAttributeDescription> attributes() const;
		std::span<const UniformDescription> uniforms() const;
		const PushConstantDescription& pushConstant() const;
		std::span<const TextureDescription> textures() const;
		VkPrimitiveTopology primitiveTopology() const;

	private:
		std::vector< ShaderDescription> _shaders;
		std::vector<VertexAttributeDescription> _attributes;
		std::vector<UniformDescription> _uniforms;
		PushConstantDescription _pushConstant;
		std::vector<TextureDescription> _textures;
		VkPrimitiveTopology _primitiveTopology{ VK_PRIMITIVE_TOPOLOGY_POINT_LIST };
	};
	/*****************************************************************************************************************/

	class VKL_EXPORT Pipeline
	{
	public:
		Pipeline() = delete;
		Pipeline(const Device& device, const SwapChain& swapChain, const PipelineDescription& description, const RenderPass& renderPass, std::type_index typeIndex);
		Pipeline(const Pipeline&) = delete;
		Pipeline(Pipeline&&) noexcept = default;
		Pipeline& operator=(Pipeline&&) noexcept = default;
		Pipeline& operator=(const Pipeline&) = delete;


		VkPipeline handle() const;
		VkDescriptorSetLayout descriptorSetLayoutHandle() const;
		VkPipelineLayout pipelineLayoutHandle() const;

		std::type_index type() const;

		void cleanUp(const Device& device);

	private:

		void createDescriptorSetLayout(const Device& device, const SwapChain& swapChain, const PipelineDescription& description, const RenderPass& renderPass);
		void createPipeline(const Device& device, const SwapChain& swapChain, const PipelineDescription& description, const RenderPass& renderPass);

		VkPipeline _pipeline{ VK_NULL_HANDLE };
		VkPipelineLayout _pipelineLayout{ VK_NULL_HANDLE };
		VkDescriptorSetLayout _descriptorSetLayout{ VK_NULL_HANDLE };
		std::type_index _type;
	};
	/*****************************************************************************************************************/

}