# GoogleTest requires at least C++14
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)
FetchContent_Declare(
  googletest
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
  dictionary_unit_test.cc
  error_unit_test.cc
  expand_unit_test.cc
  file_unit_test.cc
)

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
                                        

