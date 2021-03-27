#include <vkl/UniformBuffer.h>

#include <vkl/Device.h>
#include <vkl/SwapChain.h>

#include <cstring>

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

        for (int frame = 0; frame < _buffers.size(); ++frame)
        {
            if (!isValid(frame))
                _dirty = std::numeric_limits<int>::lowest();
        }
    }

    void UniformBuffer::update(const Device& device, const SwapChain& swapChain)
    {
        if (_dirty == swapChain.frame())
            _dirty = -1;

        if (_dirty == -1)
            return;

        //This is if we've made a new object, grind to a halt
        if (_dirty == std::numeric_limits<int>::lowest())
        {
            vkDeviceWaitIdle(device.handle());
            for (int frame = 0; frame < _buffers.size(); ++frame)
            {
                updateBuffer(device, frame);
            }
            _dirty = -1;
        }

        if (_dirty == std::numeric_limits<int>::max())
            _dirty = (int)swapChain.frame();

        updateBuffer(device, swapChain.frame());
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

    void UniformBuffer::cleanUp(const Device& device)
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

    void UniformBuffer::updateBuffer(const Device& device, size_t frame)
    {
        auto& current = _buffers[frame];

        if (isValid(frame))
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

        if (!isValid(frame))
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

}