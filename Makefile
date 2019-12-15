OBJ := bootpack.o dsctbl.o fifo.o graphic.o hankaku.o interrupt.o keyboard.o mouse.o nasmfunc.o
BUILD := ./build-cache/
SRC := ./src/

$(BUILD)%.bin: $(SRC)%.asm Makefile
	nasm -o $@ $<

$(BUILD)%.o: $(SRC)%.c $(SRC)%.h Makefile
	gcc -c -m32 -fno-pic -fno-stack-protector -o $@ $<

$(BUILD)hankaku.o: $(SRC)hankaku.c Makefile
	gcc -c -m32 -o $@ $<

$(BUILD)nasmfunc.o: $(SRC)nasmfunc.asm Makefile
	nasm -f elf32 -o $@ $<

$(BUILD)bootpack.bin: $(addprefix $(BUILD), $(OBJ)) Makefile
	ld -m elf_i386  -e HariMain -o $(BUILD)bootpack.bin $(BUILD)*.o -T har.ld

$(BUILD)geocide.sys: $(BUILD)asmhead.bin $(BUILD)bootpack.bin Makefile 
	cat $< $(BUILD)bootpack.bin > $@

$(BUILD)geocide.img: $(BUILD)ipl.bin $(BUILD)geocide.sys Makefile
	mformat -f 1440 -C -B $< -i $@ ::
	mcopy $(BUILD)geocide.sys -i $@ ::



run: $(BUILD)geocide.img
	qemu-system-i386 -m 32 -vga std -fda $< -show-cursor -monitor stdio

clean:
	rm -f $(BUILD)*.bin $(BUILD)*.o $(BUILD)geocide.*

.PHONY: clean
.SILENT:
