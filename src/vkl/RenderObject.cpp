#include <vkl/RenderObject.h>

#include <vkl/SwapChain.h>
#include <vkl/Device.h>
#include <vkl/VertexBuffer.h>
#include <vkl/UniformBuffer.h>
#include <vkl/TextureBuffer.h>
#include <vkl/DrawCall.h>
#include <vkl/IndexBuffer.h>
#include <vkl/PipelineFactory.h>

namespace vkl
{
	void RenderObject::recordCommands(const SwapChain& swapChain, const PipelineManager& pipelines, VkCommandBuffer buffer, const VkExtent2D& extent)
	{
		const Pipeline* pipeline = pipelines.pipelineForType(std::type_index(typeid(*this)));
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
				vkCmdDraw(buffer, (uint32_t)dc->count(), 1, (uint32_t)dc->offset(), 0);
			}
		}

	}

	std::shared_ptr<const PipelineDescription> RenderObject::pipelineDescription() const
	{
		return PipelineMetaFactory::instance().description(std::type_index(typeid(*this)));
	}

	void RenderObject::addVBO(const Device& device, const SwapChain& swapChain, std::shared_ptr<const VertexBuffer> vbo, uint32_t binding)
	{
		_vbos.push_back({ binding, vbo });
		std::sort(_vbos.begin(), _vbos.end(), [](const auto& lhs, const auto& rhs) {
			return lhs.first < rhs.first;
			});
	}
	void RenderObject::addUniform(const Device& device, const SwapChain& swapChain, std::shared_ptr<const UniformBuffer> uniform, uint32_t binding)
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
	void RenderObject::addTexture(const Device& device, const SwapChain& swapChain, std::shared_ptr<const TextureBuffer> texture, uint32_t binding)
	{
		for (int i = 0; i < swapChain.framesInFlight(); ++i)
		{
			VkDescriptorImageInfo imageInfo{};
			imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
			imageInfo.imageView = texture->imageViewHandle();
			imageInfo.sampler = texture->samplerHandle();

			VkWriteDescriptorSet descriptorWrite{};
			descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			descriptorWrite.dstSet = _descriptorSets[i];
			descriptorWrite.dstBinding = binding;
			descriptorWrite.dstArrayElement = 0;
			descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
			descriptorWrite.descriptorCount = 1;
			descriptorWrite.pImageInfo = &imageInfo;

			vkUpdateDescriptorSets(device.handle(), 1, &descriptorWrite, 0, nullptr);
		}

		//TODO
		_textures.push_back({ binding, texture });
		std::sort(_textures.begin(), _textures.end(), [](const auto& lhs, const auto& rhs) {
			return lhs.first < rhs.first;
			});

	}
	void RenderObject::addDrawCall(const Device& device, const SwapChain& swapChain, std::shared_ptr<const DrawCall> draw)
	{
		_drawCalls.push_back(draw);
	}
	void RenderObject::setPushConstant(std::shared_ptr<const PushConstantBase> pc)
	{
		_pushConstant = pc;
	}
	void RenderObject::reset()
	{
		_textures.clear();
		_pushConstant = nullptr;
		_drawCalls.clear();
		_vbos.clear();
		_uniforms.clear();
	}
	void RenderObject::init(const Device& device, const SwapChain& swapChain, BufferManager& bufferManager, const PipelineManager& pipelines)
	{
		initPipeline(device, swapChain, bufferManager, pipelines);
	}
	void RenderObject::initPipeline(const Device& device, const SwapChain& swapChain, BufferManager& bufferManager, const PipelineManager& pipelines)
	{
		const Pipeline* pipeline = pipelines.pipelineForType(std::type_index(typeid(*this)));
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
