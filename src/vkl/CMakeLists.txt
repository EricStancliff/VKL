

set(vkl_source
	./RenderPass.cpp
	./SwapChain.cpp
	./Window.cpp
	./CommandDispatcher.cpp
	./Common.cpp
	./Device.cpp
	./Instance.cpp
	./Reflect.cpp
	./Surface.cpp
	./Pipeline.cpp
	./Shader.cpp
	./RenderObject.cpp
 	./VertexBuffer.cpp
	./UniformBuffer.cpp
	./TextureBuffer.cpp
	./DrawCall.cpp
)

set(vkl_include_dir ${CMAKE_SOURCE_DIR}/include/vkl)
set(vkl_includes
	${vkl_include_dir}/Window.h
	${vkl_include_dir}/CommandDispatcher.h
	${vkl_include_dir}/Common.h
	${vkl_include_dir}/Device.h
	${vkl_include_dir}/Instance.h
	${vkl_include_dir}/Reflect.h
	${vkl_include_dir}/Reflect.inl
	${vkl_include_dir}/RenderPass.h
	${vkl_include_dir}/Surface.h
	${vkl_include_dir}/SwapChain.h
	${vkl_include_dir}/VKL_EXPORT.h
	${vkl_include_dir}/Pipeline.h
	${vkl_include_dir}/Shader.h
	${vkl_include_dir}/RenderObject.h
	${vkl_include_dir}/VertexBuffer.h
	${vkl_include_dir}/UniformBuffer.h
	${vkl_include_dir}/TextureBuffer.h
	${vkl_include_dir}/DrawCall.h
)

add_library(vkl SHARED ${vkl_source} ${vkl_includes})

get_filename_component(VULKAN_LIB_DIR ${Vulkan_LIBRARY} DIRECTORY)

if(WIN32)
set(SHADERC_LIB ${VULKAN_LIB_DIR}/shaderc_shared.lib)
else()
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(SHADERC REQUIRED shaderc)
    set(SHADERC_LIB ${SHADERC_LIBRARIES})
endif()

target_link_libraries(vkl PUBLIC unofficial::vulkan-memory-allocator::vulkan-memory-allocator glfw Vulkan::Vulkan ${SHADERC_LIB})

target_compile_definitions(vkl PRIVATE VKL_LIB)
target_include_directories(vkl PUBLIC ${vkl_include_dir} ${Vulkan_INCLUDE_DIR} PRIVATE )


Configure_Library(vkl)
