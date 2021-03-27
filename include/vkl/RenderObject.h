#pragma once
#include <vkl/Common.h>
#include <vkl/Reflect.h>
#include <vkl/Pipeline.h>

namespace vkl
{
	class BufferManager;

	class VKL_EXPORT RenderObjectDescription : public reflect::reflection_data_base
	{
	public:
		PipelineDescription& pipelineDescription() {
			return _pipeline;
		}
		const PipelineDescription& pipelineDescription() const {
			return _pipeline;
		}

		void setAbstract(bool a)
		{
			_abstract = a;
		}
		bool isAbstract() const
		{
			return _abstract;
		}
	private:
		PipelineDescription _pipeline;
		bool _abstract{ false };
	};

	class VKL_EXPORT RenderObject : public reflect::object
	{
		REFLECTED_TYPE_CUSTOM(RenderObject, reflect::object, RenderObjectDescription)
		static void populateReflection(RenderObjectDescription& reflection);
	public:
		RenderObject() = default;
		virtual void init(const Device& device, const SwapChain& swapChain, BufferManager& bufferManager, const PipelineManager& pipelines);

		virtual void recordCommands(const SwapChain& swapChain, const PipelineManager& pipelines, VkCommandBuffer buffer, const VkExtent2D& extent);

		const PipelineDescription& pipelineDescription() const;
	protected:
		void addVBO(const Device& device, const SwapChain& swapChain, std::shared_ptr<const VertexBuffer> vbo, uint32_t binding);
		void addUniform(const Device& device, const SwapChain& swapChain, std::shared_ptr<const UniformBuffer> uniform, uint32_t binding);
		void addTexture(const Device& device, const SwapChain& swapChain, std::shared_ptr<const TextureBuffer>texture, uint32_t binding);
		void addDrawCall(const Device& device, const SwapChain& swapChain, std::shared_ptr<const DrawCall> draw);

		void setPushConstant(std::shared_ptr<const PushConstantBase> pc);

		void reset();

		void initPipeline(const Device& device, const SwapChain& swapChain, BufferManager& bufferManager, const PipelineManager& pipelines);
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