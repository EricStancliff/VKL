#pragma once
#include <Common.h>

namespace vkl
{
	class VKL_EXPORT IndexBuffer
	{
	public:
		IndexBuffer() = delete;
		IndexBuffer(const Device& device, const SwapChain& swapChain);

		void setData(std::span<const uint32_t> indices);
		void update(const Device& device, const SwapChain& swapChain);

		void* data() const;
		size_t elementSize() const;
		size_t count() const;

		VkBuffer handle(size_t frameIndex) const;

		bool isValid(size_t frameIndex) const;

	private:
		void* _data{ nullptr };
		size_t _elementSize{ 0 };
		size_t _count{ 0 };
		size_t _oldCount{ 0 };

		struct BufferInfo
		{
			VkBuffer _buffer{ VK_NULL_HANDLE };
			VmaAllocation _memory{ nullptr };
			void* _mapped{ nullptr };
		};

		std::vector<BufferInfo> _buffers;

		int _dirty{ -1 };

	};
}