#include <PNGLoader.h>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

namespace vkl
{
    VKL_EXPORT void* loadPNGData(const char* filePath, int& width, int& height, int& channels)
    {
        auto retVal = (void*)stbi_load(filePath, &width, &height, &channels, STBI_rgb_alpha);
        channels = 4;
        return retVal;

    }
    VKL_EXPORT void freePNGData(void* data)
    {
        stbi_image_free(data);
    }

    VKL_EXPORT void* loadJPGData(const char* filePath, int& width, int& height, int& channels)
    {
        auto retVal = (void*)stbi_load(filePath, &width, &height, &channels, STBI_rgb_alpha);
        channels = 4;
        return retVal;
    }
    VKL_EXPORT void freeJPGData(void* data)
    {
        stbi_image_free(data);
    }
}