message(STATUS "Processing PASM SOURCE")

set(CPACK_NSIS_MODIFY_PATH ON)

set (BIN ${CMAKE_BINARY_DIR}/bin)
set (LIB ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${BIN})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${LIB})

message(STATUS "set EXE DIRECTORY  " ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
message(STATUS "set LIBRARY DIRECTORY  " ${CMAKE_LIBRARY_OUTPUT_DIRECTORY})

# set _CRT_SECURE_NO_WARNINGS for Windows
if (${CMAKE_HOST_SYSTEM_NAME} STREQUAL "Windows")
    message(STATUS "set CRT_SECURE_NO_WARNINGS for Windows")
    add_definitions(-D_CRT_SECURE_NO_WARNINGS)
endif()

message(STATUS "set C Standard 17")
set(CMAKE_CXX_STANDARD 17 CACHE STRING "v")
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_CXX_EXTENSIONS OFF)

# Bison
message(STATUS "targeting Bison")
find_package(BISON REQUIRED)
BISON_TARGET(Bison_Parser pasm.y ${CMAKE_CURRENT_BINARY_DIR}/pasm.tab.cc
    COMPILE_FLAGS "-rall"
    DEFINES_FILE ${CMAKE_CURRENT_BINARY_DIR}/pasm.tab.h)

# For win_flex turn on --wincompat
message(STATUS "targeting Flex")
find_package(FLEX REQUIRED)
string(FIND "${FLEX_EXECUTABLE}" "win_flex.exe" IS_WIN_FLEX)

set (FLEX_OPT "")
if (NOT ${IS_WIN_FLEX} EQUAL -1)
    set (FLEX_OPT "--wincompat")
endif()
FLEX_TARGET(Flex_Scanner pasm.l ${CMAKE_CURRENT_BINARY_DIR}/pasm.flex.cc
    COMPILE_FLAGS ${FLEX_OPT})

# set flex bison dependency   
ADD_FLEX_BISON_DEPENDENCY(Flex_Scanner Bison_Parser)

# parser scanner library
add_library(parser
    ${BISON_Bison_Parser_OUTPUTS}
    ${FLEX_Flex_Scanner_OUTPUTS}
    parseargs.cc 
)
    
# engine
add_library(engine 
    dictionary.cc
    error.cc 
    expand.cc 
    file.cc
    genlist.cc
    genoutput.cc
    globals.cc
    hash.cc
    memory.cc
    node.cc
    opcodes.cc       
    stacks.cc
    str.cc
    sym.cc
    pass.cc)
 
# executable
add_executable(${PROJECT_NAME}
    main.cc)

# add dependencies
add_dependencies(${PROJECT_NAME} eolconvert) 
add_dependencies(${PROJECT_NAME} execute_eolconvert)
add_dependencies(parser eolconvert)
add_dependencies(engine parser) 

target_link_libraries(${PROJECT_NAME} 
    parser 
    engine)
    
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

# install
install (TARGETS ${PROJECT_NAME} RUNTIME DESTINATION bin)

if (${CMAKE_HOST_SYSTEM_NAME} STREQUAL "Windows")
    set(mypath $ENV{Path})    
    string(FIND "${mypath}" "\\Pasm\\" HAS_PATH)    

    if (${HAS_PATH} EQUAL -1)
        execute_process(COMMAND "PowerShell" "setx PATH %PATH%\";%PROGRAMFILES\(x86\)%\\Pasm\\bin;\"")
    endif()    
endif()

message(STATUS "Processing source completed.")
