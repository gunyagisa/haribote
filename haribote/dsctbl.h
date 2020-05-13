#pragma once

// access right
#define AR_INTGATE32    0x008e
#define AR_TSS32        0x0089
#define AR_DATA32_RW	0x4092
#define AR_CODE32_ER	0x409a
#define AR_LDT          0x0082

// table address
#define GDT_ADDR	0x00270000
#define IDT_ADDR	0x0026f800

// limit
#define LIMIT_IDT	0x000007ff
#define LIMIT_GDT	0x0000ffff

//segment descriptor
typedef struct SEGMENT_DESCRIPTOR {
  short limit_low, base_low;
  char base_mid, access_right;
  char limit_high, base_high;
} SEGMENT_DESCRIPTOR;

//interrupt descriptor
typedef struct GATE_DESCRIPTOR {
  short offset_low, selector;
  char dw_count, access_right;
  short offset_high;
} GATE_DESCRIPTOR; 

// dsctbl.c
void init_gdtidt(void);
void set_sgmntdsc(struct SEGMENT_DESCRIPTOR *sd, unsigned int limit, int base, int access_right);
void set_gatedsc(struct GATE_DESCRIPTOR *gd, int offset, int selector, int access_right);
