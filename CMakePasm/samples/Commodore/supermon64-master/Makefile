all: supermon64.prg

.PHONY: clean cleanprg cleanmerge

clean: cleanprg cleanmerge cleanlst

cleanprg:
	rm *.prg
    
cleanlst:
	rm *.lst

cleanmerge:
	rm mergerelocate
    
supermon64.prg: relocate.prg supermon64-8000.prg supermon64-C000.prg mergerelocate
	./mergerelocate relocate.prg supermon64-8000.prg supermon64-C000.prg supermon64.prg

supermon64-8000.prg: supermon64.asm
	pasm -v supermon64.asm -o supermon64-8000.prg -d ORG '$$8000' -c64

supermon64-C000.prg: supermon64.asm
	pasm -v supermon64.asm -o supermon64-C000.prg -d ORG '$$C000' -c64

relocate.prg: relocate.asm
	pasm -v relocate.asm -o relocate.prg -c64

mergerelocate.exe: mergeRelocate.cpp
	gcc mergerelocate.cpp -o mergerelocate -std=c++11 -lstdc++
