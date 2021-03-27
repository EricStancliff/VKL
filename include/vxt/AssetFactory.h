#pragma once

#include <vxt/VXT_EXPORT.h>
#include <vkl/Common.h>
#include <filesystem>

namespace vxt
{
	class VXT_EXPORT FileAsset
	{
	public:
		virtual bool processFile(std::string_view file) = 0;
	};

	class VXT_EXPORT DeviceAsset
	{
	public:
		virtual bool buildAsset(std::shared_ptr<const FileAsset> fileAsset, const vkl::Device& device, const vkl::SwapChain& swapChain, vkl::BufferManager& bufferManager) = 0;
	};

	class VXT_EXPORT AssetDriver
	{
	public:
		virtual bool supportsAsset(std::string_view file) const = 0;
		virtual std::string_view driverName() const = 0;

		virtual std::shared_ptr<const FileAsset> buildFileAsset(std::string_view file) const = 0;
		virtual std::shared_ptr<const DeviceAsset> buildDeviceAsset(std::shared_ptr<const FileAsset> fileAsset, const vkl::Device& device, const vkl::SwapChain& swapChain, vkl::BufferManager& bufferManager) const = 0;
	};

	class VXT_EXPORT AssetFactory
	{
	public:
		~AssetFactory() = default;
		AssetFactory(const AssetFactory&) = delete;
		AssetFactory(AssetFactory&&) noexcept = default;
		AssetFactory& operator=(AssetFactory&&) noexcept = default;
		AssetFactory& operator=(const AssetFactory&) = delete;

		static AssetFactory& instance();

		std::shared_ptr<const FileAsset> fileAsset(std::string_view file, std::string_view driverHint = "");

		std::shared_ptr<const DeviceAsset> deviceAsset(std::string_view file, const vkl::Device& device, const vkl::SwapChain& swapChain, vkl::BufferManager& bufferManager, std::string_view driverHint = "");

		template <typename T>
		std::shared_ptr<const T> deviceAsset(std::string_view file, const vkl::Device& device, const vkl::SwapChain& swapChain, vkl::BufferManager& bufferManager, std::string_view driverHint = "")
		{
			return std::dynamic_pointer_cast<const T>(deviceAsset(file, device, swapChain, bufferManager, driverHint));
		}

		bool registerDriver(std::shared_ptr<const AssetDriver> driver);

		void addSearchPath(const std::filesystem::path& path, bool recurse = false);
		bool resolveAbsoloutePath(std::string_view input, std::filesystem::path& output) const;

	private:
		AssetFactory() = default;

		std::shared_ptr<const AssetDriver> findDriverForFile(std::string_view file);
		std::shared_ptr<const AssetDriver> findDriverFromName(std::string_view file);

		std::vector<std::pair<std::string_view, std::shared_ptr<const FileAsset>>> _fileAssets;
		std::vector<std::pair<std::string_view, std::shared_ptr<const AssetDriver>>> _drivers;

		std::vector<std::pair<std::filesystem::path, bool>> _searchPaths;
	};
}

#define ASSET_DRIVER \
static bool driver_initialized();\
static bool _driver_initialized;

#define REGISTER_DRIVER(Type) \
bool Type::driver_initialized(){\
	static bool initialized = vxt::AssetFactory::instance().registerDriver(std::make_shared<Type>());\
	return initialized;\
}\
bool Type::_driver_initialized = Type::driver_initialized();