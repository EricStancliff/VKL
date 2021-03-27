add_subdirectory(triangle)
add_subdirectory(texture)
add_subdirectory(multiview)
add_subdirectory(model)

if(UNIX)
configure_file("./linuxruntime.bash.in" "${VKL_OUTPUT_DIR}/linuxruntime.bash" )
endif()