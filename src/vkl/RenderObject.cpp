#include <RenderObject.h>
#include <Reflect.h>

#include <SwapChain.h>
#include <Device.h>
#include <VertexBuffer.h>
#include <UniformBuffer.h>
#include <TextureBuffer.h>
#include <DrawCall.h>
#include <IndexBuffer.h>

namespace vkl
{
	void RenderObject::populateReflection(RenderObjectDescription& reflection)
	{
		reflection.setAbstract(true);
	}


	RenderObject::RenderObject(const Device& device, const SwapChain& swapChain, BufferManager& bufferManager, const PipelineManager& pipelines)
	{
	}

	void RenderObject::recordCommands(const SwapChain& swapChain, const PipelineManager& pipelines, VkCommandBuffer buffer, const VkExtent2D& extent)
	{
		const Pipeline* pipeline = pipelines.pipelineForType(this->reflect().index);
		if (!pipeline)
			return;

		vkCmdBindPipeline(buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline->handle());

		std::vector<VkBuffer> vbos;
		std::vector<VkDeviceSize> offsets;
		for (auto&& vbo : _vbos)
		{
			vbos.push_back(vbo.second->handle(swapChain.frame()));
			offsets.push_back(0);
		}

		if(!vbos.empty())
			vkCmdBindVertexBuffers(buffer, 0, (uint32_t)vbos.size(), vbos.data(), offsets.data());

		vkCmdBindDescriptorSets(buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline->pipelineLayoutHandle(), 0, 1, &_descriptorSets[swapChain.frame()], 0, nullptr);

		for (auto&& dc : _drawCalls)
		{
			//TODO - let draw calls scissor

			VkRect2D scissor{};

			scissor.offset = { 0, 0 };
			scissor.extent = extent;

			vkCmdSetScissor(buffer, 0, 1, &scissor);

			if (dc->indexBuffer())
			{
				vkCmdBindIndexBuffer(buffer, dc->indexBuffer()->handle(swapChain.frame()), 0, VK_INDEX_TYPE_UINT32);
				vkCmdDrawIndexed(buffer, (uint32_t)dc->count(), 1, 0, (uint32_t)dc->offset(), 0);
			}
			else
			{
				vkCmdDraw(buffer, (uint32_t)dc->count(), 0, (uint32_t)dc->offset(), 0);
			}
		}

	}

	const PipelineDescription& RenderObject::pipelineDescription() const
	{
		return static_cast<const RenderObjectDescription*>(reflect().data)->pipelineDescription();
	}

	void RenderObject::addVBO(const Device& device, const SwapChain& swapChain, std::shared_ptr<VertexBuffer> vbo, uint32_t binding)
	{
		_vbos.push_back({ binding, vbo });
		std::sort(_vbos.begin(), _vbos.end(), [](const auto& lhs, const auto& rhs) {
			return lhs.first < rhs.first;
			});
	}
	void RenderObject::addUniform(const Device& device, const SwapChain& swapChain, std::shared_ptr<UniformBuffer> uniform, uint32_t binding)
	{
		for (int i = 0; i < swapChain.framesInFlight(); ++i)
		{
			VkDescriptorBufferInfo bufferInfo{};
			bufferInfo.buffer = uniform->handle(i);
			bufferInfo.offset = 0;
			bufferInfo.range = uniform->size();

			VkWriteDescriptorSet descriptorWrite{};
			descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			descriptorWrite.dstSet = _descriptorSets[i];
			descriptorWrite.dstBinding = binding;
			descriptorWrite.dstArrayElement = 0;
			descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			descriptorWrite.descriptorCount = 1;
			descriptorWrite.pBufferInfo = &bufferInfo;

			vkUpdateDescriptorSets(device.handle(), 1, &descriptorWrite, 0, nullptr);
		}
		
		_uniforms.push_back({ binding, uniform });
		std::sort(_uniforms.begin(), _uniforms.end(), [](const auto& lhs, const auto& rhs) {
			return lhs.first < rhs.first;
			});

	}
	void RenderObject::addTexture(const Device& device, const SwapChain& swapChain, std::shared_ptr<TextureBuffer> texture, uint32_t binding)
	{
		//TODO
		_textures.push_back({ binding, texture });
		std::sort(_textures.begin(), _textures.end(), [](const auto& lhs, const auto& rhs) {
			return lhs.first < rhs.first;
			});

	}
	void RenderObject::addDrawCall(const Device& device, const SwapChain& swapChain, std::shared_ptr<DrawCall> draw)
	{
		_drawCalls.push_back(draw);
	}
	void RenderObject::init(const Device& device, const SwapChain& swapChain, BufferManager& bufferManager, const PipelineManager& pipelines)
	{
		auto pipeline = pipelines.pipelineForType(reflect::reflect(this).index);
		if (!pipeline)
		{
			throw std::runtime_error("Error");
			return;
		}

		_descriptorSets.resize(swapChain.framesInFlight());
		std::vector<VkDescriptorSetLayout> layouts(swapChain.framesInFlight(), pipeline->descriptorSetLayoutHandle());

		VkDescriptorSetAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		allocInfo.descriptorPool = pipeline->descriptorPoolHandle();
		allocInfo.descriptorSetCount = static_cast<uint32_t>(swapChain.framesInFlight());
		allocInfo.pSetLayouts = layouts.data();

		if (vkAllocateDescriptorSets(device.handle(), &allocInfo, _descriptorSets.data()) != VK_SUCCESS) {
			throw std::runtime_error("Error");
		}

	}
}

IMPL_REFLECTION(vkl::RenderObject)