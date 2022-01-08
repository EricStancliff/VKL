cmake_minimum_required(VERSION 3.13)
project(vkl)

set (CMAKE_CXX_STANDARD 20)
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

if(MSVC)
set(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} /WX")
add_compile_options(/wd4251)
endif()

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/cmake)
include(TargetSettings)
include(InstallVcpkgLibrary)

set(VKL_OUTPUT_DIR ${CMAKE_CURRENT_BINARY_DIR}/bin)

#Vulkan SDK needs to be "installed" on your machine w/o our help
find_package(Vulkan REQUIRED)

set(VKL_DATA_DIR ${CMAKE_CURRENT_SOURCE_DIR}/data)

#pull and bootstrap vcpkg
message(STATUS "Pulling and Bootstrapping VcPkg from github")
include(FetchContent)
FetchContent_Declare(
  VCPKG
  GIT_REPOSITORY https://github.com/microsoft/vcpkg.git
  GIT_TAG        2021.12.01
)
FetchContent_MakeAvailable(VCPKG)

if(MSVC)
execute_process(COMMAND ${vcpkg_SOURCE_DIR}/bootstrap-vcpkg.bat  OUTPUT_VARIABLE OUTPUT_STRING)
else()
execute_process(COMMAND ${vcpkg_SOURCE_DIR}/bootstrap-vcpkg.sh  OUTPUT_VARIABLE OUTPUT_STRING)
endif()

set(VCPKG_TOOLCHAIN_FILE "${vcpkg_SOURCE_DIR}/scripts/buildsystems/vcpkg.cmake" CACHE FILEPATH "Path to your vcpkg toolchain file.")


InitVcpkg()
get_property(VCPKG_TARGET_TRIPLET GLOBAL PROPERTY VCPKG_TARGET_TRIPLET)
set(EXTERNAL_DIR ${CMAKE_CURRENT_BINARY_DIR}/external/${VCPKG_TARGET_TRIPLET})

set(vkl_include_dir ${CMAKE_CURRENT_SOURCE_DIR}/include)

InstallExternal(glm)
InstallExternal(glfw3)
InstallExternal_Ext(vulkan-memory-allocator unofficial-vulkan-memory-allocator)
InstallExternalNoFind(stb)
InstallExternalNoFind(tinygltf)
InstallExternal(tbb)
#InstallExternal(rttr)
#Ugh... Issue with rttr 0.9.6 which is the lastest official release so that's what's in vcpkg.  The below workaround has been solved for like two years.
#if(MSVC)
#set (CMAKE_CXX_FLAGS "/permissive /EHsc /Zc:twoPhase-")
#endif()

set(DEBUGGING_PATHS "%PATH%")
set(DEBUGGING_PATHS "${DEBUGGING_PATHS};${EXTERNAL_DIR}/bin")
set(DEBUGGING_PATHS "${DEBUGGING_PATHS};${EXTERNAL_DIR}/debug/bin")

#ughhhhh
set(DEBUGGING_PATHS "${DEBUGGING_PATHS};${VKL_OUTPUT_DIR}/Debug")
set(DEBUGGING_PATHS "${DEBUGGING_PATHS};${VKL_OUTPUT_DIR}/Release")
set(DEBUGGING_PATHS "${DEBUGGING_PATHS};${VKL_OUTPUT_DIR}/RelWithDebInfo")
set(DEBUGGING_PATHS "${DEBUGGING_PATHS};${VKL_OUTPUT_DIR}/MinSizeRel")
set_property(GLOBAL PROPERTY DEBUGGING_PATHS ${DEBUGGING_PATHS})

get_filename_component(VULKAN_LIB_DIR ${Vulkan_LIBRARY} DIRECTORY)
if(WIN32)
set(VULKAN_LAYERS_PATH "${VULKAN_LIB_DIR}/../Bin")
else()
set(VULKAN_LAYERS_PATH "${VULKAN_LIB_DIR}")
endif()
get_filename_component(VULKAN_LAYERS_PATH_ABS ${VULKAN_LAYERS_PATH} ABSOLUTE)

set(DEBUGGER_ENV "VK_LAYER_PATH=${VULKAN_LAYERS_PATH_ABS}")
set_property(GLOBAL PROPERTY DEBUGGER_ENV ${DEBUGGER_ENV})

if(UNIX)
get_filename_component(COMPILER_DIR ${CMAKE_CXX_COMPILER} DIRECTORY)
set(COMPILER_LIB_DIR "${COMPILER_DIR}/../lib64")
get_filename_component(COMPILER_LIB_DIR_ABS ${COMPILER_LIB_DIR} ABSOLUTE)
endif()

#add projects
add_subdirectory(src)
