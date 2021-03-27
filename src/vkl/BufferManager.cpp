#include <vkl/BufferManager.h>

#include <vkl/Device.h>
#include <vkl/SwapChain.h>
#include <vkl/VertexBuffer.h>
#include <vkl/IndexBuffer.h>
#include <vkl/TextureBuffer.h>
#include <vkl/UniformBuffer.h>

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
		for (auto&& tex : _textureBuffers)
			tex->update(device, swapChain);

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
	std::shared_ptr<TextureBuffer> BufferManager::createTextureBuffer(const Device& device, const SwapChain& swapChain, void* imageData, size_t width, size_t height, size_t components, const TextureOptions& options)
	{
		auto newOne = std::make_shared<TextureBuffer>(device, swapChain, imageData, width, height, components, options);
		_textureBuffers.emplace_back(newOne);
		return newOne;
	}
	std::shared_ptr<UniformBuffer> BufferManager::createUniformBuffer(const Device& device, const SwapChain& swapChain)
	{
		auto newOne = std::make_shared<UniformBuffer>(device, swapChain);
		_uniformBuffers.emplace_back(newOne);
		return newOne;
	}
	void BufferManager::cleanUp(const Device& device)
	{
		for (auto&& ib : _indexBuffers)
			ib->cleanUp(device);
		for (auto&& vbo : _vertexBuffers)
			vbo->cleanUp(device);
		for (auto&& ubo : _uniformBuffers)
			ubo->cleanUp(device);
		for (auto&& tex : _textureBuffers)
			tex->cleanUp(device);

		_indexBuffers.clear();
		_vertexBuffers.clear();
		_uniformBuffers.clear();
		_textureBuffers.clear();
	}
}