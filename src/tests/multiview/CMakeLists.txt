
set(vkl_include_dir ${CMAKE_SOURCE_DIR}/include)


add_executable(multiview_vkl main.cpp)

target_link_libraries(multiview_vkl PUBLIC vkl vxt)

target_include_directories(multiview_vkl PUBLIC ${vkl_include_dir})


Configure_Test(multiview_vkl)
