#include <vxt/PNGLoader.h>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#include <string>
#include <iostream>
namespace vxt
{
    VXT_EXPORT void* loadPNGData(const char* filePath, int& width, int& height, int& channels)
    {
        auto retVal = (void*)stbi_load(filePath, &width, &height, &channels, STBI_rgb_alpha);
        channels = 4;
        std::string fail = stbi_failure_reason();
        if (!fail.empty())
        {
            std::cerr << fail << std::endl;
        }
        return retVal;

    }
    VXT_EXPORT void freePNGData(void* data)
    {
        stbi_image_free(data);
    }

    VXT_EXPORT void* loadJPGData(const char* filePath, int& width, int& height, int& channels)
    {
        auto retVal = (void*)stbi_load(filePath, &width, &height, &channels, STBI_rgb_alpha);
        channels = 4;
        std::string fail = stbi_failure_reason();
        if (!fail.empty())
        {
            std::cerr << fail << std::endl;
        }
        return retVal;
    }
    VXT_EXPORT void freeJPGData(void* data)
    {
        stbi_image_free(data);
    }
    VXT_EXPORT void* loadJPGData_fromMem(const void* data, size_t size, int& width, int& height, int& channels)
    {
        auto retVal = (void*)stbi_load_from_memory((const stbi_uc*)data, (int)size, &width, &height, &channels, STBI_rgb_alpha);
        channels = 4;
        std::string fail = stbi_failure_reason();
        if (!fail.empty())
        {
            std::cerr << fail << std::endl;
        }
        return retVal;
    }
    VXT_EXPORT void* loadPNGData_fromMem(const void* data, size_t size, int& width, int& height, int& channels)
    {
        auto retVal = (void*)stbi_load_from_memory((const stbi_uc*)data, (int)size, &width, &height, &channels, STBI_rgb_alpha);
        channels = 4;
        std::string fail = stbi_failure_reason();
        if (!fail.empty())
        {
            std::cerr << fail << std::endl;
        }
        return retVal;
    }
}