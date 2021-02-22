
set(vkl_include_dir ${CMAKE_SOURCE_DIR}/include/vkl)


add_executable(triangle_vkl main.cpp)

target_link_libraries(triangle_vkl PUBLIC vkl)

target_compile_definitions(triangle_vkl PRIVATE VKL_LIB)
target_include_directories(triangle_vkl PUBLIC ${vkl_include_dir})


Configure_Test(triangle_vkl)
