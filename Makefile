OBJ = bootpack.o dsctbl.o fifo.o graphic.o hankaku.o interrupt.o keyboard.o mouse.o nasmfunc.o memory.o sheet.o timer.o mtask.o window.o console.o file.o myfunc.o
BUILD = ./build-cache/
SRC = ./src/

QEMU = qemu-system-i386
CC = clang
CFLAGS=-Wall -c -march=i486 -m32 -fno-pic -nostdlib -fno-stack-protector

$(BUILD)%.bin: $(SRC)%.asm Makefile
	nasm -o $@ $<

$(BUILD)%.o: $(SRC)%.c Makefile
	$(CC) $(CFLAGS) -o $@ $<

$(BUILD)%.o: $(SRC)%.s Makefile
	nasm -f elf32 -o $@ $<

$(BUILD)hankaku.o: $(SRC)hankaku.c Makefile
	$(CC) -c -m32 -o $@ $<

$(BUILD)nasmfunc.o: $(SRC)nasmfunc.asm Makefile
	nasm -f elf32 -o $@ $<

$(BUILD)bootpack.bin: $(addprefix $(BUILD), $(OBJ)) Makefile
	ld -m elf_i386  -e HariMain -o $(BUILD)bootpack.bin $(addprefix $(BUILD), $(OBJ)) -T har.ld -Map mapfile

$(BUILD)geocide.sys: $(BUILD)asmhead.bin $(BUILD)bootpack.bin Makefile 
	cat $< $(BUILD)bootpack.bin > $@

$(BUILD)%.hrb: $(SRC)%.asm
	nasm -f elf32 -o $(BUILD)tmp.o $<
	ld $(BUILD)tmp.o -o $@ -e HariMain -m elf_i386 -T binary.ld

$(BUILD)%.hrb: $(BUILD)%.o $(BUILD)a_nasm.o
	ld $< $(BUILD)a_nasm.o -o $@ -e HariMain -m elf_i386 -T binary.ld

$(BUILD)geocide.img: $(BUILD)ipl.bin $(BUILD)geocide.sys $(BUILD)hello.hrb $(BUILD)hello3.hrb $(BUILD)bug.hrb $(BUILD)bug2.hrb Makefile
	mformat -f 1440 -C -B $< -i $@ ::
	mcopy $(BUILD)geocide.sys -i $@ ::
	mcopy $(SRC)ipl.asm -i $@ ::
	mcopy ./Makefile -i $@ ::
	mcopy ./build-cache/*.hrb -i $@ ::

run: $(BUILD)geocide.img
	$(QEMU) -m 32 -d guest_errors -fda $< -show-cursor -monitor stdio

clean:
	rm -f $(BUILD)*
	rm -f mapfile

.PHONY: clean
.SILENT:
