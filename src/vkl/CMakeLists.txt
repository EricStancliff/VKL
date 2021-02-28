

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
	)

set(vkl_include_dir ${CMAKE_SOURCE_DIR}/include)
set(vkl_includes
	${vkl_include_dir}/vkl/BufferManager.h
	${vkl_include_dir}/vkl/CommandDispatcher.h
	${vkl_include_dir}/vkl/Common.h
	${vkl_include_dir}/vkl/Device.h
	${vkl_include_dir}/vkl/DrawCall.h
	${vkl_include_dir}/vkl/IndexBuffer.h
	${vkl_include_dir}/vkl/Instance.h
	${vkl_include_dir}/vkl/Pipeline.h
	${vkl_include_dir}/vkl/Reflect.h
	${vkl_include_dir}/vkl/Reflect.inl
	${vkl_include_dir}/vkl/RenderObject.h
	${vkl_include_dir}/vkl/RenderPass.h
	${vkl_include_dir}/vkl/Shader.h
	${vkl_include_dir}/vkl/Surface.h
	${vkl_include_dir}/vkl/SwapChain.h
	${vkl_include_dir}/vkl/TextureBuffer.h
	${vkl_include_dir}/vkl/UniformBuffer.h
	${vkl_include_dir}/vkl/VertexBuffer.h
	${vkl_include_dir}/vkl/VKL_EXPORT.h
	${vkl_include_dir}/vkl/Window.h
	)

add_library(vkl SHARED ${vkl_source} ${vkl_includes})

get_filename_component(VULKAN_LIB_DIR ${Vulkan_LIBRARY} DIRECTORY)

if(WIN32)
set(SHADERC_LIB ${VULKAN_LIB_DIR}/shaderc_shared.lib)
else()
set(SHADERC_LIB ${VULKAN_LIB_DIR}/libshaderc_shared.so)
endif()

target_link_libraries(vkl PUBLIC unofficial::vulkan-memory-allocator::vulkan-memory-allocator glfw Vulkan::Vulkan ${SHADERC_LIB} glm)

target_compile_definitions(vkl PRIVATE VKL_LIB -DVKL_DATA_DIR="${VKL_DATA_DIR}")
target_include_directories(vkl PUBLIC ${vkl_include_dir} ${Vulkan_INCLUDE_DIR} "${EXTERNAL_DIR}/include" PRIVATE )


Configure_Library(vkl)
