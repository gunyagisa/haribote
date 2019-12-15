#ifndef MEMORY_H_
#define MEMORY_H_

#define MEMMAN_FREES    4096  // about 32KB

struct FREEINFO {
    unsigned int addr, size;
};

struct MEMMAN {
    unsigned int frees, maxfrees, lostsize, losts;
    struct FREEINFO free[MEMMAN_FREES];
};

unsigned int memtest_sub(unsigned int start, unsigned int end);
unsigned int memtest(unsigned int start, unsigned int end);
void memman_init(struct MEMMAN *man);
unsigned int memman_total(struct MEMMAN *man);
unsigned int memman_alloc(struct MEMMAN *man, unsigned int size);
unsigned int memman_free(struct MEMMAN *man, unsigned int addr, unsigned int size);

#endif
