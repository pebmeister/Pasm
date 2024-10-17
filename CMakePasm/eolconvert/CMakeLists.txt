message(STATUS "processing eolconvert")

set(CPACK_NSIS_MODIFY_PATH ON)
set(EOLBIN ${CMAKE_BINARY_DIR}/eolconvert/bin/)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${EOLBIN}$<CONFIG>/)
message(STATUS "EOLCONVERT EXE DIRECTORY " ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})

# set _CRT_SECURE_NO_WARNINGS for Windows
if (${CMAKE_HOST_SYSTEM_NAME} STREQUAL "Windows")
    add_definitions(-D_CRT_SECURE_NO_WARNINGS)
endif()

# executable
add_executable(eolconvert
    eolconvert.cc)

add_custom_target(execute_eolconvert
    COMMAND ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}eolconvert
)

# include directory for main
target_include_directories(${PROJECT_NAME} PUBLIC 
    "${CMAKE_CURRENT_LIST_DIR}"
    "${CMAKE_CURRENT_BINARY_DIR}")

# include directory for parser
target_include_directories(parser PUBLIC
    "${CMAKE_CURRENT_LIST_DIR}"
    "${CMAKE_CURRENT_BINARY_DIR}")

# include directory for engine
target_include_directories(engine PUBLIC 
    "${CMAKE_CURRENT_LIST_DIR}"
    "${CMAKE_CURRENT_BINARY_DIR}")

if (NOT ${CMAKE_HOST_SYSTEM_NAME} STREQUAL "Windows")
    install (DIRECTORY  ${CMAKE_EXE_DIRECTORY}/../../CMakePasm/samples/
            DIRECTORY_PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_WRITE GROUP_EXECUTE WORLD_READ WORLD_WRITE WORLD_EXECUTE
            FILE_PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_WRITE GROUP_EXECUTE WORLD_READ WORLD_WRITE WORLD_EXECUTE
            DESTINATION pasm/samples/
        )
endif()       

message(STATUS "eolconvert cmake exit")
 