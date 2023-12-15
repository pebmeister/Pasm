 
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CPACK_NSIS_MODIFY_PATH ON)

# set _CRT_SECURE_NO_WARNINGS for Windows
if (${CMAKE_HOST_SYSTEM_NAME} STREQUAL "Windows")
    add_definitions(-D_CRT_SECURE_NO_WARNINGS)
endif()

set(CMAKE_CXX_STANDARD 17 CACHE STRING "v")
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_CXX_EXTENSIONS OFF)

# Bison
find_package(BISON REQUIRED)
BISON_TARGET(Bison_Parser pasm.y ${CMAKE_CURRENT_BINARY_DIR}/pasm.tab.cc
    COMPILE_FLAGS "-rall"
    DEFINES_FILE ${CMAKE_CURRENT_BINARY_DIR}/pasm.tab.h)

# For win_flex turn on --wincompat
find_package(FLEX REQUIRED)
string(FIND "${FLEX_EXECUTABLE}" "win_flex.exe" IS_WIN_FLEX)

if (NOT ${IS_WIN_FLEX} EQUAL -1)
    FLEX_TARGET(Flex_Scanner pasm.l ${CMAKE_CURRENT_BINARY_DIR}/pasm.flex.cc
    COMPILE_FLAGS --wincompat)
else()
    FLEX_TARGET(Flex_Scanner pasm.l ${CMAKE_CURRENT_BINARY_DIR}/pasm.flex.cc
    )
endif()

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

# make this first
add_dependencies(${PROJECT_NAME} eolconvert) 

add_dependencies(parser eolconvert) 

add_dependencies(engine eolconvert) 

add_dependencies(${PROJECT_NAME} execute_eolconvert)

# force flex/bison
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
    message(STATUS "Path is set to $ENV{Path}.")
endif()
