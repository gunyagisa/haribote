#include "sheet.h"
#include "memory.h"

struct SHTCTL *shtctl_init(struct MEMMAN *memman, unsigned char *vram, int xsize, int ysize)
{
    struct SHTCTL *shtctl = (struct SHTCTL *) memman_alloc_4k(memman, sizeof(struct SHTCTL));

    if (shtctl == 0)
        goto err;
    shtctl->map = (unsigned char *) memman_alloc_4k(memman, xsize * ysize);
    if (shtctl->map == 0) {
        memman_free_4k(memman, (int) shtctl, sizeof(struct SHTCTL));
        goto err;
    }
    shtctl->vram = vram;
    shtctl->xsize = xsize;
    shtctl->ysize = ysize;
    shtctl->top = -1;
    for (int i = 0;i < SHEET_MAX;++i) {
        shtctl->sheets0[i].flags = 0;
        shtctl->sheets0[i].ctl = shtctl;
    }

err:
    return shtctl;
}

struct SHEET *sheet_alloc(struct SHTCTL *ctl)
{
    struct SHEET *sht;
    for (int i = 0;i < SHEET_MAX;++i) {
        if (ctl->sheets0[i].flags == 0) {
            sht = &ctl->sheets0[i];
            sht->flags = SHEET_USE;
            sht->height = -1;
            return sht;
        }
    }
    return 0;
}

void sheet_setbuf(struct SHEET *sht, unsigned char *buf, int xsize, int ysize, int col_inv)
{
    sht->buf = buf;
    sht->bxsize = xsize;
    sht->bysize = ysize;
    sht->col_inv = col_inv;
}

void sheet_updown(struct SHEET *sht, int height)
{
    struct SHTCTL *ctl = sht->ctl;
    int old = sht->height;

    if (height > ctl->top + 1) {
        height = ctl->top + 1;
    }
    if (height < -1) {
        height = -1;
    }

    sht->height = height;

    if (old > height) {
        if (height >= 0) {
            for (int h = old;h > height;h--) {
                ctl->sheets[h] = ctl->sheets[h - 1];
                ctl->sheets[h]->height = h;
            }
            ctl->sheets[height] = sht;
            sheet_refreshmap(ctl, sht->vx0, sht->vy0, sht->vx0 + sht->bxsize, sht->vy0 + sht->bysize, height + 1);
            sheet_refreshsub(ctl, sht->vx0, sht->vy0, sht->vx0 + sht->bxsize, sht->vy0 + sht->bysize, height + 1, old);
        } else {
            if (ctl->top > old) {
                for (int h = old;h < ctl->top;h++) {
                    ctl->sheets[h] = ctl->sheets[h + 1];
                    ctl->sheets[h]->height = h;
                }
            }
            ctl->top--;
        }
        sheet_refreshmap(ctl, sht->vx0, sht->vy0, sht->vx0 + sht->bxsize, sht->vy0 + sht->bysize, 0);
        sheet_refreshsub(ctl, sht->vx0, sht->vy0, sht->vx0 + sht->bxsize, sht->vy0 + sht->bysize, 0, old - 1);
    } else if (old < height) {
        if (old >= 0) {
            for (int h = 0;h < height;++h) {
                ctl->sheets[h] = ctl->sheets[h + 1];
                ctl->sheets[h]->height = h;
            }
            ctl->sheets[height] = sht;
        } else {
            for (int h = ctl->top;h >= height;h--) {
                ctl->sheets[h + 1] = ctl->sheets[h];
                ctl->sheets[h + 1]->height = h + 1;
            }
            ctl->sheets[height] = sht;
            ctl->top++;
        }
        sheet_refreshmap(ctl, sht->vx0, sht->vy0, sht->vx0 + sht->bxsize, sht->vy0 + sht->bysize, height);
        sheet_refreshsub(ctl, sht->vx0, sht->vy0, sht->vx0 + sht->bxsize, sht->vy0 + sht->bysize, height, height);
    }
}

void sheet_refresh(struct SHEET *sht, int bx0, int by0, int bx1, int by1)
{
    if (sht->height >= 0) {
        sheet_refreshsub(sht->ctl, sht->vx0 + bx0, sht->vy0 + by0, sht->vx0 + bx1, sht->vy0 + by1, sht->height, sht->height);
    }
}

void sheet_refreshsub(struct SHTCTL *ctl, int vx0, int vy0, int vx1, int vy1, int h0, int h1)
{
    int vx, vy;
    unsigned char *buf, sid, *vram = ctl->vram, *map = ctl->map;
    struct SHEET *sht;

    if (vx0 < 0) { vx0 = 0; }
    if (vy0 < 0) { vy0 = 0; }
    if (vx1 > ctl->xsize) { vx1 = ctl->xsize; }
    if (vy1 > ctl->ysize) { vy1 = ctl->ysize; }

    for (int h = h0;h <= h1;h++) {
        sht = ctl->sheets[h];
        buf = sht->buf;
        sid = sht - ctl->sheets0;

        int bx0, by0, bx1, by1;
        bx0 = vx0 - sht->vx0;
        by0 = vy0 - sht->vy0;
        bx1 = vx1 - sht->vx0;
        by1 = vy1 - sht->vy0;
        if (bx0 < 0) { bx0 = 0; }
        if (by0 < 0) { by0 = 0; }
        if (bx1 > sht->bxsize) { bx1 = sht->bxsize; }
        if (by1 > sht->bysize) { by1 = sht->bysize; }
        for (int by = by0;by < by1;by++) {
            vy = sht->vy0 + by;
            for (int bx = bx0;bx < bx1;bx++) {
                vx = sht->vx0 + bx;
                if (map[vx + vy * ctl->xsize] == sid) {
                    vram[vx + vy * ctl->xsize] = buf[bx + by * sht->bxsize];
                }
            }
        }
    }
}

void sheet_slide(struct SHEET *sht, int vx0, int vy0)
{
    struct SHTCTL *ctl = sht->ctl;
    int old_vx0 = sht->vx0, old_vy0 = sht->vy0;
    sht->vx0 = vx0;
    sht->vy0 = vy0;
    if (sht->height >= 0) {
        sheet_refreshmap(ctl, old_vx0, old_vy0, old_vx0 + sht->bxsize, old_vy0 + sht->bysize, 0);
        sheet_refreshmap(ctl, vx0, vy0, vx0 + sht->bxsize, vy0 + sht->bysize, sht->height);
        sheet_refreshsub(ctl, old_vx0, old_vy0, old_vx0 + sht->bxsize, old_vy0 + sht->bysize, 0, sht->height - 1);
        sheet_refreshsub(ctl, vx0, vy0, vx0 + sht->bxsize, vy0 + sht->bysize, sht->height, sht->height);
    }
}

void sheet_free(struct SHEET *sht)
{
    if (sht->height >= 0) {
        sheet_updown(sht, -1);
    }
    sht->flags = 0;
}

void sheet_refreshmap(struct SHTCTL *ctl, int vx0, int vy0, int vx1, int vy1, int h0)
{
    unsigned char *buf, sid, *map = ctl->map;
    struct SHEET *sht;
    if (vx0 < 0) { vx0 = 0; }
    if (vy0 < 0) { vy0 = 0; }
    if (vx1 > ctl->xsize) { vx1 = ctl->xsize; }
    if (vy1 > ctl->ysize) { vy1 = ctl->ysize; }

    for (int h = h0;h <= ctl->top;h++) {
        sht = ctl->sheets[h];
        buf = sht->buf;
        sid = sht - ctl->sheets0;

        int bx0, by0, bx1, by1;
        bx0 = vx0 - sht->vx0;
        by0 = vy0 - sht->vy0;
        bx1 = vx1 - sht->vx0;
        by1 = vy1 - sht->vy0;
        if (bx0 < 0) { bx0 = 0; }
        if (by0 < 0) { by0 = 0; }
        if (bx1 > sht->bxsize) { bx1 = sht->bxsize; }
        if (by1 > sht->bysize) { by1 = sht->bysize; }
        int vx, vy;
        for (int by = by0;by < by1;++by) {
            vy = sht->vy0 + by;
            for (int bx = bx0;bx < bx1;++bx) {
                vx = sht->vx0 + bx;
                if (buf[by + bx * sht->bxsize] != sht->col_inv) {
                    map[vx + vy * ctl->xsize] = sid;
                }
            }
        }
    }
}

