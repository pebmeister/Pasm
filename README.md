# Pasm
platform independent version of Pasm 6502 assembler

requires flex and bison to build
=======
# Pasm
platform independent version of Pasm 6502 assembler

requires flex and bison to build

To Confgure:
cmake -S CMakePasm -B PasmBuild

To Build:
cmake --build PasmBuild --config Release

At this point the executable will be in the bin directory of PasmBuild

To Install on Unix or Linux etc:
sudo cmake --install PasmBuild --config Release

To install Windows:
open Adminisrator PowerShell
cmake --install PasmBuild --config Release



