#pragma once
#include <vkl/Common.h>

namespace vkl
{
	class VKL_EXPORT UniformBuffer
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

			void cleanUp(const Device& device);

		private:
			void* _data{ nullptr };
			size_t _size{ 0 };

			void updateBuffer(const Device& device, size_t frame);

			struct BufferInfo
			{
				VkBuffer _buffer{ VK_NULL_HANDLE };
				VmaAllocation _memory{ nullptr };
				void* _mapped{ nullptr };
			};

			std::vector<BufferInfo> _buffers;

			std::vector<bool> _dirties;
	};

		template <typename T>
		class TypedUniform : public UniformBuffer
		{
		public:
			TypedUniform() = delete;
			TypedUniform(const Device& device, const SwapChain& swapChain) : UniformBuffer(device, swapChain) {}

			void setData(const T& data)
			{
				_ourData = data;
				UniformBuffer::setData((void*)&_ourData, sizeof(T));
			}

		private:
			T _ourData{};
	};

		class PushConstantBase
		{
		public:
			virtual const void* data() const = 0;
			virtual size_t size() const = 0;
		};

		template <typename T>
		class PushConstant : public PushConstantBase
		{
		public:
			void setData(const T& data)
			{
				_ourData = data;
			}

			const void* data() const override
			{
				return (const void*)&_ourData;
			}

			size_t size() const override
			{
				return sizeof(T);
			}

			T& typedData()
			{
				return _ourData;
			}
		private:
			T _ourData{};
		};
}