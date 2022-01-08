

add_executable(texture_vkl main.cpp)

target_link_libraries(texture_vkl PUBLIC vkl vxt)

target_include_directories(texture_vkl PUBLIC ${vkl_include_dir})

target_compile_definitions(texture_vkl PRIVATE -DVKL_DATA_DIR="${VKL_DATA_DIR}")

Configure_Test(texture_vkl)
