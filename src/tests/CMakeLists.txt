add_subdirectory(multiview)
add_subdirectory(texture)
add_subdirectory(triangle)


if(UNIX)
configure_file("./linuxruntime.bash.in" "${VKL_OUTPUT_DIR}/linuxruntime.bash" )
endif()