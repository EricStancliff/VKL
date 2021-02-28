

set(vxt_source
	./Camera.cpp
	./CameraManip.cpp
	./PNGLoader.cpp
	)

set(vkl_include_dir ${CMAKE_SOURCE_DIR}/include)
set(vxt_includes
	${vkl_include_dir}/vxt/Camera.h
	${vkl_include_dir}/vxt/PNGLoader.h
	${vkl_include_dir}/vxt/LinearAlgebra.h
	${vkl_include_dir}/vxt/VXT_EXPORT.h
	${vkl_include_dir}/vxt/PNGLoader.h
	)

add_library(vxt SHARED ${vxt_source} ${vxt_includes})

target_link_libraries(vxt PUBLIC vkl glm)

target_compile_definitions(vxt PRIVATE VXT_LIB -DVKL_DATA_DIR="${VKL_DATA_DIR}")
target_include_directories(vxt PUBLIC ${vxt_include_dir})


Configure_Library(vxt)
