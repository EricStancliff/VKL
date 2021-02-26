
set(vkl_include_dir ${CMAKE_SOURCE_DIR}/include/vkl)


add_executable(texture_vkl main.cpp)

target_link_libraries(texture_vkl PUBLIC vkl)

target_include_directories(texture_vkl PUBLIC ${vkl_include_dir})


Configure_Test(texture_vkl)
