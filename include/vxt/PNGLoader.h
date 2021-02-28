#pragma once
#include <vxt/VXT_EXPORT.h>
namespace vxt
{
	VXT_EXPORT void* loadPNGData(const char* filePath, int& width, int& height, int& channels);
	VXT_EXPORT void freePNGData(void* data);
	VXT_EXPORT void* loadJPGData(const char* filePath, int& width, int& height, int& channels);
	VXT_EXPORT void freeJPGData(void* data);
}