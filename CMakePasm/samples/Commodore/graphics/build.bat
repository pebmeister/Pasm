cls
pasm .\graphics.asm -dir ..\include\ -o .\graphics.bin 
cartconv -t normal -l 32768 -p -i .\graphics.bin -o .\graphics.crt
