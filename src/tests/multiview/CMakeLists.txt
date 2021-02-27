
set(vkl_include_dir ${CMAKE_SOURCE_DIR}/include/vkl)


add_executable(multiview_vkl main.cpp)

target_link_libraries(multiview_vkl PUBLIC vkl)

target_include_directories(multiview_vkl PUBLIC ${vkl_include_dir})


Configure_Test(multiview_vkl)
