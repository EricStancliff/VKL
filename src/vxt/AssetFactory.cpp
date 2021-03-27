#include <vxt/AssetFactory.h>
#include <iostream>
#include <cassert>

namespace vxt
{
	AssetFactory& AssetFactory::instance()
	{
		static AssetFactory factory;
		return factory;
	}

	bool AssetFactory::registerDriver(std::shared_ptr<const AssetDriver> driver)
	{
		auto find = std::lower_bound(_drivers.begin(), _drivers.end(), driver->driverName(), [](const auto& lhs, const auto& rhs) {
			return lhs.first < rhs;
			});
		if (find != _drivers.end() && find->first == driver->driverName())
		{
			//already registered
			std::cerr << "Driver " << driver->driverName() << " already registered";
			assert(false);
			return false;
		}

		_drivers.insert(find, { driver->driverName(), driver });

		return true;
	}

	void AssetFactory::addSearchPath(const std::filesystem::path& path, bool recurse)
	{
		auto find = std::lower_bound(_searchPaths.begin(), _searchPaths.end(), path, [](const auto& lhs, const auto& rhs) {
			return lhs.first < rhs;
			});
		if (find != _searchPaths.end() && find->first == path)
		{
			//already registered
			std::cerr << "Search path " << path << " already registered";
			assert(false);
			return;
		}

		_searchPaths.insert(find, { path, recurse });
	}

	bool AssetFactory::resolveAbsoloutePath(std::string_view input, std::filesystem::path& out) const
	{
		auto path = std::filesystem::path(input);

		std::error_code ec;
		if (std::filesystem::exists(path, ec))
			return true;

		for (auto&& sp : _searchPaths)
		{
			if (sp.second)
			{
				for (auto p : std::filesystem::recursive_directory_iterator(sp.first, ec))
				{
					if (p.path().filename() == path.filename())
					{
						out = p.path();
						return true;
					}
				}
			}
			else
			{
				for (auto p : std::filesystem::directory_iterator(sp.first, ec))
				{
					if (p.path().filename() == path.filename())
					{
						out = p.path();
						return true;
					}
				}
			}
		}

		return false;
	}

	std::shared_ptr<const AssetDriver> AssetFactory::findDriverForFile(std::string_view file)
	{
		for (auto&& driver : _drivers)
			if (driver.second->supportsAsset(file))
				return driver.second;
		return nullptr;
	}
	
	std::shared_ptr<const AssetDriver> AssetFactory::findDriverFromName(std::string_view name)
	{
		auto find = std::lower_bound(_drivers.begin(), _drivers.end(), name, [](const auto& lhs, const auto& rhs) {
			return lhs.first < rhs;
			});
		if (find != _drivers.end() && find->first == name)
		{
			return find->second;
		}
		return nullptr;
	}

	std::shared_ptr<const FileAsset> AssetFactory::fileAsset(std::string_view file, std::string_view driverHint)
	{
		auto find = std::lower_bound(_fileAssets.begin(), _fileAssets.end(), file, [](const auto& lhs, const auto& rhs) {
			return lhs.first < rhs;
			});
		if (find != _fileAssets.end() && find->first == file)
		{
			//in our cache
			return find->second;
		}

		std::shared_ptr<const AssetDriver> driver = nullptr;
		if (!driverHint.empty())
		{
			driver = findDriverFromName(file);
		}

		if (!driver)
		{
			driver = findDriverForFile(file);
		}

		if (!driver)
		{
			std::cerr << "Failed to find driver for " << file << " with hint " << driverHint;
			return nullptr;
		}

		auto asset = driver->buildFileAsset(file);
		if (asset)
		{
			_fileAssets.insert(find, { file, asset });
			return asset;
		}

		std::cerr << "Failed to load asset for " << file << " with driver " << driver->driverName();
		return nullptr;
	}
	
	std::shared_ptr<const DeviceAsset> AssetFactory::deviceAsset(std::string_view file, const vkl::Device& device, const vkl::SwapChain& swapChain, vkl::BufferManager& bufferManager, std::string_view driverHint)
	{
		std::shared_ptr<const AssetDriver> driver = nullptr;
		if (!driverHint.empty())
		{
			driver = findDriverFromName(file);
		}

		if (!driver)
		{
			driver = findDriverForFile(file);
		}

		if (!driver)
		{
			std::cerr << "Failed to find driver for " << file << " with hint " << driverHint;
			return nullptr;
		}

		auto fileA = fileAsset(file, driverHint);
		if (!fileA)
		{
			std::cerr << "Failed to build device asset for " << file;
		}

		return driver->buildDeviceAsset(fileA, device, swapChain, bufferManager);
	}
}