# minimum version
cmake_minimum_required(VERSION 3.20)

#project
project(pasm VERSION 2.0)

#source code
add_subdirectory(source)

#unittests
add_subdirectory(unittests)

#eolcoonvert
add_subdirectory(eolconvert)
