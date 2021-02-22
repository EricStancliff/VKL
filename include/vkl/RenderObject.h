#pragma once
#include <Common.h>
#include <Reflect.h>
#include <Pipeline.h>

namespace vkl
{
	class BufferManager;

	class RenderObjectDescription : public reflect::reflection_data_base
	{
	public:
		PipelineDescription& pipelineDescription() {
			return _pipeline;
		}
		const PipelineDescription& pipelineDescription() const {
			return _pipeline;
		}
	private:
		PipelineDescription _pipeline;

	};

	class RenderObject : public reflect::object
	{
		REFLECTED_TYPE_CUSTOM(RenderObject, reflect::object, RenderObjectDescription)
		static void populateReflection(RenderObjectDescription& reflection);
	public:
		RenderObject() = delete;
		RenderObject(const Device& device, const SwapChain& swapChain, const BufferManager& bufferManager, const PipelineManager& pipelines);

		virtual void recordCommands(const SwapChain& swapChain, const PipelineManager& pipelines, VkCommandBuffer buffer);

		const PipelineDescription& pipelineDescription() const;
	protected:
		void addVBO(const Device& device, const SwapChain& swapChain, std::shared_ptr<VertexBuffer> vbo, uint32_t binding);
		void addUniform(const Device& device, const SwapChain& swapChain, std::shared_ptr<UniformBuffer> uniform, uint32_t binding);
		void addTexture(const Device& device, const SwapChain& swapChain, std::shared_ptr<TextureBuffer>texture, uint32_t binding);
		void addDrawCall(const Device& device, const SwapChain& swapChain, std::shared_ptr<DrawCall> draw);

	private:
		std::vector<std::pair<uint32_t, std::shared_ptr<VertexBuffer>>> _vbos;
		std::vector<std::pair<uint32_t, std::shared_ptr<UniformBuffer>>> _uniforms;
		std::vector<std::pair<uint32_t, std::shared_ptr<TextureBuffer>>> _textures;
		std::vector<std::shared_ptr<DrawCall>> _drawCalls;

		std::vector<VkDescriptorSet> _descriptorSets;
	};
}