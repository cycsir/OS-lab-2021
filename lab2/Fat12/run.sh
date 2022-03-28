#!/usr/bin/bash
nasm -f elf32 -o func.o func.asm
g++ -m32 main.cpp func.o


#
#chmod +x run.sh
#./run.sh