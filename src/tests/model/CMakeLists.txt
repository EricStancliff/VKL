
set(vkl_include_dir ${CMAKE_SOURCE_DIR}/include)


add_executable(model_vkl main.cpp)

target_link_libraries(model_vkl PUBLIC vkl vxt)

target_include_directories(model_vkl PUBLIC ${vkl_include_dir})

target_compile_definitions(model_vkl PRIVATE -DVKL_DATA_DIR="${VKL_DATA_DIR}")

Configure_Test(model_vkl)
