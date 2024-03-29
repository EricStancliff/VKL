
function(InitVcpkg)
#find a vcpkg toolchain file
set(VCPKG_TOOLCHAIN_FILE "" CACHE FILEPATH "Path to your vcpkg toolchain file.")

if(NOT EXISTS ${VCPKG_TOOLCHAIN_FILE})
MESSAGE(FATAL_ERROR "You must configure your vcpkg toolchain file to continue... path_to_vcpkg/scripts/buildsystems/vcpkg.cmake")
endif()

get_filename_component(VCPKG_TOOLCHAIN_DIR ${VCPKG_TOOLCHAIN_FILE} DIRECTORY)

if(WIN32)
get_filename_component(VCPKG_EXE ${VCPKG_TOOLCHAIN_DIR}/../../vcpkg.exe ABSOLUTE)
else()
get_filename_component(VCPKG_EXE ${VCPKG_TOOLCHAIN_DIR}/../../vcpkg ABSOLUTE)
endif()

if(NOT EXISTS ${VCPKG_EXE})
MESSAGE(FATAL_ERROR "Your vcpkg executable (${VCPKG_EXE} is missing.  Did you bootstrap?")
endif()

set(CMAKE_TOOLCHAIN_FILE ${VCPKG_TOOLCHAIN_FILE})
message(STATUS "Using toolchain file: ${CMAKE_TOOLCHAIN_FILE}.")
message(STATUS "Using vcpkg exe: ${VCPKG_EXE}.")

if(WIN32)
set(VCPKG_TARGET_TRIPLET x64-windows)
else()
set(VCPKG_TARGET_TRIPLET x64-linux)
endif()

set_property(GLOBAL PROPERTY VCPKG_EXECECUTABLE ${VCPKG_EXE})
set_property(GLOBAL PROPERTY CMAKE_TOOLCHAIN_FILE ${CMAKE_TOOLCHAIN_FILE})
set_property(GLOBAL PROPERTY VCPKG_TARGET_TRIPLET ${VCPKG_TARGET_TRIPLET})

set(ENV{VCPKG_DEFAULT_TRIPLET} ${VCPKG_TARGET_TRIPLET})

endfunction()

###PRIVATE
#pull but don't find
function(InstallExternal_Ext_Private Target)
get_property(EXTERNAL_DIR GLOBAL PROPERTY EXTERNAL_DIR)
get_property(VCPKG_EXE GLOBAL PROPERTY VCPKG_EXECECUTABLE)
get_property(VCPKG_TARGET_TRIPLET GLOBAL PROPERTY VCPKG_TARGET_TRIPLET)
set(INSTALL_COMMAND "${VCPKG_EXE} install ${Target}:${VCPKG_TARGET_TRIPLET} ")
message(STATUS "***** Installing 3rd party dependencies *****")
message(STATUS ${INSTALL_COMMAND})
execute_process(COMMAND ${VCPKG_EXE} install ${Target} --x-install-root=${EXTERNAL_DIR}/.. OUTPUT_VARIABLE OUTPUT_STRING)
message(STATUS "${OUTPUT_STRING}")
endfunction()

#pull and find
function(InstallExternal_Ext Target CMakeName)
get_property(EXTERNAL_DIR GLOBAL PROPERTY EXTERNAL_DIR)
InstallExternal_Ext_Private(${Target})
set(CMAKE_PREFIX_PATH ${CMAKE_PREFIX_PATH} ${EXTERNAL_DIR}/share/${CMakeName})
find_package("${CMakeName}" CONFIG REQUIRED)
endfunction()

###PUBLIC
#pull and find
function(InstallExternal Target)
InstallExternal_Ext(${Target} ${Target})
endfunction()

#pull but don't find
function(InstallExternalNoFind Target)
InstallExternal_Ext_Private(${Target})
endfunction()