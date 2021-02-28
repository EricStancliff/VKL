#include <vkl/IndexBuffer.h>
#include <vkl/Device.h>
#include <vkl/SwapChain.h>

namespace vkl
{

    IndexBuffer::IndexBuffer(const Device& device, const SwapChain& swapChain)
    {
        _buffers.resize(swapChain.framesInFlight());
    }

    void IndexBuffer::setData(std::span<const uint32_t> indices)
    {
        _data = (void*)indices.data();
        _elementSize = sizeof(uint32_t);
        _oldCount = _count;
        _count = indices.size();
        _dirty = std::numeric_limits<int>::max();
    }

    void IndexBuffer::update(const Device& device, const SwapChain& swapChain)
    {
        if (_dirty == swapChain.frame())
            _dirty = -1;

        if (_dirty == -1)
            return;

        if (_dirty == std::numeric_limits<int>::max())
            _dirty = (int)swapChain.frame();

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
            bufferInfo.usage = VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
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

    void* IndexBuffer::data() const
    {
        return _data;
    }

    size_t IndexBuffer::elementSize() const
    {
        return _elementSize;
    }

    size_t IndexBuffer::count() const
    {
        return _count;
    }

    VkBuffer IndexBuffer::handle(size_t frameIndex) const
    {
        return _buffers[frameIndex]._buffer;
    }

    bool IndexBuffer::isValid(size_t frameIndex) const
    {
        return _buffers[frameIndex]._buffer != VK_NULL_HANDLE;
    }
    void IndexBuffer::cleanUp(const Device& device)
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