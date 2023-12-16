cl mergerelocate.cpp -o mergerelocate -std=c++11 -lstdc++
pasm -v relocate.asm -o relocate.prg -c64
pasm -v supermon64.asm -o supermon64-C000.prg -d ORG '$$C000' -c64
pasm -v supermon64.asm -o supermon64-8000.prg -d ORG '$$8000' -c64
mergerelocate relocate.prg supermon64-8000.prg supermon64-C000.prg supermon64.prg
