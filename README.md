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

Example of how you could configure and build below

`
$ cd ~/ws/VKL
$ mkdir builds
$ cd builds
$ export CC=/usr/local/gcc10/bin/gcc
$ export CXX=/usr/local/gcc10/bin/g++
$ source /usr/local/VulkanSDK/1.2.162.1/setup-env.sh 
$ alias cmake="/usr/local/cmake/cmake-3.19.6-Linux-x86_64/bin/cmake" 
$ cmake ../ -G "Unix Makefiles" -DVCPKG_TOOLCHAIN_FILE=~/vcpkg/scripts/buildsystems/vcpkg.cmake
$ make
`

To run, go to builds/bin, source linuxruntime.bash, and run

