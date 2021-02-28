
set(vkl_include_dir ${CMAKE_SOURCE_DIR}/include)


add_executable(triangle_vkl main.cpp)

target_link_libraries(triangle_vkl PUBLIC vkl vxt)

target_include_directories(triangle_vkl PUBLIC ${vkl_include_dir})


Configure_Test(triangle_vkl)
