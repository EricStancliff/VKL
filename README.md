# VKL
Vulkan Library

# Requirements
Vulkan SDK Installed
vcpkg installed

# Build Process
## Windows
Use Cmake
Configure for VS2019 x64 only
Dependencies should automatically populate

## Linux
You will need a compiler which accepts C++20, and need to install vcpkg https://github.com/Microsoft/vcpkg .

    export VC_PKG_TOOLCHAIN=$PATH_TO_BUILT_VCPKG/scripts/buildsystems/vcpkg.cmake
    ./buildDebug.sh
