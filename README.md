# Debugger-Linux-x86
To build your own debuggee program use:
1) Written in assembly language:
```
nasm -f elf32 prog.asm

ld -melf_i386 prog.o -o prog
```
2) Written in C:
```
gcc -m32 -g -fvar-tracking prog.c -o prog
```
To launch example debuggee program -  Hello World in IDE put as program arguments:
```
../debuggee_examples/prog
```
