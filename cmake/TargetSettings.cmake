
function(Configure_Test Target)

if(WIN32)
set_target_properties(${Target} PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "$<TARGET_FILE_DIR:${Target}>")

get_property(DEBUGGING_PATHS GLOBAL PROPERTY DEBUGGING_PATHS)
get_property(DEBUGGER_ENV GLOBAL PROPERTY DEBUGGER_ENV)

set(VS_DEBUGGER_ENV "PATH=${DEBUGGING_PATHS}\n")
set(VS_DEBUGGER_ENV "${VS_DEBUGGER_ENV}${DEBUGGER_ENV}")
message(STATUS ${VS_DEBUGGER_ENV})

get_property(DEBUGGING_PATHS_DEBUG GLOBAL PROPERTY DEBUGGING_PATHS_DEBUG)
get_property(DEBUGGER_ENV_DEBUG GLOBAL PROPERTY DEBUGGER_ENV_DEBUG)

set(VS_DEBUGGER_ENV_DEBUG "PATH=${DEBUGGING_PATHS_DEBUG}\n")
set(VS_DEBUGGER_ENV_DEBUG "${VS_DEBUGGER_ENV_DEBUG}${DEBUGGER_ENV_DEBUG}")
message(STATUS ${VS_DEBUGGER_ENV_DEBUG})


set_target_properties(${Target} PROPERTIES 
VS_DEBUGGER_ENVIRONMENT "${DEBUGGING_PATHS_DEBUG}"

)
endif()

if(WIN32)
set_target_properties( ${Target}
    PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY_DEBUG ${VKL_OUTPUT_DIR}/Debug
    RUNTIME_OUTPUT_DIRECTORY_RELEASE ${VKL_OUTPUT_DIR}/Release
    RUNTIME_OUTPUT_DIRECTORY_RELWITHDEBINFO ${VKL_OUTPUT_DIR}/RelWithDebInfo
    RUNTIME_OUTPUT_DIRECTORY_MINSIZEREL ${VKL_OUTPUT_DIR}/MinSizeRel

    ARCHIVE_OUTPUT_DIRECTORY_DEBUG ${VKL_OUTPUT_DIR}/Debug
    ARCHIVE_OUTPUT_DIRECTORY_RELEASE ${VKL_OUTPUT_DIR}/Release
    ARCHIVE_OUTPUT_DIRECTORY_RELWITHDEBINFO ${VKL_OUTPUT_DIR}/RelWithDebInfo
    ARCHIVE_OUTPUT_DIRECTORY_MINSIZEREL ${VKL_OUTPUT_DIR}/MinSizeRel

    LIBRARY_OUTPUT_DIRECTORY_DEBUG ${VKL_OUTPUT_DIR}/Debug
    LIBRARY_OUTPUT_DIRECTORY_RELEASE ${VKL_OUTPUT_DIR}/Release
    LIBRARY_OUTPUT_DIRECTORY_RELWITHDEBINFO ${VKL_OUTPUT_DIR}/RelWithDebInfo
    LIBRARY_OUTPUT_DIRECTORY_MINSIZEREL ${VKL_OUTPUT_DIR}/MinSizeRel

    FOLDER "Tests"
)
else()
set_target_properties( ${Target}
    PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${VKL_OUTPUT_DIR}"
    FOLDER "Tests"
)
endif()
endfunction()

function(Configure_Library Target)
if(WIN32)
set_target_properties( ${Target}
    PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY_DEBUG ${VKL_OUTPUT_DIR}/Debug
    RUNTIME_OUTPUT_DIRECTORY_RELEASE ${VKL_OUTPUT_DIR}/Release
    RUNTIME_OUTPUT_DIRECTORY_RELWITHDEBINFO ${VKL_OUTPUT_DIR}/RelWithDebInfo
    RUNTIME_OUTPUT_DIRECTORY_MINSIZEREL ${VKL_OUTPUT_DIR}/MinSizeRel

    ARCHIVE_OUTPUT_DIRECTORY_DEBUG ${VKL_OUTPUT_DIR}/Debug
    ARCHIVE_OUTPUT_DIRECTORY_RELEASE ${VKL_OUTPUT_DIR}/Release
    ARCHIVE_OUTPUT_DIRECTORY_RELWITHDEBINFO ${VKL_OUTPUT_DIR}/RelWithDebInfo
    ARCHIVE_OUTPUT_DIRECTORY_MINSIZEREL ${VKL_OUTPUT_DIR}/MinSizeRel

    LIBRARY_OUTPUT_DIRECTORY_DEBUG ${VKL_OUTPUT_DIR}/Debug
    LIBRARY_OUTPUT_DIRECTORY_RELEASE ${VKL_OUTPUT_DIR}/Release
    LIBRARY_OUTPUT_DIRECTORY_RELWITHDEBINFO ${VKL_OUTPUT_DIR}/RelWithDebInfo
    LIBRARY_OUTPUT_DIRECTORY_MINSIZEREL ${VKL_OUTPUT_DIR}/MinSizeRel

    FOLDER "Libraries"
)
else()
set_target_properties( ${Target}
    PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY "${VKL_OUTPUT_DIR}"
    FOLDER "Libraries"
)
endif()
endfunction()
