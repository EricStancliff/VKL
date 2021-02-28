#include <vkl/TextureBuffer.h>
#include <cmath>
#include <cassert>

#include <vkl/Device.h>
#include <vkl/SwapChain.h>

namespace vkl
{


	TextureBuffer::TextureBuffer(const Device& device, const SwapChain& swapChain, void* imageData, size_t width, size_t height, size_t components)
	{
		_data = imageData;
		_width = width;
		_height = height;
		_components = components;
		assert(components == 4);//all we support right now

		VkDeviceSize imageSize = _width * _height * _components;
		auto mipLevels = static_cast<uint32_t>(std::floor(std::log2(std::max(_width, _height)))) + 1;

		VkBufferCreateInfo bufferInfo{};
		bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		bufferInfo.size = imageSize;
		bufferInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
		bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

		VkBuffer stagingBuffer{};
		VmaAllocation stagingBufferMemory{};
		VmaAllocationCreateInfo createInfo{};
		VmaAllocationInfo mappingInfo{};
		createInfo.requiredFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
		createInfo.usage = VMA_MEMORY_USAGE_CPU_TO_GPU;
		createInfo.flags = VMA_ALLOCATION_CREATE_MAPPED_BIT;


		vmaCreateBuffer(device.allocatorHandle(), &bufferInfo, &createInfo, &stagingBuffer, &stagingBufferMemory, &mappingInfo);

		memcpy(mappingInfo.pMappedData, _data, static_cast<size_t>(imageSize));

		createImage(device, (uint32_t)_width, (uint32_t)_height, mipLevels, VK_SAMPLE_COUNT_1_BIT, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_TILING_OPTIMAL,
			VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, _image, _memory);

		transitionImageLayout(device, swapChain, swapChain.frame(), _image, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, mipLevels);
		copyBufferToImage(device, swapChain, swapChain.frame(), stagingBuffer, _image, static_cast<uint32_t>(_width), static_cast<uint32_t>(_height));

		_createFrame = swapChain.frame();
		_stagingBuffer = stagingBuffer;
		_stagingMemory = stagingBufferMemory;

		generateMipmaps(device, swapChain, swapChain.frame(), _image, VK_FORMAT_R8G8B8A8_SRGB, (uint32_t)_width, (uint32_t)_height, mipLevels);

		_imageView = createImageView(device, _image, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_ASPECT_COLOR_BIT, mipLevels);

		VkPhysicalDeviceProperties properties{};
		vkGetPhysicalDeviceProperties(device.physicalDeviceHandle(), &properties);

		VkSamplerCreateInfo samplerInfo{};
		samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
		samplerInfo.magFilter = VK_FILTER_LINEAR;
		samplerInfo.minFilter = VK_FILTER_LINEAR;
		samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		samplerInfo.anisotropyEnable = VK_TRUE;
		samplerInfo.maxAnisotropy = properties.limits.maxSamplerAnisotropy;
		samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
		samplerInfo.unnormalizedCoordinates = VK_FALSE;
		samplerInfo.compareEnable = VK_FALSE;
		samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
		samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
		samplerInfo.minLod = 0.0f;
		samplerInfo.maxLod = static_cast<float>(mipLevels);
		samplerInfo.mipLodBias = 0.0f;

		if (vkCreateSampler(device.handle(), &samplerInfo, nullptr, &_sampler) != VK_SUCCESS) {
			throw std::runtime_error("failed to create texture sampler!");
		}
	}

	TextureBuffer::~TextureBuffer()
	{
	}

	void* TextureBuffer::data() const
	{
		return _data;
	}

	size_t TextureBuffer::width() const
	{
		return _height;
	}

	size_t TextureBuffer::height() const
	{
		return _width;
	}

	size_t TextureBuffer::components() const
	{
		return _components;
	}

	bool TextureBuffer::isValid(size_t frameIndex) const
	{
		return _image != VK_NULL_HANDLE;
	}
	VkImage TextureBuffer::handle() const
	{
		return _image;
	}
	VkImageView TextureBuffer::imageViewHandle() const
	{
		return _imageView;
	}
	VkSampler TextureBuffer::samplerHandle() const
	{
		return _sampler;
	}
	void TextureBuffer::cleanUp(const Device& device)
	{
		vkDestroySampler(device.handle(), _sampler, nullptr);
		vkDestroyImageView(device.handle(), _imageView, nullptr);
		vmaDestroyImage(device.allocatorHandle(), _image, _memory);
	}
	void TextureBuffer::update(const Device& device, const SwapChain& swapChain)
	{
		if (!_seenFirstUpdate)
		{
			_seenFirstUpdate = true;
			return;
		}

		if (_stagingBuffer != VK_NULL_HANDLE && _createFrame == swapChain.frame())
		{
			vmaDestroyBuffer(device.allocatorHandle(), _stagingBuffer, _stagingMemory);
			_stagingBuffer = VK_NULL_HANDLE;
			_stagingMemory = {};
		}
	}
}