
set(vkl_include_dir ${CMAKE_SOURCE_DIR}/include)


add_executable(texture_vkl main.cpp)

target_link_libraries(texture_vkl PUBLIC vkl vxt)

target_include_directories(texture_vkl PUBLIC ${vkl_include_dir})


Configure_Test(texture_vkl)
