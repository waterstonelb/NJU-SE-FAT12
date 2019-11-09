run:
	@nasm -f elf64 my_print.asm
	@gcc main.c my_print.o
	@rlwrap ./a.out
	@rm my_print.o a.out

clean:
	@rm my_print.o a.out