#pragma once
#include <vkl/Common.h>
#include <memory>
namespace vkl
{
	class VKL_EXPORT DrawCall
	{
	public:
		DrawCall() = default;
		~DrawCall() = default;

		std::shared_ptr<const IndexBuffer> indexBuffer() const;
		size_t count() const;
		size_t offset() const;

		void setIndexBuffer(std::shared_ptr<const IndexBuffer> buffer);
		void setCount(size_t count);
		void setOffset(size_t offset);

	private:
		std::shared_ptr<const IndexBuffer> _indexBuffer;
		size_t _offset{ 0 };
		size_t _count{ 0 };
	};
}