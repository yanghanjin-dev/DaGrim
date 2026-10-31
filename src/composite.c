#include "app.h"

static int mul8(int a, int b)
{
    return (a * b) / 255;
}

void blend_pixel(uint8_t *dst, const uint8_t *src, int opacity, int blend)
{
    int sa, da, inv, oa;
    int sb, sg, sr, db, dg, dr;
    int bb, bg, br;

    sa = mul8(src[3], opacity);
    if (sa <= 0) return;

    da = dst[3];
    sb = src[0];
    sg = src[1];
    sr = src[2];
    db = dst[0];
    dg = dst[1];
    dr = dst[2];

    if (da <= 0 || blend == BLEND_NORMAL) {
        bb = sb;
        bg = sg;
        br = sr;
    } else if (blend == BLEND_MULTIPLY) {
        bb = mul8(sb, db);
        bg = mul8(sg, dg);
        br = mul8(sr, dr);
    } else if (blend == BLEND_SCREEN) {
        bb = 255 - mul8(255 - sb, 255 - db);
        bg = 255 - mul8(255 - sg, 255 - dg);
        br = 255 - mul8(255 - sr, 255 - dr);
    } else if (blend == BLEND_OVERLAY) {
        bb = db < 128 ? clampu8(mul8(sb, db) * 2) : clampu8(255 - mul8(255 - sb, 255 - db) * 2);
        bg = dg < 128 ? clampu8(mul8(sg, dg) * 2) : clampu8(255 - mul8(255 - sg, 255 - dg) * 2);
        br = dr < 128 ? clampu8(mul8(sr, dr) * 2) : clampu8(255 - mul8(255 - sr, 255 - dr) * 2);
    } else if (blend == BLEND_DARKEN) {
        bb = sb < db ? sb : db;
        bg = sg < dg ? sg : dg;
        br = sr < dr ? sr : dr;
    } else if (blend == BLEND_LIGHTEN) {
        bb = sb > db ? sb : db;
        bg = sg > dg ? sg : dg;
        br = sr > dr ? sr : dr;
    } else if (blend == BLEND_ADD) {
        bb = clampu8(sb + db);
        bg = clampu8(sg + dg);
        br = clampu8(sr + dr);
    } else {
        bb = clampu8(abs(sb - db));
        bg = clampu8(abs(sg - dg));
        br = clampu8(abs(sr - dr));
    }

    inv = 255 - sa;
    oa = sa + mul8(da, inv);
    if (oa <= 0) {
        dst[0] = dst[1] = dst[2] = dst[3] = 0;
        return;
    }
    dst[0] = (uint8_t)((bb * sa + db * mul8(da, inv)) / oa);
    dst[1] = (uint8_t)((bg * sa + dg * mul8(da, inv)) / oa);
    dst[2] = (uint8_t)((br * sa + dr * mul8(da, inv)) / oa);
    dst[3] = (uint8_t)oa;
}

static void fill_checker_rect(uint8_t *dst, int dw, int x0, int y0, int x1, int y1)
{
    int x, y;
    for (y = y0; y < y1; y++) {
        for (x = x0; x < x1; x++) {
            int c = (((x >> 3) ^ (y >> 3)) & 1) ? 82 : 52;
            uint8_t *p = dst + ((size_t)y * (size_t)dw + (size_t)x) * 4;
            p[0] = p[1] = p[2] = (uint8_t)c;
            p[3] = 255;
        }
    }
}

static int realloc_disp(int w, int h)
{
    size_t n;
    if (w < 1 || h < 1) return 0;
    if (g.disp && g.buf_w == w && g.buf_h == h) return 1;
    free(g.disp);
    n = (size_t)w * (size_t)h * 4;
    g.disp = xmalloc(n);
    g.buf_w = w;
    g.buf_h = h;
    g.comp_ok = 0;
    g.dirty_all = 1;
    return 1;
}

void comp_invalidate_all(void)
{
    g.comp_ok = 0;
    g.dirty_all = 1;
    g.has_dirty = 0;
}

void comp_invalidate_rect(int x, int y, int w, int h)
{
    RECT n;
    if (g.dirty_all) {
        g.comp_ok = 0;
        return;
    }
    n.left = x;
    n.top = y;
    n.right = x + w;
    n.bottom = y + h;
    if (!g.has_dirty) {
        g.dirty = n;
        g.has_dirty = 1;
    } else {
        UnionRect(&g.dirty, &g.dirty, &n);
    }
    g.comp_ok = 0;
}

static void composite_rect(int x0, int y0, int x1, int y1, int checker)
{
    int i, x, y;
    Document *d = &g.doc;
    uint8_t *dst = g.disp;

    x0 = clampi(x0, 0, d->w);
    y0 = clampi(y0, 0, d->h);
    x1 = clampi(x1, 0, d->w);
    y1 = clampi(y1, 0, d->h);
    if (x1 <= x0 || y1 <= y0) return;

    if (checker) fill_checker_rect(dst, d->w, x0, y0, x1, y1);
    else {
        for (y = y0; y < y1; y++) {
            memset(dst + ((size_t)y * (size_t)d->w + (size_t)x0) * 4, 0, (size_t)(x1 - x0) * 4);
        }
    }

    for (i = 0; i < d->count; i++) {
        Layer *L = &d->layers[i];
        int lx0, ly0, lx1, ly1;
        if (!L->visible || L->opacity <= 0 || !L->bgra) continue;
        lx0 = clampi(x0 > L->x ? x0 : L->x, 0, d->w);
        ly0 = clampi(y0 > L->y ? y0 : L->y, 0, d->h);
        lx1 = clampi(x1 < L->x + L->w ? x1 : L->x + L->w, 0, d->w);
        ly1 = clampi(y1 < L->y + L->h ? y1 : L->y + L->h, 0, d->h);
        for (y = ly0; y < ly1; y++) {
            int ly = y - L->y;
            for (x = lx0; x < lx1; x++) {
                int lx = x - L->x;
                uint8_t *sp = L->bgra + ((size_t)ly * (size_t)L->w + (size_t)lx) * 4;
                uint8_t *dp = dst + ((size_t)y * (size_t)d->w + (size_t)x) * 4;
                blend_pixel(dp, sp, L->opacity, L->blend);
            }
        }
    }
}

void comp_ensure(void)
{
    Document *d = &g.doc;
    int x0, y0, x1, y1;
    if (d->w < 1 || d->h < 1) return;
    if (!realloc_disp(d->w, d->h)) return;
    if (g.comp_ok) return;

    if (g.dirty_all || !g.has_dirty) {
        x0 = 0;
        y0 = 0;
        x1 = d->w;
        y1 = d->h;
    } else {
        x0 = g.dirty.left;
        y0 = g.dirty.top;
        x1 = g.dirty.right;
        y1 = g.dirty.bottom;
    }
    composite_rect(x0, y0, x1, y1, 1);
    g.comp_ok = 1;
    g.dirty_all = 0;
    g.has_dirty = 0;
}

int flatten_document(const Document *d, uint8_t *dst)
{
    int i, x, y;
    size_t n;
    if (!d || !dst || d->w < 1 || d->h < 1) return 0;
    n = (size_t)d->w * (size_t)d->h * 4;
    memset(dst, 0, n);
    for (i = 0; i < d->count; i++) {
        const Layer *L = &d->layers[i];
        int x0, y0, x1, y1;
        if (!L->visible || L->opacity <= 0 || !L->bgra) continue;
        x0 = clampi(L->x, 0, d->w);
        y0 = clampi(L->y, 0, d->h);
        x1 = clampi(L->x + L->w, 0, d->w);
        y1 = clampi(L->y + L->h, 0, d->h);
        for (y = y0; y < y1; y++) {
            int ly = y - L->y;
            for (x = x0; x < x1; x++) {
                int lx = x - L->x;
                const uint8_t *sp = L->bgra + ((size_t)ly * (size_t)L->w + (size_t)lx) * 4;
                uint8_t *dp = dst + ((size_t)y * (size_t)d->w + (size_t)x) * 4;
                blend_pixel(dp, sp, L->opacity, L->blend);
            }
        }
    }
    return 1;
}
