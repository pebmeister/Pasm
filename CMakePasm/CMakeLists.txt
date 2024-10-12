message(STATUS "Processing PASM TOPLEVEL")

# minimum version
cmake_minimum_required(VERSION 3.20)

#project
project(pasm VERSION 3.0)

#source code
add_subdirectory(source)

#unittests
add_subdirectory(unittests)

#eolconvert
add_subdirectory(eolconvert)
