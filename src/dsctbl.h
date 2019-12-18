#pragma once

// access right
#define AR_INTGATE32		0x008e

// table address
#define GDT_ADDR	0x00270000
#define IDT_ADDR	0x0026f800

//segment discriptor
typedef struct SEGMENT_DISCRIPTOR {
	short limit_low, base_low;
	char base_mid, access_right;
	char limit_high, base_high;
} SEGMENT_DISCRIPTOR;

//interrupt discriptor
typedef struct GATE_DISCRIPTOR {
	short offset_low, selector;
	char dw_count, access_right;
	short offset_high;
} GATE_DISCRIPTOR; 

// dsctbl.c
void init_gdtidt(void);
void set_sgmntdsc(SEGMENT_DISCRIPTOR *sd, unsigned int limit, int base, int access_right);
void set_gatedsc(GATE_DISCRIPTOR *gd, int offset, int selector, int access_right);
