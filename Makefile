OBJ = bootpack.o dsctbl.o fifo.o graphic.o hankaku.o interrupt.o keyboard.o mouse.o nasmfunc.o memory.o sheet.o timer.o mtask.o window.o console.o file.o myfunc.o
HRB = beepdown.hrb color.hrb hello.hrb hello3.hrb hello4.hrb winhello.hrb
BUILD = ./build-cache/
SRC = ./haribote/
APP_SRC = ./app/

QEMU = qemu-system-i386
CC = clang
CFLAGS=-Wall -c -march=i486 -m32 -fno-pic -nostdlib -fno-stack-protector

$(BUILD)%.bin: $(SRC)%.asm
	nasm -o $@ $<

$(BUILD)%.o: $(SRC)%.c
	$(CC) $(CFLAGS) -o $@ $<

$(BUILD)%.o: $(SRC)%.s
	nasm -f elf32 -o $@ $<

$(BUILD)hankaku.o: $(SRC)hankaku.c
	$(CC) -c -m32 -o $@ $<

$(BUILD)nasmfunc.o: $(SRC)nasmfunc.asm Makefile
	nasm -f elf32 -o $@ $<

$(BUILD)bootpack.bin: $(addprefix $(BUILD), $(OBJ))
	ld -m elf_i386  -e HariMain -o $@ $^ -T har.ld

$(BUILD)geocide.sys: $(BUILD)asmhead.bin $(BUILD)bootpack.bin
	cat $< $(BUILD)bootpack.bin > $@

$(BUILD)%.o: $(APP_SRC)%.c
	$(CC) $(CFLAGS) $< -o $@

$(BUILD)%.hrb: $(APP_SRC)%.asm
	nasm -f elf32 -o $(BUILD)tmp.o $<
	ld $(BUILD)tmp.o -o $@ -e HariMain -m elf_i386 -T binary.ld

$(BUILD)%.hrb: $(BUILD)%.o 
	ld $^ -o $@ -e HariMain -m elf_i386 -T binary.ld -L $(APP_SRC) -lapi

$(BUILD)geocide.img: $(BUILD)ipl.bin $(BUILD)geocide.sys $(addprefix $(BUILD), $(HRB)) Makefile
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
