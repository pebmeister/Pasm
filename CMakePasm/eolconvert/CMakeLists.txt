 
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CPACK_NSIS_MODIFY_PATH ON)

# set _CRT_SECURE_NO_WARNINGS for Windows
if (${CMAKE_HOST_SYSTEM_NAME} STREQUAL "Windows")
    add_definitions(-D_CRT_SECURE_NO_WARNINGS)
endif()

# executable
add_executable(eolconvert
    eolconvert.cc)

if (${CMAKE_HOST_SYSTEM_NAME} STREQUAL "Windows")
    add_custom_target(execute_eolconvert
      DEPENDS ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/eolconvert
      COMMAND ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/$<CONFIG>/eolconvert
      "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/../../CMakePasm/samples" "samples\\"
    )
else()
    add_custom_target(execute_eolconvert
      DEPENDS ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/eolconvert
      COMMAND ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/eolconvert
      "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/../../CMakePasm/samples/"
    )
endif()

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

install (DIRECTORY  ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/../../CMakePasm/samples/
        DIRECTORY_PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_WRITE GROUP_EXECUTE WORLD_READ WORLD_WRITE WORLD_EXECUTE
        FILE_PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_WRITE GROUP_EXECUTE WORLD_READ WORLD_WRITE WORLD_EXECUTE
        DESTINATION pasm/samples/
    )
        