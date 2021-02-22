#pragma once
#include <Common.h>

namespace vkl
{
	class VKL_EXPORT VertexBuffer
	{
	public:
		VertexBuffer() = delete;
		VertexBuffer(const Device& device, const SwapChain& swapChain);

		void setData(void* data, size_t elementSize, size_t count);
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

	template <typename T>
	class TypedVBO : public VertexBuffer
	{
	public:
		TypedVBO() = delete;
		TypedVBO(const Device& device, uint32_t binding) : VertexBuffer(device, binding) {}

		void setData(std::span<const T> data)
		{
			setData(data.data(), sizeof(T), data.size());
		}

	};
}