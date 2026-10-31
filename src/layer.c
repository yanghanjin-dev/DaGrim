#include "app.h"

int layer_create(Layer *L, int w, int h, const wchar_t *name)
{
    if (!L || w < 1 || h < 1 || w > DAGRIM_MAX_SIZE || h > DAGRIM_MAX_SIZE) return 0;
    memset(L, 0, sizeof(*L));
    L->w = w;
    L->h = h;
    L->opacity = 255;
    L->visible = 1;
    L->blend = BLEND_NORMAL;
    L->bgra = xcalloc((size_t)w * (size_t)h, 4);
    if (name) wcsncpy(L->name, name, 63);
    else wcsncpy(L->name, L"레이어", 63);
    L->name[63] = 0;
    return 1;
}

int layer_copy(Layer *dst, const Layer *src)
{
    size_t n;
    if (!dst || !src) return 0;
    memset(dst, 0, sizeof(*dst));
    *dst = *src;
    dst->bgra = NULL;
    if (!src->bgra || src->w < 1 || src->h < 1) return 1;
    n = (size_t)src->w * (size_t)src->h * 4;
    dst->bgra = xmalloc(n);
    memcpy(dst->bgra, src->bgra, n);
    return 1;
}

void layer_free(Layer *L)
{
    if (!L) return;
    free(L->bgra);
    memset(L, 0, sizeof(*L));
}

void layer_fill_white(Layer *L)
{
    size_t i, n;
    if (!L || !L->bgra) return;
    n = (size_t)L->w * (size_t)L->h;
    for (i = 0; i < n; i++) {
        L->bgra[i * 4 + 0] = 255;
        L->bgra[i * 4 + 1] = 255;
        L->bgra[i * 4 + 2] = 255;
        L->bgra[i * 4 + 3] = 255;
    }
}

int layer_scale_from(Layer *L, const uint8_t *src, int sw, int sh, int nw, int nh)
{
    uint8_t *dst;
    int x, y;
    size_t n;
    if (!L || !src || sw < 1 || sh < 1) return 0;
    if (nw < 1 || nh < 1 || nw > DAGRIM_MAX_SIZE || nh > DAGRIM_MAX_SIZE) return 0;

    n = (size_t)nw * (size_t)nh * 4;
    if (L->bgra && L->w == nw && L->h == nh)
        dst = L->bgra;
    else
        dst = xmalloc(n);

    if (nw == sw && nh == sh) {
        if (dst != src) memcpy(dst, src, n);
    } else {
        for (y = 0; y < nh; y++) {
            float fy = ((float)y + 0.5f) * (float)sh / (float)nh - 0.5f;
            int y0, y1;
            float ty;
            if (fy < 0.0f) fy = 0.0f;
            if (fy > (float)(sh - 1)) fy = (float)(sh - 1);
            y0 = (int)floorf(fy);
            y1 = y0 + 1;
            if (y1 >= sh) y1 = sh - 1;
            ty = fy - (float)y0;
            for (x = 0; x < nw; x++) {
                float fx = ((float)x + 0.5f) * (float)sw / (float)nw - 0.5f;
                int x0, x1, c;
                float tx;
                const uint8_t *p00, *p10, *p01, *p11;
                uint8_t *d;
                if (fx < 0.0f) fx = 0.0f;
                if (fx > (float)(sw - 1)) fx = (float)(sw - 1);
                x0 = (int)floorf(fx);
                x1 = x0 + 1;
                if (x1 >= sw) x1 = sw - 1;
                tx = fx - (float)x0;
                p00 = src + ((size_t)y0 * (size_t)sw + (size_t)x0) * 4;
                p10 = src + ((size_t)y0 * (size_t)sw + (size_t)x1) * 4;
                p01 = src + ((size_t)y1 * (size_t)sw + (size_t)x0) * 4;
                p11 = src + ((size_t)y1 * (size_t)sw + (size_t)x1) * 4;
                d = dst + ((size_t)y * (size_t)nw + (size_t)x) * 4;
                for (c = 0; c < 4; c++) {
                    float a = (float)p00[c] + ((float)p10[c] - (float)p00[c]) * tx;
                    float b = (float)p01[c] + ((float)p11[c] - (float)p01[c]) * tx;
                    d[c] = (uint8_t)(a + (b - a) * ty + 0.5f);
                }
            }
        }
    }

    if (dst != L->bgra) {
        free(L->bgra);
        L->bgra = dst;
        L->w = nw;
        L->h = nh;
    }
    return 1;
}
