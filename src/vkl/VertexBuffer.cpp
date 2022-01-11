#include <vkl/VertexBuffer.h>

#include <vkl/Device.h>
#include <vkl/SwapChain.h>

#include <cstring>

namespace vkl
{
    VertexBuffer::VertexBuffer(const Device& device, const SwapChain& swapChain)
    {
        _buffers.resize(swapChain.framesInFlight());
        _dirties.resize(swapChain.framesInFlight());
    }

    void VertexBuffer::setData(void* data, size_t elementSize, size_t count)
    {
        _data = data;
        _elementSize = elementSize;
        _oldCount = _count;
        _count = count;
        for (auto&& dirty : _dirties)
            dirty = true;
    }

    void VertexBuffer::update(const Device& device, const SwapChain& swapChain)
    {
        if (!_dirties[swapChain.frame()])
            return;

        _dirties[swapChain.frame()] = false;

        auto& current = _buffers[swapChain.frame()];

        if (_oldCount != _count && isValid(swapChain.frame()))
        {
            //destroy buffer
            if (current._buffer && current._memory)
            {
                vmaDestroyBuffer(device.allocatorHandle(), current._buffer, current._memory);
                current._memory = nullptr;
                current._buffer = VK_NULL_HANDLE;
                current._mapped = nullptr;
            }
        }

        if (_count == 0)
            return;


        if (!isValid(swapChain.frame()))
        {
            //create buffer
            VkBufferCreateInfo bufferInfo{};
            bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
            bufferInfo.size = _elementSize * _count;
            bufferInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
            bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

            VmaAllocationCreateInfo createAllocation{};
            createAllocation.usage = VMA_MEMORY_USAGE_CPU_TO_GPU;
            createAllocation.requiredFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
            createAllocation.flags = VMA_ALLOCATION_CREATE_MAPPED_BIT;

            VmaAllocationInfo info{};
            vmaCreateBuffer(device.allocatorHandle(), &bufferInfo, &createAllocation, &current._buffer, &current._memory, &info);

            current._mapped = info.pMappedData;
        }

        //populate buffer
        if (current._mapped && _data && _count)
            memcpy(current._mapped, _data, _elementSize * _count);

    }

    void* VertexBuffer::data() const
    {
        return _data;
    }

    size_t VertexBuffer::elementSize() const
    {
        return _elementSize;
    }

    size_t VertexBuffer::count() const
    {
        return _count;
    }

    VkBuffer VertexBuffer::handle(size_t frameIndex) const
    {
        return _buffers[frameIndex]._buffer;
    }

    bool VertexBuffer::isValid(size_t frameIndex) const
    {
        return _buffers[frameIndex]._buffer != VK_NULL_HANDLE;
    }

    void VertexBuffer::cleanUp(const Device& device)
    {
        for (auto&& buffer : _buffers)
        {
            vmaDestroyBuffer(device.allocatorHandle(), buffer._buffer, buffer._memory);
            buffer._memory = nullptr;
            buffer._buffer = VK_NULL_HANDLE;
            buffer._mapped = nullptr;
        }
        _buffers.clear();
    }

}