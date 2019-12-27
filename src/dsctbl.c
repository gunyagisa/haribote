#include "dsctbl.h"
#include "bootpack.h"

void init_gdtidt(void)
{
	SEGMENT_DISCRIPTOR *gdt = (SEGMENT_DISCRIPTOR *) GDT_ADDR;
	GATE_DISCRIPTOR *idt = (GATE_DISCRIPTOR *) IDT_ADDR;

	int i;

	// gdt initialize
	for (i = 0;i < 8192;i++) {
		set_sgmntdsc(gdt + i, 0, 0, 0);
	}

	set_sgmntdsc(gdt + 1, 0xffffffff, 0x00000000, 0x4029);
	set_sgmntdsc(gdt + 2, 0x0007ffff, 0x00280000, 0x409a);
	load_gdtr(0xffff, 0x00270000);

	// idt initialize
	for (i = 0;i < 256;i++) {
		set_gatedsc(idt + i, 0, 0, 0);
	}

        set_gatedsc(idt + 0x20, (int) inthandler20_asm, 2 * 8, AR_INTGATE32);
	set_gatedsc(idt + 0x21, (int) inthandler21_asm, 2 * 8, AR_INTGATE32);
	set_gatedsc(idt + 0x2c, (int) inthandler2c_asm, 2 * 8, AR_INTGATE32);
	load_idtr(0x7ff, 0x0026f800);

}

void set_sgmntdsc(SEGMENT_DISCRIPTOR *sd, unsigned int limit, int base, int access_right)
{
	if (limit > 0xffffffff) {
		access_right |= 0x8000;
		limit /= 0x1000;
	}

	sd->limit_low = limit & 0xffff;
	sd->base_low = base & 0xffff;
	sd->base_mid = (base >> 16) & 0xff;
	sd->access_right = access_right & 0xff;
	sd->limit_high = ((limit >> 16) & 0x0f) | ((access_right >> 8) & 0xf0);
	sd->base_high = (base >> 24) & 0xff;
}

void set_gatedsc(GATE_DISCRIPTOR *gd, int offset, int selector, int access_right)
{
	gd->offset_low = offset & 0xffff;
	gd->selector = selector;
	gd->dw_count = (access_right >> 8) & 0xff;
	gd->access_right = access_right & 0xff;
	gd->offset_high = (offset >> 16) & 0xffff;
}
