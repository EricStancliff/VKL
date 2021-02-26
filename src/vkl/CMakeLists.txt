

set(vkl_source
	./BufferManager.cpp
	./CommandDispatcher.cpp
	./Common.cpp
	./Device.cpp
	./DrawCall.cpp
	./Instance.cpp
	./IndexBuffer.cpp
	./Pipeline.cpp
	./Reflect.cpp
	./RenderObject.cpp
	./RenderPass.cpp
	./Shader.cpp
	./Surface.cpp
	./SwapChain.cpp
	./TextureBuffer.cpp
	./UniformBuffer.cpp
 	./VertexBuffer.cpp
	./Window.cpp
	./PNGLoader.cpp
	)

set(vkl_include_dir ${CMAKE_SOURCE_DIR}/include/vkl)
set(vkl_includes
	${vkl_include_dir}/BufferManager.h
	${vkl_include_dir}/CommandDispatcher.h
	${vkl_include_dir}/Common.h
	${vkl_include_dir}/Device.h
	${vkl_include_dir}/DrawCall.h
	${vkl_include_dir}/IndexBuffer.h
	${vkl_include_dir}/Instance.h
	${vkl_include_dir}/Pipeline.h
	${vkl_include_dir}/Reflect.h
	${vkl_include_dir}/Reflect.inl
	${vkl_include_dir}/RenderObject.h
	${vkl_include_dir}/RenderPass.h
	${vkl_include_dir}/Shader.h
	${vkl_include_dir}/Surface.h
	${vkl_include_dir}/SwapChain.h
	${vkl_include_dir}/TextureBuffer.h
	${vkl_include_dir}/UniformBuffer.h
	${vkl_include_dir}/VertexBuffer.h
	${vkl_include_dir}/VKL_EXPORT.h
	${vkl_include_dir}/Window.h
	${vkl_include_dir}/PNGLoader.h
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

target_link_libraries(vkl PUBLIC unofficial::vulkan-memory-allocator::vulkan-memory-allocator glfw Vulkan::Vulkan ${SHADERC_LIB} glm)

target_compile_definitions(vkl PRIVATE VKL_LIB -DVKL_DATA_DIR="${VKL_DATA_DIR}")
target_include_directories(vkl PUBLIC ${vkl_include_dir} ${Vulkan_INCLUDE_DIR} PRIVATE "${EXTERNAL_DIR}/include")


Configure_Library(vkl)
