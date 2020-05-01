OBJ = bootpack.o dsctbl.o fifo.o graphic.o hankaku.o interrupt.o keyboard.o mouse.o nasmfunc.o memory.o sheet.o timer.o mtask.o window.o
BUILD = ./build-cache/
SRC = ./src/

QEMU = qemu-system-i386
CC = clang
CFLAGS=-Wall -c -march=i486 -m32 -fno-pic -nostdlib -fno-stack-protector

$(BUILD)%.bin: $(SRC)%.asm Makefile
	nasm -o $@ $<

$(BUILD)%.o: $(SRC)%.c $(SRC)%.h Makefile
	$(CC) $(CFLAGS) -o $@ $<

$(BUILD)%.o: $(SRC)%.c Makefile
	$(CC) $(CFLAGS) -o $@ $<

$(BUILD)hankaku.o: $(SRC)hankaku.c Makefile
	$(CC) -c -m32 -o $@ $<

$(BUILD)nasmfunc.o: $(SRC)nasmfunc.asm Makefile
	nasm -f elf32 -o $@ $<

$(BUILD)bootpack.bin: $(addprefix $(BUILD), $(OBJ)) Makefile
	ld -m elf_i386  -e HariMain -o $(BUILD)bootpack.bin $(addprefix $(BUILD), $(OBJ)) -T har.ld

$(BUILD)geocide.sys: $(BUILD)asmhead.bin $(BUILD)bootpack.bin Makefile 
	cat $< $(BUILD)bootpack.bin > $@

$(BUILD)hlt.hrb: $(SRC)hlt.asm
	nasm -f elf32 -o $(BUILD)hlt.o $<
	ld -m elf_i386 -T har.ld -o $@ $(BUILD)hlt.o

$(BUILD)geocide.img: $(BUILD)ipl.bin $(BUILD)geocide.sys $(BUILD)hlt.hrb Makefile
	mformat -f 1440 -C -B $< -i $@ ::
	mcopy $(BUILD)geocide.sys -i $@ ::
	mcopy $(SRC)ipl.asm -i $@ ::
	mcopy ./Makefile -i $@ ::
	mcopy ./build-cache/hlt.hrb -i $@ ::



run: $(BUILD)geocide.img
	$(QEMU) -m 32 -d guest_errors -fda $< -show-cursor -monitor stdio

clean:
	rm -f $(BUILD)*.bin $(BUILD)*.o $(BUILD)geocide.*

.PHONY: clean
.SILENT:
