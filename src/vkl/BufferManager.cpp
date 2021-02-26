#include <BufferManager.h>

#include <Device.h>
#include <SwapChain.h>
#include <VertexBuffer.h>
#include <IndexBuffer.h>
#include <TextureBuffer.h>
#include <UniformBuffer.h>

namespace vkl
{
	BufferManager::BufferManager(const Device& device, const SwapChain& swapChain)
	{
	}
	
	BufferManager::~BufferManager()
	{
	}

	void BufferManager::update(const Device& device, const SwapChain& swapChain)
	{
		for (auto&& ib : _indexBuffers)
			ib->update(device, swapChain);
		for (auto&& vbo : _vertexBuffers)
			vbo->update(device, swapChain);
		for (auto&& ubo : _uniformBuffers)
			ubo->update(device, swapChain);
	}

	std::shared_ptr<IndexBuffer> BufferManager::createIndexBuffer(const Device& device, const SwapChain& swapChain)
	{
		auto newOne = std::make_shared<IndexBuffer>(device, swapChain);
		_indexBuffers.emplace_back(newOne);
		return newOne;
	}
	std::shared_ptr<VertexBuffer> BufferManager::createVertexBuffer(const Device& device, const SwapChain& swapChain)
	{
		auto newOne = std::make_shared<VertexBuffer>(device, swapChain);
		_vertexBuffers.emplace_back(newOne);
		return newOne;
	}
	std::shared_ptr<TextureBuffer> BufferManager::createTextureBuffer(const Device& device, const SwapChain& swapChain, void* imageData, size_t width, size_t height, size_t components)
	{
		return std::make_shared<TextureBuffer>(device, swapChain, imageData, width, height, components);
	}
	std::shared_ptr<UniformBuffer> BufferManager::createUniformBuffer(const Device& device, const SwapChain& swapChain)
	{
		auto newOne = std::make_shared<UniformBuffer>(device, swapChain);
		_uniformBuffers.emplace_back(newOne);
		return newOne;
	}
}