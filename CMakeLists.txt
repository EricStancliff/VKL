cmake_minimum_required(VERSION 3.0)
project(vkl)

set (CMAKE_CXX_STANDARD 20)
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

if(MSVC)
set(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} /WX")
add_compile_options(/wd4251)
endif()

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_SOURCE_DIR}/cmake)
include(TargetSettings)
include(InstallVcpkgLibrary)

set(VKL_OUTPUT_DIR ${CMAKE_CURRENT_BINARY_DIR}/bin)

#Vulkan SDK needs to be "installed" on your machine w/o our help
find_package(Vulkan REQUIRED)


InitVcpkg()

InstallExternal(glfw3)
InstallExternal_Ext(vulkan-memory-allocator unofficial-vulkan-memory-allocator)
InstallExternal(tbb)

#add projects
add_subdirectory(src)