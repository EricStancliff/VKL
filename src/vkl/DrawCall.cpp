#include <DrawCall.h>

namespace vkl
{
	std::shared_ptr<const IndexBuffer> DrawCall::indexBuffer() const
	{
		return _indexBuffer;
	}
	size_t DrawCall::count() const
	{
		return _count;
	}
	size_t DrawCall::offset() const
	{
		return _offset;
	}
	void DrawCall::setIndexBuffer(std::shared_ptr<const IndexBuffer> buffer)
	{
		_indexBuffer = buffer;
	}
	void DrawCall::setCount(size_t count)
	{
		_count = count;
	}
	void DrawCall::setOffset(size_t offset)
	{
		_offset = offset;
	}
}