#pragma once
#include <vkl/Common.h>
#include <memory>
#include <vkl/TextureBuffer.h>
#include <vkl/UniformBuffer.h>

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
		std::shared_ptr<TextureBuffer> createTextureBuffer(const Device& device, const SwapChain& swapChain, const void* imageData, size_t width, size_t height, size_t components, const TextureOptions& options = {});

		template <typename T>
		std::shared_ptr<TypedUniform<T>> createTypedUniform(const Device& device, const SwapChain& swapChain)
		{
			auto newOne = std::make_shared<TypedUniform<T>>(device, swapChain);
			_uniformBuffers.emplace_back(newOne);
			return newOne;
		}

		std::shared_ptr<UniformBuffer> createUniformBuffer(const Device& device, const SwapChain& swapChain);

		void cleanUnusedBuffers(const Device& device);

		void cleanUp(const Device& device);

	private:

		std::vector<std::shared_ptr<IndexBuffer>> _indexBuffers;
		std::vector<std::shared_ptr<VertexBuffer>> _vertexBuffers;
		std::vector<std::shared_ptr<TextureBuffer>> _textureBuffers;
		std::vector<std::shared_ptr<UniformBuffer>> _uniformBuffers;
	};
}