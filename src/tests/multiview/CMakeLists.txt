
set(vkl_include_dir ${CMAKE_SOURCE_DIR}/include)


add_executable(multiview_vkl main.cpp)

target_link_libraries(multiview_vkl PUBLIC vkl vxt)

target_include_directories(multiview_vkl PUBLIC ${vkl_include_dir})

target_compile_definitions(multiview_vkl PRIVATE -DVKL_DATA_DIR="${VKL_DATA_DIR}")

Configure_Test(multiview_vkl)
