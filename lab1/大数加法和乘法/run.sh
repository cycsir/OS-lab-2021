#!/usr/bin/bash
nasm -f elf helloworld.asm
ld -m elf_i386 helloworld.o -o helloworld
./helloworld

#
#chmod +x run.sh
#./run.sh 