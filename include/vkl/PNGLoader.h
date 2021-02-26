#pragma once
#include <Common.h>
namespace vkl
{
	VKL_EXPORT void* loadPNGData(const char* filePath, int& width, int& height, int& channels);
	VKL_EXPORT void freePNGData(void* data);
	VKL_EXPORT void* loadJPGData(const char* filePath, int& width, int& height, int& channels);
	VKL_EXPORT void freeJPGData(void* data);
}