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

set(VKL_DATA_DIR ${CMAKE_SOURCE_DIR}/data)

InitVcpkg()
get_property(VCPKG_TARGET_TRIPLET GLOBAL PROPERTY VCPKG_TARGET_TRIPLET)
set(EXTERNAL_DIR ${CMAKE_SOURCE_DIR}/external/${VCPKG_TARGET_TRIPLET})


InstallExternal(glm)
InstallExternal(glfw3)
InstallExternal_Ext(vulkan-memory-allocator unofficial-vulkan-memory-allocator)
InstallExternalNoFind(stb)

#InstallExternal_Ext(shaderc glslang)

set(DEBUGGING_PATHS "%PATH%")
set(DEBUGGING_PATHS "${DEBUGGING_PATHS};${EXTERNAL_DIR}/bin")
set_property(GLOBAL PROPERTY DEBUGGING_PATHS ${DEBUGGING_PATHS})

get_filename_component(VULKAN_LIB_DIR ${Vulkan_LIBRARY} DIRECTORY)
set(VULKAN_LAYERS_PATH "${VULKAN_LIB_DIR}/../Bin")
get_filename_component(VULKAN_LAYERS_PATH_ABS ${VULKAN_LAYERS_PATH} ABSOLUTE)

set(DEBUGGER_ENV "VK_LAYER_PATH=${VULKAN_LAYERS_PATH_ABS}")
set_property(GLOBAL PROPERTY DEBUGGER_ENV ${DEBUGGER_ENV})

#add projects
add_subdirectory(src)
