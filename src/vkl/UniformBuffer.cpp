#include <UniformBuffer.h>

#include <Device.h>
#include <SwapChain.h>

namespace vkl
{
    UniformBuffer::UniformBuffer(const Device& device, const SwapChain& swapChain)
    {
        _buffers.resize(swapChain.framesInFlight());
    }

    void UniformBuffer::setData(void* data, size_t size)
    {
        _data = data;
        _size = size;
        _dirty = std::numeric_limits<int>::max();
    }

    void UniformBuffer::update(const Device& device, const SwapChain& swapChain)
    {
        if (_dirty == swapChain.frame())
            _dirty = -1;

        if (_dirty == -1)
            return;

        if (_dirty == std::numeric_limits<int>::max())
            _dirty = (int)swapChain.frame();

        auto& current = _buffers[swapChain.frame()];

        if (isValid(swapChain.frame()))
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

        if (!isValid(swapChain.frame()))
        {
            //create buffer
            VkBufferCreateInfo bufferInfo{};
            bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
            bufferInfo.size = _size;
            bufferInfo.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
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
        if (current._mapped && _data)
            memcpy(current._mapped, _data, _size);

    }

    void* UniformBuffer::data() const
    {
        return _data;
    }

    size_t UniformBuffer::size() const
    {
        return _size;
    }

    VkBuffer UniformBuffer::handle(size_t frameIndex) const
    {
        return _buffers[frameIndex]._buffer;
    }

    bool UniformBuffer::isValid(size_t frameIndex) const
    {
        return _buffers[frameIndex]._buffer != VK_NULL_HANDLE;
    }

}