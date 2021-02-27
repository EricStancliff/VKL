#pragma once
#include <Common.h>
#include <memory>
namespace vkl
{
	class VKL_EXPORT BufferManager
	{
	public:
		BufferManager(const Device& device, const SwapChain& swapChain);
		~BufferManager();

		void update(const Device& device, const SwapChain& swapChain);

		std::shared_ptr<IndexBuffer> createIndexBuffer(const Device& device, const SwapChain& swapChain);
		std::shared_ptr<VertexBuffer> createVertexBuffer(const Device& device, const SwapChain& swapChain);
		std::shared_ptr<TextureBuffer> createTextureBuffer(const Device& device, const SwapChain& swapChain, void* imageData, size_t width, size_t height, size_t components);
		std::shared_ptr<UniformBuffer> createUniformBuffer(const Device& device, const SwapChain& swapChain);

		void cleanUp(const Device& device);

	private:

		std::vector<std::shared_ptr<IndexBuffer>> _indexBuffers;
		std::vector<std::shared_ptr<VertexBuffer>> _vertexBuffers;
		std::vector<std::shared_ptr<TextureBuffer>> _textureBuffers;
		std::vector<std::shared_ptr<UniformBuffer>> _uniformBuffers;
	};
}