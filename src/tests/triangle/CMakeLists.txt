
add_executable(triangle_vkl main.cpp)

target_link_libraries(triangle_vkl PUBLIC vkl vxt)

target_include_directories(triangle_vkl PUBLIC ${vkl_include_dir})

target_compile_definitions(triangle_vkl PRIVATE -DVKL_DATA_DIR="${VKL_DATA_DIR}")

Configure_Test(triangle_vkl)
