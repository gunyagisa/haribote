%.bin: %.asm Makefile
	nasm -o $@ $<

%.o: %.c %.h Makefile
	gcc -c -m32 -fno-pic -fno-stack-protector -o $@ $<

hankaku.o: hankaku.c Makefile
	gcc -c -m32 -o $@ $<

nasmfunc.o: nasmfunc.asm Makefile
	nasm -f elf32 -o $@ $<

bootpack.bin: bootpack.o nasmfunc.o hankaku.o graphic.o dsctbl.o interrupt.o fifo.o mouse.o keyboard.o har.ld Makefile
	ld -m elf_i386  -e HariMain -o bootpack.bin *.o -T har.ld

geocide.sys: asmhead.bin bootpack.bin Makefile 
	cat asmhead.bin bootpack.bin > geocide.sys

geocide.img: ipl.bin geocide.sys Makefile
	mformat -f 1440 -C -B ipl.bin -i geocide.img ::
	mcopy geocide.sys -i geocide.img ::



run: geocide.img
	qemu-system-i386 -m 32 -vga std -fda geocide.img -show-cursor -monitor stdio
	make clean

clean:
	rm -f *.bin *.o geocide.*

.PHONY: clean
