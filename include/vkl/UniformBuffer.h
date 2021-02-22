#pragma once
#include <Common.h>

namespace vkl
{
	class UniformBuffer
	{
		public:
			UniformBuffer() = delete;
			UniformBuffer(const Device& device, const SwapChain& swapChain);

			void setData(void* data, size_t size);
			void update(const Device& device, const SwapChain& swapChain);

			void* data() const;
			size_t size() const;

			VkBuffer handle(size_t frameIndex) const;

			bool isValid(size_t frameIndex) const;

		private:
			void* _data{ nullptr };
			size_t _size{ 0 };

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
		class TypedUniform : public UniformBuffer
		{
		public:
			TypedUniform() = delete;
			TypedUniform(const Device& device, uint32_t binding) : UniformBuffer(device, binding) {}

			void setData(const T& data)
			{
				_ourData = data;
				setData((void*)&_ourData, sizeof(T));
			}

		private:
			T _ourData;
	};
}