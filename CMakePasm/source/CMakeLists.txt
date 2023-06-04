set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# set _CRT_SECURE_NO_WARNINGS for Windows
if (${CMAKE_HOST_SYSTEM_NAME} STREQUAL "Windows")
    add_definitions(-D_CRT_SECURE_NO_WARNINGS)
endif()

# Bison
find_package(BISON REQUIRED)
BISON_TARGET(Bison_Parser pasm.y ${CMAKE_CURRENT_BINARY_DIR}/pasm.tab.c
             DEFINES_FILE ${CMAKE_CURRENT_BINARY_DIR}/pasm.tab.h)

# For win_flex turn on --wincompat
find_package(FLEX REQUIRED)
string(FIND "${FLEX_EXECUTABLE}" "win_flex.exe" IS_WIN_FLEX)

if (NOT ${IS_WIN_FLEX} EQUAL -1)
    FLEX_TARGET(Flex_Scanner pasm.l ${CMAKE_CURRENT_BINARY_DIR}/pasm.flex.c
    COMPILE_FLAGS --wincompat)
else()
    FLEX_TARGET(Flex_Scanner pasm.l ${CMAKE_CURRENT_BINARY_DIR}/pasm.flex.c
    )
endif()

# set flex bison dependency   
ADD_FLEX_BISON_DEPENDENCY(Flex_Scanner Bison_Parser)

# parser library
add_library(parser
        ${BISON_Bison_Parser_OUTPUTS}
        ${FLEX_Flex_Scanner_OUTPUTS}
    )
 
# executeable
add_executable(pasm 
    dictionary.c 
    error.c 
    expand.c 
    file.c
    genlist.c
    genoutput.c
    globals.c
    hash.c
    main.c
    memory.c
    node.c
    opcodes.c
    parseargs.c    
    stacks.c
    str.c
    sym.c)

target_link_libraries(pasm parser)

target_include_directories(parser PUBLIC "${CMAKE_CURRENT_LIST_DIR}"
                                        "${CMAKE_CURRENT_BINARY_DIR}"
)
target_include_directories(pasm PUBLIC "${CMAKE_CURRENT_LIST_DIR}"
                                        "${CMAKE_CURRENT_BINARY_DIR}"
)