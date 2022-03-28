all:func.asm main.cpp
		nasm -f elf32 -o func.o func.asm
		g++ -m32 main.cpp func.o -o main