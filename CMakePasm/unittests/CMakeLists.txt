message(STATUS "Processing unit_test source")

message(STATUS "set C Standard 20")
set(CMAKE_CXX_STANDARD 20 CACHE STRING "v")
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_CXX_EXTENSIONS OFF)

include(FetchContent)
FetchContent_Declare(
  googletest
  DOWNLOAD_EXTRACT_TIMESTAMP True
  URL https://github.com/google/googletest/archive/03597a01ee50ed33e9dfd640b249b4be3799d395.zip
)
# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

enable_testing()

if (${CMAKE_HOST_SYSTEM_NAME} STREQUAL "Windows")
    add_definitions(-D_CRT_SECURE_NO_WARNINGS)
endif()

add_executable(
  unittest
  unit_test.cc
  assemble_unit_test.cc
  error_unit_test.cc
  file_unit_test.cc
  hash_unit_test.cc
  node_unit_test.cc
  opcode_unit_test.cc
  str_unit_test.cc
)

#force flex/bison first
add_dependencies(unittest parser engine) 

target_link_libraries(
  unittest
  GTest::gtest_main
  parser
  engine
)

include(GoogleTest)
gtest_discover_tests(unittest)

# include directory for unittest
target_include_directories(unittest PUBLIC "${CMAKE_CURRENT_LIST_DIR}"
                                        "${CMAKE_CURRENT_BINARY_DIR}")
                                        
