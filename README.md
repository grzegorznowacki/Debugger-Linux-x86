# Debugger-Linux-x86
To build your own debuggee program use:

nasm -f elf32 prog.asm
ld -melf_i386 prog.o -o prog

To launch example debuggee program -  Hello World in IDE put as program arguments:

../debuggee_examples/prog

