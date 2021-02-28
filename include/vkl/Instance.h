#pragma once
#include <vkl/Common.h>
#include <span>

namespace vkl
{
	class VKL_EXPORT Instance
	{
	public:
		Instance(const char* appName, bool validation = false);
		Instance() = delete;
		~Instance();

		bool isValid() const;

		VkInstance handle() const;

		std::span<const char* const> getLayers() const;

		void cleanUp();
	private:
		VkInstance _instance{ VK_NULL_HANDLE };
		VkDebugUtilsMessengerEXT _debugMessenger{ VK_NULL_HANDLE };

	};
}