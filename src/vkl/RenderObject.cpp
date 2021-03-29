#include <vkl/RenderObject.h>

#include <vkl/SwapChain.h>
#include <vkl/Device.h>
#include <vkl/VertexBuffer.h>
#include <vkl/UniformBuffer.h>
#include <vkl/TextureBuffer.h>
#include <vkl/DrawCall.h>
#include <vkl/IndexBuffer.h>
#include <vkl/PipelineFactory.h>

#include <array>

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

		if (_pushConstant)
			vkCmdPushConstants(buffer, pipeline->pipelineLayoutHandle(), VK_SHADER_STAGE_VERTEX_BIT, 0, (uint32_t)_pushConstant->size(), _pushConstant->data());

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

	void RenderObject::updateDescriptors(const Device& device, const SwapChain& swapChain, const PipelineManager& pipelines)
	{
		if (!m_init)
			initPipeline(device, swapChain, pipelines);

		for (auto&& uniform : _uniforms)
		{
			if (uniform.second->isValid(swapChain.frame()))
			{
				VkDescriptorBufferInfo bufferInfo{};
				bufferInfo.buffer = uniform.second->handle(swapChain.frame());
				bufferInfo.offset = 0;
				bufferInfo.range = uniform.second->size();

				VkWriteDescriptorSet descriptorWrite{};
				descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
				descriptorWrite.dstSet = _descriptorSets[swapChain.frame()];
				descriptorWrite.dstBinding = uniform.first;
				descriptorWrite.dstArrayElement = 0;
				descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
				descriptorWrite.descriptorCount = 1;
				descriptorWrite.pBufferInfo = &bufferInfo;

				vkUpdateDescriptorSets(device.handle(), 1, &descriptorWrite, 0, nullptr);
			}
		}
		for (auto&& tex : _textures)
		{
			if (tex.second->isValid(swapChain.frame()))
			{
				VkDescriptorImageInfo imageInfo{};
				imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
				imageInfo.imageView = tex.second->imageViewHandle();
				imageInfo.sampler = tex.second->samplerHandle();

				VkWriteDescriptorSet descriptorWrite{};
				descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
				descriptorWrite.dstSet = _descriptorSets[swapChain.frame()];
				descriptorWrite.dstBinding = tex.first;
				descriptorWrite.dstArrayElement = 0;
				descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
				descriptorWrite.descriptorCount = 1;
				descriptorWrite.pImageInfo = &imageInfo;

				vkUpdateDescriptorSets(device.handle(), 1, &descriptorWrite, 0, nullptr);
			}
		}
	}

	std::shared_ptr<const PipelineDescription> RenderObject::pipelineDescription() const
	{
		return PipelineMetaFactory::instance().description(std::type_index(typeid(*this)));
	}

	void RenderObject::cleanUp(const Device& device)
	{
		vkDestroyDescriptorPool(device.handle(), _descriptorPool, nullptr);
	}

	void RenderObject::addVBO(std::shared_ptr<const VertexBuffer> vbo, uint32_t binding)
	{
		_vbos.push_back({ binding, vbo });
		std::sort(_vbos.begin(), _vbos.end(), [](const auto& lhs, const auto& rhs) {
			return lhs.first < rhs.first;
			});
	}
	void RenderObject::addUniform(std::shared_ptr<const UniformBuffer> uniform, uint32_t binding)
	{
		_uniforms.push_back({ binding, uniform });
		std::sort(_uniforms.begin(), _uniforms.end(), [](const auto& lhs, const auto& rhs) {
			return lhs.first < rhs.first;
			});

	}
	void RenderObject::addTexture(std::shared_ptr<const TextureBuffer> texture, uint32_t binding)
	{
		_textures.push_back({ binding, texture });
		std::sort(_textures.begin(), _textures.end(), [](const auto& lhs, const auto& rhs) {
			return lhs.first < rhs.first;
			});

	}
	void RenderObject::addDrawCall(std::shared_ptr<const DrawCall> draw)
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

	void RenderObject::initPipeline(const Device& device, const SwapChain& swapChain,const PipelineManager& pipelines)
	{
		const Pipeline* pipeline = pipelines.pipelineForType(std::type_index(typeid(*this)));
		if (!pipeline)
		{
			throw std::runtime_error("Error");
			return;
		}

		auto description = PipelineMetaFactory::instance().description(std::type_index(typeid(*this)));
		if (!description)
		{
			throw std::runtime_error("Error");
			return;
		}

		//Descriptor Pool
		std::array<VkDescriptorPoolSize, 2> poolSizes{};
		poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		poolSizes[0].descriptorCount = static_cast<uint32_t>(swapChain.framesInFlight()) * std::max(1u, (uint32_t)description->uniforms().size());
		poolSizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		poolSizes[1].descriptorCount = static_cast<uint32_t>(swapChain.framesInFlight()) * std::max(1u, (uint32_t)description->textures().size());

		VkDescriptorPoolCreateInfo poolInfo{};
		poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
		poolInfo.pPoolSizes = poolSizes.data();
		poolInfo.maxSets = static_cast<uint32_t>(swapChain.framesInFlight());

		if (vkCreateDescriptorPool(device.handle(), &poolInfo, nullptr, &_descriptorPool) != VK_SUCCESS) {
			throw std::runtime_error("Error");
		}


		_descriptorSets.resize(swapChain.framesInFlight());
		std::vector<VkDescriptorSetLayout> layouts(swapChain.framesInFlight(), pipeline->descriptorSetLayoutHandle());

		VkDescriptorSetAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		allocInfo.descriptorPool = _descriptorPool;
		allocInfo.descriptorSetCount = static_cast<uint32_t>(swapChain.framesInFlight());
		allocInfo.pSetLayouts = layouts.data();

		if (vkAllocateDescriptorSets(device.handle(), &allocInfo, _descriptorSets.data()) != VK_SUCCESS) {
			throw std::runtime_error("Error");
		}
		m_init = true;
	}
}
