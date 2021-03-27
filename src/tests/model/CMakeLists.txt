
set(vkl_include_dir ${CMAKE_SOURCE_DIR}/include)


add_executable(model_vkl main.cpp)

target_link_libraries(model_vkl PUBLIC vkl vxt)

target_include_directories(model_vkl PUBLIC ${vkl_include_dir})


Configure_Test(model_vkl)
