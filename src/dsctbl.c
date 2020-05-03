#include "dsctbl.h"
#include "bootpack.h"

void init_gdtidt(void)
{
	SEGMENT_DESCRIPTOR *gdt = (SEGMENT_DESCRIPTOR *) GDT_ADDR;
	GATE_DESCRIPTOR *idt = (GATE_DESCRIPTOR *) IDT_ADDR;

	int i;

	// gdt initialize
	for (i = 0;i < LIMIT_GDT / 8;i++) {
		set_sgmntdsc(gdt + i, 0, 0, 0);
	}

	set_sgmntdsc(gdt + 1, 0xffffffff, 0x00000000, AR_DATA32_RW);
	set_sgmntdsc(gdt + 2, 0x0007ffff, 0x00280000, AR_CODE32_ER);
        load_gdtr(LIMIT_GDT, GDT_ADDR);

	// idt initialize
	for (i = 0;i < LIMIT_IDT / 8;i++) {
		set_gatedsc(idt + i, 0, 0, 0);
	}

	set_gatedsc(idt + 0x0c, (int) inthandler0c_asm, 2 * 8, AR_INTGATE32);
	set_gatedsc(idt + 0x0d, (int) inthandler0d_asm, 2 * 8, AR_INTGATE32);
        set_gatedsc(idt + 0x20, (int) inthandler20_asm, 2 * 8, AR_INTGATE32);
	set_gatedsc(idt + 0x21, (int) inthandler21_asm, 2 * 8, AR_INTGATE32);
	set_gatedsc(idt + 0x2c, (int) inthandler2c_asm, 2 * 8, AR_INTGATE32);
        set_gatedsc(idt + 0x40, (int) hrb_api_asm,      2 * 8, AR_INTGATE32 + 0x60);
        load_idtr(LIMIT_IDT, IDT_ADDR);

}

void set_sgmntdsc(SEGMENT_DESCRIPTOR *sd, unsigned int limit, int base, int access_right)
{
	if (limit > 0xffff) {
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

void set_gatedsc(GATE_DESCRIPTOR *gd, int offset, int selector, int access_right)
{
	gd->offset_low = offset & 0xffff;
	gd->selector = selector;
	gd->dw_count = (access_right >> 8) & 0xff;
	gd->access_right = access_right & 0xff;
	gd->offset_high = (offset >> 16) & 0xffff;
}
