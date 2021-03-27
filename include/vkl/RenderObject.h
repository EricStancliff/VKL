#pragma once
#include <vkl/Common.h>
#include <vkl/Pipeline.h>

namespace vkl
{
	class BufferManager;
	class PipelineManager;

	class VKL_EXPORT RenderObject
	{
	public:
		RenderObject() = default;

		virtual void recordCommands(const SwapChain& swapChain, const PipelineManager& pipelines, VkCommandBuffer buffer, const VkExtent2D& extent);
		virtual void updateDescriptors(const Device& device, const SwapChain& swapChain, const PipelineManager& pipelines);

		std::shared_ptr<const PipelineDescription> pipelineDescription() const;
	protected:
		void addVBO(std::shared_ptr<const VertexBuffer> vbo, uint32_t binding);
		void addUniform(std::shared_ptr<const UniformBuffer> uniform, uint32_t binding);
		void addTexture(std::shared_ptr<const TextureBuffer>texture, uint32_t binding);
		void addDrawCall(std::shared_ptr<const DrawCall> draw);

		void setPushConstant(std::shared_ptr<const PushConstantBase> pc);

		void reset();

		void initPipeline(const Device& device, const SwapChain& swapChain,const PipelineManager& pipelines);
	private:
		std::vector<std::pair<uint32_t, std::shared_ptr<const VertexBuffer>>> _vbos;
		std::vector<std::pair<uint32_t, std::shared_ptr<const UniformBuffer>>> _uniforms;
		std::vector<std::pair<uint32_t, std::shared_ptr<const TextureBuffer>>> _textures;
		std::vector<std::shared_ptr<const DrawCall>> _drawCalls;

		std::shared_ptr<const PushConstantBase> _pushConstant;

		std::vector<VkDescriptorSet> _descriptorSets;
		bool m_init{ false };
	};
}