#pragma once
#include <vkl/Common.h>

namespace vkl
{
		class VKL_EXPORT TextureBuffer
		{
		public:
			TextureBuffer() = delete;
			TextureBuffer(const Device& device, const SwapChain& swapChain, void* imageData, size_t width, size_t height, size_t components);
			~TextureBuffer();

			void* data() const;
			size_t width() const;
			size_t height() const;
			size_t components() const;


			bool isValid(size_t frameIndex) const;

			VkImage handle() const;
			VkImageView imageViewHandle() const;
			VkSampler samplerHandle() const;

			void cleanUp(const Device& device);
			void update(const Device& device, const SwapChain& swapChain);

		private:
			void* _data{ nullptr };
			size_t _width{ 0 };
			size_t _height{ 0 };
			size_t _components{ 0 };

			size_t _createFrame{ 0 };
			VkBuffer _stagingBuffer{ VK_NULL_HANDLE };
			VmaAllocation _stagingMemory{ };
			bool _seenFirstUpdate{ false };

			VkImage _image{ VK_NULL_HANDLE };
			VmaAllocation _memory{  };
			VkImageView _imageView{ VK_NULL_HANDLE };
			VkSampler _sampler{ VK_NULL_HANDLE };
		};
}