<<<<<<< HEAD
# Pasm
platform independent version of Pasm 6502 assemnler

requires flex and bison to build
=======
# Pasm
platform independent version of Pasm 6502 assemnler

requires flex and bison to build

To build:

cmake -S CMakePasm -B PasmBuild
cmake --build PasmBuild

The executable will be in the bin directory of PasmBuild


To run the unit tests:

cd PasmBuild\unittests\
ctest

>>>>>>> origin/master
