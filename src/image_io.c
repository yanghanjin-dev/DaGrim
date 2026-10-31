#include "app.h"

#define STB_IMAGE_IMPLEMENTATION
#define STBI_NO_HDR
#define STBI_NO_PSD
#define STBI_NO_PIC
#define STBI_NO_PNM
#include "stb_image.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#define STBI_WRITE_NO_STDIO
#include "stb_image_write.h"

typedef struct {
    uint8_t *data;
    size_t len;
    size_t cap;
} MemBuf;

static void mem_write(void *ctx, void *data, int size)
{
    MemBuf *b = (MemBuf *)ctx;
    size_t need = b->len + (size_t)size;
    if (need > b->cap) {
        size_t ncap = b->cap ? b->cap * 2 : 65536;
        uint8_t *nd;
        while (ncap < need) ncap *= 2;
        nd = (uint8_t *)realloc(b->data, ncap);
        if (!nd) return;
        b->data = nd;
        b->cap = ncap;
    }
    memcpy(b->data + b->len, data, (size_t)size);
    b->len += (size_t)size;
}

static uint8_t *bgra_to_rgba(const uint8_t *bgra, int w, int h)
{
    size_t i, n = (size_t)w * (size_t)h;
    uint8_t *rgba = xmalloc(n * 4);
    for (i = 0; i < n; i++) {
        rgba[i * 4 + 0] = bgra[i * 4 + 2];
        rgba[i * 4 + 1] = bgra[i * 4 + 1];
        rgba[i * 4 + 2] = bgra[i * 4 + 0];
        rgba[i * 4 + 3] = bgra[i * 4 + 3];
    }
    return rgba;
}

int io_load_image_mem(const uint8_t *data, size_t len, uint8_t **bgra, int *w, int *h)
{
    uint8_t *rgba = NULL, *out = NULL;
    int tw = 0, th = 0, comp = 0;
    size_t i, n;

    if (!data || len == 0 || !bgra || !w || !h) return 0;
    rgba = stbi_load_from_memory(data, (int)len, &tw, &th, &comp, 4);
    if (!rgba) return 0;
    if (tw < 1 || th < 1 || tw > DAGRIM_MAX_SIZE || th > DAGRIM_MAX_SIZE) {
        stbi_image_free(rgba);
        return 0;
    }
    n = (size_t)tw * (size_t)th;
    out = xmalloc(n * 4);
    for (i = 0; i < n; i++) {
        out[i * 4 + 0] = rgba[i * 4 + 2];
        out[i * 4 + 1] = rgba[i * 4 + 1];
        out[i * 4 + 2] = rgba[i * 4 + 0];
        out[i * 4 + 3] = rgba[i * 4 + 3];
    }
    stbi_image_free(rgba);
    *bgra = out;
    *w = tw;
    *h = th;
    return 1;
}

int io_load_image(const wchar_t *path, uint8_t **bgra, int *w, int *h)
{
    uint8_t *file = NULL;
    size_t len = 0;
    int ok;
    if (!read_file(path, &file, &len)) return 0;
    ok = io_load_image_mem(file, len, bgra, w, h);
    free(file);
    return ok;
}

int io_load_dib(const void *data, size_t len, uint8_t **bgra, int *ow, int *oh)
{
    const BITMAPINFOHEADER *bih;
    const uint8_t *bits;
    const RGBQUAD *pal = NULL;
    uint8_t *out;
    int w, h, topdown = 0, bpp, stride, x, y;
    size_t offset, n, i;
    int any_alpha = 0;

    if (!data || !bgra || !ow || !oh || len < sizeof(BITMAPINFOHEADER)) return 0;
    bih = (const BITMAPINFOHEADER *)data;
    if (bih->biSize < 40 || (size_t)bih->biSize > len) return 0;
    w = bih->biWidth;
    h = bih->biHeight;
    if (h < 0) {
        h = -h;
        topdown = 1;
    }
    bpp = bih->biBitCount;
    if (w < 1 || h < 1 || w > DAGRIM_MAX_SIZE || h > DAGRIM_MAX_SIZE) return 0;
    if (bih->biCompression != BI_RGB && bih->biCompression != BI_BITFIELDS) return 0;
    if (bpp != 8 && bpp != 24 && bpp != 32) return 0;

    offset = (size_t)bih->biSize;
    if (bih->biCompression == BI_BITFIELDS && bih->biSize == sizeof(BITMAPINFOHEADER))
        offset += 12;
    if (bpp <= 8) {
        int colors = (int)bih->biClrUsed;
        if (colors == 0) colors = 1 << bpp;
        pal = (const RGBQUAD *)((const uint8_t *)data + offset);
        offset += (size_t)colors * 4;
    }
    if (offset >= len) return 0;
    bits = (const uint8_t *)data + offset;
    stride = ((w * bpp + 31) / 32) * 4;
    if (offset + (size_t)stride > len) return 0;

    n = (size_t)w * (size_t)h;
    out = xmalloc(n * 4);
    memset(out, 0, n * 4);

    for (y = 0; y < h; y++) {
        int srcy = topdown ? y : (h - 1 - y);
        const uint8_t *row;
        uint8_t *dst;
        size_t row_off = offset + (size_t)srcy * (size_t)stride;
        if (row_off + (size_t)((w * bpp + 7) / 8) > len) continue;
        row = bits + (size_t)srcy * (size_t)stride;
        dst = out + (size_t)y * (size_t)w * 4;
        if (bpp == 32) {
            memcpy(dst, row, (size_t)w * 4);
        } else if (bpp == 24) {
            for (x = 0; x < w; x++) {
                dst[x * 4 + 0] = row[x * 3 + 0];
                dst[x * 4 + 1] = row[x * 3 + 1];
                dst[x * 4 + 2] = row[x * 3 + 2];
                dst[x * 4 + 3] = 255;
            }
        } else if (pal) {
            for (x = 0; x < w; x++) {
                uint8_t idx = row[x];
                dst[x * 4 + 0] = pal[idx].rgbBlue;
                dst[x * 4 + 1] = pal[idx].rgbGreen;
                dst[x * 4 + 2] = pal[idx].rgbRed;
                dst[x * 4 + 3] = 255;
            }
        }
    }

    if (bpp == 32) {
        for (i = 0; i < n; i++) {
            if (out[i * 4 + 3]) {
                any_alpha = 1;
                break;
            }
        }
        if (!any_alpha) {
            for (i = 0; i < n; i++) out[i * 4 + 3] = 255;
        }
    }

    *bgra = out;
    *ow = w;
    *oh = h;
    return 1;
}

int io_load_hbitmap(HBITMAP hbmp, uint8_t **bgra, int *w, int *h)
{
    BITMAP bm;
    BITMAPINFO bmi;
    HDC hdc;
    uint8_t *out;
    size_t n, i;
    int any_alpha = 0;

    if (!hbmp || !bgra || !w || !h) return 0;
    if (GetObject(hbmp, sizeof(bm), &bm) == 0) return 0;
    if (bm.bmWidth < 1 || bm.bmHeight < 1 ||
        bm.bmWidth > DAGRIM_MAX_SIZE || bm.bmHeight > DAGRIM_MAX_SIZE)
        return 0;

    memset(&bmi, 0, sizeof(bmi));
    bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    bmi.bmiHeader.biWidth = bm.bmWidth;
    bmi.bmiHeader.biHeight = bm.bmHeight;
    bmi.bmiHeader.biPlanes = 1;
    bmi.bmiHeader.biBitCount = 32;
    bmi.bmiHeader.biCompression = BI_RGB;

    n = (size_t)bm.bmWidth * (size_t)bm.bmHeight;
    out = xmalloc(n * 4);
    hdc = GetDC(NULL);
    if (!GetDIBits(hdc, hbmp, 0, (UINT)bm.bmHeight, out, &bmi, DIB_RGB_COLORS)) {
        ReleaseDC(NULL, hdc);
        free(out);
        return 0;
    }
    ReleaseDC(NULL, hdc);
    {
        int y, w4 = bm.bmWidth * 4;
        uint8_t *tmp = xmalloc((size_t)w4);
        for (y = 0; y < bm.bmHeight / 2; y++) {
            uint8_t *a = out + (size_t)y * (size_t)w4;
            uint8_t *b = out + (size_t)(bm.bmHeight - 1 - y) * (size_t)w4;
            memcpy(tmp, a, (size_t)w4);
            memcpy(a, b, (size_t)w4);
            memcpy(b, tmp, (size_t)w4);
        }
        free(tmp);
    }

    for (i = 0; i < n; i++) {
        if (out[i * 4 + 3]) {
            any_alpha = 1;
            break;
        }
    }
    if (!any_alpha) {
        for (i = 0; i < n; i++) out[i * 4 + 3] = 255;
    }

    *bgra = out;
    *w = bm.bmWidth;
    *h = bm.bmHeight;
    return 1;
}

int io_save_png(const wchar_t *path, const uint8_t *bgra, int w, int h)
{
    uint8_t *rgba;
    MemBuf b = {0};
    int ok;
    if (!bgra || w < 1 || h < 1) return 0;
    rgba = bgra_to_rgba(bgra, w, h);
    ok = stbi_write_png_to_func(mem_write, &b, w, h, 4, rgba, w * 4);
    free(rgba);
    if (!ok || !b.data) {
        free(b.data);
        return 0;
    }
    ok = write_file(path, b.data, b.len);
    free(b.data);
    return ok;
}

int io_save_jpg(const wchar_t *path, const uint8_t *bgra, int w, int h, int quality)
{
    uint8_t *rgb;
    MemBuf b = {0};
    int ok;
    size_t i, n;
    if (!bgra || w < 1 || h < 1) return 0;
    n = (size_t)w * (size_t)h;
    rgb = xmalloc(n * 3);
    for (i = 0; i < n; i++) {
        int a = bgra[i * 4 + 3];
        int rb = bgra[i * 4 + 2];
        int gb = bgra[i * 4 + 1];
        int bb = bgra[i * 4 + 0];
        rgb[i * 3 + 0] = (uint8_t)((rb * a + 255 * (255 - a)) / 255);
        rgb[i * 3 + 1] = (uint8_t)((gb * a + 255 * (255 - a)) / 255);
        rgb[i * 3 + 2] = (uint8_t)((bb * a + 255 * (255 - a)) / 255);
    }
    ok = stbi_write_jpg_to_func(mem_write, &b, w, h, 3, rgb, quality);
    free(rgb);
    if (!ok || !b.data) {
        free(b.data);
        return 0;
    }
    ok = write_file(path, b.data, b.len);
    free(b.data);
    return ok;
}

static int r32(const uint8_t **p, const uint8_t *e, int32_t *o)
{
    if (*p + 4 > e) return 0;
    *o = (int32_t)((*p)[0] | ((*p)[1] << 8) | ((*p)[2] << 16) | ((uint32_t)(*p)[3] << 24));
    *p += 4;
    return 1;
}

static int rbytes(const uint8_t **p, const uint8_t *e, void *dst, size_t n)
{
    if (*p + n > e) return 0;
    memcpy(dst, *p, n);
    *p += n;
    return 1;
}

static void w32(MemBuf *b, int32_t v)
{
    uint8_t t[4];
    uint32_t u = (uint32_t)v;
    t[0] = (uint8_t)(u);
    t[1] = (uint8_t)(u >> 8);
    t[2] = (uint8_t)(u >> 16);
    t[3] = (uint8_t)(u >> 24);
    mem_write(b, t, 4);
}

int io_load_dagrim(const wchar_t *path, Document *d)
{
    uint8_t *file = NULL;
    size_t len = 0;
    const uint8_t *p, *e;
    int32_t ver, w, h, count, active, i;

    if (!read_file(path, &file, &len)) return 0;
    p = file;
    e = file + len;
    if (len < 8 || memcmp(p, "DGRM", 4) != 0) {
        free(file);
        return 0;
    }
    p += 4;
    if (!r32(&p, e, &ver) || ver != 1) { free(file); return 0; }
    if (!r32(&p, e, &w) || !r32(&p, e, &h) || !r32(&p, e, &count) || !r32(&p, e, &active)) {
        free(file);
        return 0;
    }
    if (w < 1 || h < 1 || w > DAGRIM_MAX_SIZE || h > DAGRIM_MAX_SIZE ||
        count < 0 || count > DAGRIM_MAX_LAYERS) {
        free(file);
        return 0;
    }

    doc_free(d);
    d->w = w;
    d->h = h;
    d->cap = count;
    d->count = 0;
    d->active = -1;
    if (count > 0) d->layers = xcalloc((size_t)count, sizeof(Layer));

    for (i = 0; i < count; i++) {
        Layer *L = &d->layers[i];
        int32_t nlen, x, y, lw, lh, opacity, visible, blend;
        char nameu8[256];
        size_t pix;

        if (!r32(&p, e, &nlen) || nlen < 0 || nlen > 255) { free(file); doc_free(d); return 0; }
        memset(nameu8, 0, sizeof(nameu8));
        if (nlen > 0 && !rbytes(&p, e, nameu8, (size_t)nlen)) { free(file); doc_free(d); return 0; }
        utf8_to_wide(nameu8, L->name, 64);
        L->name[63] = 0;
        if (!r32(&p, e, &x) || !r32(&p, e, &y) || !r32(&p, e, &lw) || !r32(&p, e, &lh) ||
            !r32(&p, e, &opacity) || !r32(&p, e, &visible) || !r32(&p, e, &blend)) {
            free(file);
            doc_free(d);
            return 0;
        }
        if (lw < 1 || lh < 1 || lw > DAGRIM_MAX_SIZE || lh > DAGRIM_MAX_SIZE) {
            free(file);
            doc_free(d);
            return 0;
        }
        L->x = x;
        L->y = y;
        L->w = lw;
        L->h = lh;
        L->opacity = clampi(opacity, 0, 255);
        L->visible = visible ? 1 : 0;
        L->blend = clampi(blend, 0, BLEND_DIFFERENCE);
        pix = (size_t)lw * (size_t)lh * 4;
        L->bgra = xmalloc(pix);
        if (!rbytes(&p, e, L->bgra, pix)) {
            free(file);
            doc_free(d);
            return 0;
        }
        d->count++;
    }
    d->active = clampi(active, d->count ? 0 : -1, d->count - 1);
    wcsncpy(d->path, path, MAX_PATH - 1);
    d->path[MAX_PATH - 1] = 0;
    d->dirty = 0;
    free(file);
    return 1;
}

int io_save_dagrim(const wchar_t *path, const Document *d)
{
    MemBuf b = {0};
    int i, ok;
    if (!d) return 0;
    mem_write(&b, "DGRM", 4);
    w32(&b, 1);
    w32(&b, d->w);
    w32(&b, d->h);
    w32(&b, d->count);
    w32(&b, d->active);
    for (i = 0; i < d->count; i++) {
        const Layer *L = &d->layers[i];
        char nameu8[256];
        int nlen;
        size_t pix;
        memset(nameu8, 0, sizeof(nameu8));
        wide_to_utf8(L->name, nameu8, 255);
        nlen = (int)strlen(nameu8);
        w32(&b, nlen);
        if (nlen) mem_write(&b, nameu8, nlen);
        w32(&b, L->x);
        w32(&b, L->y);
        w32(&b, L->w);
        w32(&b, L->h);
        w32(&b, L->opacity);
        w32(&b, L->visible);
        w32(&b, L->blend);
        pix = (size_t)L->w * (size_t)L->h * 4;
        if (L->bgra && pix) mem_write(&b, L->bgra, (int)pix);
    }
    ok = b.data && write_file(path, b.data, b.len);
    free(b.data);
    return ok;
}
